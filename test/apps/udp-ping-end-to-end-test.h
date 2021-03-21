/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

////////////////////////////////////////////////////////////////////////////////////////

class UdpPingEndToEndTestCase : public TestCaseWithLogValidators {
public:
    UdpPingEndToEndTestCase(std::string s) : TestCaseWithLogValidators(s) {};
    std::string test_run_dir = ".tmp-test-udp-ping-end-to-end";

    void write_basic_config(int64_t simulation_end_time_ns, int64_t simulation_seed) {
        std::ofstream config_file;
        config_file.open (test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << simulation_end_time_ns << std::endl;
        config_file << "simulation_seed=" << simulation_seed << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_udp_ping_scheduler=true" << std::endl;
        config_file << "udp_ping_schedule_filename=\"udp_ping_schedule.csv\"" << std::endl;
        config_file.close();
    }

    void write_single_topology(double link_net_device_data_rate_megabit_per_s, int64_t link_channel_delay_ns) {
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=2" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1)" << std::endl;
        topology_file << "link_channel_delay_ns=" << link_channel_delay_ns << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=" << link_net_device_data_rate_megabit_per_s << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
    }

    void test_run_and_validate_udp_ping_logs(
            int64_t simulation_end_time_ns,
            std::string test_run_dir, 
            std::vector<UdpPingInfo> write_schedule,
            std::vector<std::vector<int64_t>>& list_latency_there_ns,
            std::vector<std::vector<int64_t>>& list_latency_back_ns,
            std::vector<std::vector<int64_t>>& list_rtt_ns
    ) {

        // Write schedule file
        std::ofstream schedule_file;
        schedule_file.open (test_run_dir + "/udp_ping_schedule.csv");
        for (UdpPingInfo entry : write_schedule) {
            schedule_file
                    << entry.GetUdpPingId() << ","
                    << entry.GetFromNodeId() << ","
                    << entry.GetToNodeId() << ","
                    << entry.GetIntervalNs() << ","
                    << entry.GetStartTimeNs() << ","
                    << entry.GetDurationNs() << ","
                    << entry.GetWaitAfterwardsNs() << ","
                    << entry.GetAdditionalParameters() << ","
                    << entry.GetMetadata()
                    << std::endl;
        }
        schedule_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        UdpPingScheduler udpPingScheduler(basicSimulation, topology);
        basicSimulation->Run();
        udpPingScheduler.WriteResults();
        basicSimulation->Finalize();

        // Check finished.txt
        std::vector<std::string> finished_lines = read_file_direct(test_run_dir + "/logs_ns3/finished.txt");
        ASSERT_EQUAL(finished_lines.size(), 1);
        ASSERT_EQUAL(finished_lines[0], "Yes");

        // Validate UDP ping logs
        validate_udp_ping_logs(
                simulation_end_time_ns,
                test_run_dir,
                write_schedule,
                list_latency_there_ns,
                list_latency_back_ns,
                list_rtt_ns
       );

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/udp_ping_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_pings.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_pings.txt");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }

};

////////////////////////////////////////////////////////////////////////////////////////

class UdpPingEndToEndOneToOneManyTestCase : public UdpPingEndToEndTestCase
{
public:
    UdpPingEndToEndOneToOneManyTestCase () : UdpPingEndToEndTestCase ("udp-ping-end-to-end one-to-one-many") {};

    void DoRun () {
        test_run_dir = ".tmp-test-udp-ping-end-to-end-one-to-one-many";
        prepare_clean_run_dir(test_run_dir);

        // One-to-one (single) topology, 5s, 10000.0 Mbit/s, 1ms delay
        int64_t simulation_end_time_ns = 5000000000;
        write_basic_config(simulation_end_time_ns, 7595483);
        write_single_topology(10000.0, 1000000);

        // Several UDP pings
        std::vector<UdpPingInfo> schedule;
        schedule.push_back(UdpPingInfo(0, 1, 0, 10000, 5000, 100000, 1950000, "", "abc"));           // 1 -> 0, interval = 10us, start = 5us, duration = 100us, wait = 1.95ms
        schedule.push_back(UdpPingInfo(1, 0, 1, 10000000, 500000, 2000000000, 0, "", ""));           // 0 -> 1, interval = 10ms, start = 500us, duration = 2s, wait = 0ms
        schedule.push_back(UdpPingInfo(2, 0, 1, 1000000, 1000000, 100000000, 0, "", ""));            // 0 -> 1, interval = 1ms, start = 1ms, duration = 100ms, wait = 0ms
        schedule.push_back(UdpPingInfo(3, 0, 1, 1000000, 1000000, 100000000, 1000000, "", ""));      // 0 -> 1, interval = 1ms, start = 1ms, duration = 100ms, wait = 1ms
        schedule.push_back(UdpPingInfo(4, 0, 1, 1000000, 1000000, 100000000, 2000000, "", ""));      // 0 -> 1, interval = 1ms, start = 1ms, duration = 100ms, wait = 2ms
        schedule.push_back(UdpPingInfo(5, 1, 0, 1000000, 1000000, 100000000, 0, "", ""));            // 1 -> 0, interval = 1ms, start = 1ms, duration = 100ms, wait = 0ms

        // Perform the run
        std::vector<std::vector<int64_t>> list_latency_there_ns;
        std::vector<std::vector<int64_t>> list_latency_back_ns;
        std::vector<std::vector<int64_t>> list_rtt_ns;
        test_run_and_validate_udp_ping_logs(simulation_end_time_ns, test_run_dir, schedule, list_latency_there_ns, list_latency_back_ns, list_rtt_ns);

        // 1 -> 0, interval = 10us, start = 5us, duration = 100us, wait = 1.95ms
        ASSERT_EQUAL(list_latency_there_ns.at(0).size(), 10);
        for (size_t i = 0; i < list_latency_there_ns.at(0).size(); i++) {
            int64_t latency_there_ns = list_latency_there_ns.at(0).at(i);
            int64_t latency_back_ns = list_latency_back_ns.at(0).at(i);
            int64_t latency_rtt_ns = list_rtt_ns.at(0).at(i);
            if (i < 5) {
                ASSERT_EQUAL_APPROX(latency_there_ns, 1000000, 50);
                ASSERT_EQUAL_APPROX(latency_back_ns, 1000000, 50);
                ASSERT_EQUAL_APPROX(latency_rtt_ns, 2000000, 100);
            } else {
                ASSERT_EQUAL(latency_there_ns, -1);
                ASSERT_EQUAL(latency_back_ns, -1);
                ASSERT_EQUAL(latency_rtt_ns, -1);
            }
        }

        // 0 -> 1, interval = 10ms, start = 500us, duration = 2s, wait = 0ms
        ASSERT_EQUAL(list_latency_there_ns.at(1).size(), 200);
        for (size_t i = 0; i < list_latency_there_ns.at(1).size(); i++) {
            int64_t latency_there_ns = list_latency_there_ns.at(1).at(i);
            int64_t latency_back_ns = list_latency_back_ns.at(1).at(i);
            int64_t latency_rtt_ns = list_rtt_ns.at(1).at(i);
            ASSERT_EQUAL_APPROX(latency_there_ns, 1000000, 50);
            ASSERT_EQUAL_APPROX(latency_back_ns, 1000000, 50);
            ASSERT_EQUAL_APPROX(latency_rtt_ns, 2000000, 100);
        }

        // 0 -> 1, interval = 1ms, start = 1ms, duration = 100ms, wait = 0ms
        ASSERT_EQUAL(list_latency_there_ns.at(2).size(), 100);
        for (size_t i = 0; i < list_latency_there_ns.at(2).size(); i++) {
            int64_t latency_there_ns = list_latency_there_ns.at(2).at(i);
            int64_t latency_back_ns = list_latency_back_ns.at(2).at(i);
            int64_t latency_rtt_ns = list_rtt_ns.at(2).at(i);
            if (i < 98) {
                ASSERT_EQUAL_APPROX(latency_there_ns, 1000000, 50);
                ASSERT_EQUAL_APPROX(latency_back_ns, 1000000, 50);
                ASSERT_EQUAL_APPROX(latency_rtt_ns, 2000000, 100);
            } else {
                ASSERT_EQUAL(latency_there_ns, -1);
                ASSERT_EQUAL(latency_back_ns, -1);
                ASSERT_EQUAL(latency_rtt_ns, -1);
            }
        }

        // 0 -> 1, interval = 1ms, start = 1ms, duration = 100ms, wait = 1ms
        ASSERT_EQUAL(list_latency_there_ns.at(3).size(), 100);
        for (size_t i = 0; i < list_latency_there_ns.at(3).size(); i++) {
            int64_t latency_there_ns = list_latency_there_ns.at(3).at(i);
            int64_t latency_back_ns = list_latency_back_ns.at(3).at(i);
            int64_t latency_rtt_ns = list_rtt_ns.at(3).at(i);
            if (i < 99) {
                ASSERT_EQUAL_APPROX(latency_there_ns, 1000000, 100);
                ASSERT_EQUAL_APPROX(latency_back_ns, 1000000, 100);
                ASSERT_EQUAL_APPROX(latency_rtt_ns, 2000000, 200);
            } else {
                ASSERT_EQUAL(latency_there_ns, -1);
                ASSERT_EQUAL(latency_back_ns, -1);
                ASSERT_EQUAL(latency_rtt_ns, -1);
            }
        }

        // 0 -> 1, interval = 1ms, start = 1ms, duration = 100ms, wait = 2ms
        ASSERT_EQUAL(list_latency_there_ns.at(4).size(), 100);
        for (size_t i = 0; i < list_latency_there_ns.at(4).size(); i++) {
            int64_t latency_there_ns = list_latency_there_ns.at(4).at(i);
            int64_t latency_back_ns = list_latency_back_ns.at(4).at(i);
            int64_t latency_rtt_ns = list_rtt_ns.at(4).at(i);
            ASSERT_EQUAL_APPROX(latency_there_ns, 1000000, 150);
            ASSERT_EQUAL_APPROX(latency_back_ns, 1000000, 150);
            ASSERT_EQUAL_APPROX(latency_rtt_ns, 2000000, 300);
        }

        // 1 -> 0, interval = 1ms, start = 1ms, duration = 100ms, wait = 0ms
        ASSERT_EQUAL(list_latency_there_ns.at(5).size(), 100);
        for (size_t i = 0; i < list_latency_there_ns.at(5).size(); i++) {
            int64_t latency_there_ns = list_latency_there_ns.at(5).at(i);
            int64_t latency_back_ns = list_latency_back_ns.at(5).at(i);
            int64_t latency_rtt_ns = list_rtt_ns.at(5).at(i);
            if (i < 98) {
                ASSERT_EQUAL_APPROX(latency_there_ns, 1000000, 150);
                ASSERT_EQUAL_APPROX(latency_back_ns, 1000000, 150);
                ASSERT_EQUAL_APPROX(latency_rtt_ns, 2000000, 300);
            } else {
                ASSERT_EQUAL(latency_there_ns, -1);
                ASSERT_EQUAL(latency_back_ns, -1);
                ASSERT_EQUAL(latency_rtt_ns, -1);
            }
        }

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpPingEndToEndMultiPathTestCase : public UdpPingEndToEndTestCase
{
public:
    UdpPingEndToEndMultiPathTestCase () : UdpPingEndToEndTestCase ("udp-ping-end-to-end multi-path") {};

    void DoRun () {
        test_run_dir = ".tmp-test-udp-ping-end-to-end-multi-path";
        prepare_clean_run_dir(test_run_dir);

        int64_t simulation_end_time_ns = 100000000;
        int64_t simulation_seed = 1839582950225;

        /* topology.properties

                          8
                        /    \
                     0 -- 6
                   /         \
                  4 -- 1 -- 7 -- 3
                   \         /
                     2 -- 5

        */
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=9" << std::endl;
        topology_file << "num_undirected_edges=11" << std::endl;
        topology_file << "switches=set(0,1,2,3,4,5,6,7,8)" << std::endl;
        topology_file << "switches_which_are_tors=set(3,4)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-4,1-4,2-4,0-8,0-6,1-7,2-5,3-8,3-6,3-7,3-5)" << std::endl;
        topology_file << "all_nodes_are_endpoints=false" << std::endl;
        topology_file << "link_channel_delay_ns=map(0-4:1000,1-4:1000,2-4:1000,0-8:1000,0-6:1000,1-7:1000,2-5:1000,3-8:110000,3-6:220000,3-7:330000,3-5:440000)" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=10000" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(1000p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=iid_uniform_random_pkt(0.0)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // config_ns3.properties
        std::ofstream config_file;
        config_file.open (test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << simulation_end_time_ns << std::endl;
        config_file << "simulation_seed=" << simulation_seed << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_udp_ping_scheduler=true" << std::endl;
        config_file << "udp_ping_schedule_filename=\"udp_ping_schedule.csv\"" << std::endl;
        config_file.close();

        // Many UDP pings
        std::vector<UdpPingInfo> write_schedule;
        int num_pings = 400;
        for (int i = 0; i < num_pings; i++) {
            write_schedule.push_back(UdpPingInfo(i, 4, 3, 10000000, 0, simulation_end_time_ns, 0, "", ""));
        }

        // Write schedule file
        std::ofstream schedule_file;
        schedule_file.open (test_run_dir + "/udp_ping_schedule.csv");
        for (UdpPingInfo entry : write_schedule) {
            schedule_file
                    << entry.GetUdpPingId() << ","
                    << entry.GetFromNodeId() << ","
                    << entry.GetToNodeId() << ","
                    << entry.GetIntervalNs() << ","
                    << entry.GetStartTimeNs() << ","
                    << entry.GetDurationNs() << ","
                    << entry.GetWaitAfterwardsNs() << ","
                    << entry.GetAdditionalParameters() << ","
                    << entry.GetMetadata()
                    << std::endl;
        }
        schedule_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        UdpPingScheduler udpPingScheduler(basicSimulation, topology);
        basicSimulation->Run();
        udpPingScheduler.WriteResults();
        basicSimulation->Finalize();

        // To store the results
        std::vector<std::vector<int64_t>> list_latency_there_ns;
        std::vector<std::vector<int64_t>> list_latency_back_ns;
        std::vector<std::vector<int64_t>> list_rtt_ns;

        // Validate UDP ping logs
        validate_udp_ping_logs(
                simulation_end_time_ns,
                test_run_dir,
                write_schedule,
                list_latency_there_ns,
                list_latency_back_ns,
                list_rtt_ns
        );

        // Count the paths taken
        int64_t count_8_3 = 0;
        int64_t count_6_3 = 0;
        int64_t count_7_3 = 0;
        int64_t count_5_3 = 0;
        int64_t count_3_8 = 0;
        int64_t count_3_6 = 0;
        int64_t count_3_7 = 0;
        int64_t count_3_5 = 0;
        for (size_t i = 0; i < list_latency_there_ns.size(); i++) {
            if (list_latency_there_ns.at(i).at(0) < 220000) {
                count_8_3++;
            } else if (list_latency_there_ns.at(i).at(0) >= 220000 && list_latency_there_ns.at(i).at(0) < 330000) {
                count_6_3++;
            } else if (list_latency_there_ns.at(i).at(0) >= 330000 && list_latency_there_ns.at(i).at(0) < 440000) {
                count_7_3++;
            } else if (list_latency_there_ns.at(i).at(0) >= 440000) {
                count_5_3++;
            }
            if (list_latency_back_ns.at(i).at(0) < 220000) {
                count_3_8++;
            } else if (list_latency_back_ns.at(i).at(0) >= 220000 && list_latency_back_ns.at(i).at(0) < 330000) {
                count_3_6++;
            } else if (list_latency_back_ns.at(i).at(0) >= 330000 && list_latency_back_ns.at(i).at(0) < 440000) {
                count_3_7++;
            } else if (list_latency_back_ns.at(i).at(0) >= 440000) {
                count_3_5++;
            }
        }
        // std::cout << "Count there" << std::endl;
        // std::cout << count_8_3 << std::endl;
        // std::cout << count_6_3 << std::endl;
        // std::cout << count_7_3 << std::endl;
        // std::cout << count_5_3 << std::endl;
        // std::cout << "Count back" << std::endl;
        // std::cout << count_3_8 << std::endl;
        // std::cout << count_3_6 << std::endl;
        // std::cout << count_3_7 << std::endl;
        // std::cout << count_3_5 << std::endl;
        ASSERT_TRUE(count_8_3 >= 50);
        ASSERT_TRUE(count_6_3 >= 50);
        ASSERT_TRUE(count_7_3 >= 100);
        ASSERT_TRUE(count_5_3 >= 100);
        ASSERT_TRUE(count_3_8 >= 75);
        ASSERT_TRUE(count_3_6 >= 75);
        ASSERT_TRUE(count_3_7 >= 75);
        ASSERT_TRUE(count_3_5 >= 75);

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/udp_ping_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_pings.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_pings.txt");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////
