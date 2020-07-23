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


#include "topology-ptop.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TopologyPtop);
TypeId TopologyPtop::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::TopologyPtop")
            .SetParent<Object> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

TopologyPtop::TopologyPtop(Ptr<BasicSimulation> basicSimulation, const Ipv4RoutingHelper& ipv4RoutingHelper) {
    m_basicSimulation = basicSimulation;
    ReadRelevantConfig();
    ReadTopology();
    SetupNodes(ipv4RoutingHelper);
    SetupLinks();
}

void TopologyPtop::ReadRelevantConfig() {

    // Read the topology configuration
    m_topology_config = read_config(m_basicSimulation->GetRunDir() + "/" + m_basicSimulation->GetConfigParamOrFail("topology_filename"));
    for (auto const& entry : m_topology_config) {
        if (
                entry.first != "num_nodes" &&
                entry.first != "num_undirected_edges" &&
                entry.first != "switches" &&
                entry.first != "switches_which_are_tors" &&
                entry.first != "servers" &&
                entry.first != "undirected_edges" &&
                entry.first != "all_nodes_are_endpoints" &&
                entry.first != "link_channel_delay_ns" &&
                entry.first != "link_device_data_rate_megabit_per_s" &&
                entry.first != "link_device_max_queue_size" &&
                entry.first != "link_interface_traffic_control_qdisc"
        ) {
            throw std::runtime_error("Invalid topology property: " + entry.first);
        }
    }

    // If one want to allow applications on all nodes
    m_all_nodes_are_endpoints = parse_boolean(get_param_or_default("all_nodes_are_endpoints", "false", m_topology_config));

}

void TopologyPtop::ReadTopology() {

    // Basic
    m_num_nodes = parse_positive_int64(get_param_or_fail("num_nodes", m_topology_config));
    m_num_undirected_edges = parse_positive_int64(get_param_or_fail("num_undirected_edges", m_topology_config));

    // Node types
    std::string tmp;
    tmp = get_param_or_fail("switches", m_topology_config);
    m_switches = parse_set_positive_int64(tmp);
    all_items_are_less_than(m_switches, m_num_nodes);
    tmp = get_param_or_fail("switches_which_are_tors", m_topology_config);
    m_switches_which_are_tors = parse_set_positive_int64(tmp);
    all_items_are_less_than(m_switches_which_are_tors, m_num_nodes);
    tmp = get_param_or_fail("servers", m_topology_config);
    m_servers = parse_set_positive_int64(tmp);
    all_items_are_less_than(m_servers, m_num_nodes);

    // Adjacency list
    for (int i = 0; i < m_num_nodes; i++) {
        m_all_node_ids.insert(i);
        m_adjacency_list.push_back(std::set<int64_t>());
    }

    // Edges
    tmp = get_param_or_fail("undirected_edges", m_topology_config);
    std::set<std::string> string_set = parse_set_string(tmp);
    for (std::string s : string_set) {
        std::vector<std::string> spl = split_string(s, "-", 2);
        int64_t a = parse_positive_int64(spl[0]);
        int64_t b = parse_positive_int64(spl[1]);
        if (a == b) {
            throw std::invalid_argument(format_string("Cannot have edge to itself on node %" PRIu64 "", a));
        }
        if (a >= m_num_nodes) {
            throw std::invalid_argument(format_string("Left node identifier in edge does not exist: %" PRIu64 "", a));
        }
        if (b >= m_num_nodes) {
            throw std::invalid_argument(format_string("Right node identifier in edge does not exist: %" PRIu64 "", b));
        }
        if (b < a) {
            throw std::invalid_argument(format_string("As a convention, the left node id must be smaller than the right node id: %" PRIu64 "-%" PRIu64 "", a, b));
        }
        m_undirected_edges.push_back(std::make_pair(a, b));
        m_undirected_edges_set.insert(std::make_pair(a, b));
        m_adjacency_list[a].insert(b);
        m_adjacency_list[b].insert(a);
    }

    // Sort them for convenience
    std::sort(m_undirected_edges.begin(), m_undirected_edges.end());

    // Edge checks

    if (m_undirected_edges.size() != (size_t) m_num_undirected_edges) {
        throw std::invalid_argument("Indicated number of undirected edges does not match edge set");
    }

    if (m_undirected_edges.size() != m_undirected_edges_set.size()) {
        throw std::invalid_argument("Duplicates in edge set");
    }

    // Node type hierarchy checks

    if (!direct_set_intersection(m_servers, m_switches).empty()) {
        throw std::invalid_argument("Server and switch identifiers are not distinct");
    }

    if (direct_set_union(m_servers, m_switches).size() != (size_t) m_num_nodes) {
        throw std::invalid_argument("The servers and switches do not encompass all nodes");
    }

    if (direct_set_intersection(m_switches, m_switches_which_are_tors).size() != m_switches_which_are_tors.size()) {
        throw std::invalid_argument("Servers are marked as ToRs");
    }

    // Servers must be connected to ToRs only
    for (int64_t node_id : m_servers) {
        for (int64_t neighbor_id : m_adjacency_list[node_id]) {
            if (m_switches_which_are_tors.find(neighbor_id) == m_switches_which_are_tors.end()) {
                throw std::invalid_argument(format_string("Server node %" PRId64 " has an edge to node %" PRId64 " which is not a ToR.", node_id, neighbor_id));
            }
        }
    }

    // Check
    if (m_servers.size() > 0) {
        m_has_zero_servers = false;
    } else {
        m_has_zero_servers = true;
    }

    // Check that each node has an assignment to a system id if it is distributed
    if (m_basicSimulation->IsDistributedEnabled()) {
        size_t node_assignment_size = m_basicSimulation->GetDistributedNodeSystemIdAssignment().size();
        if (node_assignment_size != (size_t) m_num_nodes) {
            throw std::invalid_argument(
                    format_string("Incorrect amount of node-to-system-id assignments (must be %" PRId64 " but got %u)", m_num_nodes, node_assignment_size)
            );
        }
    }

    // Print summary
    printf("TOPOLOGY SUMMARY\n");
    printf("  > Number of nodes.... %" PRIu64 "\n", m_num_nodes);
    printf("    >> Switches........ %" PRIu64 " (of which %" PRIu64 " are ToRs)\n", m_switches.size(), m_switches_which_are_tors.size());
    printf("    >> Servers......... %" PRIu64 "\n", m_servers.size());
    printf("  > Undirected edges... %" PRIu64 "\n\n", m_num_undirected_edges);
    m_basicSimulation->RegisterTimestamp("Read topology");

}

void TopologyPtop::SetupNodes(const Ipv4RoutingHelper& ipv4RoutingHelper) {
    std::cout << "SETUP NODES" << std::endl;

    // Creating the nodes in their respective system ID
    std::cout << "  > Creating nodes" << std::endl;
    if (m_basicSimulation->IsDistributedEnabled()) {
        for (int64_t system_id_assigned : m_basicSimulation->GetDistributedNodeSystemIdAssignment()) {
            m_nodes.Create(1, system_id_assigned);
        }
    } else {
        m_nodes.Create(m_num_nodes);
    }
    m_basicSimulation->RegisterTimestamp("Create nodes");

    // Install Internet on all nodes
    std::cout << "  > Installing Internet stack on each node" << std::endl;
    InternetStackHelper internet;
    internet.SetRoutingHelper(ipv4RoutingHelper);
    internet.Install(m_nodes);

    std::cout << std::endl;
    m_basicSimulation->RegisterTimestamp("Install Internet stack (incl. routing) on nodes");
}

std::map<std::pair<int64_t, int64_t>, std::string> TopologyPtop::ParseUndirectedEdgeMap(std::string value) {
    std::map<std::pair<int64_t, int64_t>, std::string> result;

    // If it does not start with map, the value cannot possibly be a mapping, as such we set it as a single value
    if (!starts_with(trim(value), "map")) {
        for (std::pair<int64_t, int64_t> p : m_undirected_edges_set) {
            result[p] = value;
        }
        return result;
    }

    // Go over every entry in the mapping
    std::vector<std::pair<std::string, std::string>> pair_list = parse_map_string(value);
    for (std::pair<std::string, std::string> p : pair_list) {

        // Find the a-b
        std::vector<std::string> dash_split_list = split_string(p.first, "-", 2);
        int64_t a = parse_positive_int64(dash_split_list[0]);
        int64_t b = parse_positive_int64(dash_split_list[1]);
        if (b < a) {
            throw std::invalid_argument(format_string("As a convention, the left node id must be smaller than the right node id: %" PRIu64 "-%" PRIu64 "", a, b));
        }

        // No duplicates in the mapping
        if (result.find(std::make_pair(a, b)) != result.end()) {
            throw std::runtime_error("Duplicate key in undirected edge mapping: " + p.first);
        }

        // Must be present
        if (m_undirected_edges_set.find(std::make_pair(a, b)) == m_undirected_edges_set.end()) {
            throw std::runtime_error("Unknown undirected edge: " + p.first);
        }

        // Add to mapping
        result[std::make_pair(a, b)] = p.second;

    }

    if (m_undirected_edges_set.size() != result.size()) {
        throw std::runtime_error("Not all undirected edges were covered");
    }

    return result;
}

std::map<std::pair<int64_t, int64_t>, std::string> TopologyPtop::ParseDirectedEdgeMap(std::string value) {

    std::map<std::pair<int64_t, int64_t>, std::string> result;

    // If it does not start with map, the value cannot possibly be a mapping, as such we set it as a single value
    if (!starts_with(trim(value), "map")) {
        for (std::pair<int64_t, int64_t> p : m_undirected_edges_set) {
            result[p] = value;
            result[std::make_pair(p.second, p.first)] = value;
        }
        return result;
    }

    // Go over every entry in the mapping
    std::vector<std::pair<std::string, std::string>> pair_list = parse_map_string(value);
    for (std::pair<std::string, std::string> p : pair_list) {

        // Find the a -> b
        std::vector<std::string> dash_split_list = split_string(p.first, "->", 2);
        int64_t a = parse_positive_int64(dash_split_list[0]);
        int64_t b = parse_positive_int64(dash_split_list[1]);

        // No duplicates in the mapping
        if (result.find(std::make_pair(a, b)) != result.end()) {
            throw std::runtime_error("Duplicate key in directed edge mapping: " + p.first);
        }

        // Must be present
        if (m_undirected_edges_set.find(std::make_pair(a < b ? a : b, b > a ? b : a)) == m_undirected_edges_set.end()) {
            throw std::runtime_error("Does not belong to an undirected edge: " + p.first);
        }

        // Add to mapping
        result[std::make_pair(a, b)] = p.second;

    }

    if (m_undirected_edges_set.size() * 2 != result.size()) {
        throw std::runtime_error("Not all undirected edges were covered");
    }

    return result;
}

std::string TopologyPtop::ValidateMaxQueueSizeValue(std::string value) {
    if (ends_with(value, "p") || ends_with(value, "B")) {
        parse_positive_int64(value.substr(0, value.size() - 1)); // Nothing else, only 1000p or 1000B for example
        return value;
    } else {
        throw std::runtime_error("Invalid maximum queue size value: " + value);
    }
}

TrafficControlHelper TopologyPtop::ParseTrafficControlQdiscValue(std::string value) {
    if (value == "default") {
        TrafficControlHelper defaultHelper;
        return defaultHelper;
    } else {
        throw std::runtime_error("Unknown traffic control qdisc value: " + value);
    }
}

void TopologyPtop::SetupLinks() {
    std::cout << "SETUP LINKS" << std::endl;

    // Each link is a network on its own
    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.0");

    // Read in the link mappings

    std::cout << "  > Parsing link channel delay mapping" << std::endl;
    std::map<std::pair<int64_t, int64_t>, std::string> undirected_edge_to_channel_delay_str = ParseUndirectedEdgeMap(
        get_param_or_fail("link_channel_delay_ns", m_topology_config)
    );

    std::cout << "  > Parsing link device data rate mapping" << std::endl;
    std::map<std::pair<int64_t, int64_t>, std::string> directed_edge_to_device_data_rate_str = ParseDirectedEdgeMap(
        get_param_or_fail("link_device_data_rate_megabit_per_s", m_topology_config)
    );

    std::cout << "  > Parsing link device max. queue size mapping" << std::endl;
    std::map<std::pair<int64_t, int64_t>, std::string> directed_edge_to_device_max_queue_size_str = ParseDirectedEdgeMap(
        get_param_or_fail("link_device_max_queue_size", m_topology_config)
    );

    std::cout << "  > Parsing link interface traffic control queuing discipline mapping" << std::endl;
    std::map<std::pair<int64_t, int64_t>, std::string> directed_edge_to_interface_traffic_control_qdisc_str = ParseDirectedEdgeMap(
        get_param_or_fail("link_interface_traffic_control_qdisc", m_topology_config)
    );

    // Create Links
    std::cout << "  > Installing links" << std::endl;
    m_interface_idxs_for_edges.clear();
    for (std::pair<int64_t, int64_t> link : m_undirected_edges) {

        // Retrieve all relevant details
        std::pair<int64_t, int64_t> edge_a_to_b = link;
        std::pair<int64_t, int64_t> edge_b_to_a = std::make_pair(link.second, link.first);

        // Install the point-to-point link
        PointToPointHelper p2p;
        double channel_delay_ns = parse_positive_int64(undirected_edge_to_channel_delay_str[link]);
        p2p.SetChannelAttribute("Delay", TimeValue(NanoSeconds(channel_delay_ns)));
        NetDeviceContainer container = p2p.Install(m_nodes.Get(link.first), m_nodes.Get(link.second));

        // Retrieve the network devices installed on either end of the link
        Ptr<PointToPointNetDevice> netDeviceA = container.Get(0)->GetObject<PointToPointNetDevice>();
        Ptr<PointToPointNetDevice> netDeviceB = container.Get(1)->GetObject<PointToPointNetDevice>();

        // Data rate
        double a_to_b_data_rate_megabit_per_s = parse_positive_double(directed_edge_to_device_data_rate_str[edge_a_to_b]);
        double b_to_a_data_rate_megabit_per_s = parse_positive_double(directed_edge_to_device_data_rate_str[edge_b_to_a]);
        netDeviceA->SetDataRate(DataRate(std::to_string(a_to_b_data_rate_megabit_per_s) + "Mbps"));
        netDeviceB->SetDataRate(DataRate(std::to_string(b_to_a_data_rate_megabit_per_s) + "Mbps"));

        // Queue size
        netDeviceA->GetQueue()->SetMaxSize(ValidateMaxQueueSizeValue(directed_edge_to_device_max_queue_size_str[edge_a_to_b]));
        netDeviceB->GetQueue()->SetMaxSize(ValidateMaxQueueSizeValue(directed_edge_to_device_max_queue_size_str[edge_b_to_a]));

        // Traffic control queueing discipline
        std::string a_to_b_traffic_control_qdisc = trim(directed_edge_to_interface_traffic_control_qdisc_str[edge_a_to_b]);
        std::string b_to_a_traffic_control_qdisc = trim(directed_edge_to_interface_traffic_control_qdisc_str[edge_b_to_a]);
        if (a_to_b_traffic_control_qdisc != "disabled") {
            TrafficControlHelper helper = ParseTrafficControlQdiscValue(a_to_b_traffic_control_qdisc);
            helper.Install(netDeviceA);
        }
        if (b_to_a_traffic_control_qdisc != "disabled") {
            TrafficControlHelper helper = ParseTrafficControlQdiscValue(b_to_a_traffic_control_qdisc);
            helper.Install(netDeviceA);
        }

        // Assign IP addresses
        address.Assign(container); // This will install the default traffic control layer if not already done above
        address.NewNetwork();

        // Remove the (default) traffic control layer if undesired
        TrafficControlHelper tch_uninstaller;
        if (a_to_b_traffic_control_qdisc == "disabled") {
            tch_uninstaller.Uninstall(netDeviceA);
        }
        if (b_to_a_traffic_control_qdisc == "disabled") {
            tch_uninstaller.Uninstall(netDeviceB);
        }

        // Save to mapping
        uint32_t a = container.Get(0)->GetIfIndex();
        uint32_t b = container.Get(1)->GetIfIndex();
        m_interface_idxs_for_edges.push_back(std::make_pair(a, b));

    }

    std::cout << std::endl;
    m_basicSimulation->RegisterTimestamp("Create links and edge-to-interface-index mapping");
}

const NodeContainer& TopologyPtop::GetNodes() {
    return m_nodes;
}

int64_t TopologyPtop::GetNumNodes() {
    return m_num_nodes;
}

int64_t TopologyPtop::GetNumUndirectedEdges() {
    return m_num_undirected_edges;
}

const std::set<int64_t>& TopologyPtop::GetSwitches() {
    return m_switches;
}

const std::set<int64_t>& TopologyPtop::GetSwitchesWhichAreTors() {
    return m_switches_which_are_tors;
}

const std::set<int64_t>& TopologyPtop::GetServers() {
    return m_servers;
}

bool TopologyPtop::IsValidEndpoint(int64_t node_id) {
    if (m_all_nodes_are_endpoints) {
        return true;
    } else {
        if (m_has_zero_servers) {
            return m_switches_which_are_tors.find(node_id) != m_switches_which_are_tors.end();
        } else {
            return m_servers.find(node_id) != m_servers.end();
        }
    }
}

const std::set<int64_t>& TopologyPtop::GetEndpoints() {
    if (m_all_nodes_are_endpoints) {
        return m_all_node_ids;
    } else {
        if (m_has_zero_servers) {
            return m_switches_which_are_tors;
        } else {
            return m_servers;
        }
    }
}

const std::vector<std::pair<int64_t, int64_t>>& TopologyPtop::GetUndirectedEdges() {
    return m_undirected_edges;
}

const std::set<std::pair<int64_t, int64_t>>& TopologyPtop::GetUndirectedEdgesSet() {
    return m_undirected_edges_set;
}

const std::vector<std::set<int64_t>>& TopologyPtop::GetAllAdjacencyLists() {
    return m_adjacency_list;
}

const std::set<int64_t>& TopologyPtop::GetAdjacencyList(int64_t node_id) {
    return m_adjacency_list[node_id];
}

int64_t TopologyPtop::GetWorstCaseRttEstimateNs() {
    return m_worst_case_rtt_ns;
}

const std::vector<std::pair<uint32_t, uint32_t>>& TopologyPtop::GetInterfaceIdxsForEdges() {
    return m_interface_idxs_for_edges;
}

}
