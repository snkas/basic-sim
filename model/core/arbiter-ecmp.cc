/*
 * Copyright (c) 2020 ETH Zurich
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
 *
 * Author: Simon
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
    int s = m_candidate_list.at(target_node_id).size();
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
 * Calculates a hash from the 5-tuple.
 *
 * Inspired by: https://github.com/mkheirkhah/ecmp (February 20th, 2020)
 *
 * @param header               IPv4 header
 * @param p                    Packet
 * @param node_id              Node identifier
 * @param no_other_headers     True iff there are no other headers outside of the IPv4 one,
 *                             irrespective of what the IP protocol field claims
 *
 * @return Hash value
 */
uint64_t
ArbiterEcmp::ComputeFiveTupleHash(const Ipv4Header &header, Ptr<const Packet> p, int32_t node_id, bool no_other_headers)
{
    std::memcpy(&m_hash_input_buff[0], &node_id, 4);

    // Source IP address
    uint32_t src_ip = header.GetSource().Get();
    std::memcpy(&m_hash_input_buff[4], &src_ip, 4);

    // Destination IP address
    uint32_t dst_ip = header.GetDestination().Get();
    std::memcpy(&m_hash_input_buff[8], &dst_ip, 4);

    // Protocol
    uint8_t protocol = header.GetProtocol();
    std::memcpy(&m_hash_input_buff[12], &protocol, 1);

    // If we have been notified that whatever is in the protocol field,
    // does not mean there is another header to peek at, we do not peek
    if (no_other_headers) {
        m_hash_input_buff[13] = 0;
        m_hash_input_buff[14] = 0;
        m_hash_input_buff[15] = 0;
        m_hash_input_buff[16] = 0;

    } else {

        // If there are ports we can use, add them to the input
        switch (protocol) {
            case UDP_PROT_NUMBER: {
                UdpHeader udpHeader;
                p->PeekHeader(udpHeader);
                uint16_t src_port = udpHeader.GetSourcePort();
                uint16_t dst_port = udpHeader.GetDestinationPort();
                std::memcpy(&m_hash_input_buff[13], &src_port, 2);
                std::memcpy(&m_hash_input_buff[15], &dst_port, 2);
                break;
            }
            case TCP_PROT_NUMBER: {
                TcpHeader tcpHeader;
                p->PeekHeader(tcpHeader);
                uint16_t src_port = tcpHeader.GetSourcePort();
                uint16_t dst_port = tcpHeader.GetDestinationPort();
                std::memcpy(&m_hash_input_buff[13], &src_port, 2);
                std::memcpy(&m_hash_input_buff[15], &dst_port, 2);
                break;
            }
            default: {
                m_hash_input_buff[13] = 0;
                m_hash_input_buff[14] = 0;
                m_hash_input_buff[15] = 0;
                m_hash_input_buff[16] = 0;
                break;
            }
        }
    }

    m_hasher.clear();
    return m_hasher.GetHash32(m_hash_input_buff, 17);
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
