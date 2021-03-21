/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

////////////////////////////////////////////////////////////////////////////////////////

class ManualEndToEndTestCase : public TestCaseWithLogValidators {
public:
    ManualEndToEndTestCase () : TestCaseWithLogValidators ("manual end-to-end") {};
    const std::string test_run_dir = ".tmp-test-manual-end-to-end";

    void DoRun () {

        // Create temporary run directory
        prepare_clean_run_dir(test_run_dir);

        // Basic configuration
        std::ofstream config_file;
        config_file.open (test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=2000000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "tcp_config=basic" << std::endl;
        config_file.close();

        // Topology
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=2" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=30" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        TcpConfigHelper::Configure(basicSimulation);

        // Install burst server on node 1
        UdpBurstServerHelper burstServerHelper(
                InetSocketAddress(topology->GetNodes().Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 1029),
                basicSimulation->GetLogsDir()
        );
        ApplicationContainer udpServerApp = burstServerHelper.Install(topology->GetNodes().Get(1));
        udpServerApp.Start(NanoSeconds(0));
        udpServerApp.Get(0)->GetObject<UdpBurstServer>()->RegisterIncomingBurst(0, true);

        // Bursting 0 --> 1
        UdpBurstClientHelper source_burst0(
                InetSocketAddress(topology->GetNodes().Get(0)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 0),
                InetSocketAddress(topology->GetNodes().Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 1029),
                0,
                15.0,
                NanoSeconds(700000000),
                "",
                false,
                basicSimulation->GetLogsDir()
        );
        ApplicationContainer udpClientApp = source_burst0.Install(topology->GetNodes().Get(0));
        udpClientApp.Start(NanoSeconds(1000));

        // Install TCP flow server on all
        TcpFlowServerHelper tcpFlowServer(InetSocketAddress(Ipv4Address::GetAny(), 1024));
        ApplicationContainer app = tcpFlowServer.Install(topology->GetNodes().Get(0));
        app.Start(NanoSeconds(0));
        app = tcpFlowServer.Install(topology->GetNodes().Get(1));
        app.Start(NanoSeconds(0));

        // 0 --> 1
        TcpFlowClientHelper source0(
                InetSocketAddress(topology->GetNodes().Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 0),
                InetSocketAddress(topology->GetNodes().Get(1)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1024),
                0,
                1000000,
                "",
                true,
                basicSimulation->GetLogsDir()
        );
        app = source0.Install(topology->GetNodes().Get(0));
        app.Start(NanoSeconds(0));

        // 1 --> 0
        TcpFlowClientHelper source1(
                InetSocketAddress(topology->GetNodes().Get(1)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 0),
                InetSocketAddress(topology->GetNodes().Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1024),
                1,
                89999,
                "",
                true,
                basicSimulation->GetLogsDir()
        );
        app = source1.Install(topology->GetNodes().Get(1));
        app.Start(NanoSeconds(7000));

        // Install echo server on all nodes
        UdpPingServerHelper pingServerHelper(
            InetSocketAddress(topology->GetNodes().Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1025)
        );
        app = pingServerHelper.Install(topology->GetNodes().Get(0));
        app.Start(NanoSeconds(0));
        UdpPingServerHelper pingServerHelper2(
                InetSocketAddress(topology->GetNodes().Get(1)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1025)
        );
        app = pingServerHelper2.Install(topology->GetNodes().Get(1));
        app.Start(NanoSeconds(0));

        // Pinging 0 --> 1
        UdpPingClientHelper source_ping0(
                InetSocketAddress(topology->GetNodes().Get(0)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 0),
                InetSocketAddress(topology->GetNodes().Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 1025),
                0,
                NanoSeconds(100000000),
                NanoSeconds(1000000000),
                NanoSeconds(0),
                ""
        );
        app = source_ping0.Install(topology->GetNodes().Get(0));
        app.Start(NanoSeconds(100));

        // Pinging 1 --> 0
        UdpPingClientHelper source_ping1(
                InetSocketAddress(topology->GetNodes().Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 0),
                InetSocketAddress(topology->GetNodes().Get(0)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 1025),
                1,
                NanoSeconds(100000000),
                NanoSeconds(1000000000),
                NanoSeconds(0),
                ""
        );
        app = source_ping1.Install(topology->GetNodes().Get(1));
        app.Start(NanoSeconds(0));

        // Run simulation
        basicSimulation->Run();
        basicSimulation->Finalize();

        // Check finished.txt
        std::vector<std::string> finished_lines = read_file_direct(test_run_dir + "/logs_ns3/finished.txt");
        ASSERT_EQUAL(finished_lines.size(), 1);
        ASSERT_EQUAL(finished_lines[0], "Yes");

        // Check UDP burst completion information
        int64_t udp_client_sent = udpClientApp.Get(0)->GetObject<UdpBurstClient>()->GetSent();
        std::vector<std::tuple<int64_t, uint64_t>> incoming_1_info = udpServerApp.Get(0)->GetObject<UdpBurstServer>()->GetIncomingBurstsInformation();
        ASSERT_EQUAL_APPROX((double) udp_client_sent, 0.7 * 15 * 1000 * 1000 / 8.0 / 1500.0, 0.00001); // Everything will be sent
        ASSERT_EQUAL(incoming_1_info.size(), 1);
        ASSERT_EQUAL(std::get<0>(incoming_1_info.at(0)), 0);
        ASSERT_EQUAL_APPROX((double) std::get<1>(incoming_1_info.at(0)), 0.7 * 15 * 1000 * 1000 / 8.0 / 1500.0, 100.0); // Not everything will arrive due to TCP competition

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_0_progress.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_0_rtt.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_0_rto.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_0_cwnd.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_0_cwnd_inflated.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_0_ssthresh.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_0_inflight.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_0_state.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_0_cong_state.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_1_progress.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_1_rtt.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_1_rto.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_1_cwnd.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_1_cwnd_inflated.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_1_ssthresh.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_1_inflight.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_1_state.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_1_cong_state.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_burst_0_outgoing.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_burst_0_incoming.csv");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }

};

////////////////////////////////////////////////////////////////////////////////////////

class ManualTriggerStopExceptionsTestCase : public TestCaseWithLogValidators {
public:
    ManualTriggerStopExceptionsTestCase () : TestCaseWithLogValidators ("manual trigger-stop-exceptions") {};
    const std::string test_run_dir = ".tmp-test-manual-trigger-stop-exceptions";

    Ptr<BasicSimulation> basicSimulation;
    Ptr<TopologyPtop> topology;

    void setup_basic() {

        // Create temporary run directory
        prepare_clean_run_dir(test_run_dir);

        // Basic configuration
        std::ofstream config_file;
        config_file.open (test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=2000000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "tcp_config=basic" << std::endl;
        config_file.close();

        // Topology
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=2" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=30" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // Perform basic simulation
        basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        TcpConfigHelper::Configure(basicSimulation);

    }

    void finish_basic() {

        // Finalize
        basicSimulation->Finalize();

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/tcp_flow_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_pings.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_pings.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_0_progress.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_0_rtt.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_0_rto.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_0_cwnd.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_0_cwnd_inflated.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_0_ssthresh.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_0_inflight.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_0_state.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_0_cong_state.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_burst_0_outgoing.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_burst_0_incoming.csv");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }

    void DoRun () {

        ////////////////////////////////////////////////////////////////////
        // TCP flow server

        setup_basic();

        // Install TCP flow server on node 1
        TcpFlowServerHelper tcpFlowServer(InetSocketAddress(Ipv4Address::GetAny(), 1024));
        ApplicationContainer app = tcpFlowServer.Install(topology->GetNodes().Get(1));
        app.Start(NanoSeconds(0));
        app.Stop(Seconds(0.1));

        ASSERT_EXCEPTION_MATCH_WHAT(basicSimulation->Run(), "TCP flow server is not intended to be stopped after being started.");
        finish_basic();

        ////////////////////////////////////////////////////////////////////
        // TCP flow client

        setup_basic();

        // Install TCP flow server on node 1
        TcpFlowServerHelper tcpFlowServer2(InetSocketAddress(Ipv4Address::GetAny(), 1024));
        app = tcpFlowServer2.Install(topology->GetNodes().Get(1));
        app.Start(NanoSeconds(0));

        // Flow 0 --> 1
        TcpFlowClientHelper source0(
                InetSocketAddress(topology->GetNodes().Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 0),
                InetSocketAddress(topology->GetNodes().Get(1)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1024),
                0,
                1000000,
                "",
                true,
                basicSimulation->GetLogsDir()
        );
        app = source0.Install(topology->GetNodes().Get(0));
        app.Start(NanoSeconds(0));
        app.Stop(Seconds(0.1));

        ASSERT_EXCEPTION_MATCH_WHAT(basicSimulation->Run(), "TCP flow client cannot be stopped like a regular application, it finished only by the socket closing.");
        finish_basic();

        ////////////////////////////////////////////////////////////////////
        // UDP burst server

        setup_basic();

        // Install burst server on node 1
        UdpBurstServerHelper burstServerHelper(
                InetSocketAddress(topology->GetNodes().Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 1025),
                basicSimulation->GetLogsDir()
        );
        app = burstServerHelper.Install(topology->GetNodes().Get(1));
        app.Start(NanoSeconds(0));
        app.Stop(Seconds(0.1));

        ASSERT_EXCEPTION_MATCH_WHAT(basicSimulation->Run(), "UDP burst server is not permitted to be stopped.");
        finish_basic();

        ////////////////////////////////////////////////////////////////////
        // UDP burst client

        setup_basic();

        // Install burst server on node 1
        UdpBurstServerHelper burstServerHelper2(
                InetSocketAddress(topology->GetNodes().Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 1025),
                basicSimulation->GetLogsDir()
        );
        app = burstServerHelper2.Install(topology->GetNodes().Get(1));
        app.Start(NanoSeconds(0));
        app.Stop(Seconds(0.2));
        app.Get(0)->GetObject<UdpBurstServer>()->RegisterIncomingBurst(0, true);

        // Bursting 0 --> 1
        UdpBurstClientHelper source_burst0(
                InetSocketAddress(topology->GetNodes().Get(0)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 0),
                InetSocketAddress(topology->GetNodes().Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 1025),
                0,
                10.0,
                NanoSeconds(1000000000),
                "",
                false,
                basicSimulation->GetLogsDir()
        );
        app = source_burst0.Install(topology->GetNodes().Get(0));
        app.Start(NanoSeconds(100));
        app.Stop(Seconds(0.1));

        ASSERT_EXCEPTION_MATCH_WHAT(basicSimulation->Run(), "UDP burst client cannot be stopped like a regular application, it finished only when it is done sending.");
        finish_basic();

        ////////////////////////////////////////////////////////////////////
        // UDP ping server

        setup_basic();

        // Install ping server on node 1
        UdpPingServerHelper pingServerHelper(
            InetSocketAddress(topology->GetNodes().Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 1025)
        );
        app = pingServerHelper.Install(topology->GetNodes().Get(1));
        app.Start(NanoSeconds(0));
        app.Stop(Seconds(0.1));

        ASSERT_EXCEPTION_MATCH_WHAT(basicSimulation->Run(), "UDP ping server is not permitted to be stopped.");
        finish_basic();

        ////////////////////////////////////////////////////////////////////
        // UDP ping client

        setup_basic();

        // Install ping server on node 1
        UdpPingServerHelper pingServerHelper2(
                InetSocketAddress(topology->GetNodes().Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 1025)
        );
        app = pingServerHelper2.Install(topology->GetNodes().Get(1));
        app.Start(NanoSeconds(0));
        app.Stop(Seconds(0.1));

        // Pinging 0 --> 1
        UdpPingClientHelper source_ping0(
                InetSocketAddress(topology->GetNodes().Get(0)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 0),
                InetSocketAddress(topology->GetNodes().Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 1025),
                0,
                NanoSeconds(100000000),
                NanoSeconds(1000000000),
                NanoSeconds(0),
                ""
        );
        app = source_ping0.Install(topology->GetNodes().Get(0));
        app.Start(NanoSeconds(100));
        app.Stop(Seconds(0.1));

        ASSERT_EXCEPTION_MATCH_WHAT(basicSimulation->Run(), "UDP ping client cannot be stopped like a regular application, only of its own volition.");
        finish_basic();

    }

};

////////////////////////////////////////////////////////////////////////////////////////
