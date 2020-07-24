/*
 * Copyright (c) 2019 ETH Zurich
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
 * Originally based on, but since heavily adapted/extended, the scratch/main authored by Hussain.
 */

#ifndef TOPOLOGY_PTOP_H
#define TOPOLOGY_PTOP_H

#include <utility>
#include "ns3/core-module.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/topology.h"
#include "ns3/exp-util.h"
#include "ns3/basic-simulation.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/command-line.h"
#include "ns3/traffic-control-helper.h"
#include "ns3/point-to-point-ab-helper.h"
#include "ns3/topology-ptop-queue-selector.h"
#include "ns3/topology-ptop-tc-qdisc-selector.h"

namespace ns3 {

class TopologyPtop : public Topology
{
public:

    // Constructors
    static TypeId GetTypeId (void);
    TopologyPtop(Ptr<BasicSimulation> basicSimulation, const Ipv4RoutingHelper& ipv4RoutingHelper);

    // Parsers to assist
    std::map<std::pair<int64_t, int64_t>, std::string> ParseUndirectedEdgeMap(std::string value);
    std::map<std::pair<int64_t, int64_t>, std::string> ParseDirectedEdgeMap(std::string value);

    // Accessors
    const NodeContainer& GetNodes();
    int64_t GetNumNodes();
    int64_t GetNumUndirectedEdges();
    const std::set<int64_t>& GetSwitches();
    const std::set<int64_t>& GetSwitchesWhichAreTors();
    const std::set<int64_t>& GetServers();
    bool IsValidEndpoint(int64_t node_id);
    const std::set<int64_t>& GetEndpoints();
    const std::vector<std::pair<int64_t, int64_t>>& GetUndirectedEdges();
    const std::set<std::pair<int64_t, int64_t>>& GetUndirectedEdgesSet();
    const std::vector<std::set<int64_t>>& GetAllAdjacencyLists();
    const std::set<int64_t>& GetAdjacencyList(int64_t node_id);
    int64_t GetWorstCaseRttEstimateNs();
    const std::vector<std::pair<uint32_t, uint32_t>>& GetInterfaceIdxsForEdges();

private:

    Ptr<BasicSimulation> m_basicSimulation;

    // Construction
    void ReadRelevantConfig();
    void ReadTopology();
    void SetupNodes(const Ipv4RoutingHelper& ipv4RoutingHelper);
    std::map<std::pair<int64_t, int64_t>, int64_t> ParseLinkChannelDelayNsProperty();
    std::map<std::pair<int64_t, int64_t>, double> ParseLinkDeviceDataRateMegabitPerSecProperty();
    std::map<std::pair<int64_t, int64_t>, ObjectFactory> ParseLinkDeviceQueueProperty();
    std::map<std::pair<int64_t, int64_t>, std::string> ParseLinkInterfaceTrafficControlQdiscProperty();
    void SetupLinks();

    // Configuration properties
    std::map<std::string, std::string> m_topology_config;
    int64_t m_worst_case_rtt_ns = 1000000000;
    bool m_all_nodes_are_endpoints;

    // Graph properties
    int64_t m_num_nodes;
    int64_t m_num_undirected_edges;
    std::set<int64_t> m_switches;
    std::set<int64_t> m_switches_which_are_tors;
    std::set<int64_t> m_servers;
    std::set<int64_t> m_all_node_ids;
    std::vector<std::pair<int64_t, int64_t>> m_undirected_edges;
    std::set<std::pair<int64_t, int64_t>> m_undirected_edges_set;
    std::vector<std::set<int64_t>> m_adjacency_list;
    bool m_has_zero_servers;

    // From generating ns3 objects
    NodeContainer m_nodes;
    std::vector<std::pair<uint32_t, uint32_t>> m_interface_idxs_for_edges;

};

}

#endif //TOPOLOGY_PTOP_H
