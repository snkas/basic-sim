/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

////////////////////////////////////////////////////////////////////////////////////////

class PtopTrackingLinkNetDeviceUtilizationBaseTestCase : public TestCaseWithLogValidators
{
public:
    PtopTrackingLinkNetDeviceUtilizationBaseTestCase (std::string s) : TestCaseWithLogValidators (s) {};
    std::string test_run_dir = ".tmp-test-ptop-tracking-link-net-device-utilization";

    void write_basic_config(std::string log_for_links) {
        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=1950000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_link_net_device_utilization_tracking=true" << std::endl;
        config_file << "link_net_device_utilization_tracking_interval_ns=100000000" << std::endl;
        config_file << "link_net_device_utilization_tracking_enable_for_links=" << log_for_links << std::endl;
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
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file << "all_nodes_are_endpoints=true" << std::endl;
        topology_file.close();
    }

    void run_default() {

        // Create simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);

        // Create topology
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // Schedule UDP bursts
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology);

        // Install utilization trackers
        PtopLinkNetDeviceUtilizationTracking utilTrackerHelper = PtopLinkNetDeviceUtilizationTracking(basicSimulation, topology);

        // Run simulation
        basicSimulation->Run();

        // Write utilization results
        utilTrackerHelper.WriteResults();

        // Finalize the simulation
        basicSimulation->Finalize();

    }

    void cleanup() {
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/udp_burst_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_utilization.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_utilization_compressed.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_utilization_compressed.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_utilization_summary.txt");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);
    }

};

////////////////////////////////////////////////////////////////////////////////////////

class PtopTrackingLinkNetDeviceUtilizationSimpleTestCase : public PtopTrackingLinkNetDeviceUtilizationBaseTestCase
{
public:
    PtopTrackingLinkNetDeviceUtilizationSimpleTestCase () : PtopTrackingLinkNetDeviceUtilizationBaseTestCase ("ptop-tracking-link-net-device-utilization simple") {};
    void DoRun () {
        test_run_dir = ".tmp-test-ptop-tracking-link-net-device-utilization-simple";
        prepare_clean_run_dir(test_run_dir);

        write_basic_config("all");
        write_four_side_topology();
        std::ofstream udp_burst_schedule_file;
        udp_burst_schedule_file.open (test_run_dir + "/udp_burst_schedule.csv");
        udp_burst_schedule_file << "0,0,1,50,0,500000000,," << std::endl;
        udp_burst_schedule_file << "1,2,3,90,250000000,500000000,," << std::endl;
        udp_burst_schedule_file << "2,3,2,90,250000000,500000000,," << std::endl;
        udp_burst_schedule_file << "3,1,3,90,250000000,5000000000,," << std::endl;
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

        // Check results
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_net_device_utilization;
        std::map<std::pair<int64_t, int64_t>, double> link_overall_utilization_as_fraction;
        validate_link_net_device_utilization_logs(test_run_dir, dir_a_b_list, 1950000000, 100000000, link_net_device_utilization, link_overall_utilization_as_fraction);

        // Specific interval's link net-device utilization
        for (std::pair<int64_t, int64_t> dir_a_b : dir_a_b_list) {
            ASSERT_EQUAL(link_net_device_utilization.at(dir_a_b).size(), 20);
            for (int j = 0; j < 20; j++) {
                int64_t busy_time_ns = std::get<2>(link_net_device_utilization.at(dir_a_b).at(j));
                if (dir_a_b.first == 0 and dir_a_b.second == 1) {
                    if (j < 5) {
                        ASSERT_TRUE(busy_time_ns >= 50000000);
                    } else if (j == 5) {
                        ASSERT_TRUE(busy_time_ns <= 100000);
                    } else {
                        ASSERT_EQUAL(busy_time_ns, 0);
                    }
                } else if ((dir_a_b.first == 2 and dir_a_b.second == 3) ||
                           (dir_a_b.first == 3 and dir_a_b.second == 2)) {
                    if (j == 2 or j == 7) {
                        ASSERT_TRUE(busy_time_ns >= 45000000);
                    } else if (j > 2 && j < 7) {
                        ASSERT_TRUE(busy_time_ns >= 90000000);
                    } else {
                        ASSERT_EQUAL(busy_time_ns, 0);
                    }
                } else if (dir_a_b.first == 1 and dir_a_b.second == 3) {
                    if (j == 2 || j == 19) {
                        ASSERT_TRUE(busy_time_ns >= 45000000);
                    } else if (j > 2) {
                        ASSERT_TRUE(busy_time_ns >= 90000000);
                    } else {
                        ASSERT_EQUAL(busy_time_ns, 0);
                    }
                } else {
                    ASSERT_EQUAL(busy_time_ns, 0);
                }
            }
        }

        // Check that overall utilization matches up
        ASSERT_EQUAL_APPROX(link_overall_utilization_as_fraction.at(std::make_pair(0, 1)), 25.0 / 195.0, 0.2);
        ASSERT_EQUAL(link_overall_utilization_as_fraction.at(std::make_pair(1, 0)), 0);
        ASSERT_EQUAL(link_overall_utilization_as_fraction.at(std::make_pair(0, 2)), 0);
        ASSERT_EQUAL(link_overall_utilization_as_fraction.at(std::make_pair(2, 0)), 0);
        ASSERT_EQUAL_APPROX(link_overall_utilization_as_fraction.at(std::make_pair(2, 3)), 45.0 / 195.0, 0.2);
        ASSERT_EQUAL_APPROX(link_overall_utilization_as_fraction.at(std::make_pair(3, 2)), 45.0 / 195.0, 0.2);
        ASSERT_EQUAL_APPROX(link_overall_utilization_as_fraction.at(std::make_pair(1, 3)), 153.0 / 195.0, 0.2);
        ASSERT_EQUAL(link_overall_utilization_as_fraction.at(std::make_pair(3, 1)), 0);

        // Clean up
        cleanup();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PtopTrackingLinkNetDeviceUtilizationSpecificLinksTestCase : public PtopTrackingLinkNetDeviceUtilizationBaseTestCase
{
public:
    PtopTrackingLinkNetDeviceUtilizationSpecificLinksTestCase () : PtopTrackingLinkNetDeviceUtilizationBaseTestCase ("ptop-tracking-link-net-device-utilization specific-links") {};
    void DoRun () {
        test_run_dir = ".tmp-test-ptop-tracking-link-net-device-utilization-specific-links";
        prepare_clean_run_dir(test_run_dir);

        write_basic_config("set(0->1, 3->1, 2->0)");
        write_four_side_topology();
        std::ofstream udp_burst_schedule_file;
        udp_burst_schedule_file.open (test_run_dir + "/udp_burst_schedule.csv");
        udp_burst_schedule_file << "0,0,1,50,0,500000000,," << std::endl;
        udp_burst_schedule_file << "1,2,3,90,250000000,500000000,," << std::endl;
        udp_burst_schedule_file << "2,3,2,90,250000000,500000000,," << std::endl;
        udp_burst_schedule_file << "3,1,3,90,250000000,5000000000,," << std::endl;
        udp_burst_schedule_file.close();

        // Run it
        run_default();

        // Directed edge list
        std::vector <std::pair<int64_t, int64_t>> dir_a_b_list;
        dir_a_b_list.push_back(std::make_pair(0, 1));
        dir_a_b_list.push_back(std::make_pair(2, 0));
        dir_a_b_list.push_back(std::make_pair(3, 1));

        // Check results
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_net_device_utilization;
        std::map<std::pair<int64_t, int64_t>, double> link_overall_utilization_as_fraction;
        validate_link_net_device_utilization_logs(test_run_dir, dir_a_b_list, 1950000000, 100000000, link_net_device_utilization, link_overall_utilization_as_fraction);

        // Specific interval's link net-device utilization
        for (std::pair<int64_t, int64_t> dir_a_b : dir_a_b_list) {
            ASSERT_EQUAL(link_net_device_utilization.at(dir_a_b).size(), 20);
            for (int j = 0; j < 20; j++) {
                int64_t busy_time_ns = std::get<2>(link_net_device_utilization.at(dir_a_b).at(j));
                if (dir_a_b.first == 0 and dir_a_b.second == 1) {
                    if (j < 5) {
                        ASSERT_TRUE(busy_time_ns >= 50000000);
                    } else if (j == 5) {
                        ASSERT_TRUE(busy_time_ns <= 100000);
                    } else {
                        ASSERT_EQUAL(busy_time_ns, 0);
                    }
                } else {
                    ASSERT_EQUAL(busy_time_ns, 0);
                }
            }
        }

        // Check that overall utilization matches up
        ASSERT_EQUAL_APPROX(link_overall_utilization_as_fraction.at(std::make_pair(0, 1)), 25.0 / 195.0, 0.2);
        ASSERT_EQUAL(link_overall_utilization_as_fraction.at(std::make_pair(2, 0)), 0);
        ASSERT_EQUAL(link_overall_utilization_as_fraction.at(std::make_pair(3, 1)), 0);

        // Clean up
        cleanup();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PtopTrackingLinkNetDeviceUtilizationNotEnabledTestCase : public TestCaseWithLogValidators
{
public:
    PtopTrackingLinkNetDeviceUtilizationNotEnabledTestCase () : TestCaseWithLogValidators ("ptop-tracking-link-net-device-utilization not-enabled") {};
    const std::string test_run_dir = ".tmp-test-ptop-tracking-link-net-device-utilization-not-enabled";

    void DoRun () {
        prepare_clean_run_dir(test_run_dir);

        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=1950000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_link_net_device_utilization_tracking=false" << std::endl;
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

        // Install utilization trackers
        PtopLinkNetDeviceUtilizationTracking utilTrackerHelper = PtopLinkNetDeviceUtilizationTracking(basicSimulation, topology);

        // Run simulation
        basicSimulation->Run();

        // Write utilization results
        utilTrackerHelper.WriteResults();

        // Finalize the simulation
        basicSimulation->Finalize();

        // Nothing should have been logged
        ASSERT_FALSE(file_exists(test_run_dir + "/logs_ns3/link_net_device_utilization.csv"));
        ASSERT_FALSE(file_exists(test_run_dir + "/logs_ns3/link_net_device_utilization_compressed.csv"));
        ASSERT_FALSE(file_exists(test_run_dir + "/logs_ns3/link_net_device_utilization_compressed.txt"));
        ASSERT_FALSE(file_exists(test_run_dir + "/logs_ns3/link_net_device_utilization_summary.txt"));

        // Clean up
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/udp_burst_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_utilization.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_utilization_compressed.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_utilization_compressed.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_utilization_summary.txt");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////
