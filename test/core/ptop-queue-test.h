/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/traffic-control-layer.h"

////////////////////////////////////////////////////////////////////////////////////////

class PtopQueueTestCase : public TestCaseWithLogValidators
{
public:
    PtopQueueTestCase (std::string s) : TestCaseWithLogValidators (s) {};
    std::string test_run_dir = ".tmp-test-ptop-queue";

    void prepare_ptop_queue_test_config() {
        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=10000000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file.close();
    }

    void cleanup_ptop_queue_test() {
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);
    }

};

////////////////////////////////////////////////////////////////////////////////////////

class PtopQueueValidTestCase : public PtopQueueTestCase
{
public:
    PtopQueueValidTestCase () : PtopQueueTestCase ("ptop-queue valid") {};
    void DoRun () {
        test_run_dir = ".tmp-test-ptop-queue-valid";
        prepare_clean_run_dir(test_run_dir);
        prepare_ptop_queue_test_config();
        
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=8" << std::endl;
        topology_file << "num_undirected_edges=7" << std::endl;
        topology_file << "switches=set(4)" << std::endl;
        topology_file << "switches_which_are_tors=set(4)" << std::endl;
        topology_file << "servers=set(0,1,2,3,5,6,7)" << std::endl;
        topology_file << "undirected_edges=set(0-4,1-4,2-4,3-4,4-5,4-6,4-7)" << std::endl;
        topology_file << "all_nodes_are_endpoints=false" << std::endl;
        topology_file << "link_channel_delay_ns=map(0-4: 400,1-4: 500,2-4: 600,3-4: 700,4-5: 900,4-6: 10000,4-7: 11000)" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=map(0->4: 2.8,1->4: 3.1,2->4: 3.4,3->4: 3.7,4->5: 4.7,4->6: 5.4,4->7: 6.1,4->0: 1.2,4->1: 1.9,4->2: 2.6,4->3: 3.3,5->4: 4.3,6->4: 4.6,7->4: 4.9)" << std::endl;
        topology_file << "link_net_device_queue=map(0->4: drop_tail(4p),1->4: drop_tail(4B),2->4: drop_tail(4p),3->4: drop_tail(4B),4->5: drop_tail(5p),4->6: drop_tail(6p),4->7: drop_tail(7p),4->0: drop_tail(77p),4->1: drop_tail(1p),4->2: drop_tail(2p),4->3: drop_tail(3p),5->4: drop_tail(4B),6->4: drop_tail(4p),7->4: drop_tail(4B))" << std::endl;
        topology_file << "link_net_device_receive_error_model=iid_uniform_random_pkt(0.2)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=fq_codel(300000; 1000000; 10240p)" << std::endl;
        topology_file.close();
        
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // And now we are going to go test all the network devices installed and their channels in-between
        for (const std::pair<int64_t, int64_t>& edge : topology->GetUndirectedEdges()) {
            Ptr<PointToPointNetDevice> deviceAtoB = topology->GetSendingNetDeviceForLink(edge);
            Ptr<PointToPointNetDevice> deviceBtoA = topology->GetSendingNetDeviceForLink(std::make_pair(edge.second, edge.first));
            std::vector<std::pair<std::pair<int64_t, int64_t>, Ptr<PointToPointNetDevice>>> links_with_devices;
            links_with_devices.push_back(std::make_pair(edge, deviceAtoB));
            links_with_devices.push_back(std::make_pair(std::make_pair(edge.second, edge.first), deviceBtoA));
            for (std::pair<std::pair<int64_t, int64_t>, Ptr<PointToPointNetDevice>> link_and_device : links_with_devices) {

                // Under investigation
                std::pair<int64_t, int64_t> link = link_and_device.first;
                Ptr<PointToPointNetDevice> device = link_and_device.second;

                // Queue
                // In this test, the even queue have packets as max size, the uneven have bytes
                // The max size is 77 if the second node identifier is 0, else the second node identifier
                PointerValue ptr;
                device->GetAttribute ("TxQueue", ptr);
                Ptr<Queue<Packet>> txQueue = ptr.Get<Queue<Packet>>();
                Ptr<QueueBase> abcQueue = txQueue->GetObject<QueueBase>();
                if (link.first % 2 == 0) {
                    ASSERT_EQUAL(abcQueue->GetMaxSize().GetUnit(), PACKETS);
                    NS_TEST_ASSERT_MSG_EQ(abcQueue->GetMaxSize().GetValue(), (link.second == 0 ? 77 : link.second), "For link: " + std::to_string(link.first) + " -> " + std::to_string(link.second));
                } else {
                    ASSERT_EQUAL(abcQueue->GetMaxSize().GetUnit(), BYTES);
                    ASSERT_EQUAL(abcQueue->GetMaxSize().GetValue(), (link.second == 0 ? 77 : link.second));
                }

                // Check if the node identifiers here and on the other side match up
                ASSERT_EQUAL(device->GetNode()->GetId(), link.first);
                int64_t node_id_one = device->GetChannel()->GetObject<PointToPointChannel>()->GetDevice(0)->GetNode()->GetId();
                int64_t node_id_two = device->GetChannel()->GetObject<PointToPointChannel>()->GetDevice(1)->GetNode()->GetId();
                ASSERT_EQUAL(device->GetChannel()->GetObject<PointToPointChannel>()->GetNDevices(), 2);
                ASSERT_TRUE((node_id_one == link.first && node_id_two == link.second) || (node_id_one == link.second && node_id_two == link.first));

            }

        }

        basicSimulation->Finalize();
        cleanup_ptop_queue_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PtopQueueInvalidTestCase : public PtopQueueTestCase
{
public:
    PtopQueueInvalidTestCase () : PtopQueueTestCase ("ptop-queue invalid") {};

    void DoRun () {
        test_run_dir = ".tmp-test-ptop-queue-invalid";
        prepare_clean_run_dir(test_run_dir);
        prepare_ptop_queue_test_config();

        // Invalid queue type
        std::ofstream topology_file;
        Ptr<BasicSimulation> basicSimulation;
        basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_net_device_queue=non_existent_queue(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_ptop_queue_test();

        // Invalid drop tail queue size
        prepare_clean_run_dir(test_run_dir);
        prepare_ptop_queue_test_config();
        basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(-8p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_ptop_queue_test();

        // Invalid drop tail queue size
        prepare_clean_run_dir(test_run_dir);
        prepare_ptop_queue_test_config();
        basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100abc)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_ptop_queue_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////
