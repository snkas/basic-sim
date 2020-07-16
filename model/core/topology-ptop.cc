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

    // Link properties
    m_link_data_rate_megabit_per_s = parse_positive_double(m_basicSimulation->GetConfigParamOrFail("link_data_rate_megabit_per_s"));
    m_link_delay_ns = parse_positive_int64(m_basicSimulation->GetConfigParamOrFail("link_delay_ns"));
    m_link_max_queue_size_pkts = parse_positive_int64(m_basicSimulation->GetConfigParamOrFail("link_max_queue_size_pkts"));

    // Qdisc properties
    m_disable_qdisc_endpoint_tors_xor_servers = parse_boolean(m_basicSimulation->GetConfigParamOrFail("disable_qdisc_endpoint_tors_xor_servers"));
    m_disable_qdisc_non_endpoint_switches = parse_boolean(m_basicSimulation->GetConfigParamOrFail("disable_qdisc_non_endpoint_switches"));

    // Distributed
    m_enable_distributed = parse_boolean(m_basicSimulation->GetConfigParamOrDefault("enable_distributed", "false"));
    if (m_enable_distributed) {
        m_distributed_logical_processes_k = parse_positive_int64(m_basicSimulation->GetConfigParamOrFail("distributed_logical_processes_k"));
        m_distributed_node_logical_process_assignment = parse_list_positive_int64(m_basicSimulation->GetConfigParamOrFail("distributed_node_logical_process_assignment"));
    } else {
        m_distributed_logical_processes_k = 1;
        m_distributed_node_logical_process_assignment.clear();
    }

}

void TopologyPtop::ReadTopology() {

    // Read the topology configuration
    std::map<std::string, std::string> config = read_config(m_basicSimulation->GetRunDir() + "/" + m_basicSimulation->GetConfigParamOrFail("filename_topology"));

    // Basic
    m_num_nodes = parse_positive_int64(get_param_or_fail("num_nodes", config));
    m_num_undirected_edges = parse_positive_int64(get_param_or_fail("num_undirected_edges", config));

    // Node types
    std::string tmp;
    tmp = get_param_or_fail("switches", config);
    m_switches = parse_set_positive_int64(tmp);
    all_items_are_less_than(m_switches, m_num_nodes);
    tmp = get_param_or_fail("switches_which_are_tors", config);
    m_switches_which_are_tors = parse_set_positive_int64(tmp);
    all_items_are_less_than(m_switches_which_are_tors, m_num_nodes);
    tmp = get_param_or_fail("servers", config);
    m_servers = parse_set_positive_int64(tmp);
    all_items_are_less_than(m_servers, m_num_nodes);

    // Adjacency list
    for (int i = 0; i < m_num_nodes; i++) {
        m_adjacency_list.push_back(std::set<int64_t>());
    }

    // Edges
    tmp = get_param_or_fail("undirected_edges", config);
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
        m_undirected_edges.push_back(std::make_pair(a < b ? a : b, a < b ? b : a));
        m_undirected_edges_set.insert(std::make_pair(a < b ? a : b, a < b ? b : a));
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

    // Print summary
    printf("TOPOLOGY SUMMARY\n");
    printf("  > Number of nodes.... %" PRIu64 "\n", m_num_nodes);
    printf("    >> Switches........ %" PRIu64 " (of which %" PRIu64 " are ToRs)\n", m_switches.size(), m_switches_which_are_tors.size());
    printf("    >> Servers......... %" PRIu64 "\n", m_servers.size());
    printf("  > Undirected edges... %" PRIu64 "\n\n", m_num_undirected_edges);
    m_basicSimulation->RegisterTimestamp("Read topology");


    if (m_enable_distributed) {

        // At least one logical process
        if (m_distributed_logical_processes_k < 1) {
            throw std::invalid_argument(
                    format_string("Distributed number of logical processes k=%" PRId64 " must be at least 1", m_distributed_logical_processes_k)
            );
        }

        // Each node must have an assignment to a logical process
        if (m_distributed_node_logical_process_assignment.size() != (size_t) m_num_nodes) {
            throw std::invalid_argument(
                    format_string("Incorrect amount of node assignments (must be %" PRId64 " but got %u)", m_num_nodes, m_distributed_node_logical_process_assignment.size())
            );
        }

        // Check assignment
        std::vector<int> logical_process_counter(m_distributed_logical_processes_k, 0);
        for (int i = 0; i < m_num_nodes; i++) {
            if (m_distributed_node_logical_process_assignment[i] < 0 || m_distributed_node_logical_process_assignment[i] >= m_distributed_logical_processes_k) {
                throw std::invalid_argument(
                        format_string(
                                "Node %d is assigned to an invalid logical process %" PRId64 " (k=%" PRId64 ")",
                                i,
                                m_distributed_node_logical_process_assignment[i],
                                m_distributed_logical_processes_k
                        )
                );
            }
            logical_process_counter[m_distributed_node_logical_process_assignment[i]]++;
        }

        // All good, showing summary
        printf("DISTRIBUTED IS ENABLED\n");
        printf("  > Number of logical processes... %" PRIu64 "\n", m_distributed_logical_processes_k);
        printf("  > Logical process information:\n");
        for (int i = 0; i < m_distributed_logical_processes_k; i++) {
            printf("    >> Logical process %d has %d node(s)\n", i, logical_process_counter[i]);
        }
        printf("\n");

    }

    // MTU = 1500 byte, +2 with the p2p header.
    // There are n_q packets in the queue at most, e.g. n_q + 2 (incl. transmit and within mandatory 1-packet qdisc)
    // Queueing + transmission delay/hop = (n_q + 2) * 1502 byte / link data rate
    // Propagation delay/hop = link delay
    //
    // If the topology is big, lets assume 10 hops either direction worst case, so 20 hops total
    // If the topology is not big, < 10 undirected edges, we just use 2 * number of undirected edges as worst-case hop count
    //
    // num_hops * (((n_q + 2) * 1502 byte) / link data rate) + link delay)
    int num_hops = std::min((int64_t) 20, m_num_undirected_edges * 2);
    m_worst_case_rtt_ns = num_hops * (((m_link_max_queue_size_pkts + 2) * 1502) / (m_link_data_rate_megabit_per_s * 125000 / 1000000000) + m_link_delay_ns);
    printf("Estimated worst-case RTT: %.3f ms\n\n", m_worst_case_rtt_ns / 1e6);

}

void TopologyPtop::SetupNodes(const Ipv4RoutingHelper& ipv4RoutingHelper) {
    std::cout << "SETUP NODES" << std::endl;
    std::cout << "  > Creating nodes and installing Internet stack on each" << std::endl;

    // Install Internet on all nodes
    m_nodes.Create(m_num_nodes);
    InternetStackHelper internet;
    internet.SetRoutingHelper(ipv4RoutingHelper);
    internet.Install(m_nodes);

    std::cout << std::endl;
    m_basicSimulation->RegisterTimestamp("Create and install nodes");
}

void TopologyPtop::SetupLinks() {
    std::cout << "SETUP LINKS" << std::endl;

    // Each link is a network on its own
    Ipv4AddressHelper address;
    address.SetBase("10.0.0.0", "255.255.255.0");

    // Direct network device attributes
    std::cout << "  > Point-to-point network device attributes:" << std::endl;

    // Point-to-point network device helper
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(std::to_string(m_link_data_rate_megabit_per_s) + "Mbps"));
    p2p.SetChannelAttribute("Delay", TimeValue(NanoSeconds(m_link_delay_ns)));
    std::cout << "    >> Data rate......... " << m_link_data_rate_megabit_per_s << " Mbit/s" << std::endl;
    std::cout << "    >> Delay............. " << m_link_delay_ns << " ns" << std::endl;
    std::cout << "    >> Max. queue size... " << m_link_max_queue_size_pkts << " packets" << std::endl;
    std::string p2p_net_device_max_queue_size_pkts_str = format_string("%" PRId64 "p", m_link_max_queue_size_pkts);

    // Notify about topology state
    if (m_has_zero_servers) {
        std::cout << "  > Because there are no servers, ToRs are considered valid flow-endpoints" << std::endl;
    } else {
        std::cout << "  > Only servers are considered valid flow-endpoints" << std::endl;
    }

    // Queueing disciplines
    std::cout << "  > Traffic control queueing disciplines:" << std::endl;

    // Queueing discipline for endpoints (i.e., servers if they are defined, else the ToRs)
    TrafficControlHelper tch_endpoints;
    if (m_disable_qdisc_endpoint_tors_xor_servers) {
        std::cout << "    >> Flow-endpoints....... none (disabled)" << std::endl;
    } else {
        std::string interval = format_string("%" PRId64 "ns", m_worst_case_rtt_ns);
        std::string target = format_string("%" PRId64 "ns", m_worst_case_rtt_ns / 20);
        tch_endpoints.SetRootQueueDisc("ns3::FqCoDelQueueDisc", "Interval", StringValue(interval), "Target", StringValue(target));
        printf("    >> Flow-endpoints....... fq-co-del (interval = %.2f ms, target = %.2f ms)\n", m_worst_case_rtt_ns / 1e6, m_worst_case_rtt_ns / 1e6 / 20);
    }

    // Queueing discipline for non-endpoints (i.e., if servers are defined, all switches, else the switches which are not ToRs)
    TrafficControlHelper tch_not_endpoints;
    if (m_disable_qdisc_non_endpoint_switches) {
        std::cout << "    >> Non-flow-endpoints... none (disabled)" << std::endl;
    } else {
        std::string interval = format_string("%" PRId64 "ns", m_worst_case_rtt_ns);
        std::string target = format_string("%" PRId64 "ns", m_worst_case_rtt_ns / 20);
        tch_not_endpoints.SetRootQueueDisc("ns3::FqCoDelQueueDisc", "Interval", StringValue(interval), "Target", StringValue(target));
        printf("    >> Non-flow-endpoints... fq-co-del (interval = %.2f ms, target = %.2f ms)\n", m_worst_case_rtt_ns / 1e6, m_worst_case_rtt_ns / 1e6 / 20);
    }

    // Create Links
    std::cout << "  > Installing links" << std::endl;
    m_interface_idxs_for_edges.clear();
    for (std::pair <int64_t, int64_t> link : m_undirected_edges) {

        // Install link
        NetDeviceContainer container = p2p.Install(m_nodes.Get(link.first), m_nodes.Get(link.second));
        container.Get(0)->GetObject<PointToPointNetDevice>()->GetQueue()->SetMaxSize(p2p_net_device_max_queue_size_pkts_str);
        container.Get(1)->GetObject<PointToPointNetDevice>()->GetQueue()->SetMaxSize(p2p_net_device_max_queue_size_pkts_str);

        // Install traffic control
        if (IsValidEndpoint(link.first)) {
            if (!m_disable_qdisc_endpoint_tors_xor_servers) {
                tch_endpoints.Install(container.Get(0));
            }
        } else {
            if (!m_disable_qdisc_non_endpoint_switches) {
                tch_not_endpoints.Install(container.Get(0));
            }
        }
        if (IsValidEndpoint(link.second)) {
            if (!m_disable_qdisc_endpoint_tors_xor_servers) {
                tch_endpoints.Install(container.Get(1));
            }
        } else {
            if (!m_disable_qdisc_non_endpoint_switches) {
                tch_not_endpoints.Install(container.Get(1));
            }
        }

        // Assign IP addresses
        address.Assign(container); // This will install the default traffic control layer if not already done above
        address.NewNetwork();

        // Remove the (default) traffic control layer if undesired
        TrafficControlHelper tch_uninstaller;
        if (IsValidEndpoint(link.first)) {
            if (m_disable_qdisc_endpoint_tors_xor_servers) {
                tch_uninstaller.Uninstall(container.Get(0));
            }
        } else {
            if (m_disable_qdisc_non_endpoint_switches) {
                tch_uninstaller.Uninstall(container.Get(0));
            }
        }
        if (IsValidEndpoint(link.second)) {
            if (m_disable_qdisc_endpoint_tors_xor_servers) {
                tch_uninstaller.Uninstall(container.Get(1));
            }
        } else {
            if (m_disable_qdisc_non_endpoint_switches) {
                tch_uninstaller.Uninstall(container.Get(1));
            }
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
    if (m_has_zero_servers) {
        return m_switches_which_are_tors.find(node_id) != m_switches_which_are_tors.end();
    } else {
        return m_servers.find(node_id) != m_servers.end();
    }
}

const std::set<int64_t>& TopologyPtop::GetEndpoints() {
    if (m_has_zero_servers) {
        return m_switches_which_are_tors;
    } else {
        return m_servers;
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
