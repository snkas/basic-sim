/*
 * Copyright (c) 2020 ETH Zurich
 * Copyright (c) 2016 Universita' degli Studi di Napoli Federico II
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "arbiter-ecmp.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (ArbiterEcmp);
TypeId ArbiterEcmp::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::ArbiterEcmp")
            .SetParent<ArbiterPtop> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

ArbiterEcmp::ArbiterEcmp(
        Ptr<Node> this_node,
        NodeContainer nodes,
        Ptr<TopologyPtop> topology,
        std::vector<std::vector<uint32_t>> candidate_list
) : ArbiterPtop(this_node, nodes, topology)
{
    m_candidate_list = candidate_list;
}

int32_t ArbiterEcmp::TopologyPtopDecide(int32_t source_node_id, int32_t target_node_id, const std::set<int64_t>& neighbor_node_ids, Ptr<const Packet> pkt, Ipv4Header const &ipHeader, bool is_request_for_source_ip_so_no_next_header) {
    uint32_t hash = ComputeFiveTupleHash(ipHeader, pkt, m_node_id, is_request_for_source_ip_so_no_next_header);
    size_t s = m_candidate_list.at(target_node_id).size();
    if (s == 0) {
        throw std::invalid_argument(format_string(
                "There are no candidate ECMP next hops available at current node %d for a packet from source %d to destination %d",
                m_node_id, source_node_id, target_node_id
        ));
    }
    return m_candidate_list.at(target_node_id).at(hash % s);
}

ArbiterEcmp::~ArbiterEcmp() {
    // Left empty intentionally
}

/**
 * Calculates a hash (murmur3, ns-3 default) from the 5-tuple
 * along with the node identifier as per-node perturbation.
 *
 * It uses additionally an expectedly unique
 * perturbation constant of 1333527522 (randomly generated)
 * such that other functions that use the same hashing
 * procedure do not result in the same hashes.
 *
 * Based on:
 * uint32_t Ipv4QueueDiscItem::Hash
 * https://www.nsnam.org/doxygen/ipv4-queue-disc-item_8cc_source.html#l00115
 *
 * @param header               IPv4 header
 * @param p                    Packet
 * @param node_id              Node identifier
 * @param no_other_headers     True iff there are no other headers outside of the IPv4 one,
 *                             irrespective of what the IP protocol field claims
 *
 * @return Hash value (uint32_t)
 */
uint32_t
ArbiterEcmp::ComputeFiveTupleHash(const Ipv4Header &header, Ptr<const Packet> p, int32_t node_id, bool no_other_headers)
{
    NS_ABORT_MSG_IF(node_id < 0, "Node identifier cannot be negative as it is converted to an unsigned integer.");
    uint32_t unsigned_node_id = (uint32_t) node_id;

    // IP header fields
    Ipv4Address source_ip = header.GetSource ();
    Ipv4Address destination_ip = header.GetDestination ();
    uint8_t protocol = header.GetProtocol ();
    uint16_t frag_offset = header.GetFragmentOffset ();

    // Retrieve the source and destination port from the TCP or UDP header only if:
    // (a) We have NOT been notified that even though the protocol field might be non-zero,
    //     there is actually not another header to peek at
    // (b) The packet is not fragmented (fragment offset is zero)
    uint16_t source_port = 0;
    uint16_t destination_port = 0;
    if (!no_other_headers) {
        if (protocol == 6 && frag_offset == 0) { // TCP
            TcpHeader tcpHdr;
            p->PeekHeader(tcpHdr);
            source_port = tcpHdr.GetSourcePort();
            destination_port = tcpHdr.GetDestinationPort();
            NS_ABORT_MSG_IF(
                    source_port == 0 || destination_port == 0,
                    "Invalid port numbers; this indicates the TCP header is likely not present whereas it is expected to be"
            );

        } else if (protocol == 17 && frag_offset == 0) {  // UDP
            UdpHeader udpHdr;
            p->PeekHeader(udpHdr);
            source_port = udpHdr.GetSourcePort();
            destination_port = udpHdr.GetDestinationPort();
            NS_ABORT_MSG_IF(
                    source_port == 0 || destination_port == 0,
                    "Invalid port numbers; this indicates the UDP header is likely not present whereas it is expected to be"
            );
        }
    }

    // Serialize the 5-tuple, per-node perturbation (the node identifier) and the constant ECMP perturbation
    uint32_t ecmp_perturbation = 1333527522;
    uint8_t buf[21];
    source_ip.Serialize (buf);
    destination_ip.Serialize (buf + 4);
    buf[8] = protocol;
    buf[9] = (source_port >> 8) & 0xff;
    buf[10] = source_port & 0xff;
    buf[11] = (destination_port >> 8) & 0xff;
    buf[12] = destination_port & 0xff;
    buf[13] = (unsigned_node_id >> 24) & 0xff; // Per-node perturbation used is the node identifier
    buf[14] = (unsigned_node_id >> 16) & 0xff;
    buf[15] = (unsigned_node_id >> 8) & 0xff;
    buf[16] = unsigned_node_id & 0xff;
    buf[17] = (ecmp_perturbation >> 24) & 0xff; // Constant ECMP perturbation
    buf[18] = (ecmp_perturbation >> 16) & 0xff;
    buf[19] = (ecmp_perturbation >> 8) & 0xff;
    buf[20] = ecmp_perturbation & 0xff;

    // Linux calculates jhash2 (jenkins hash), we calculate murmur3
    // because it is already available in ns-3
    return Hash32 ((char*) buf, 21);

}

std::string ArbiterEcmp::StringReprOfForwardingState() {
    std::ostringstream res;
    res << "ECMP state of node " << m_node_id << std::endl;
    for (int i = 0; i < m_topology->GetNumNodes(); i++) {
        res << "  -> " << i << ": {";
        bool first = true;
        for (int j : m_candidate_list.at(i)) {
            if (!first) {
                res << ",";
            }
            res << j;
            first = false;
        }
        res << "}" << std::endl;
    }
    return res.str();
}

}
