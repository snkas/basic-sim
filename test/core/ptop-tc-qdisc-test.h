/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/traffic-control-layer.h"

////////////////////////////////////////////////////////////////////////////////////////

class PtopTcQdiscTestCase : public TestCaseWithLogValidators
{
public:
    PtopTcQdiscTestCase (std::string s) : TestCaseWithLogValidators (s) {};
    std::string test_run_dir = ".tmp-test-ptop-tc-qdisc";

    void prepare_ptop_tc_qdisc_test_config() {
        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=10000000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file.close();
    }

    void cleanup_ptop_tc_qdisc_test() {
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

class PtopTcQdiscRedValidTestCase : public PtopTcQdiscTestCase
{
public:
    PtopTcQdiscRedValidTestCase () : PtopTcQdiscTestCase ("ptop-tc-qdisc-red valid") {};
    void DoRun () {
        test_run_dir = ".tmp-test-ptop-tc-qdisc-red-valid";
        prepare_clean_run_dir(test_run_dir);
        prepare_ptop_tc_qdisc_test_config();

        // Desired qdisc configurations
        std::vector<std::tuple<std::string, int64_t, int64_t, bool, int64_t, double, int64_t, int64_t, int64_t, bool, double, bool, bool>> desired_configs;
        desired_configs.push_back(std::make_tuple("disabled", 0, 2, true, 1, 1.0, 0, 0, 0, true, 0.0, true, true));
        desired_configs.push_back(std::make_tuple("simple_red(drop; 2; 1.0; 1; 2; 3p; 0.5; wait; not_gentle)", 2, 0, false, 2, 1.0, 1, 2, 3, true, 0.5, true, false));
        desired_configs.push_back(std::make_tuple("simple_red(ecn; 10000; 0.5; 0; 100; 100p; 0.1; no_wait; not_gentle)", 2, 3, true, 10000, 0.5, 0, 100, 100, true, 0.1, false, false));
        desired_configs.push_back(std::make_tuple("simple_red(ecn; 777; 0.001; 10; 10; 20p; 1.0; wait; gentle)", 3, 2, true, 777, 0.001, 10, 10, 20, true, 1.0, true, true));
        desired_configs.push_back(std::make_tuple("simple_red(drop; 1; 0.4; 10; 10; 10p; 0.9; no_wait; not_gentle)", 1, 2, false, 1, 0.4, 10, 10, 10, true, 0.9, false, false));
        desired_configs.push_back(std::make_tuple("simple_red(ecn; 1500; 0.9; 0; 0; 3p; 0.01; wait; gentle)", 2, 1, true, 1500, 0.9, 0, 0, 3, true, 0.01, true, true));
        desired_configs.push_back(std::make_tuple("simple_red(ecn; 1500; 0.01; 10; 20; 50000B; 0.01; wait; gentle)", 2, 4, true, 1500, 0.01, 10, 20, 50000, false, 0.01, true, true));
        desired_configs.push_back(std::make_tuple("simple_red(drop; 1500; 0.02; 100; 200; 500p; 1.0; wait; gentle)", 4, 2, false, 1500, 0.02, 100, 200, 500, true, 1.0, true, true));

        // Create string for topology encoding
        std::string link_interface_traffic_control_qdisc_str = "map(";
        size_t i = 0;
        std::map<std::pair<int64_t, int64_t>, std::tuple<std::string, bool, int64_t, double, int64_t, int64_t, int64_t, bool, double, bool, bool>> pair_to_desired_config;
        for (std::tuple<std::string, int64_t, int64_t, bool, int64_t, double, int64_t, int64_t, int64_t, bool, double, bool, bool> c : desired_configs) {
            pair_to_desired_config.insert(std::make_pair(std::make_pair(std::get<1>(c), std::get<2>(c)), std::make_tuple(std::get<0>(c), std::get<3>(c), std::get<4>(c), std::get<5>(c), std::get<6>(c), std::get<7>(c), std::get<8>(c), std::get<9>(c), std::get<10>(c), std::get<11>(c), std::get<12>(c))));
            if (i != 0) {
                link_interface_traffic_control_qdisc_str += ",";
            }
            link_interface_traffic_control_qdisc_str += std::to_string(std::get<1>(c)) + "->" + std::to_string(std::get<2>(c)) + ": " + std::get<0>(c);
            i++;
        }
        link_interface_traffic_control_qdisc_str += ")";
        std::cout << link_interface_traffic_control_qdisc_str << std::endl;

        // Create mapping

        // Write topology
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=5" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(2)" << std::endl;
        topology_file << "switches_which_are_tors=set(2)" << std::endl;
        topology_file << "servers=set(0,1,3,4)" << std::endl;
        topology_file << "undirected_edges=set(0-2,1-2,2-3,2-4)" << std::endl;
        topology_file << "all_nodes_are_endpoints=true" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=10" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(60p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=" << link_interface_traffic_control_qdisc_str << std::endl;
        topology_file.close();

        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Basic sizes
        ASSERT_EQUAL(topology->GetNumNodes(), 5);
        ASSERT_EQUAL(topology->GetNumUndirectedEdges(), 4);
        ASSERT_EQUAL(topology->GetSwitches().size(), 1);
        ASSERT_EQUAL(topology->GetSwitchesWhichAreTors().size(), 1);
        ASSERT_EQUAL(topology->GetServers().size(), 4);
        ASSERT_EQUAL(topology->GetUndirectedEdges().size(), 4);
        ASSERT_EQUAL(topology->GetUndirectedEdgesSet().size(), 4);
        ASSERT_EQUAL(topology->GetAllAdjacencyLists().size(), 5);
        std::set<int64_t> endpoints = topology->GetEndpoints();
        ASSERT_EQUAL(endpoints.size(), 5);

        // Test all the queueing disciplines installed
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

                // Traffic control queueing discipline
                Ptr<QueueDisc> queueDisc = topology->GetNodes().Get(link.first)->GetObject<TrafficControlLayer>()->GetRootQueueDiscOnDevice(device);
                std::tuple<std::string, bool, int64_t, double, int64_t, int64_t, int64_t, bool, double, bool, bool> desired_config = pair_to_desired_config.at(link);
                if (std::get<0>(desired_config) != "disabled") {

                    // simple_red
                    Ptr<RedQueueDisc> realDisc = queueDisc->GetObject<RedQueueDisc>();
                    ASSERT_NOT_EQUAL(realDisc, 0);

                    // Whether to mark ECN
                    BooleanValue use_ecn_att;
                    realDisc->GetAttribute ("UseEcn", use_ecn_att);
                    ASSERT_EQUAL(use_ecn_att.Get(), std::get<1>(desired_config));

                    // Whether to drop
                    BooleanValue use_hard_drop_att;
                    realDisc->GetAttribute ("UseHardDrop", use_hard_drop_att);
                    ASSERT_EQUAL(use_hard_drop_att.Get(), !std::get<1>(desired_config));

                    // Mean packet size
                    UintegerValue mean_pkt_size_att;
                    realDisc->GetAttribute ("MeanPktSize", mean_pkt_size_att);
                    ASSERT_EQUAL(mean_pkt_size_att.Get(), (size_t) std::get<2>(desired_config));

                    // Queue weight for EWMA average queue size estimate
                    DoubleValue qw_att;
                    realDisc->GetAttribute ("QW", qw_att);
                    ASSERT_EQUAL(qw_att.Get(), std::get<3>(desired_config));

                    // RED minimum threshold (packets)
                    DoubleValue min_th_att;
                    realDisc->GetAttribute ("MinTh", min_th_att);
                    ASSERT_EQUAL(min_th_att.Get(), std::get<4>(desired_config));

                    // RED maximum threshold (packets)
                    DoubleValue max_th_att;
                    realDisc->GetAttribute ("MaxTh", max_th_att);
                    ASSERT_EQUAL(max_th_att.Get(), std::get<5>(desired_config));

                    // Maximum queue size (packets)
                    QueueSizeValue max_size_att;
                    realDisc->GetAttribute ("MaxSize", max_size_att);
                    QueueSize max_size_queue_size = max_size_att.Get();
                    ASSERT_EQUAL(max_size_queue_size.GetValue(), std::get<6>(desired_config));
                    ASSERT_EQUAL(max_size_queue_size.GetUnit(), std::get<7>(desired_config) ? QueueSizeUnit::PACKETS : QueueSizeUnit::BYTES);

                    // Max probability
                    DoubleValue l_interm_att;
                    realDisc->GetAttribute ("LInterm", l_interm_att);
                    ASSERT_EQUAL(l_interm_att.Get(), 1.0 / std::get<8>(desired_config));

                    // Gentle
                    BooleanValue wait_att;
                    realDisc->GetAttribute ("Wait", wait_att);
                    ASSERT_EQUAL(wait_att.Get(), std::get<9>(desired_config));

                    // Gentle
                    BooleanValue gentle_att;
                    realDisc->GetAttribute ("Gentle", gentle_att);
                    ASSERT_EQUAL(gentle_att.Get(), std::get<10>(desired_config));

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

class PtopTcQdiscFqCodelValidTestCase : public PtopTcQdiscTestCase
{
public:
    PtopTcQdiscFqCodelValidTestCase () : PtopTcQdiscTestCase ("ptop-tc-qdisc fq-codel-valid") {};
    void DoRun () {
        test_run_dir = ".tmp-test-ptop-tc-qdisc-fq-codel-valid";
        prepare_clean_run_dir(test_run_dir);
        prepare_ptop_tc_qdisc_test_config();
        
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
        topology_file << "link_net_device_receive_error_model=map(0->4: iid_uniform_random_pkt(0.0), 1->4: iid_uniform_random_pkt(0.1), 2->4: iid_uniform_random_pkt(0.2), 3->4: iid_uniform_random_pkt(0.3), 5->4:iid_uniform_random_pkt(0.5), 6->4:iid_uniform_random_pkt(0.6), 7->4:iid_uniform_random_pkt(0.7), 4->0: iid_uniform_random_pkt(0.4), 4->1: none, 4->2: iid_uniform_random_pkt(0.4), 4->3: none, 4->5: none, 4->6: iid_uniform_random_pkt(0.400000), 4->7: iid_uniform_random_pkt(1.0))" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=map(0->4: default, 1->4: fq_codel(1000; 400; 1400p), 2->4: disabled, 3->4: default, 5->4:disabled, 6->4:default, 7->4:fq_codel(7000; 400; 7400p), 4->0: disabled, 4->1: fq_codel(4000; 100; 4100p), 4->2: default, 4->3: disabled, 4->5: default, 4->6: disabled, 4->7: fq_codel(4000; 700; 4700p))" << std::endl;
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

                // Traffic control queueing discipline (based on formula (a * 2 + b * 7) % 3 = {0, 1, 2} = {fq_codel, default, disabled)
                Ptr<QueueDisc> queueDisc = topology->GetNodes().Get(link.first)->GetObject<TrafficControlLayer>()->GetRootQueueDiscOnDevice(device);
                if ((link.first * 2 + link.second * 7) % 3 == 0) {

                    // fq_codel
                    Ptr<FqCoDelQueueDisc> realDisc = queueDisc->GetObject<FqCoDelQueueDisc>();
                    ASSERT_NOT_EQUAL(realDisc, 0);

                    // Interval
                    StringValue interval_att;
                    realDisc->GetAttribute ("Interval", interval_att);
                    ASSERT_EQUAL(interval_att.Get(), std::to_string(link.first * 1000) + "ns");

                    // Target
                    StringValue target_att;
                    realDisc->GetAttribute ("Target", target_att);
                    ASSERT_EQUAL(target_att.Get(), std::to_string(link.second * 100) + "ns");

                    // Maximum queue size
                    QueueSizeValue max_size_att;
                    realDisc->GetAttribute ("MaxSize", max_size_att);
                    QueueSize max_size_queue_size = max_size_att.Get();
                    ASSERT_EQUAL(max_size_queue_size.GetValue(), link.first * 1000 + link.second * 100);
                    ASSERT_EQUAL(max_size_queue_size.GetUnit(), QueueSizeUnit::PACKETS);

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

class PtopTcQdiscInvalidTestCase : public PtopTcQdiscTestCase
{
public:
    PtopTcQdiscInvalidTestCase () : PtopTcQdiscTestCase ("ptop-tc-qdisc invalid") {};
    void DoRun () {

        std::ofstream topology_file;
        Ptr<BasicSimulation> basicSimulation;

        std::vector<std::pair<std::string, std::string>> qdisc_and_expected_what;
        qdisc_and_expected_what.push_back(std::make_pair("some_non_existent_qdisc(100)", "Invalid traffic control qdisc value: some_non_existent_qdisc(100)"));
        qdisc_and_expected_what.push_back(std::make_pair("fifo(100)", "Invalid maximum FIFO queue size value: 100"));
        qdisc_and_expected_what.push_back(std::make_pair("fifo(100b)", "Invalid maximum FIFO queue size value: 100b"));
        qdisc_and_expected_what.push_back(std::make_pair("fifo(100P)", "Invalid maximum FIFO queue size value: 100P"));
        qdisc_and_expected_what.push_back(std::make_pair("fifo(-1p)", "Negative int64 value not permitted: -1"));
        qdisc_and_expected_what.push_back(std::make_pair("pfifo_fast(100)", "Invalid maximum pfifo_fast queue size value: 100"));
        qdisc_and_expected_what.push_back(std::make_pair("pfifo_fast(100b)", "Invalid maximum pfifo_fast queue size value: 100b"));
        qdisc_and_expected_what.push_back(std::make_pair("pfifo_fast(100P)", "Invalid maximum pfifo_fast queue size value: 100P"));
        qdisc_and_expected_what.push_back(std::make_pair("pfifo_fast(-1p)", "Negative int64 value not permitted: -1"));
        qdisc_and_expected_what.push_back(std::make_pair("simple_red(abc; 1500; 1.0; 10; 20; 30p; 1.0; no_wait; gentle)", "Invalid RED action: abc"));
        qdisc_and_expected_what.push_back(std::make_pair("simple_red(ecn; 1500; 0.5; 10; 9; 30p; 0.9; no_wait; not_gentle)", "RED minimum threshold (10) cannot exceed maximum threshold (9)"));
        qdisc_and_expected_what.push_back(std::make_pair("simple_red(drop; 1500; 0.001; 10; 20; 19p; 0.2; no_wait; not_gentle)", "RED maximum threshold (20) cannot exceed maximum queue size (19)"));
        qdisc_and_expected_what.push_back(std::make_pair("simple_red(drop; 1500; 0.9; -4; 20; 30p; 0.7; no_wait; gentle)", "Negative int64 value not permitted: -4"));
        qdisc_and_expected_what.push_back(std::make_pair("simple_red(drop; 1500; 1.0; 10; 20; 300000B; 0.0; wait; not_gentle)", "Maximum probability must be in range (0, 1.0]: 0.0"));
        qdisc_and_expected_what.push_back(std::make_pair("simple_red(ecn; 1500; 0.1; 10; 20; 30p; 1.0001; no_wait; gentle)", "Maximum probability must be in range (0, 1.0]: 1.0001"));
        qdisc_and_expected_what.push_back(std::make_pair("simple_red(ecn; 1500; 0.0; 10; 20; 30p; 0.999; no_wait; not_gentle)", "Queue weight must be in range (0, 1.0]: 0.0"));
        qdisc_and_expected_what.push_back(std::make_pair("simple_red(drop; 1500; -0.3; 10; 20; 30p; 0.999; wait; gentle)", "Queue weight must be in range (0, 1.0]: -0.3"));
        qdisc_and_expected_what.push_back(std::make_pair("simple_red(drop; 1500; 1.1; 10; 20; 10000B; 0.999; wait; not_gentle)", "Queue weight must be in range (0, 1.0]: 1.1"));
        qdisc_and_expected_what.push_back(std::make_pair("simple_red(drop; 1500; 1.0; 10; 20; 30p; 1.0; no_wait; true)", "Invalid RED gentle: true"));
        qdisc_and_expected_what.push_back(std::make_pair("simple_red(drop; 1500; 1.0; 10; 20; 30p; 1.0; abcd; not_gentle)", "Invalid RED wait: abcd"));
        qdisc_and_expected_what.push_back(std::make_pair("simple_red(drop; 0; 1.0; 10; 20; 30p; 1.0; wait; not_gentle)", "Value must be >= 1: 0"));
        qdisc_and_expected_what.push_back(std::make_pair("simple_red(drop; -1; 1.0; 10; 20; 30p; 1.0; wait; not_gentle)", "Negative int64 value not permitted: -1"));
        qdisc_and_expected_what.push_back(std::make_pair("simple_red(drop; 1500; 0.6; 10; 20; 10000P; 0.999; wait; not_gentle)", "Invalid maximum RED queue size value: 10000P"));
        qdisc_and_expected_what.push_back(std::make_pair("fq_codel(10000; 1000; 1000000B)", "Invalid maximum fq_codel queue size value: 1000000B"));
        qdisc_and_expected_what.push_back(std::make_pair("fq_codel(20000; 1435; 1000P)", "Invalid maximum fq_codel queue size value: 1000P"));

        // Check each invalid case
        for (std::pair<std::string, std::string> scenario : qdisc_and_expected_what) {
            test_run_dir = ".tmp-test-ptop-tc-qdisc-invalid";
            prepare_clean_run_dir(test_run_dir);
            prepare_ptop_tc_qdisc_test_config();

            basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
            topology_file.open(test_run_dir + "/topology.properties");
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
            topology_file << "link_interface_traffic_control_qdisc=" << scenario.first << std::endl;
            topology_file.close();
            ASSERT_EXCEPTION_MATCH_WHAT(
                    CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()),
                    scenario.second
            );
            basicSimulation->Finalize();
            cleanup_ptop_tc_qdisc_test();
        }

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PtopTcQdiscRedDropMarkingTestCase : public TestCaseWithLogValidators
{
public:
    PtopTcQdiscRedDropMarkingTestCase () : TestCaseWithLogValidators ("ptop-tc-qdisc-red drop-marking") {};
    const std::string test_run_dir = ".tmp-test-ptop-tc-qdisc-red-drop-marking";

    void DoRun () {
        prepare_clean_run_dir(test_run_dir);

        // Write configuration
        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=1950000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_link_net_device_queue_tracking=true" << std::endl;
        config_file << "link_net_device_queue_tracking_enable_for_links=all" << std::endl;
        config_file << "enable_link_interface_tc_qdisc_queue_tracking=true" << std::endl;
        config_file << "link_interface_tc_qdisc_queue_tracking_enable_for_links=all" << std::endl;
        config_file << "enable_udp_burst_scheduler=true" << std::endl;
        config_file << "udp_burst_schedule_filename=\"udp_burst_schedule.csv\"" << std::endl;
        config_file.close();

        // Write topology
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=3" << std::endl;
        topology_file << "num_undirected_edges=2" << std::endl;
        topology_file << "switches=set(0)" << std::endl;
        topology_file << "switches_which_are_tors=set(0)" << std::endl;
        topology_file << "servers=set(1,2)" << std::endl;
        topology_file << "undirected_edges=set(0-1, 0-2)" << std::endl;
        topology_file << "all_nodes_are_endpoints=true" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=10" << std::endl;
        topology_file << "link_net_device_queue=map(0->1: drop_tail(1p), 1->0: drop_tail(100p), 0->2: drop_tail(100p), 2->0: drop_tail(1p))" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=map(0->1: simple_red(drop; 1500; 1.0; 100; 500; 4000p; 0.1; no_wait; gentle), 1->0: fifo(100p), 0->2: fifo(100p), 2->0: simple_red(drop; 1500; 1.0; 20; 60; 200p; 0.3; wait; gentle))" << std::endl;
        topology_file.close();

        // Write UDP burst file
        std::ofstream udp_burst_schedule_file;
        udp_burst_schedule_file.open (test_run_dir + "/udp_burst_schedule.csv");
        // 1000 Mbit/s for 2000000 ns = 167 packets
        // 1000 Mbit/s for 6000000 ns = 500 packets
        // 1000 Mbit/s for 12000000 ns = 1000 packets
        // 2000 Mbit/s for 12000000 ns = 2000 packets
        udp_burst_schedule_file << "0,0,1,1000,0,12000000,," << std::endl;
        udp_burst_schedule_file << "1,2,0,1000,0,2000000,," << std::endl;
        topology_file.close();

        // Create simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);

        // Create topology
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // Schedule UDP bursts
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology);

        // Install link net-device queue trackers
        PtopLinkNetDeviceQueueTracking netDeviceQueueTracking = PtopLinkNetDeviceQueueTracking(basicSimulation, topology); // Requires enable_link_net_device_queue_tracking=true

        // Install link interface traffic-control qdisc queue trackers
        PtopLinkInterfaceTcQdiscQueueTracking tcQdiscQueueTracking = PtopLinkInterfaceTcQdiscQueueTracking(basicSimulation, topology); // Requires enable_link_interface_tc_qdisc_queue_tracking=true

        // Run simulation
        basicSimulation->Run();

        // Write UDP bursts results
        udpBurstScheduler.WriteResults();

        // Write link net-device queue results
        netDeviceQueueTracking.WriteResults();

        // Write link interface traffic-control qdisc queue results
        tcQdiscQueueTracking.WriteResults();

        // Finalize the simulation
        basicSimulation->Finalize();

        // Links
        std::vector<std::pair<int64_t, int64_t>> links;
        links.push_back(std::make_pair(0, 1));
        links.push_back(std::make_pair(1, 0));
        links.push_back(std::make_pair(0, 2));
        links.push_back(std::make_pair(2, 0));

        // Get the link net-device queue development
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_net_device_queue_pkt;
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_net_device_queue_byte;
        validate_link_net_device_queue_logs(test_run_dir, links, link_net_device_queue_pkt, link_net_device_queue_byte);

        // Get the link interface traffic-control qdisc queue development
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_interface_tc_qdisc_queue_pkt;
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_interface_tc_qdisc_queue_byte;
        validate_link_interface_tc_qdisc_queue_logs(test_run_dir, links, link_interface_tc_qdisc_queue_pkt, link_interface_tc_qdisc_queue_byte);

        // Re-used for convenience
        std::vector<std::tuple<int64_t, int64_t, int64_t>> qdisc_queue_intervals = link_interface_tc_qdisc_queue_pkt.at(std::make_pair(0, 1));
        int64_t largest_queue_size_pkt = -1;

        Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
        x->SetAttribute ("Min", DoubleValue (0.0));
        x->SetAttribute ("Max", DoubleValue (1.0));

        //////////////////////////////////
        // 0 -> 1

        // It got 1000 packets put into its queue at a rate of 1000 Mbit/s
        // In the time of the burst, it drained 10 Mbit/s, which is 10 packets
        // As such, 990 total packets would be the maximum queue size for first-in-first-out

        // 0 -> 1 has a 1 packets link net-device queue and simple_red(drop; 1.0; 100; 500; 4000; 0.2; no_wait; gentle) as its qdisc
        int queue_size_analysis = 0;
        double prob = 0.1;
        int count = 0;
        for (int i = 0; i < 1000; i++) {
            if (queue_size_analysis < 100) {
                queue_size_analysis += 1;
            } else {
                count += 1;
                double p_a;
                if (queue_size_analysis < 500) {
                    p_a = prob * ((queue_size_analysis - 100.0) / (500.0 - 100.0));
                } else {
                    p_a = prob + (1.0 - prob) * ((queue_size_analysis - 500.0) / (1000.0 - 500.0));
                }

                // no_wait modification rule
                double p_b;
                if (count * p_a < 1.0) {
                    p_b = p_a / (1.0 - count * p_a);
                } else {
                    p_b = 1.0;
                }
                if (x->GetValue() >= p_b) {
                    queue_size_analysis += 1;
                } else {
                    count = 0;
                }
            }
        }
        qdisc_queue_intervals = link_interface_tc_qdisc_queue_pkt.at(std::make_pair(0, 1));
        largest_queue_size_pkt = -1;
        for (std::tuple<int64_t, int64_t, int64_t> interval : qdisc_queue_intervals) {
            int64_t interval_num_pkt = std::get<2>(interval);
            largest_queue_size_pkt = std::max(largest_queue_size_pkt, interval_num_pkt);
        }
        queue_size_analysis = queue_size_analysis - 10; // For the 10 Mbit/s sending out 10 packets over the queueing
        ASSERT_EQUAL_APPROX(largest_queue_size_pkt, queue_size_analysis, 10);

        //////////////////////////////////
        // 2 -> 0

        // It got 167 packets put into its queue at a rate of 1000 Mbit/s
        // In the time of the burst, it drained 10 Mbit/s, which is 1.667 packets
        // As such, 165-ish total packets would be the maximum queue size for first-in-first-out

        // 2 -> 0 has a 1 packets link net-device queue and simple_red(drop; 1500; 1.0; 20; 60; 200p; 0.3; wait; gentle) as its qdisc
        queue_size_analysis = 0;
        prob = 0.3;
        count = 0;
        for (int i = 0; i < 167; i++) {
            if (queue_size_analysis < 20) {
                queue_size_analysis += 1;
            } else {
                count += 1;
                double p_a;
                if (queue_size_analysis < 60) {
                    p_a = prob * ((queue_size_analysis - 20.0) / (60.0 - 20.0));
                } else {
                    p_a = prob + (1.0 - prob) * ((queue_size_analysis - 60.0) / (120.0 - 60.0));
                }

                // wait modification rule
                double p_b;
                if (count * p_a < 1.0) {
                    p_b = 0.0;
                } else if (count * p_a < 2.0) {
                    p_b = p_a / (2.0 - count * p_a);
                } else {
                    p_b = 1.0;
                }
                if (x->GetValue() >= p_b) {
                    queue_size_analysis += 1;
                } else {
                    count = 0;
                }
            }
        }
        qdisc_queue_intervals = link_interface_tc_qdisc_queue_pkt.at(std::make_pair(2, 0));
        largest_queue_size_pkt = -1;
        for (std::tuple<int64_t, int64_t, int64_t> interval : qdisc_queue_intervals) {
            int64_t interval_num_pkt = std::get<2>(interval);
            largest_queue_size_pkt = std::max(largest_queue_size_pkt, interval_num_pkt);
        }
        queue_size_analysis = queue_size_analysis - 2; // For the 10 Mbit/s sending out 2 packets over the queueing
        ASSERT_EQUAL_APPROX(largest_queue_size_pkt, queue_size_analysis, 5);

        // Template:
        // std::vector<std::tuple<int64_t, int64_t, int64_t>> qdisc_queue_intervals = link_interface_tc_qdisc_queue_pkt.at(std::make_pair(0, 1));
        // int64_t largest_queue_size_pkt = -1;
        // for (std::tuple<int64_t, int64_t, int64_t> interval : qdisc_queue_intervals) {
        //     int64_t interval_start_ns = std::get<0>(interval);
        //     int64_t interval_end_ns = std::get<1>(interval);
        //     int64_t interval_num_pkt = std::get<2>(interval);
        //     largest_queue_size_pkt = std::max(largest_queue_size_pkt, interval_num_pkt);
        // }
        // ASSERT_EQUAL_APPROX(largest_queue_size_pkt, AMOUNT, DEVIATION);

        // Clean up
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/udp_burst_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_incoming.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_incoming.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_outgoing.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_outgoing.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_queue_byte.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_queue_pkt.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_byte.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_pkt.csv");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PacketSaveTracer
{
public:
    std::vector<Ptr<Packet>> packets;
    void NetDevicePhyTxEndCallback(Ptr<Packet const> p) {
        packets.push_back(p->Copy());
    }
};

namespace ns3 {

class IpTosGeneratorEnableEcn : public IpTosGenerator {
public:
    static TypeId GetTypeId(void);
    uint8_t GenerateIpTos(TypeId appTypeId, Ptr<Application> app);
};

NS_OBJECT_ENSURE_REGISTERED (IpTosGeneratorEnableEcn);
TypeId IpTosGeneratorEnableEcn::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::IpTosGeneratorEnableEcn")
            .SetParent<IpTosGenerator> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

uint8_t IpTosGeneratorEnableEcn::GenerateIpTos(TypeId appTypeId, Ptr<Application> app) {
    return 1;
}

}

class PtopTcQdiscRedEcnMarkingTestCase : public TestCaseWithLogValidators
{
public:
    PtopTcQdiscRedEcnMarkingTestCase () : TestCaseWithLogValidators ("ptop-tc-qdisc-red ecn-marking") {};
    const std::string test_run_dir = ".tmp-test-ptop-tc-qdisc-red-ecn-marking";

    void DoRun () {
        prepare_clean_run_dir(test_run_dir);

        // Write configuration
        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=1950000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_link_net_device_queue_tracking=true" << std::endl;
        config_file << "link_net_device_queue_tracking_enable_for_links=all" << std::endl;
        config_file << "enable_link_interface_tc_qdisc_queue_tracking=true" << std::endl;
        config_file << "link_interface_tc_qdisc_queue_tracking_enable_for_links=all" << std::endl;
        config_file << "enable_udp_burst_scheduler=true" << std::endl;
        config_file << "udp_burst_schedule_filename=\"udp_burst_schedule.csv\"" << std::endl;
        config_file.close();

        // Write topology
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=3" << std::endl;
        topology_file << "num_undirected_edges=2" << std::endl;
        topology_file << "switches=set(0,1,2)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1,2)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1, 1-2)" << std::endl;
        topology_file << "all_nodes_are_endpoints=true" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=10" << std::endl;
        topology_file << "link_net_device_queue=map(0->1: drop_tail(100p), 1->0: drop_tail(100p), 1->2: drop_tail(100p), 2->1: drop_tail(100p))" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=map(0->1: simple_red(ecn; 1500; 1.0; 100; 200; 4000p; 0.1; no_wait; gentle), 1->0: fifo(100p), 1->2: fifo(100p), 2->1: fifo(100p))" << std::endl;
        topology_file.close();

        // Write UDP burst file
        std::ofstream udp_burst_schedule_file;
        udp_burst_schedule_file.open (test_run_dir + "/udp_burst_schedule.csv");
        // 1000 Mbit/s for 2000000 ns = 167 packets
        // 1000 Mbit/s for 6000000 ns = 500 packets
        // 1000 Mbit/s for 12000000 ns = 1000 packets
        // 2000 Mbit/s for 12000000 ns = 2000 packets
        udp_burst_schedule_file << "0,0,2,1000,0,6000000,," << std::endl;
        topology_file.close();

        // Create simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);

        // Create topology
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // Track the packet being sent
        PacketSaveTracer packetSaveTracer;
        Ptr<PointToPointNetDevice> netDevice1to2 = topology->GetSendingNetDeviceForLink(std::make_pair(1, 2));
        netDevice1to2->TraceConnectWithoutContext("PhyTxEnd", MakeCallback(&PacketSaveTracer::NetDevicePhyTxEndCallback, &packetSaveTracer));

        // Schedule UDP bursts
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology, CreateObject<UdpSocketGeneratorDefault>(), CreateObject<IpTosGeneratorEnableEcn>());

        // Install link net-device queue trackers
        PtopLinkNetDeviceQueueTracking netDeviceQueueTracking = PtopLinkNetDeviceQueueTracking(basicSimulation, topology); // Requires enable_link_net_device_queue_tracking=true

        // Install link interface traffic-control qdisc queue trackers
        PtopLinkInterfaceTcQdiscQueueTracking tcQdiscQueueTracking = PtopLinkInterfaceTcQdiscQueueTracking(basicSimulation, topology); // Requires enable_link_interface_tc_qdisc_queue_tracking=true

        // Run simulation
        basicSimulation->Run();

        // Write UDP bursts results
        udpBurstScheduler.WriteResults();

        // Write link net-device queue results
        netDeviceQueueTracking.WriteResults();

        // Write link interface traffic-control qdisc queue results
        tcQdiscQueueTracking.WriteResults();

        // Finalize the simulation
        basicSimulation->Finalize();

        // Links
        std::vector<std::pair<int64_t, int64_t>> links;
        links.push_back(std::make_pair(0, 1));
        links.push_back(std::make_pair(1, 0));
        links.push_back(std::make_pair(1, 2));
        links.push_back(std::make_pair(2, 1));

        // Get the link net-device queue development
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_net_device_queue_pkt;
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_net_device_queue_byte;
        validate_link_net_device_queue_logs(test_run_dir, links, link_net_device_queue_pkt, link_net_device_queue_byte);

        // Get the link interface traffic-control qdisc queue development
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_interface_tc_qdisc_queue_pkt;
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_interface_tc_qdisc_queue_byte;
        validate_link_interface_tc_qdisc_queue_logs(test_run_dir, links, link_interface_tc_qdisc_queue_pkt, link_interface_tc_qdisc_queue_byte);

        /////////////////////////////////////////////////////
        // Step 1: measure how many of them were ECN marked

        size_t num_not_ect = 0;
        size_t num_ect1 = 0;
        size_t num_ect0 = 0;
        size_t num_ce = 0;
        for (Ptr<Packet> p : packetSaveTracer.packets) {
            PppHeader pppHeader;
            p->RemoveHeader(pppHeader);
            Ipv4Header ipv4Header;
            p->RemoveHeader(ipv4Header);
            switch (ipv4Header.GetEcn()) {
                case Ipv4Header::ECN_NotECT:
                    num_not_ect++;
                    break;
                case Ipv4Header::ECN_ECT1:
                    num_ect1++;
                    break;
                case Ipv4Header::ECN_ECT0:
                    num_ect0++;
                    break;
                case Ipv4Header::ECN_CE:
                    num_ce++;
                    break;
            }
        }
        // std::cout << "NOT-ECT: " << num_not_ect << std::endl;
        // std::cout << "ECT-1:   " << num_ect1 << std::endl;
        // std::cout << "ECT-0:   " << num_ect0 << std::endl;
        // std::cout << "CE:      " << num_ce << std::endl;

        /////////////////////////////////////////////////////
        // Step 2: calculate what would be the expectation

        // It got 1000 packets put into its queue at a rate of 1000 Mbit/s
        // In the time of the burst, it drained 10 Mbit/s, which is 10 packets
        // As such, 990 total packets would be the maximum queue size for first-in-first-out

        // 0 -> 1:
        // Net-device queue: FIFO of 100 packets
        // Queuing discipline: simple_red(ecn; 1500; 1.0; 100; 200; 4000p; 0.1; no_wait; gentle)
        Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
        x->SetAttribute ("Min", DoubleValue (0.0));
        x->SetAttribute ("Max", DoubleValue (1.0));
        size_t num_expected_unmarked = 0;
        size_t num_expected_marked = 0;
        double prob = 0.1;
        int count = 0;
        for (int i = 0; i < 500; i++) { // 500 packets are put into the queue one-by-one

            // First the net-device queue is filled up (100 packets)
            if (i < 100) {
                num_expected_unmarked++;
            } else {
                int total_in_qdisc_queue = i - 100;
                if (total_in_qdisc_queue < 100) {
                    num_expected_unmarked++;
                } else {
                    count += 1;
                    double p_a;
                    if (total_in_qdisc_queue < 200) {
                        p_a = prob * ((total_in_qdisc_queue - 100.0) / (200.0 - 100.0));
                    } else {
                        p_a = prob + (1.0 - prob) * ((total_in_qdisc_queue - 200.0) / (400.0 - 200.0));
                    }

                    // no_wait modification rule
                    double p_b;
                    if (count * p_a < 1.0) {
                        p_b = p_a / (1.0 - count * p_a);
                    } else {
                        p_b = 1.0;
                    }
                    if (x->GetValue() >= p_b) {
                        num_expected_unmarked++;
                    } else {
                        num_expected_marked++;
                        count = 0;
                    }


                }

            }

        }
        // std::cout << "Expected marked:    " << num_expected_marked << std::endl;
        // std::cout << "Expected unmarked:  " << num_expected_unmarked << std::endl;

        /////////////////////////////////////////////////////
        // Step 3: compare actual vs. expectation
        ASSERT_EQUAL(0, num_not_ect);
        ASSERT_EQUAL_APPROX(num_expected_unmarked, num_ect1, 5);
        ASSERT_EQUAL(0, num_ect0);
        ASSERT_EQUAL_APPROX(num_expected_marked, num_ce, 5);

        // Clean up
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/udp_burst_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_incoming.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_incoming.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_outgoing.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_outgoing.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_queue_byte.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_queue_pkt.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_byte.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_pkt.csv");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class IpTosGeneratorFromAdditionalParameters : public IpTosGenerator {
public:
    static TypeId GetTypeId(void);
    uint8_t GenerateIpTos(TypeId appTypeId, Ptr<Application> app);
};

NS_OBJECT_ENSURE_REGISTERED (IpTosGeneratorFromAdditionalParameters);
TypeId IpTosGeneratorFromAdditionalParameters::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::IpTosGeneratorFromAdditionalParameters")
            .SetParent<IpTosGenerator> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

uint8_t IpTosGeneratorFromAdditionalParameters::GenerateIpTos(TypeId appTypeId, Ptr<Application> app) {
    if (appTypeId == TcpFlowClient::GetTypeId()) {
        Ptr<TcpFlowClient> client = app->GetObject<TcpFlowClient>();
        return parse_positive_int64(client->GetAdditionalParameters());
    } else {
        return 55;
    }
}

class PtopTcQdiscPfifoFastAbsoluteTestCase : public TestCaseWithLogValidators
{
public:
    PtopTcQdiscPfifoFastAbsoluteTestCase () : TestCaseWithLogValidators ("ptop-tc-qdisc-pfifo-fast absolute") {};
    const std::string test_run_dir = ".tmp-test-ptop-tc-qdisc-pfifo-fast-absolute";

    void DoRun () {
        prepare_clean_run_dir(test_run_dir);

        // Write configuration
        int64_t simulation_end_time_ns = 2000000000;
        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << simulation_end_time_ns << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "tcp_config=basic" << std::endl;
        config_file << "enable_link_net_device_queue_tracking=true" << std::endl;
        config_file << "link_net_device_queue_tracking_enable_for_links=all" << std::endl;
        config_file << "enable_link_interface_tc_qdisc_queue_tracking=true" << std::endl;
        config_file << "link_interface_tc_qdisc_queue_tracking_enable_for_links=all" << std::endl;
        config_file << "enable_tcp_flow_scheduler=true" << std::endl;
        config_file << "tcp_flow_schedule_filename=\"tcp_flow_schedule.csv\"" << std::endl;
        config_file << "tcp_flow_enable_logging_for_tcp_flow_ids=all" << std::endl;
        config_file.close();

        // Write topology
        //
        // 0 -- 1 -- 2
        //      |
        //      3
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,2,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,1-3)" << std::endl;
        topology_file << "all_nodes_are_endpoints=true" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=10" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(1p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=pfifo_fast(100p)" << std::endl;
        topology_file.close();

        // Couple of flows
        std::vector<TcpFlowScheduleEntry> write_schedule;
        // 0 = 0b00000000 -> Strip ECN -> 0b00000000 -> Priority cares only for 3-6
        // 0b00000000
        //      ^^^^
        //
        // -> 0b00000000 -> Shift one right to get in 0-15 range value: 0b00000000 = 0
        // ... and 0 maps to band 1 = medium priority
        write_schedule.push_back(TcpFlowScheduleEntry(0, 0, 3, 1000000, 0, "0", ""));
        // 55 = 0b00110111 -> Strip ECN -> 0b00110100 -> Priority cares only for 3-6
        // 0b00110100
        //      ^^^^
        //
        // -> 0b00010100 -> Shift one right to get in 0-15 range value: 0b00001010 = 10
        // ... and 10 maps to band 0 = highest priority
        write_schedule.push_back(TcpFlowScheduleEntry(1, 2, 3, 1000000, 0, "55", ""));

        // Write schedule file
        std::ofstream schedule_file;
        schedule_file.open (test_run_dir + "/tcp_flow_schedule.csv");
        for (TcpFlowScheduleEntry entry : write_schedule) {
            schedule_file
                    << entry.GetTcpFlowId() << ","
                    << entry.GetFromNodeId() << ","
                    << entry.GetToNodeId() << ","
                    << entry.GetSizeByte() << ","
                    << entry.GetStartTimeNs() << ","
                    << entry.GetAdditionalParameters() << ","
                    << entry.GetMetadata()
                    << std::endl;
        }
        schedule_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        TcpConfigHelper::Configure(basicSimulation);
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology, {22000}, CreateObject<ClientRemotePortSelectorDefault>(22000), CreateObject<TcpSocketGeneratorDefault>(), CreateObject<IpTosGeneratorFromAdditionalParameters>());
        PtopLinkNetDeviceQueueTracking netDeviceQueueTracking = PtopLinkNetDeviceQueueTracking(basicSimulation, topology);
        PtopLinkInterfaceTcQdiscQueueTracking tcQdiscQueueTracking = PtopLinkInterfaceTcQdiscQueueTracking(basicSimulation, topology);
        basicSimulation->Run();
        tcpFlowScheduler.WriteResults();
        netDeviceQueueTracking.WriteResults();
        tcQdiscQueueTracking.WriteResults();
        basicSimulation->Finalize();

        // For which logging was enabled
        std::set<int64_t> tcp_flow_ids_with_logging;
        for (size_t i = 0; i < write_schedule.size(); i++) {
            tcp_flow_ids_with_logging.insert(i);
        }

        // For results
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;

        // Validate TCP flow logs
        validate_tcp_flow_logs(
                simulation_end_time_ns,
                test_run_dir,
                write_schedule,
                tcp_flow_ids_with_logging,
                end_time_ns_list,
                sent_byte_list,
                finished_list
        );

        // Check that the end times are according to absolute prioritization expectations
        int64_t end_time_ns_first = end_time_ns_list.at(0);
        int64_t end_time_ns_second = end_time_ns_list.at(1);
        ASSERT_EQUAL_APPROX(end_time_ns_first, 1600000000, 100000000);  // 1.6 seconds +- 100ms
        ASSERT_EQUAL_APPROX(end_time_ns_second, 800000000, 100000000);  // 0.8 seconds +- 100ms
        std::cout << end_time_ns_first << std::endl;
        std::cout << end_time_ns_second << std::endl;

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/tcp_flow_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_queue_byte.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_queue_pkt.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_byte.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_pkt.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.txt");
        for (int64_t i : tcp_flow_ids_with_logging) {
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_progress.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_rtt.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_rto.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cwnd.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cwnd_inflated.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_ssthresh.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_inflight.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_state.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cong_state.csv");
        }
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////
