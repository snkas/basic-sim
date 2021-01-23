/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

////////////////////////////////////////////////////////////////////////////////////////

class UdpPingPingmeshTestCase : public TestCaseWithLogValidators {
public:
    UdpPingPingmeshTestCase(std::string s) : TestCaseWithLogValidators(s) {};
    std::string test_run_dir = ".tmp-test-udp-ping-pingmesh";
    
    std::vector<UdpPingInfo> write_pingmesh_to_udp_ping_schedule(int64_t pingmesh_interval_ns, std::string pingmesh_endpoints_pair_str, std::vector<int64_t> endpoints) {
        std::vector<UdpPingInfo> result;
        std::ofstream schedule_file;
        schedule_file.open (test_run_dir + "/udp_ping_schedule.csv");
        if (pingmesh_endpoints_pair_str == "all") {
            int n = 0;
            for (int64_t i : endpoints) {
                for (int64_t j : endpoints) {
                    if (i != j) {
                        schedule_file << n << "," << i << "," << j << "," << pingmesh_interval_ns << ",0,100000000000,0,," << std::endl;
                        result.push_back(UdpPingInfo(
                                n,
                                i,
                                j,
                                pingmesh_interval_ns,
                                0,
                                100000000000,
                                0,
                                "",
                                ""
                        ));
                        n += 1;
                    }
                }
            }
        } else {
            std::set<std::pair<int64_t, int64_t>> directed_pair_set = parse_set_directed_pair_positive_int64(pingmesh_endpoints_pair_str);
            int n = 0;
            for (std::pair<int64_t, int64_t> ab : directed_pair_set) {
                schedule_file << n << "," << ab.first << "," << ab.second << "," << pingmesh_interval_ns << ",0,100000000000,0,," << std::endl;
                result.push_back(UdpPingInfo(
                        n,
                        ab.first,
                        ab.second,
                        pingmesh_interval_ns,
                        0,
                        100000000000,
                        0,
                        "",
                        ""
                ));
                n += 1;
            }
        }
        schedule_file.close();
        return result;
    }

    void write_basic_config(int64_t simulation_end_time_ns) {
        std::ofstream config_file;
        config_file.open (test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << simulation_end_time_ns << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_udp_ping_scheduler=true" << std::endl;
        config_file << "udp_ping_schedule_filename=udp_ping_schedule.csv" << std::endl;
        config_file.close();
    }

    //
    // 3 - 4 - 5
    // |   |   |
    // 0 - 1 - 2
    //
    void write_six_topology() {
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=6" << std::endl;
        topology_file << "num_undirected_edges=7" << std::endl;
        topology_file << "switches=set(0,1,2,3,4,5)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1,2,3,5)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,3-4,4-5,0-3,1-4,2-5)" << std::endl;
        topology_file << "link_channel_delay_ns=50000000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=10000" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(10000p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
    }

};

////////////////////////////////////////////////////////////////////////////////////////

class UdpPingPingmeshNineAllTestCase : public UdpPingPingmeshTestCase
{
public:
    UdpPingPingmeshNineAllTestCase () : UdpPingPingmeshTestCase ("udp-ping-pingmesh nine-all") {};

    void DoRun () {
        test_run_dir = ".tmp-test-udp-ping-pingmesh-nine-all";
        prepare_clean_run_dir(test_run_dir);

        // 5 seconds, every 100ms a ping
        write_basic_config(5000000000);
        std::vector<int64_t> endpoints;
        endpoints.push_back(0);
        endpoints.push_back(1);
        endpoints.push_back(2);
        endpoints.push_back(3);
        endpoints.push_back(5);
        std::vector<UdpPingInfo> udp_ping_schedule = write_pingmesh_to_udp_ping_schedule(100000000, "all", endpoints);
        write_six_topology();

        std::map<std::pair<int64_t, int64_t>, int64_t> pair_to_expected_latency;
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(0, 1), 50000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(0, 2), 100000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(0, 3), 50000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(0, 5), 150000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(1, 0), 50000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(1, 2), 50000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(1, 3), 100000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(1, 5), 100000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(2, 0), 100000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(2, 1), 50000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(2, 3), 150000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(2, 5), 50000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(3, 0), 50000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(3, 1), 100000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(3, 2), 150000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(3, 5), 100000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(5, 0), 150000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(5, 1), 100000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(5, 2), 50000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(5, 3), 100000000));

        std::vector<std::pair<int64_t, int64_t>> ping_pairs;
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 6; j++) {
                if (i != 4 && j != 4 && i != j) {
                    ping_pairs.push_back(std::make_pair(i, j));
                }
            }
        }

        // To store the results
        std::vector<std::vector<int64_t>> list_latency_there_ns;
        std::vector<std::vector<int64_t>> list_latency_back_ns;
        std::vector<std::vector<int64_t>> list_rtt_ns;

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        UdpPingScheduler udpPingScheduler(basicSimulation, topology);
        basicSimulation->Run();
        udpPingScheduler.WriteResults();
        basicSimulation->Finalize();

        // Perform the run and get results
        validate_udp_ping_logs(
                5000000000,
                test_run_dir,
                udp_ping_schedule,
                list_latency_there_ns,
                list_latency_back_ns,
                list_rtt_ns
                );

        // Check the actual values
        for (size_t i = 0; i < ping_pairs.size(); i++) {
            int invalid_count = 0;
            for (int64_t latency_there_ns : list_latency_there_ns.at(i)) {
                if (latency_there_ns != -1) {
                    ASSERT_EQUAL_APPROX(latency_there_ns, pair_to_expected_latency.at(ping_pairs.at(i)), 500);
                } else {
                    invalid_count += 1;
                }
            }
            for (int64_t latency_back_ns : list_latency_back_ns.at(i)) {
                if (latency_back_ns != -1) {
                    ASSERT_EQUAL_APPROX(latency_back_ns, pair_to_expected_latency.at(ping_pairs.at(i)), 500);
                }
            }
            for (int64_t rtt_ns : list_rtt_ns.at(i)) {
                if (rtt_ns != -1) {
                    ASSERT_EQUAL_APPROX(rtt_ns, 2 * pair_to_expected_latency.at(ping_pairs.at(i)), 500);
                }
            }
            ASSERT_TRUE(invalid_count <= 3);
            ASSERT_EQUAL(list_latency_there_ns.at(i).size(), 50);
            ASSERT_EQUAL(list_latency_back_ns.at(i).size(), 50);
            ASSERT_EQUAL(list_rtt_ns.at(i).size(), 50);
        }

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

class UdpPingPingmeshFivePairsTestCase : public UdpPingPingmeshTestCase
{
public:
    UdpPingPingmeshFivePairsTestCase () : UdpPingPingmeshTestCase ("udp-ping-pingmesh five-pairs") {};

    void DoRun () {
        test_run_dir = ".tmp-test-udp-ping-pingmesh-five-pairs";
        prepare_clean_run_dir(test_run_dir);

        // 5 seconds, every 100ms a ping
        write_basic_config(5000000000);
        std::vector<int64_t> endpoints;
        endpoints.push_back(0);
        endpoints.push_back(1);
        endpoints.push_back(2);
        endpoints.push_back(3);
        endpoints.push_back(5);
        std::vector<UdpPingInfo> udp_ping_schedule = write_pingmesh_to_udp_ping_schedule(100000000, "set(2->1, 1->2, 0->5, 5->2, 3->1)", endpoints);
        write_six_topology();
        std::map<std::pair<int64_t, int64_t>, int64_t> pair_to_expected_latency;
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(0, 5), 150000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(1, 2), 50000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(2, 1), 50000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(3, 1), 100000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(5, 2), 50000000));

        std::vector<std::pair<int64_t, int64_t>> ping_pairs;
        ping_pairs.push_back(std::make_pair(0, 5));
        ping_pairs.push_back(std::make_pair(1, 2));
        ping_pairs.push_back(std::make_pair(2, 1));
        ping_pairs.push_back(std::make_pair(3, 1));
        ping_pairs.push_back(std::make_pair(5, 2));

        // To store the results
        std::vector<std::vector<int64_t>> list_latency_there_ns;
        std::vector<std::vector<int64_t>> list_latency_back_ns;
        std::vector<std::vector<int64_t>> list_rtt_ns;

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        UdpPingScheduler udpPingScheduler(basicSimulation, topology);
        basicSimulation->Run();
        udpPingScheduler.WriteResults();
        basicSimulation->Finalize();

        // Perform the run and get results
        validate_udp_ping_logs(
                5000000000,
                test_run_dir,
                udp_ping_schedule,
                list_latency_there_ns,
                list_latency_back_ns,
                list_rtt_ns
        );

        // Check the actual values
        for (size_t i = 0; i < ping_pairs.size(); i++) {
            int invalid_count = 0;
            for (int64_t latency_there_ns : list_latency_there_ns.at(i)) {
                if (latency_there_ns != -1) {
                    ASSERT_EQUAL_APPROX(latency_there_ns, pair_to_expected_latency.at(ping_pairs.at(i)), 500);
                } else {
                    invalid_count += 1;
                }
            }
            for (int64_t latency_back_ns : list_latency_back_ns.at(i)) {
                if (latency_back_ns != -1) {
                    ASSERT_EQUAL_APPROX(latency_back_ns, pair_to_expected_latency.at(ping_pairs.at(i)), 500);
                }
            }
            for (int64_t rtt_ns : list_rtt_ns.at(i)) {
                if (rtt_ns != -1) {
                    ASSERT_EQUAL_APPROX(rtt_ns, 2 * pair_to_expected_latency.at(ping_pairs.at(i)), 500);
                }
            }
            ASSERT_TRUE(invalid_count <= 3);
            ASSERT_EQUAL(list_latency_there_ns.at(i).size(), 50);
            ASSERT_EQUAL(list_latency_back_ns.at(i).size(), 50);
            ASSERT_EQUAL(list_rtt_ns.at(i).size(), 50);
        }

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

class UdpPingPingmeshCompetitionTcpTestCase : public UdpPingPingmeshTestCase
{
public:
    UdpPingPingmeshCompetitionTcpTestCase () : UdpPingPingmeshTestCase ("udp-ping-pingmesh competition-tcp") {};

    void DoRun () {
        test_run_dir = ".tmp-test-udp-ping-pingmesh-competition-tcp";
        prepare_clean_run_dir(test_run_dir);

        // 5 seconds, every 100ms a ping
        std::ofstream config_file;
        config_file.open (test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=5000000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_udp_ping_scheduler=true" << std::endl;
        config_file << "udp_ping_schedule_filename=udp_ping_schedule.csv" << std::endl;
        config_file << "enable_tcp_flow_scheduler=true" << std::endl;
        config_file << "tcp_flow_schedule_filename=tcp_flow_schedule.csv" << std::endl;
        config_file.close();

        std::ofstream tcp_flow_schedule_file;
        tcp_flow_schedule_file.open (test_run_dir + "/tcp_flow_schedule.csv");
        tcp_flow_schedule_file << "0,0,1,1000000000,0,," << std::endl;
        tcp_flow_schedule_file.close();

        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=2" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1)" << std::endl;
        topology_file << "link_channel_delay_ns=100000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=10" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100000B)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        std::vector<std::pair<int64_t, int64_t>> ping_pairs;
        ping_pairs.push_back(std::make_pair(0, 1));
        ping_pairs.push_back(std::make_pair(1, 0));

        std::vector<int64_t> endpoints;
        endpoints.push_back(0);
        endpoints.push_back(1);
        std::vector<UdpPingInfo> result = write_pingmesh_to_udp_ping_schedule(100000000, "all", endpoints);

        // To store the results
        std::vector<std::vector<int64_t>> list_latency_there_ns;
        std::vector<std::vector<int64_t>> list_latency_back_ns;
        std::vector<std::vector<int64_t>> list_rtt_ns;

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        UdpPingScheduler udpPingScheduler(basicSimulation, topology);
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology);
        basicSimulation->Run();
        udpPingScheduler.WriteResults();
        tcpFlowScheduler.WriteResults();
        basicSimulation->Finalize();

        // Perform the run and get results
        validate_udp_ping_logs(
                5000000000,
                test_run_dir,
                result,
                list_latency_there_ns,
                list_latency_back_ns,
                list_rtt_ns
        );

        // Check the actual values
        for (size_t i = 0; i < ping_pairs.size(); i++) {
            int invalid_count = 0;
            for (int64_t latency_there_ns : list_latency_there_ns.at(i)) {
                if (latency_there_ns != -1) {
                    ASSERT_TRUE(latency_there_ns >= 100000 && latency_there_ns <= 81300000);
                } else {
                    invalid_count += 1;
                }
            }
            for (int64_t latency_back_ns : list_latency_back_ns.at(i)) {
                if (latency_back_ns != -1) {
                    ASSERT_TRUE(latency_back_ns >= 100000 && latency_back_ns <= 81300000);
                }
            }
            for (int64_t rtt_ns : list_rtt_ns.at(i)) {
                if (rtt_ns != -1) {
                    ASSERT_TRUE(rtt_ns >= 200000 && rtt_ns <= 162600000);
                }
            }
            ASSERT_EQUAL(list_latency_there_ns.at(i).size(), 50);
            ASSERT_EQUAL(list_latency_back_ns.at(i).size(), 50);
            ASSERT_EQUAL(list_rtt_ns.at(i).size(), 50);
        }

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/udp_ping_schedule.csv");
        remove_file_if_exists(test_run_dir + "/tcp_flow_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_pings.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_pings.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.txt");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpPingPingmeshOnePairNoRecTestCase : public UdpPingPingmeshTestCase
{
public:
    UdpPingPingmeshOnePairNoRecTestCase () : UdpPingPingmeshTestCase ("udp-ping-pingmesh one-pair-no-rec") {};

    void DoRun () {
        test_run_dir = ".tmp-test-udp-ping-pingmesh-one-pair-no-rec";
        prepare_clean_run_dir(test_run_dir);

        // 0.049999999 seconds, every 10ms a ping, but takes 50ms to return back
        write_basic_config(49999999);
        std::vector<int64_t> endpoints;
        endpoints.push_back(0);
        endpoints.push_back(1);
        endpoints.push_back(2);
        endpoints.push_back(3);
        endpoints.push_back(5);
        std::vector<UdpPingInfo> udp_ping_schedule = write_pingmesh_to_udp_ping_schedule(10000000, "set(2->1)", endpoints);
        write_six_topology();

        // Ping pairs
        std::vector<std::pair<int64_t, int64_t>> ping_pairs;
        ping_pairs.push_back(std::make_pair(2, 1));

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

        // Perform the run and get results
        validate_udp_ping_logs(
                49999999,
                test_run_dir,
                udp_ping_schedule,
                list_latency_there_ns,
                list_latency_back_ns,
                list_rtt_ns
        );

        // Check the actual values
        for (size_t i = 0; i < ping_pairs.size(); i++) {
            for (int64_t latency_there_ns : list_latency_there_ns.at(i)) {
                ASSERT_EQUAL(latency_there_ns, -1);
            }
            for (int64_t latency_back_ns : list_latency_back_ns.at(i)) {
                ASSERT_EQUAL(latency_back_ns, -1);
            }
            for (int64_t rtt_ns : list_rtt_ns.at(i)) {
                ASSERT_EQUAL(rtt_ns, -1);
            }
            ASSERT_EQUAL(list_latency_there_ns.at(i).size(), 5);
            ASSERT_EQUAL(list_latency_back_ns.at(i).size(), 5);
            ASSERT_EQUAL(list_rtt_ns.at(i).size(), 5);
        }

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

class UdpPingPingmeshInvalidPairATestCase : public UdpPingPingmeshTestCase
{
public:
    UdpPingPingmeshInvalidPairATestCase () : UdpPingPingmeshTestCase ("udp-ping-pingmesh invalid-pair-a") {};

    void DoRun () {
        test_run_dir = ".tmp-test-udp-ping-pingmesh-invalid-pair-a";
        prepare_clean_run_dir(test_run_dir);

        // 5 seconds, every 100ms a ping
        write_basic_config(5000000000);
        std::vector<int64_t> endpoints;
        endpoints.push_back(0);
        endpoints.push_back(1);
        endpoints.push_back(2);
        endpoints.push_back(3);
        endpoints.push_back(5);
        write_pingmesh_to_udp_ping_schedule(100000000, "set(6->3)", endpoints);
        write_six_topology();

        // Initialize basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        ASSERT_EXCEPTION(UdpPingScheduler(basicSimulation, topology));

        basicSimulation->Finalize();

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/udp_ping_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpPingPingmeshInvalidPairBTestCase : public UdpPingPingmeshTestCase
{
public:
    UdpPingPingmeshInvalidPairBTestCase () : UdpPingPingmeshTestCase ("udp-ping-pingmesh invalid-pair-b") {};

    void DoRun () {
        test_run_dir = ".tmp-test-udp-ping-pingmesh-invalid-pair-b";
        prepare_clean_run_dir(test_run_dir);

        // 5 seconds, every 100ms a ping
        write_basic_config(5000000000);
        std::vector<int64_t> endpoints;
        endpoints.push_back(0);
        endpoints.push_back(1);
        endpoints.push_back(2);
        endpoints.push_back(3);
        endpoints.push_back(5);
        write_pingmesh_to_udp_ping_schedule(100000000, "set(2->6)", endpoints);
        write_six_topology();

        // Initialize basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        ASSERT_EXCEPTION(UdpPingScheduler(basicSimulation, topology));

        basicSimulation->Finalize();

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/udp_ping_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpPingPingmeshNotEnabledTestCase : public UdpPingPingmeshTestCase
{
public:
    UdpPingPingmeshNotEnabledTestCase () : UdpPingPingmeshTestCase ("udp-ping-pingmesh not-enabled") {};

    void DoRun () {

        // Run directory
        test_run_dir = ".tmp-test-udp-ping-pingmesh-not-enabled";
        prepare_clean_run_dir(test_run_dir);

        // Config file
        std::ofstream config_file;
        config_file.open (test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << 1000000000 << std::endl;
        config_file << "simulation_seed=" << 1111111111 << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_udp_ping_scheduler=false" << std::endl;
        config_file.close();

        // Topology
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(2)" << std::endl;
        topology_file << "switches_which_are_tors=set(2)" << std::endl;
        topology_file << "servers=set(0, 1, 3)" << std::endl;
        topology_file << "undirected_edges=set(0-2,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=200000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=30" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=iid_uniform_random_pkt(0.0)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        UdpPingScheduler udpPingScheduler(basicSimulation, topology);
        basicSimulation->Run();
        udpPingScheduler.WriteResults();
        basicSimulation->Finalize();

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/udp_ping_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }

};

////////////////////////////////////////////////////////////////////////////////////////
