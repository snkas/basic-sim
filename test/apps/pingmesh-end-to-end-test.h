/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

////////////////////////////////////////////////////////////////////////////////////////

class PingmeshEndToEndTestCase : public TestCaseWithLogValidators {
public:
    PingmeshEndToEndTestCase(std::string s) : TestCaseWithLogValidators(s) {};
    const std::string temp_dir = ".tmp-pingmesh-end-to-end-test";

    void prepare_test_dir() {
        mkdir_if_not_exists(temp_dir);
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
    }

    void write_basic_config(int64_t simulation_end_time_ns, int64_t pingmesh_interval_ns, std::string pingmesh_endpoint_pairs) {
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << simulation_end_time_ns << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_pingmesh_scheduler=true" << std::endl;
        config_file << "pingmesh_interval_ns=" << pingmesh_interval_ns << std::endl;
        config_file << "pingmesh_endpoint_pairs=" << pingmesh_endpoint_pairs << std::endl;
        config_file.close();
    }

    //
    // 3 - 4 - 5
    // |   |   |
    // 0 - 1 - 2
    //
    void write_six_topology() {
        std::ofstream topology_file;
        topology_file.open (temp_dir + "/topology.properties");
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

class PingmeshEndToEndNineAllTestCase : public PingmeshEndToEndTestCase
{
public:
    PingmeshEndToEndNineAllTestCase () : PingmeshEndToEndTestCase ("pingmesh-end-to-end nine-all") {};

    void DoRun () {
        prepare_test_dir();

        // 5 seconds, every 100ms a ping
        write_basic_config(5000000000, 100000000, "all");
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
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        PingmeshScheduler pingmeshScheduler(basicSimulation, topology);
        basicSimulation->Run();
        pingmeshScheduler.WriteResults();
        basicSimulation->Finalize();

        // Perform the run and get results
        validate_pingmesh_logs(
                5000000000,
                temp_dir,
                ping_pairs,
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
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/pingmesh.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/pingmesh.txt");
        remove_dir_if_exists(temp_dir + "/logs_ns3");
        remove_dir_if_exists(temp_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PingmeshEndToEndFivePairsTestCase : public PingmeshEndToEndTestCase
{
public:
    PingmeshEndToEndFivePairsTestCase () : PingmeshEndToEndTestCase ("pingmesh-end-to-end five-pairs") {};

    void DoRun () {
        prepare_test_dir();

        // 5 seconds, every 100ms a ping
        write_basic_config(5000000000, 100000000, "set(2->1, 1->2, 0->5, 5->2, 3->1)");
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
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        PingmeshScheduler pingmeshScheduler(basicSimulation, topology);
        basicSimulation->Run();
        pingmeshScheduler.WriteResults();
        basicSimulation->Finalize();

        // Perform the run and get results
        validate_pingmesh_logs(
                5000000000,
                temp_dir,
                ping_pairs,
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
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/pingmesh.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/pingmesh.txt");
        remove_dir_if_exists(temp_dir + "/logs_ns3");
        remove_dir_if_exists(temp_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PingmeshEndToEndCompetitionTcpTestCase : public PingmeshEndToEndTestCase
{
public:
    PingmeshEndToEndCompetitionTcpTestCase () : PingmeshEndToEndTestCase ("pingmesh-end-to-end competition-tcp") {};

    void DoRun () {
        prepare_test_dir();

        // 5 seconds, every 100ms a ping
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=5000000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_pingmesh_scheduler=true" << std::endl;
        config_file << "pingmesh_interval_ns=100000000" << std::endl;
        config_file << "pingmesh_endpoint_pairs=all" << std::endl;
        config_file << "enable_tcp_flow_scheduler=true" << std::endl;
        config_file << "tcp_flow_schedule_filename=tcp_flow_schedule.csv" << std::endl;
        config_file.close();

        std::ofstream tcp_flow_schedule_file;
        tcp_flow_schedule_file.open (temp_dir + "/tcp_flow_schedule.csv");
        tcp_flow_schedule_file << "0,0,1,1000000000,0,," << std::endl;
        tcp_flow_schedule_file.close();

        std::ofstream topology_file;
        topology_file.open (temp_dir + "/topology.properties");
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

        // To store the results
        std::vector<std::vector<int64_t>> list_latency_there_ns;
        std::vector<std::vector<int64_t>> list_latency_back_ns;
        std::vector<std::vector<int64_t>> list_rtt_ns;

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        PingmeshScheduler pingmeshScheduler(basicSimulation, topology);
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology);
        basicSimulation->Run();
        pingmeshScheduler.WriteResults();
        tcpFlowScheduler.WriteResults();
        basicSimulation->Finalize();

        // Perform the run and get results
        validate_pingmesh_logs(
                5000000000,
                temp_dir,
                ping_pairs,
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
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/tcp_flow_schedule.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/pingmesh.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/pingmesh.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/tcp_flows.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/tcp_flows.txt");
        remove_dir_if_exists(temp_dir + "/logs_ns3");
        remove_dir_if_exists(temp_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PingmeshEndToEndOnePairNoRecTestCase : public PingmeshEndToEndTestCase
{
public:
    PingmeshEndToEndOnePairNoRecTestCase () : PingmeshEndToEndTestCase ("pingmesh-end-to-end one-pair-no-rec") {};

    void DoRun () {
        prepare_test_dir();

        // 0.049999999 seconds, every 10ms a ping, but takes 50ms to return back
        write_basic_config(49999999, 10000000, "set(2->1)");
        write_six_topology();

        // Ping pairs
        std::vector<std::pair<int64_t, int64_t>> ping_pairs;
        ping_pairs.push_back(std::make_pair(2, 1));

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        PingmeshScheduler pingmeshScheduler(basicSimulation, topology);
        basicSimulation->Run();
        pingmeshScheduler.WriteResults();
        basicSimulation->Finalize();

        // To store the results
        std::vector<std::vector<int64_t>> list_latency_there_ns;
        std::vector<std::vector<int64_t>> list_latency_back_ns;
        std::vector<std::vector<int64_t>> list_rtt_ns;

        // Perform the run and get results
        validate_pingmesh_logs(
                49999999,
                temp_dir,
                ping_pairs,
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
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/pingmesh.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/pingmesh.txt");
        remove_dir_if_exists(temp_dir + "/logs_ns3");
        remove_dir_if_exists(temp_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PingmeshEndToEndInvalidPairEqualTestCase : public PingmeshEndToEndTestCase
{
public:
    PingmeshEndToEndInvalidPairEqualTestCase () : PingmeshEndToEndTestCase ("pingmesh-end-to-end invalid-pair-equal") {};

    void DoRun () {
        prepare_test_dir();

        // 5 seconds, every 100ms a ping
        write_basic_config(5000000000, 100000000, "set(2->2)");
        write_six_topology();

        // Initialize basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        ASSERT_EXCEPTION(PingmeshScheduler(basicSimulation, topology));

        basicSimulation->Finalize();

        // Make sure these are removed
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(temp_dir + "/logs_ns3");
        remove_dir_if_exists(temp_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PingmeshEndToEndInvalidPairATestCase : public PingmeshEndToEndTestCase
{
public:
    PingmeshEndToEndInvalidPairATestCase () : PingmeshEndToEndTestCase ("pingmesh-end-to-end invalid-pair-a") {};

    void DoRun () {
        prepare_test_dir();

        // 5 seconds, every 100ms a ping
        write_basic_config(5000000000, 100000000, "set(6->3)");
        write_six_topology();

        // Initialize basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        ASSERT_EXCEPTION(PingmeshScheduler(basicSimulation, topology));

        basicSimulation->Finalize();

        // Make sure these are removed
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(temp_dir + "/logs_ns3");
        remove_dir_if_exists(temp_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PingmeshEndToEndInvalidPairBTestCase : public PingmeshEndToEndTestCase
{
public:
    PingmeshEndToEndInvalidPairBTestCase () : PingmeshEndToEndTestCase ("pingmesh-end-to-end invalid-pair-b") {};

    void DoRun () {
        prepare_test_dir();

        // 5 seconds, every 100ms a ping
        write_basic_config(5000000000, 100000000, "set(2->6)");
        write_six_topology();

        // Initialize basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        ASSERT_EXCEPTION(PingmeshScheduler(basicSimulation, topology));

        basicSimulation->Finalize();

        // Make sure these are removed
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(temp_dir + "/logs_ns3");
        remove_dir_if_exists(temp_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PingmeshEndToEndInvalidDuplicatePairTestCase : public PingmeshEndToEndTestCase
{
public:
    PingmeshEndToEndInvalidDuplicatePairTestCase () : PingmeshEndToEndTestCase ("pingmesh-end-to-end invalid-duplicate-pair") {};

    void DoRun () {
        prepare_test_dir();

        // 5 seconds, every 100ms a ping
        write_basic_config(5000000000, 100000000, "set(2->3,3-> 5,3->1,3->5,1->0)");
        write_six_topology();

        // Initialize basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        ASSERT_EXCEPTION(PingmeshScheduler(basicSimulation, topology));

        basicSimulation->Finalize();

        // Make sure these are removed
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(temp_dir + "/logs_ns3");
        remove_dir_if_exists(temp_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PingmeshEndToEndNotEnabledTestCase : public PingmeshEndToEndTestCase
{
public:
    PingmeshEndToEndNotEnabledTestCase () : PingmeshEndToEndTestCase ("pingmesh-end-to-end not-enabled") {};

    void DoRun () {

        // Run directory
        prepare_test_dir();

        // Config file
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << 1000000000 << std::endl;
        config_file << "simulation_seed=" << 1111111111 << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_pingmesh_scheduler=false" << std::endl;
        config_file.close();

        // Topology
        std::ofstream topology_file;
        topology_file.open (temp_dir + "/topology.properties");
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
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        PingmeshScheduler pingmeshScheduler(basicSimulation, topology);
        basicSimulation->Run();
        pingmeshScheduler.WriteResults();
        basicSimulation->Finalize();

        // Make sure these are removed
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(temp_dir + "/logs_ns3");
        remove_dir_if_exists(temp_dir);

    }

};

////////////////////////////////////////////////////////////////////////////////////////
