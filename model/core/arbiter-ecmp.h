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

#ifndef ARBITER_ECMP_H
#define ARBITER_ECMP_H

#include "ns3/arbiter-ptop.h"
#include "ns3/hash.h"

namespace ns3 {

class ArbiterEcmp : public ArbiterPtop
{
public:
    static TypeId GetTypeId (void);

    // Constructor for ECMP forwarding state
    ArbiterEcmp(
            Ptr<Node> this_node,
            NodeContainer nodes,
            Ptr<TopologyPtop> topology,
            std::vector<std::vector<uint32_t>> candidate_list
    );
    virtual ~ArbiterEcmp();

    // ECMP implementation
    int32_t TopologyPtopDecide(
            int32_t source_node_id,
            int32_t target_node_id,
            const std::set<int64_t>& neighbor_node_ids,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip
    );

    // ECMP routing table
    std::string StringReprOfForwardingState();

    // Made public for testing
    uint32_t ComputeFiveTupleHash(const Ipv4Header &header, Ptr<const Packet> p, int32_t node_id, bool no_other_headers);

private:
    std::vector<std::vector<uint32_t>> m_candidate_list;

};

}

#endif //ARBITER_ECMP_H
