/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/test.h"
#include "ns3/topology-ptop.h"
#include "../test-helpers.h"
#include "ns3/ipv4-arbiter-routing-helper.h"

using namespace ns3;

const std::string topology_ptop_test_dir = ".tmp-topology-ptop-test";

void prepare_topology_ptop_test_config() {
    mkdir_if_not_exists(topology_ptop_test_dir);

    std::ofstream config_file(topology_ptop_test_dir + "/config_ns3.properties");
    config_file << "simulation_end_time_ns=10000000000" << std::endl;
    config_file << "simulation_seed=123456789" << std::endl;
    config_file << "topology_filename=\"topology.properties.temp\"" << std::endl;
    config_file.close();
}

void cleanup_topology_ptop_test() {
    remove_file_if_exists(topology_ptop_test_dir + "/config_ns3.properties");
    remove_file_if_exists(topology_ptop_test_dir + "/topology.properties.temp");
    remove_file_if_exists(topology_ptop_test_dir + "/logs_ns3/finished.txt");
    remove_file_if_exists(topology_ptop_test_dir + "/logs_ns3/timing_results.txt");
    remove_dir_if_exists(topology_ptop_test_dir + "/logs_ns3");
    remove_dir_if_exists(topology_ptop_test_dir);
}

////////////////////////////////////////////////////////////////////////////////////////

class TopologyEmptyTestCase : public TestCase
{
public:
    TopologyEmptyTestCase () : TestCase ("topology empty") {};
    void DoRun () {
        prepare_topology_ptop_test_config();
        
        std::ofstream topology_file;
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=0" << std::endl;
        topology_file << "num_undirected_edges=0" << std::endl;
        topology_file << "switches=set()" << std::endl;
        topology_file << "switches_which_are_tors=set()" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set()" << std::endl;
        topology_file << "all_nodes_are_endpoints=true" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_max_queue_size=100p" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        
        ASSERT_EQUAL(topology->GetNumNodes(), 0);
        ASSERT_EQUAL(topology->GetNumUndirectedEdges(), 0);
        ASSERT_EQUAL(topology->GetSwitches().size(), 0);
        ASSERT_EQUAL(topology->GetSwitchesWhichAreTors().size(), 0);
        ASSERT_EQUAL(topology->GetServers().size(), 0);
        ASSERT_EQUAL(topology->GetUndirectedEdges().size(), 0);
        ASSERT_EQUAL(topology->GetUndirectedEdgesSet().size(), 0);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists().size(), 0);
        ASSERT_EQUAL(topology->GetEndpoints().size(), 0);
        
        basicSimulation->Finalize();
        cleanup_topology_ptop_test();
    }
};

class TopologySingleTestCase : public TestCase
{
public:
    TopologySingleTestCase () : TestCase ("topology single") {};
    void DoRun () {
        prepare_topology_ptop_test_config();

        std::ofstream topology_file;
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=2" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1)" << std::endl;
        topology_file << "all_nodes_are_endpoints=false" << std::endl;
        topology_file << "link_channel_delay_ns=map(0-1: 10000)" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=map(0->1: 100, 1->0: 100)" << std::endl;
        topology_file << "link_device_max_queue_size=map(0->1: 100p, 1->0: 100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=map(0->1: disabled, 1->0: disabled)" << std::endl;
        topology_file.close();
        
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Basic sizes
        ASSERT_EQUAL(topology->GetNumNodes(), 2);
        ASSERT_EQUAL(topology->GetNumUndirectedEdges(), 1);
        ASSERT_EQUAL(topology->GetSwitches().size(), 2);
        ASSERT_EQUAL(topology->GetSwitchesWhichAreTors().size(), 2);
        ASSERT_EQUAL(topology->GetServers().size(), 0);
        ASSERT_EQUAL(topology->GetUndirectedEdges().size(), 1);
        ASSERT_EQUAL(topology->GetUndirectedEdgesSet().size(), 1);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists().size(), 2);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists()[0].size(), 1);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists()[1].size(), 1);
        ASSERT_EQUAL(topology->GetEndpoints().size(), 2);

        // Check contents
        ASSERT_TRUE(topology->IsValidEndpoint(0));
        ASSERT_TRUE(topology->IsValidEndpoint(1));
        ASSERT_TRUE(set_int64_contains(topology->GetSwitches(), 0));
        ASSERT_TRUE(set_int64_contains(topology->GetSwitches(), 1));
        ASSERT_TRUE(set_int64_contains(topology->GetSwitchesWhichAreTors(), 0));
        ASSERT_TRUE(set_int64_contains(topology->GetSwitchesWhichAreTors(), 1));
        ASSERT_EQUAL(topology->GetUndirectedEdges()[0].first, 0);
        ASSERT_EQUAL(topology->GetUndirectedEdges()[0].second, 1);
        ASSERT_TRUE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(0, 1)));
        ASSERT_TRUE(set_int64_contains(topology->GetAllAdjacencyLists()[0], 1));
        ASSERT_TRUE(set_int64_contains(topology->GetAllAdjacencyLists()[1], 0));
        std::set<int64_t> endpoints = topology->GetEndpoints();
        ASSERT_TRUE(set_int64_contains(endpoints, 0));
        ASSERT_TRUE(set_int64_contains(endpoints, 1));

        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

    }
};

class TopologyTorTestCase : public TestCase
{
public:
    TopologyTorTestCase () : TestCase ("topology tor") {};
    void DoRun () {
        prepare_topology_ptop_test_config();
        
        std::ofstream topology_file;
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=8" << std::endl;
        topology_file << "num_undirected_edges=7" << std::endl;
        topology_file << "switches=set(4)" << std::endl;
        topology_file << "switches_which_are_tors=set(4)" << std::endl;
        topology_file << "servers=set(0,1,2,3,5,6,7)" << std::endl;
        topology_file << "undirected_edges=set(0-4,1-4,2-4,3-4,4-5,4-6,4-7)" << std::endl;
        topology_file << "all_nodes_are_endpoints=false" << std::endl;
        topology_file << "link_channel_delay_ns=map(0-4: 10000,1-4: 10000,2-4: 10000,3-4: 10000,4-5: 10000,4-6: 7000,4-7: 10000)" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=50" << std::endl;
        topology_file << "link_device_max_queue_size=100000B" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=map(0->4: default,1->4: default,2->4: default,3->4: default,4->5: default,4->6: default,4->7: default,4->0: default,4->1: default,4->2: disabled,4->3: disabled,5->4: disabled,6->4: disabled,7->4: disabled)" << std::endl;
        topology_file.close();
        
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Basic sizes
        ASSERT_EQUAL(topology->GetNumNodes(), 8);
        ASSERT_EQUAL(topology->GetNumUndirectedEdges(), 7);
        ASSERT_EQUAL(topology->GetSwitches().size(), 1);
        ASSERT_EQUAL(topology->GetSwitchesWhichAreTors().size(), 1);
        ASSERT_EQUAL(topology->GetServers().size(), 7);
        ASSERT_EQUAL(topology->GetUndirectedEdges().size(), 7);
        ASSERT_EQUAL(topology->GetUndirectedEdgesSet().size(), 7);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists().size(), 8);
        std::set<int64_t> endpoints = topology->GetEndpoints();
        ASSERT_EQUAL(endpoints.size(), 7);

        // Check contents
        for (int i = 0; i < 8; i++) {
            if (i == 4) {
                ASSERT_EQUAL(topology->GetAllAdjacencyLists()[i].size(), 7);
                ASSERT_FALSE(topology->IsValidEndpoint(i));
                ASSERT_TRUE(set_int64_contains(topology->GetSwitches(), i));
                ASSERT_TRUE(set_int64_contains(topology->GetSwitchesWhichAreTors(), i));
                ASSERT_FALSE(set_int64_contains(topology->GetServers(), i));
                ASSERT_FALSE(set_int64_contains(endpoints, i));
                for (int j = 0; j < 8; j++) {
                    if (j != 4) {
                        ASSERT_TRUE(set_int64_contains(topology->GetAllAdjacencyLists()[i], j));
                    }
                }
            } else {
                ASSERT_EQUAL(topology->GetAllAdjacencyLists()[i].size(), 1);
                ASSERT_TRUE(topology->IsValidEndpoint(i));
                ASSERT_FALSE(set_int64_contains(topology->GetSwitches(), i));
                ASSERT_FALSE(set_int64_contains(topology->GetSwitchesWhichAreTors(), i));
                ASSERT_TRUE(set_int64_contains(topology->GetServers(), i));
                ASSERT_TRUE(set_int64_contains(endpoints, i));
                ASSERT_TRUE(set_int64_contains(topology->GetAllAdjacencyLists()[i], 4));
                int a = i > 4 ? 4 : i;
                int b = i > 4 ? i : 4;
                ASSERT_TRUE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(a, b)));
            }
        }

        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

    }
};

class TopologyLeafSpineTestCase : public TestCase
{
public:
    TopologyLeafSpineTestCase () : TestCase ("topology leaf-spine") {};
    void DoRun () {
        prepare_topology_ptop_test_config();

        std::ofstream topology_file;
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=49" << std::endl;
        topology_file << "num_undirected_edges=72" << std::endl;
        topology_file << "switches=set(0,1,2,3,4,5,6,7,8,9,10,11,12)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1,2,3,4,5,6,7,8)" << std::endl;
        topology_file << "servers=set(13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48)" << std::endl;
        topology_file << "undirected_edges=set(0-9,0-10,0-11,0-12,0-13,0-14,0-15,0-16,1-9,1-10,1-11,1-12,1-17,1-18,1-19,1-20,2-9,2-10,2-11,2-12,2-21,2-22,2-23,2-24,3-9,3-10,3-11,3-12,3-25,3-26,3-27,3-28,4-9,4-10,4-11,4-12,4-29,4-30,4-31,4-32,5-9,5-10,5-11,5-12,5-33,5-34,5-35,5-36,6-9,6-10,6-11,6-12,6-37,6-38,6-39,6-40,7-9,7-10,7-11,7-12,7-41,7-42,7-43,7-44,8-9,8-10,8-11,8-12,8-45,8-46,8-47,8-48)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_max_queue_size=100p" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Basic values
        uint32_t num_spines = 4;
        uint32_t num_leafs = 9;
        uint32_t num_servers = num_spines * num_leafs;

        // Basic sizes
        ASSERT_EQUAL(topology->GetNumNodes(), num_spines + num_leafs + num_servers);
        ASSERT_EQUAL(topology->GetNumUndirectedEdges(), num_spines * num_leafs + num_servers);
        ASSERT_EQUAL(topology->GetSwitches().size(), num_spines + num_leafs);
        ASSERT_EQUAL(topology->GetSwitchesWhichAreTors().size(), num_leafs);
        ASSERT_EQUAL(topology->GetServers().size(), num_servers);
        ASSERT_EQUAL(topology->GetUndirectedEdges().size(), num_spines * num_leafs + num_servers);
        ASSERT_EQUAL(topology->GetUndirectedEdgesSet().size(), num_spines * num_leafs + num_servers);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists().size(), num_spines + num_leafs + num_servers);

        // Leafs
        for (int i = 0; i < 9; i++) {
            ASSERT_EQUAL(topology->GetAllAdjacencyLists()[i].size(), 8);
            ASSERT_FALSE(topology->IsValidEndpoint(i));
            ASSERT_TRUE(set_int64_contains(topology->GetSwitches(), i));
            ASSERT_TRUE(set_int64_contains(topology->GetSwitchesWhichAreTors(), i));
            ASSERT_FALSE(set_int64_contains(topology->GetServers(), i));

            // Links to the spines
            for (int j = 9; j < 13; j++) {
                ASSERT_TRUE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(i, j)));
            }

            // Links to the servers
            for (int j = 13 + i * 4; j < 13 + (i + 1) * 4; j++) {
                ASSERT_TRUE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(i, j)));
            }

        }

        // Spines
        for (int i = 9; i < 13; i++) {
            ASSERT_EQUAL(topology->GetAllAdjacencyLists()[i].size(), 9);
            ASSERT_FALSE(topology->IsValidEndpoint(i));
            ASSERT_TRUE(set_int64_contains(topology->GetSwitches(), i));
            ASSERT_FALSE(set_int64_contains(topology->GetSwitchesWhichAreTors(), i));
            ASSERT_FALSE(set_int64_contains(topology->GetServers(), i));
        }

        // Servers
        for (int i = 13; i < 49; i++) {
            ASSERT_EQUAL(topology->GetAllAdjacencyLists()[i].size(), 1);
            ASSERT_TRUE(topology->IsValidEndpoint(i));
            ASSERT_FALSE(set_int64_contains(topology->GetSwitches(), i));
            ASSERT_FALSE(set_int64_contains(topology->GetSwitchesWhichAreTors(), i));
            ASSERT_TRUE(set_int64_contains(topology->GetServers(), i));
            int tor = (i - 13) / 4;
            ASSERT_TRUE(set_int64_contains(topology->GetAllAdjacencyLists()[i], tor));
            ASSERT_TRUE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(tor, i)));
        }

        basicSimulation->Finalize();
        cleanup_topology_ptop_test();

    }
};

class TopologyRingTestCase : public TestCase
{
public:
    TopologyRingTestCase () : TestCase ("topology ring") {};
    void DoRun () {
        prepare_topology_ptop_test_config();

        std::ofstream topology_file;
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-3,0-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=100000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=0.1" << std::endl;
        topology_file << "link_device_max_queue_size=1p" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=default" << std::endl;
        topology_file.close();
        
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Basic sizes
        ASSERT_EQUAL(topology->GetNumNodes(), 4);
        ASSERT_EQUAL(topology->GetNumUndirectedEdges(), 4);
        ASSERT_EQUAL(topology->GetSwitches().size(), 4);
        ASSERT_EQUAL(topology->GetSwitchesWhichAreTors().size(), 2);
        ASSERT_EQUAL(topology->GetServers().size(), 0);
        ASSERT_EQUAL(topology->GetUndirectedEdges().size(), 4);
        ASSERT_EQUAL(topology->GetUndirectedEdgesSet().size(), 4);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists().size(), 4);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists()[0].size(), 2);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists()[1].size(), 2);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists()[2].size(), 2);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists()[3].size(), 2);

        // Check contents
        ASSERT_TRUE(topology->IsValidEndpoint(0));
        ASSERT_FALSE(topology->IsValidEndpoint(1));
        ASSERT_FALSE(topology->IsValidEndpoint(2));
        ASSERT_TRUE(topology->IsValidEndpoint(3));
        for (int i = 0; i < 4; i++) {
            ASSERT_EQUAL(topology->GetAllAdjacencyLists()[i].size(), 2);
            ASSERT_TRUE(set_int64_contains(topology->GetSwitches(), i));
            ASSERT_FALSE(set_int64_contains(topology->GetServers(), i));
            if (i == 0 || i == 3) {
                ASSERT_TRUE(set_int64_contains(topology->GetSwitchesWhichAreTors(), i));
                ASSERT_TRUE(set_int64_contains(topology->GetAllAdjacencyLists()[i], 1));
                ASSERT_TRUE(set_int64_contains(topology->GetAllAdjacencyLists()[i], 2));
            } else {
                ASSERT_FALSE(set_int64_contains(topology->GetSwitchesWhichAreTors(), i));
                ASSERT_TRUE(set_int64_contains(topology->GetAllAdjacencyLists()[i], 0));
                ASSERT_TRUE(set_int64_contains(topology->GetAllAdjacencyLists()[i], 3));
            }
        }
        ASSERT_TRUE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(0, 1)));
        ASSERT_TRUE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(0, 2)));
        ASSERT_TRUE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(1, 3)));
        ASSERT_TRUE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(2, 3)));
        ASSERT_FALSE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(1, 0)));
        ASSERT_FALSE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(2, 0)));
        ASSERT_FALSE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(3, 1)));
        ASSERT_FALSE(set_pair_int64_contains(topology->GetUndirectedEdgesSet(), std::make_pair<int64_t, int64_t>(3, 2)));

        basicSimulation->Finalize();
        cleanup_topology_ptop_test();
    }
};

class TopologyInvalidTestCase : public TestCase
{
public:
    TopologyInvalidTestCase () : TestCase ("topology invalid") {};
    void DoRun () {
        prepare_topology_ptop_test_config();
        
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(topology_ptop_test_dir);
        
        std::ofstream topology_file;

        // Incorrect number of nodes
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=1" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(1-0)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_max_queue_size=100p" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        cleanup_topology_ptop_test();

        // Incorrect number of edges
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=3" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1,2)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(1-0,1-2)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_max_queue_size=100p" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        cleanup_topology_ptop_test();

        // Non-existent node id
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=2" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1,2)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(1-0)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_max_queue_size=100p" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        cleanup_topology_ptop_test();

        // Not all node ids are covered
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=3" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(1-0)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_max_queue_size=100p" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        cleanup_topology_ptop_test();

        // Duplicate edge
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=2" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(1-0,0-1)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_max_queue_size=100p" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        cleanup_topology_ptop_test();

        // Duplicate node id
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=2" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(1-0)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_max_queue_size=100p" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        cleanup_topology_ptop_test();

        // Edge between server and non-ToR switch
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(1)" << std::endl;
        topology_file << "servers=set(2,3)" << std::endl;
        topology_file << "undirected_edges=set(1-0,2-1,3-0)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_max_queue_size=100p" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        cleanup_topology_ptop_test();

        // Edge between server and server
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(1)" << std::endl;
        topology_file << "servers=set(2,3)" << std::endl;
        topology_file << "undirected_edges=set(1-0,2-1,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_max_queue_size=100p" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        cleanup_topology_ptop_test();

        // Server marked as ToR
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(1,2)" << std::endl;
        topology_file << "switches_which_are_tors=set(1,2,3)" << std::endl;
        topology_file << "servers=set(0,3)" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_max_queue_size=100p" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        cleanup_topology_ptop_test();

        // Servers and switches not distinct
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(1,2)" << std::endl;
        topology_file << "switches_which_are_tors=set(1,2)" << std::endl;
        topology_file << "servers=set(0,3,1)" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_max_queue_size=100p" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        cleanup_topology_ptop_test();

        // Edge to itself
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(1,2)" << std::endl;
        topology_file << "switches_which_are_tors=set(1,2)" << std::endl;
        topology_file << "servers=set(0,3)" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-2)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_max_queue_size=100p" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        cleanup_topology_ptop_test();

        // Edge left out of bound
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(1,2)" << std::endl;
        topology_file << "switches_which_are_tors=set(1,2)" << std::endl;
        topology_file << "servers=set(0,3)" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-4)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_max_queue_size=100p" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        cleanup_topology_ptop_test();

        // Edge right out of bound
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(1,2)" << std::endl;
        topology_file << "switches_which_are_tors=set(1,2)" << std::endl;
        topology_file << "servers=set(0,3)" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,4-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_max_queue_size=100p" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        cleanup_topology_ptop_test();

        // Duplicate edges
        topology_file.open (topology_ptop_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(1,2)" << std::endl;
        topology_file << "switches_which_are_tors=set(1,2)" << std::endl;
        topology_file << "servers=set(0,3)" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3,2 -3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_max_queue_size=100p" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        cleanup_topology_ptop_test();

        basicSimulation->Finalize();

    }
};

////////////////////////////////////////////////////////////////////////////////////////
