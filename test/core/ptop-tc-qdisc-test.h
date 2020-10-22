/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/traffic-control-layer.h"

////////////////////////////////////////////////////////////////////////////////////////

const std::string ptop_tc_qdisc_test_dir = ".tmp-ptop-tc-qdisc-test";

void prepare_ptop_tc_qdisc_test_config() {
    mkdir_if_not_exists(ptop_tc_qdisc_test_dir);
    std::ofstream config_file(ptop_tc_qdisc_test_dir + "/config_ns3.properties");
    config_file << "simulation_end_time_ns=10000000000" << std::endl;
    config_file << "simulation_seed=123456789" << std::endl;
    config_file << "ptop_tc_qdisc_filename=\"topology.properties.temp\"" << std::endl;
    config_file.close();
}

void cleanup_ptop_tc_qdisc_test() {
    remove_file_if_exists(ptop_tc_qdisc_test_dir + "/config_ns3.properties");
    remove_file_if_exists(ptop_tc_qdisc_test_dir + "/topology.properties.temp");
    remove_file_if_exists(ptop_tc_qdisc_test_dir + "/logs_ns3/finished.txt");
    remove_file_if_exists(ptop_tc_qdisc_test_dir + "/logs_ns3/timing_results.txt");
    remove_file_if_exists(ptop_tc_qdisc_test_dir + "/logs_ns3/timing_results.csv");
    remove_dir_if_exists(ptop_tc_qdisc_test_dir + "/logs_ns3");
    remove_dir_if_exists(ptop_tc_qdisc_test_dir);
}

////////////////////////////////////////////////////////////////////////////////////////

class PtopTcQdiscValidTestCase : public TestCase
{
public:
    PtopTcQdiscValidTestCase () : TestCase ("ptop-tc-qdisc valid") {};
    void DoRun () {
        prepare_ptop_tc_qdisc_test_config();
        
        std::ofstream topology_file;
        topology_file.open (ptop_tc_qdisc_test_dir + "/topology.properties.temp");
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
        topology_file << "link_net_device_receive_error_model=map(0->4: iid_uniform_random_pkt(0.0), 1->4: iid_uniform_random_pkt(0.1), 2->4: iid_uniform_random_pkt(0.2), 3->4: iid_uniform_random_pkt(0.3), 5->4:iid_uniform_random_pkt(0.5), 6->4:iid_uniform_random_pkt(0.6), 7->4:iid_uniform_random_pkt(0.7), 4->0: iid_uniform_random_pkt(0.4), 4->1: none, 4->2: iid_uniform_random_pkt(0.4), 4->3: none, 4->5: none, 4->6: iid_uniform_random_pkt(0.400000), 4->7: iid_uniform_random_pkt(1.0))" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=map(0->4: default, 1->4: fq_codel_better_rtt, 2->4: disabled, 3->4: default, 5->4:disabled, 6->4:default, 7->4:fq_codel_better_rtt, 4->0: disabled, 4->1: fq_codel_better_rtt, 4->2: default, 4->3: disabled, 4->5: default, 4->6: disabled, 4->7: fq_codel_better_rtt)" << std::endl;
        topology_file.close();
        
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(ptop_tc_qdisc_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // And now we are going to go test all the network devices installed and their channels in-between
        for (const std::pair<int64_t, int64_t>& edge : topology->GetUndirectedEdges()) {
            Ptr<PointToPointNetDevice> deviceAtoB = topology->GetNetDeviceForLink(edge);
            Ptr<PointToPointNetDevice> deviceBtoA = topology->GetNetDeviceForLink(std::make_pair(edge.second, edge.first));
            std::vector<std::pair<std::pair<int64_t, int64_t>, Ptr<PointToPointNetDevice>>> links_with_devices;
            links_with_devices.push_back(std::make_pair(edge, deviceAtoB));
            links_with_devices.push_back(std::make_pair(std::make_pair(edge.second, edge.first), deviceBtoA));
            for (std::pair<std::pair<int64_t, int64_t>, Ptr<PointToPointNetDevice>> link_and_device : links_with_devices) {

                // Under investigation
                std::pair<int64_t, int64_t> link = link_and_device.first;
                Ptr<PointToPointNetDevice> device = link_and_device.second;

                // Traffic control queueing discipline (based on formula (a * 2 + b * 7) % 3 = {0, 1, 2} = {fq_codel_better_rtt, default, disabled)
                Ptr<QueueDisc> queueDisc = topology->GetNodes().Get(link.first)->GetObject<TrafficControlLayer>()->GetRootQueueDiscOnDevice(device);
                if ((link.first * 2 + link.second * 7) % 3 == 0) {
                    // fq_codel_better_rtt
                    Ptr<FqCoDelQueueDisc> realDisc = queueDisc->GetObject<FqCoDelQueueDisc>();
                    ASSERT_NOT_EQUAL(realDisc, 0);

                    // Improved interval (= RTT estimate)
                    StringValue interval_att;
                    realDisc->GetAttribute ("Interval", interval_att);
                    ASSERT_EQUAL(interval_att.Get(), std::to_string(topology->GetWorstCaseRttEstimateNs()) + "ns");

                    // Improved target (= RTT estimate / 20)
                    StringValue target_att;
                    realDisc->GetAttribute ("Target", target_att);
                    ASSERT_EQUAL(target_att.Get(), std::to_string(topology->GetWorstCaseRttEstimateNs() / 20) + "ns");

                } else if ((link.first * 2 + link.second * 7) % 3 == 1) {
                    // default (currently, fq codel is default)
                    ASSERT_NOT_EQUAL(queueDisc->GetObject<FqCoDelQueueDisc>(), 0);

                } else {
                    // disabled
                    ASSERT_EQUAL(queueDisc, 0);
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
        cleanup_ptop_tc_qdisc_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PtopTcQdiscInvalidTestCase : public TestCase
{
public:
    PtopTcQdiscInvalidTestCase () : TestCase ("ptop-tc-qdisc invalid") {};
    void DoRun () {
        
        std::ofstream topology_file;
        Ptr<BasicSimulation> basicSimulation;

        // Invalid traffic control queue disc
        prepare_ptop_tc_qdisc_test_config();
        basicSimulation = CreateObject<BasicSimulation>(ptop_tc_qdisc_test_dir);
        topology_file.open (ptop_tc_qdisc_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=some_invalid_qdisc_value_which_does_not_exist" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_ptop_tc_qdisc_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////
