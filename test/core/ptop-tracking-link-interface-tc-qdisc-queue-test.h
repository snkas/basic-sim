/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

////////////////////////////////////////////////////////////////////////////////////////

class PtopTrackingLinkInterfaceTcQdiscQueueBaseTestCase : public TestCaseWithLogValidators
{
public:
    PtopTrackingLinkInterfaceTcQdiscQueueBaseTestCase (std::string s) : TestCaseWithLogValidators (s) {};
    std::string test_run_dir = ".tmp-test-ptop-tracking-interface-tc-qdisc-queue";

    void write_basic_config(std::string log_for_links) {
        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=1950000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_link_interface_tc_qdisc_queue_tracking=true" << std::endl;
        config_file << "link_interface_tc_qdisc_queue_tracking_enable_for_links=" << log_for_links << std::endl;
        config_file << "enable_udp_burst_scheduler=true" << std::endl;
        config_file << "udp_burst_schedule_filename=\"udp_burst_schedule.csv\"" << std::endl;
        config_file.close();
    }

    void write_four_side_topology() {
        std::ofstream topology_file;
        topology_file.open(test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-3,0-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(1p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=fifo(100p)" << std::endl;
        topology_file << "all_nodes_are_endpoints=true" << std::endl;
        topology_file.close();
    }

    void cleanup() {
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/udp_burst_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_pkt.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_byte.csv");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);
    }

    void run_default() {

        // Create simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);

        // Create topology
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // Schedule UDP bursts
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology);

        // Install link net-device queue trackers
        PtopLinkInterfaceTcQdiscQueueTracking netDeviceQueueTracking = PtopLinkInterfaceTcQdiscQueueTracking(basicSimulation, topology); // Requires enable_link_interface_tc_qdisc_queue_tracking=true

        // Run simulation
        basicSimulation->Run();

        // Write link net-device queue results
        netDeviceQueueTracking.WriteResults();

        // Finalize the simulation
        basicSimulation->Finalize();

    }

};

////////////////////////////////////////////////////////////////////////////////////////

class PtopTrackingLinkInterfaceTcQdiscQueueSimpleTestCase : public PtopTrackingLinkInterfaceTcQdiscQueueBaseTestCase
{
public:
    PtopTrackingLinkInterfaceTcQdiscQueueSimpleTestCase () : PtopTrackingLinkInterfaceTcQdiscQueueBaseTestCase ("ptop-tracking-link-interface-tc-qdisc-queue simple") {};

    void DoRun () {
        test_run_dir = ".tmp-test-ptop-tracking-interface-tc-qdisc-queue-simple";
        prepare_clean_run_dir(test_run_dir);

        // Configuration files
        write_basic_config("all");
        write_four_side_topology();
        std::ofstream udp_burst_schedule_file;
        udp_burst_schedule_file.open(test_run_dir + "/udp_burst_schedule.csv");
        udp_burst_schedule_file << "0,0,1,50,0,500000000,," << std::endl;
        udp_burst_schedule_file << "1,2,3,90,250000000,500000000,," << std::endl;
        udp_burst_schedule_file << "2,3,2,90,250000000,500000000,," << std::endl;
        udp_burst_schedule_file << "3,1,3,120,250000000,5000000000,," << std::endl;
        udp_burst_schedule_file.close();

        // Run it
        run_default();

        // Directed edge list
        std::vector <std::pair<int64_t, int64_t>> dir_a_b_list;
        dir_a_b_list.push_back(std::make_pair(0, 1));
        dir_a_b_list.push_back(std::make_pair(1, 0));
        dir_a_b_list.push_back(std::make_pair(0, 2));
        dir_a_b_list.push_back(std::make_pair(2, 0));
        dir_a_b_list.push_back(std::make_pair(1, 3));
        dir_a_b_list.push_back(std::make_pair(3, 1));
        dir_a_b_list.push_back(std::make_pair(3, 2));
        dir_a_b_list.push_back(std::make_pair(2, 3));

        // Validate logs
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_interface_tc_qdisc_queue_pkt;
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_interface_tc_qdisc_queue_byte;
        validate_link_interface_tc_qdisc_queue_logs(test_run_dir, dir_a_b_list, link_interface_tc_qdisc_queue_pkt, link_interface_tc_qdisc_queue_byte);

        // Check if it matches with the traffic expectation for each pair
        for (std::pair <int64_t, int64_t> a_b : dir_a_b_list) {

            // Every entry for this pair
            for (uint32_t i = 0; i < link_interface_tc_qdisc_queue_byte.at(a_b).size(); i++) {
                int64_t interval_start_ns = std::get<0>(link_interface_tc_qdisc_queue_byte.at(a_b)[i]);
                // int64_t interval_end_ns = std::get<1>(link_interface_tc_qdisc_queue_byte.at(a_b)[i]);
                int64_t num_byte = std::get<2>(link_interface_tc_qdisc_queue_byte.at(a_b)[i]);
                int64_t num_pkt = std::get<2>(link_interface_tc_qdisc_queue_pkt.at(a_b)[i]);

                // Now we check that the number of packets/bytes make sense given the traffic started
                if (a_b.first == 1 && a_b.second == 3 && interval_start_ns >= 250000000) {
                    if (interval_start_ns >= 252000000) {
                        ASSERT_TRUE(num_byte > 0);
                        ASSERT_TRUE(num_pkt > 0);
                    }
                    if (interval_start_ns >= 400000000) {
                        ASSERT_TRUE(num_pkt >= 99);
                        ASSERT_TRUE(num_byte >= 99 * 1500);
                    }
                } else {
                    // All other links cannot have a queue building up
                    ASSERT_EQUAL(num_byte, 0);
                    ASSERT_EQUAL(num_pkt, 0);
                }

            }

        }

        // Finally clean up
        cleanup();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PtopTrackingLinkInterfaceTcQdiscQueueSpecificLinksTestCase : public PtopTrackingLinkInterfaceTcQdiscQueueBaseTestCase
{
public:
    PtopTrackingLinkInterfaceTcQdiscQueueSpecificLinksTestCase () : PtopTrackingLinkInterfaceTcQdiscQueueBaseTestCase ("ptop-tracking-link-interface-tc-qdisc-queue specific-links") {};

    void DoRun () {
        test_run_dir = ".tmp-test-ptop-tracking-interface-tc-qdisc-queue-specific-links";
        prepare_clean_run_dir(test_run_dir);

        // Configuration files
        write_basic_config("set(0->1,2->0,3->2)");
        write_four_side_topology();
        std::ofstream udp_burst_schedule_file;
        udp_burst_schedule_file.open(test_run_dir + "/udp_burst_schedule.csv");
        udp_burst_schedule_file << "0,3,2,120,250000000,5000000000,," << std::endl;
        udp_burst_schedule_file << "1,1,3,120,250000000,5000000000,," << std::endl;
        udp_burst_schedule_file.close();

        // Run it
        run_default();

        // Directed edge list
        std::vector <std::pair<int64_t, int64_t>> dir_a_b_list;
        dir_a_b_list.push_back(std::make_pair(0, 1));
        dir_a_b_list.push_back(std::make_pair(2, 0));
        dir_a_b_list.push_back(std::make_pair(3, 2));

        // Validate logs
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_interface_tc_qdisc_queue_pkt;
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_interface_tc_qdisc_queue_byte;
        validate_link_interface_tc_qdisc_queue_logs(test_run_dir, dir_a_b_list, link_interface_tc_qdisc_queue_pkt, link_interface_tc_qdisc_queue_byte);

        // Check if it matches with the traffic expectation for each pair
        for (std::pair<int64_t, int64_t> a_b : dir_a_b_list) {

            // Every entry for this pair
            for (uint32_t i = 0; i < link_interface_tc_qdisc_queue_byte.at(a_b).size(); i++) {
                int64_t interval_start_ns = std::get<0>(link_interface_tc_qdisc_queue_byte.at(a_b)[i]);
                // int64_t interval_end_ns = std::get<1>(link_interface_tc_qdisc_queue_byte.at(a_b)[i]);
                int64_t num_byte = std::get<2>(link_interface_tc_qdisc_queue_byte.at(a_b)[i]);
                int64_t num_pkt = std::get<2>(link_interface_tc_qdisc_queue_pkt.at(a_b)[i]);

                // Now we check that the number of packets/bytes make sense given the traffic started
                if (a_b.first == 3 && a_b.second == 2 && interval_start_ns >= 250000000) {
                    if (interval_start_ns >= 252000000) {
                        ASSERT_TRUE(num_byte > 0);
                        ASSERT_TRUE(num_pkt > 0);
                    }
                    if (interval_start_ns >= 400000000) {
                        ASSERT_TRUE(num_pkt >= 99);
                        ASSERT_TRUE(num_byte >= 99 * 1500);
                    }
                } else {
                    // All other links cannot have a queue building up
                    ASSERT_EQUAL(num_byte, 0);
                    ASSERT_EQUAL(num_pkt, 0);
                }

            }

        }

        // Finally clean up
        cleanup();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PtopTrackingLinkInterfaceTcQdiscQueueNotEnabledTestCase : public TestCaseWithLogValidators
{
public:
    PtopTrackingLinkInterfaceTcQdiscQueueNotEnabledTestCase () : TestCaseWithLogValidators ("ptop-tracking-link-interface-tc-qdisc-queue not-enabled") {};
    const std::string test_run_dir = ".tmp-test-ptop-tracking-interface-tc-qdisc-queue-not-enabled";

    void DoRun () {
        prepare_clean_run_dir(test_run_dir);
        
        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=1950000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_link_interface_tc_qdisc_queue_tracking=false" << std::endl;
        config_file << "enable_udp_burst_scheduler=true" << std::endl;
        config_file << "udp_burst_schedule_filename=\"udp_burst_schedule.csv\"" << std::endl;
        config_file.close();

        std::ofstream udp_burst_schedule_file;
        udp_burst_schedule_file.open (test_run_dir + "/udp_burst_schedule.csv");
        udp_burst_schedule_file << "0,0,1,50,0,500000000,," << std::endl;
        udp_burst_schedule_file.close();

        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-3,0-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file << "all_nodes_are_endpoints=true" << std::endl;
        topology_file.close();

        // Create simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);

        // Create topology
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // Schedule UDP bursts
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology);

        // Install TC qdisc queue trackers
        PtopLinkInterfaceTcQdiscQueueTracking tcQdiscQueueTracker = PtopLinkInterfaceTcQdiscQueueTracking(basicSimulation, topology);

        // Run simulation
        basicSimulation->Run();

        // Write TC qdisc queue trackers
        tcQdiscQueueTracker.WriteResults();

        // Finalize the simulation
        basicSimulation->Finalize();

        // Nothing should have been logged
        ASSERT_FALSE(file_exists(test_run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_pkt.csv"));
        ASSERT_FALSE(file_exists(test_run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_byte.csv"));

        // Clean-up
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/udp_burst_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PtopTrackingLinkInterfaceTcQdiscQueueNoQdiscTestCase : public TestCaseWithLogValidators
{
public:
    PtopTrackingLinkInterfaceTcQdiscQueueNoQdiscTestCase () : TestCaseWithLogValidators ("ptop-tracking-link-interface-tc-qdisc-queue no-qdisc") {};
    const std::string test_run_dir = ".tmp-test-ptop-tracking-interface-tc-qdisc-queue-no-qdisc";

    void DoRun () {
        prepare_clean_run_dir(test_run_dir);
        
        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=1950000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_link_interface_tc_qdisc_queue_tracking=true" << std::endl;
        config_file.close();

        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=3" << std::endl;
        topology_file << "num_undirected_edges=2" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(1)" << std::endl;
        topology_file << "servers=set(2)" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2)" << std::endl;
        topology_file << "link_channel_delay_ns=10" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=map(0->1: fifo(50p), 1->0: fifo(88p), 1->2: disabled, 2->1: fifo(44p))" << std::endl;
        topology_file << "all_nodes_are_endpoints=true" << std::endl;
        topology_file.close();

        // Create simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);

        // Create topology
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // Schedule UDP bursts
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology);

        // Install TC qdisc queue trackers
        ASSERT_EXCEPTION_MATCH_WHAT(
            PtopLinkInterfaceTcQdiscQueueTracking(basicSimulation, topology),
            "Cannot enable traffic-control qdisc queue tracking on an interface which does not have a qdisc (1 -> 2)."
        );

        // Finalize the simulation
        basicSimulation->Finalize();

        // Nothing should have been logged
        ASSERT_FALSE(file_exists(test_run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_pkt.csv"));
        ASSERT_FALSE(file_exists(test_run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_byte.csv"));

        // Clean-up
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
