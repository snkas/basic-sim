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

#include "ns3/core-module.h"
#include "ns3/node-container.h"
#include "ns3/topology.h"
#include "ns3/exp-util.h"
#include "ns3/basic-simulation.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-helper.h"
#include "ns3/point-to-point-ab-helper.h"

namespace ns3 {

class TopologyPtop;

class TopologyPtopQueueSelector : public Object {
public:
    static TypeId GetTypeId(void);
    TopologyPtopQueueSelector() {};
    virtual ~TopologyPtopQueueSelector() {};
    virtual std::pair<ObjectFactory, QueueSize> ParseQueueValue(Ptr<TopologyPtop> topology, std::string value) = 0;
};

class TopologyPtopReceiveErrorModelSelector : public Object {
public:
    static TypeId GetTypeId(void);
    TopologyPtopReceiveErrorModelSelector() {};
    virtual ~TopologyPtopReceiveErrorModelSelector() {};
    virtual std::pair<bool, Ptr<ErrorModel>> ParseReceiveErrorModelValue(Ptr<TopologyPtop> topology, std::string value) = 0;
};

class TopologyPtopTcQdiscSelector : public Object {
public:
    static TypeId GetTypeId(void);
    TopologyPtopTcQdiscSelector() {};
    virtual ~TopologyPtopTcQdiscSelector() {};
    virtual std::tuple<bool, TrafficControlHelper, QueueSize> ParseTcQdiscValue(Ptr<TopologyPtop> topology, std::string value) = 0;
};

class TopologyPtop : public Topology
{
public:

    // Constructors
    static TypeId GetTypeId (void);
    TopologyPtop(
            Ptr<BasicSimulation> basicSimulation,
            const Ipv4RoutingHelper& ipv4RoutingHelper,
            Ptr<TopologyPtopQueueSelector> queueSelector,
            Ptr<TopologyPtopReceiveErrorModelSelector> receiveErrorModelSelector,
            Ptr<TopologyPtopTcQdiscSelector> tcQdiscSelector
    );
    TopologyPtop(
            Ptr<BasicSimulation> basicSimulation,
            const Ipv4RoutingHelper& ipv4RoutingHelper
    );
    virtual ~TopologyPtop();

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
    std::vector<std::pair<int64_t, int64_t>> GetLinks();
    std::set<std::pair<int64_t, int64_t>> GetLinksSet();
    const std::vector<std::pair<int64_t, int64_t>>& GetUndirectedEdges();
    const std::set<std::pair<int64_t, int64_t>>& GetUndirectedEdgesSet();
    const std::vector<std::set<int64_t>>& GetAllAdjacencyLists();
    const std::set<int64_t>& GetAdjacencyList(int64_t node_id);
    int64_t GetWorstCaseRttEstimateNs();
    const std::vector<std::pair<uint32_t, uint32_t>>& GetInterfaceIdxsForUndirectedEdges();
    const std::vector<std::pair<Ptr<PointToPointNetDevice>, Ptr<PointToPointNetDevice>>>& GetNetDevicesForUndirectedEdges();
    Ptr<PointToPointNetDevice> GetSendingNetDeviceForLink(std::pair<int64_t, int64_t> link);

private:

    // Handle to basic simulation
    Ptr<BasicSimulation> m_basicSimulation;
    std::map<std::string, std::string> m_topology_config;
    bool m_all_nodes_are_endpoints;

    // Reading
    void ReadTopologyConfig();
    void ParseTopologyGraph();
    void ParseLinkChannelDelayNsProperty();
    void ParseLinkNetDeviceDataRateMegabitPerSecProperty();
    void ParseLinkNetDeviceQueueProperty();
    void ParseLinkNetDeviceReceiveErrorModelProperty();
    void ParseLinkInterfaceTrafficControlQdiscProperty();
    void ParseTopologyLinkProperties();
    Ptr<TopologyPtopQueueSelector> m_queueSelector;
    Ptr<TopologyPtopReceiveErrorModelSelector> m_receiveErrorModelSelector;
    Ptr<TopologyPtopTcQdiscSelector> m_tcQdiscSelector;

    // Estimations
    void EstimateWorstCaseRtt();

    // Ns-3 construction
    void SetupNodes(const Ipv4RoutingHelper& ipv4RoutingHelper);
    void SetupLinks();

    // Topology layout properties
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

    // Topology link properties
    std::map<std::pair<int64_t, int64_t>, int64_t> m_link_channel_delay_ns_mapping;
    std::map<std::pair<int64_t, int64_t>, double> m_link_net_device_data_rate_megabit_per_s_mapping;
    std::map<std::pair<int64_t, int64_t>, std::pair<ObjectFactory, QueueSize>> m_link_net_device_queue_mapping;
    std::map<std::pair<int64_t, int64_t>, std::pair<bool, Ptr<ErrorModel>>> m_link_net_device_receive_error_model_mapping;
    std::map<std::pair<int64_t, int64_t>, std::tuple<bool, TrafficControlHelper, QueueSize>> m_link_interface_traffic_control_qdisc_mapping;

    // Estimations
    bool m_worst_case_rtt_was_estimated = false;
    int64_t m_worst_case_rtt_estimate_ns = 0;

    // From generating ns-3 objects
    NodeContainer m_nodes;
    std::vector<std::pair<uint32_t, uint32_t>> m_interface_idxs_for_undirected_edges;
    std::vector<std::pair<Ptr<PointToPointNetDevice>, Ptr<PointToPointNetDevice>>> m_net_devices_for_undirected_edges;
    std::map<std::pair<uint32_t, uint32_t>, Ptr<PointToPointNetDevice>> m_link_to_sending_net_device;

};

}

#endif //TOPOLOGY_PTOP_H
