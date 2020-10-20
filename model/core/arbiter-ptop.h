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

#ifndef ARBITER_PTOP_H
#define ARBITER_PTOP_H

#include "ns3/topology-ptop.h"
#include "ns3/arbiter.h"

namespace ns3 {

class ArbiterPtop : public Arbiter
{

public:
    static TypeId GetTypeId (void);
    ArbiterPtop(Ptr<Node> this_node, NodeContainer nodes, Ptr<TopologyPtop> topology);
    virtual ~ArbiterPtop();

    // Topology implementation
    ArbiterResult Decide(
            int32_t source_node_id,
            int32_t target_node_id,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip
    );

    /**
     * From among the neighbors, decide where the packet needs to be routed to.
     *
     * @param source_node_id                                Node where the packet originated from
     * @param target_node_id                                Node where the packet has to go to
     * @param neighbor_node_ids                             All neighboring nodes from which to choose
     * @param pkt                                           Packet
     * @param ipHeader                                      IP header instance
     * @param is_socket_request_for_source_ip               True iff it is a request for a source IP address,
     *                                                      as such the returning next hop is only used to get the
     *                                                      interface IP address
     *
     * @return Neighbor node id to which to forward to (-1 means to none)
     */
    virtual int32_t TopologyPtopDecide(
            int32_t source_node_id,
            int32_t target_node_id,
            const std::set<int64_t>& neighbor_node_ids,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip
    ) = 0;

    virtual std::string StringReprOfForwardingState() = 0;

protected:
    Ptr<TopologyPtop> m_topology;
    std::vector<uint32_t> m_neighbor_node_id_to_if_idx;

};

}

#endif //ARBITER_PTOP_H
