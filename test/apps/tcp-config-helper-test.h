/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

////////////////////////////////////////////////////////////////////////////////////////

class TcpConfigHelperDefaultTestCase : public TestCaseWithLogValidators
{
public:
    TcpConfigHelperDefaultTestCase () : TestCaseWithLogValidators ("tcp-config-helper default") {};
    const std::string test_run_dir = ".tmp-test-tcp-config-helper-ns3-default";
    
    void DoRun () {
        prepare_clean_run_dir(test_run_dir);

        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=1950000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "tcp_config=default" << std::endl;
        config_file << "enable_tcp_flow_scheduler=true" << std::endl;
        config_file << "tcp_flow_schedule_filename=\"tcp_flow_schedule.csv\"" << std::endl;
        config_file.close();

        std::ofstream tcp_flow_schedule_file;
        tcp_flow_schedule_file.open (test_run_dir + "/tcp_flow_schedule.csv");
        tcp_flow_schedule_file << "0,0,3,1000000,10000,," << std::endl; // Flow 0: 0 -> 3, 1MB, starting at t=10000ns
        tcp_flow_schedule_file.close();

        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-3,0-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
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

        // Configure TCP in only the basic sense
        TcpConfigHelper::Configure(basicSimulation);

        // Check all ns3-default TCP attributes
        ASSERT_EQUAL(GetInitialTimeValue("ns3::TcpSocketBase", "ClockGranularity").Get().GetNanoSeconds(), 1 * 1000 * 1000);  // 1ms
        ASSERT_EQUAL(GetInitialUintValue("ns3::TcpSocket", "InitialCwnd"), 10);
        ASSERT_EQUAL(GetInitialUintValue("ns3::TcpSocket", "SndBufSize"), 131072);
        ASSERT_EQUAL(GetInitialUintValue("ns3::TcpSocket", "RcvBufSize"), 131072);
        ASSERT_EQUAL(GetInitialUintValue("ns3::TcpSocket", "SegmentSize"), 536);
        ASSERT_EQUAL(GetInitialBooleanValue("ns3::TcpSocketBase", "Timestamp"), true);
        ASSERT_EQUAL(GetInitialBooleanValue("ns3::TcpSocketBase", "Sack"), true);
        ASSERT_EQUAL(GetInitialBooleanValue("ns3::TcpSocketBase", "WindowScaling"), true);
        ASSERT_EQUAL(GetInitialBooleanValue("ns3::TcpSocketState", "EnablePacing"), false);
        ASSERT_EQUAL(GetInitialUintValue("ns3::TcpSocket", "DelAckCount"), 2);
        ASSERT_EQUAL(GetInitialBooleanValue("ns3::TcpSocket", "TcpNoDelay"), true);
        ASSERT_EQUAL(GetInitialDoubleValue("ns3::TcpSocketBase", "MaxSegLifetime"), (int64_t) 120 * 1000 * 1000 * 1000 / 1e9); // 120s
        ASSERT_EQUAL(GetInitialTimeValue("ns3::TcpSocketBase", "MinRto").Get().GetNanoSeconds(), (int64_t) 1 * 1000 * 1000 * 1000); // 1s
        ASSERT_EQUAL(GetInitialTimeValue("ns3::RttEstimator", "InitialEstimation").Get().GetNanoSeconds(), (int64_t) 1 * 1000 * 1000 * 1000); // 1s
        ASSERT_EQUAL(GetInitialTimeValue("ns3::TcpSocket", "ConnTimeout").Get().GetNanoSeconds(), (int64_t) 3 * 1000 * 1000 * 1000); // 3s
        ASSERT_EQUAL(GetInitialTimeValue("ns3::TcpSocket", "DelAckTimeout").Get().GetNanoSeconds(), (int64_t) 200 * 1000 * 1000); // 200ms
        ASSERT_EQUAL(GetInitialTimeValue("ns3::TcpSocket", "PersistTimeout").Get().GetNanoSeconds(), (int64_t) 6 * 1000 * 1000 * 1000); // 6s

        // Schedule TCP flows
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology);

        // Run simulation
        basicSimulation->Run();

        // Finalize the simulation
        basicSimulation->Finalize();

        // Clean-up
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/tcp_flow_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);
        
    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpConfigHelperBasicTestCase : public TestCaseWithLogValidators
{
public:
    TcpConfigHelperBasicTestCase () : TestCaseWithLogValidators ("tcp-config-helper basic") {};
    const std::string test_run_dir = ".tmp-test-tcp-config-helper-basic";

    void DoRun () {
        prepare_clean_run_dir(test_run_dir);

        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=1950000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "tcp_config=basic" << std::endl;
        config_file << "enable_tcp_flow_scheduler=true" << std::endl;
        config_file << "tcp_flow_schedule_filename=\"tcp_flow_schedule.csv\"" << std::endl;
        config_file.close();

        std::ofstream tcp_flow_schedule_file;
        tcp_flow_schedule_file.open (test_run_dir + "/tcp_flow_schedule.csv");
        tcp_flow_schedule_file << "0,0,3,1000000,10000,," << std::endl; // Flow 0: 0 -> 3, 1MB, starting at t=10000ns
        tcp_flow_schedule_file.close();

        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-3,0-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
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

        // Configure TCP in only the basic sense
        TcpConfigHelper::Configure(basicSimulation);

        // Check all basic TCP attributes
        ASSERT_EQUAL(GetInitialTimeValue("ns3::TcpSocketBase", "ClockGranularity").Get().GetNanoSeconds(), 1 * 1000 * 1000);  // 1ms
        ASSERT_EQUAL(GetInitialUintValue("ns3::TcpSocket", "InitialCwnd"), 10);
        ASSERT_EQUAL(GetInitialUintValue("ns3::TcpSocket", "SndBufSize"), (int64_t) 131072 * 8192);
        ASSERT_EQUAL(GetInitialUintValue("ns3::TcpSocket", "RcvBufSize"), (int64_t) 131072 * 8192);
        ASSERT_EQUAL(GetInitialUintValue("ns3::TcpSocket", "SegmentSize"), 1380);
        ASSERT_EQUAL(GetInitialBooleanValue("ns3::TcpSocketBase", "Timestamp"), true);
        ASSERT_EQUAL(GetInitialBooleanValue("ns3::TcpSocketBase", "Sack"), true);
        ASSERT_EQUAL(GetInitialBooleanValue("ns3::TcpSocketBase", "WindowScaling"), true);
        ASSERT_EQUAL(GetInitialBooleanValue("ns3::TcpSocketState", "EnablePacing"), false);
        ASSERT_EQUAL(GetInitialUintValue("ns3::TcpSocket", "DelAckCount"), 2);
        ASSERT_EQUAL(GetInitialBooleanValue("ns3::TcpSocket", "TcpNoDelay"), true);
        ASSERT_EQUAL(GetInitialDoubleValue("ns3::TcpSocketBase", "MaxSegLifetime"), (int64_t) 120 * 1000 * 1000 * 1000 / 1e9); // 120s
        ASSERT_EQUAL(GetInitialTimeValue("ns3::TcpSocketBase", "MinRto").Get().GetNanoSeconds(), 200 * 1000 * 1000); // 200ms
        ASSERT_EQUAL(GetInitialTimeValue("ns3::RttEstimator", "InitialEstimation").Get().GetNanoSeconds(), (int64_t) 1 * 1000 * 1000 * 1000); // 1s
        ASSERT_EQUAL(GetInitialTimeValue("ns3::TcpSocket", "ConnTimeout").Get().GetNanoSeconds(), (int64_t) 1 * 1000 * 1000 * 1000); // 1s
        ASSERT_EQUAL(GetInitialTimeValue("ns3::TcpSocket", "DelAckTimeout").Get().GetNanoSeconds(), 200 * 1000 * 1000); // 200ms
        ASSERT_EQUAL(GetInitialTimeValue("ns3::TcpSocket", "PersistTimeout").Get().GetNanoSeconds(), (int64_t) 6 * 1000 * 1000 * 1000); // 6s

        // Schedule TCP flows
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology);

        // Run simulation
        basicSimulation->Run();

        // Finalize the simulation
        basicSimulation->Finalize();

        // Clean-up
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/tcp_flow_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpConfigHelperCustomTestCase : public TestCaseWithLogValidators
{
public:
    TcpConfigHelperCustomTestCase () : TestCaseWithLogValidators ("tcp-config-helper custom") {};
    const std::string test_run_dir = ".tmp-test-tcp-config-helper-custom";

    void DoRun () {
        prepare_clean_run_dir(test_run_dir);

        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=1950000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;

        config_file << "tcp_config=custom" << std::endl;
        config_file << "tcp_clock_granularity_ns=10003" << std::endl;
        config_file << "tcp_init_cwnd_pkt=24" << std::endl;
        config_file << "tcp_snd_buf_size_byte=46829" << std::endl;
        config_file << "tcp_rcv_buf_size_byte=2849952" << std::endl;
        config_file << "tcp_segment_size_byte=873" << std::endl;
        config_file << "tcp_opt_timestamp_enabled=false" << std::endl;
        config_file << "tcp_opt_sack_enabled=false" << std::endl;
        config_file << "tcp_opt_win_scaling_enabled=false" << std::endl;
        config_file << "tcp_opt_pacing_enabled=true" << std::endl;
        config_file << "tcp_delayed_ack_packet_count=5" << std::endl;
        config_file << "tcp_no_delay=false" << std::endl;
        config_file << "tcp_max_seg_lifetime_ns=33835959" << std::endl;
        config_file << "tcp_min_rto_ns=28395" << std::endl;
        config_file << "tcp_initial_rtt_estimate_ns=387395" << std::endl;
        config_file << "tcp_connection_timeout_ns=248294" << std::endl;
        config_file << "tcp_delayed_ack_timeout_ns=2835925" << std::endl;
        config_file << "tcp_persist_timeout_ns=47398502" << std::endl;

        config_file << "enable_tcp_flow_scheduler=true" << std::endl;
        config_file << "tcp_flow_schedule_filename=\"tcp_flow_schedule.csv\"" << std::endl;
        config_file.close();

        std::ofstream tcp_flow_schedule_file;
        tcp_flow_schedule_file.open (test_run_dir + "/tcp_flow_schedule.csv");
        tcp_flow_schedule_file << "0,0,3,1000000,10000,," << std::endl; // Flow 0: 0 -> 3, 1MB, starting at t=10000ns
        tcp_flow_schedule_file.close();

        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-3,0-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
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

        // Configure TCP in only the basic sense
        TcpConfigHelper::Configure(basicSimulation);

        // Schedule TCP flows
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology);

        // Check all custom set TCP attributes
        ASSERT_EQUAL(GetInitialTimeValue("ns3::TcpSocketBase", "ClockGranularity").Get().GetNanoSeconds(), 10003);
        ASSERT_EQUAL(GetInitialUintValue("ns3::TcpSocket", "InitialCwnd"), 24);
        ASSERT_EQUAL(GetInitialUintValue("ns3::TcpSocket", "SndBufSize"), 46829);
        ASSERT_EQUAL(GetInitialUintValue("ns3::TcpSocket", "RcvBufSize"), 2849952);
        ASSERT_EQUAL(GetInitialUintValue("ns3::TcpSocket", "SegmentSize"), 873);
        ASSERT_EQUAL(GetInitialBooleanValue("ns3::TcpSocketBase", "Timestamp"), false);
        ASSERT_EQUAL(GetInitialBooleanValue("ns3::TcpSocketBase", "Sack"), false);
        ASSERT_EQUAL(GetInitialBooleanValue("ns3::TcpSocketBase", "WindowScaling"), false);
        ASSERT_EQUAL(GetInitialBooleanValue("ns3::TcpSocketState", "EnablePacing"), true);
        ASSERT_EQUAL(GetInitialUintValue("ns3::TcpSocket", "DelAckCount"), 5);
        ASSERT_EQUAL(GetInitialBooleanValue("ns3::TcpSocket", "TcpNoDelay"), false);
        ASSERT_EQUAL(GetInitialDoubleValue("ns3::TcpSocketBase", "MaxSegLifetime"), 33835959 / 1e9);
        ASSERT_EQUAL(GetInitialTimeValue("ns3::TcpSocketBase", "MinRto").Get().GetNanoSeconds(), 28395);
        ASSERT_EQUAL(GetInitialTimeValue("ns3::RttEstimator", "InitialEstimation").Get().GetNanoSeconds(), 387395);
        ASSERT_EQUAL(GetInitialTimeValue("ns3::TcpSocket", "ConnTimeout").Get().GetNanoSeconds(), 248294);
        ASSERT_EQUAL(GetInitialTimeValue("ns3::TcpSocket", "DelAckTimeout").Get().GetNanoSeconds(), 2835925);
        ASSERT_EQUAL(GetInitialTimeValue("ns3::TcpSocket", "PersistTimeout").Get().GetNanoSeconds(), 47398502);

        // Run simulation
        basicSimulation->Run();

        // Finalize the simulation
        basicSimulation->Finalize();

        // Clean-up
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/tcp_flow_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpConfigHelperInvalidTestCase : public TestCaseWithLogValidators
{
public:
    TcpConfigHelperInvalidTestCase () : TestCaseWithLogValidators ("tcp-config-helper invalid") {};
    const std::string test_run_dir = ".tmp-test-tcp-config-helper-invalid";

    void DoRun () {
        prepare_clean_run_dir(test_run_dir);

        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=1000000000" << std::endl;
        config_file << "simulation_seed=987654321" << std::endl;
        config_file << "tcp_config=this_is_not_a_valid_tcp_config_type" << std::endl;
        config_file.close();

        // Create simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);

        // Configure TCP in only the basic sense
        ASSERT_EXCEPTION_MATCH_WHAT(TcpConfigHelper::Configure(basicSimulation), "Invalid TCP configuration type: this_is_not_a_valid_tcp_config_type");

        // Finalize the simulation
        basicSimulation->Finalize();

        // Clean-up
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////
