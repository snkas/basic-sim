/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstEndToEndTestCase : public TestCaseWithLogValidators {
public:
    UdpBurstEndToEndTestCase(std::string s) : TestCaseWithLogValidators(s) {};
    const std::string temp_dir = ".tmp-udp-burst-end-to-end-test";

    void prepare_test_dir() {
        mkdir_if_not_exists(temp_dir);
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/udp_burst_schedule.csv");
    }

    void write_basic_config(int64_t simulation_end_time_ns, int64_t simulation_seed, std::string detailed_logging_enabled_for) {
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << simulation_end_time_ns << std::endl;
        config_file << "simulation_seed=" << simulation_seed << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_udp_burst_scheduler=true" << std::endl;
        config_file << "udp_burst_schedule_filename=\"udp_burst_schedule.csv\"" << std::endl;
        config_file << "udp_burst_enable_logging_for_udp_burst_ids=set(" << detailed_logging_enabled_for << ")" << std::endl;
        config_file.close();
    }

    void write_single_topology(double link_net_device_data_rate_megabit_per_s, int64_t link_channel_delay_ns) {
        std::ofstream topology_file;
        topology_file.open (temp_dir + "/topology.properties");
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

    void test_run_and_validate_udp_burst_logs(
            int64_t simulation_end_time_ns,
            std::string temp_dir, 
            std::vector<UdpBurstInfo> write_schedule,
            std::vector<double>& list_outgoing_rate_megabit_per_s,
            std::vector<double>& list_incoming_rate_megabit_per_s
    ) {

        // Make sure these are removed
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/udp_bursts.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/udp_bursts.csv");

        // Write schedule file
        std::ofstream schedule_file;
        schedule_file.open (temp_dir + "/udp_burst_schedule.csv");
        for (UdpBurstInfo entry : write_schedule) {
            schedule_file
                    << entry.GetUdpBurstId() << ","
                    << entry.GetFromNodeId() << ","
                    << entry.GetToNodeId() << ","
                    << entry.GetTargetRateMegabitPerSec() << ","
                    << entry.GetStartTimeNs() << ","
                    << entry.GetDurationNs() << ","
                    << entry.GetAdditionalParameters() << ","
                    << entry.GetMetadata()
                    << std::endl;
        }
        schedule_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        TcpOptimizer::OptimizeUsingWorstCaseRtt(basicSimulation, topology->GetWorstCaseRttEstimateNs());
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology);
        basicSimulation->Run();
        udpBurstScheduler.WriteResults();
        basicSimulation->Finalize();

        // Check finished.txt
        std::vector<std::string> finished_lines = read_file_direct(temp_dir + "/logs_ns3/finished.txt");
        ASSERT_EQUAL(finished_lines.size(), 1);
        ASSERT_EQUAL(finished_lines[0], "Yes");

        // Validate UDP burst logs
        validate_udp_burst_logs(
                simulation_end_time_ns,
                temp_dir,
                write_schedule,
                list_outgoing_rate_megabit_per_s,
                list_incoming_rate_megabit_per_s
       );

        // Make sure these are removed
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/udp_burst_schedule.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/udp_bursts_outgoing.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/udp_bursts_outgoing.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/udp_bursts_incoming.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/udp_bursts_incoming.txt");
        for (UdpBurstInfo entry : write_schedule) {
            remove_file_if_exists(temp_dir + "/logs_ns3/udp_burst_" + std::to_string(entry.GetUdpBurstId()) + "_outgoing.csv");
            remove_file_if_exists(temp_dir + "/logs_ns3/udp_burst_" + std::to_string(entry.GetUdpBurstId()) + "_incoming.csv");
        }
        remove_dir_if_exists(temp_dir + "/logs_ns3");
        remove_dir_if_exists(temp_dir);

    }

};

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstEndToEndOneToOneEqualStartTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndOneToOneEqualStartTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end 1-to-1 equal-start") {};

    void DoRun () {
        prepare_test_dir();

        int64_t simulation_end_time_ns = 5000000000;

        // One-to-one, 5s, 10.0 Mbit/s, 100 microseconds delay
        write_basic_config(simulation_end_time_ns, 123456, "0,1");
        write_single_topology(10.0, 100000);

        // A UDP burst each way
        std::vector<UdpBurstInfo> schedule;
        schedule.push_back(UdpBurstInfo(0, 0, 1, 9, 1000000000, 3000000000, "", "abc"));
        schedule.push_back(UdpBurstInfo(1, 1, 0, 9, 1000000000, 3000000000, "", ""));

        // Perform the run
        std::vector<double> list_outgoing_rate_megabit_per_s;
        std::vector<double> list_incoming_rate_megabit_per_s;
        test_run_and_validate_udp_burst_logs(simulation_end_time_ns, temp_dir, schedule, list_outgoing_rate_megabit_per_s, list_incoming_rate_megabit_per_s);

        // As they are started at the same point and should have the same behavior, progress should be equal
        ASSERT_EQUAL(list_outgoing_rate_megabit_per_s.size(), 2);
        ASSERT_EQUAL(list_outgoing_rate_megabit_per_s.at(0), list_outgoing_rate_megabit_per_s.at(1));
        ASSERT_TRUE(list_outgoing_rate_megabit_per_s.at(0) >= 8.5);
        ASSERT_EQUAL(list_incoming_rate_megabit_per_s.size(), 2);
        ASSERT_EQUAL(list_incoming_rate_megabit_per_s.at(0), list_incoming_rate_megabit_per_s.at(1));
        ASSERT_TRUE(list_incoming_rate_megabit_per_s.at(0) >= 8.5);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstEndToEndSingleOverflowTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndSingleOverflowTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end single-overflow") {};

    void DoRun () {
        prepare_test_dir();

        int64_t simulation_end_time_ns = 5000000000;

        // One-to-one, 5s, 10.0 Mbit/s, 100 microseconds delay
        write_basic_config(simulation_end_time_ns, 123456, "0");
        write_single_topology(10.0, 100000);

        // A UDP burst each way
        std::vector<UdpBurstInfo> schedule;
        schedule.push_back(UdpBurstInfo(0, 0, 1, 25, 1000000000, 3000000000, "", "abc"));

        // Perform the run
        std::vector<double> list_outgoing_rate_megabit_per_s;
        std::vector<double> list_incoming_rate_megabit_per_s;
        test_run_and_validate_udp_burst_logs(simulation_end_time_ns, temp_dir, schedule, list_outgoing_rate_megabit_per_s, list_incoming_rate_megabit_per_s);

        // Not everything will arrive
        ASSERT_EQUAL(list_outgoing_rate_megabit_per_s.size(), 1);
        ASSERT_TRUE(list_outgoing_rate_megabit_per_s.at(0) >= 24.5);
        ASSERT_EQUAL(list_incoming_rate_megabit_per_s.size(), 1);
        ASSERT_TRUE(list_incoming_rate_megabit_per_s.at(0) >= 9.5);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstEndToEndDoubleEnoughTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndDoubleEnoughTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end double-enough") {};

    void DoRun () {
        prepare_test_dir();

        int64_t simulation_end_time_ns = 5000000000;

        // One-to-one, 5s, 10.0 Mbit/s, 100 microseconds delay
        write_basic_config(simulation_end_time_ns, 123456, "0,1");
        write_single_topology(10.0, 100000);

        // A UDP burst each way
        std::vector<UdpBurstInfo> schedule;
        schedule.push_back(UdpBurstInfo(0, 0, 1, 3, 1000000000, 3000000000, "", "abc"));
        schedule.push_back(UdpBurstInfo(1, 0, 1, 4, 1000000000, 3000000000, "", "abc"));

        // Perform the run
        std::vector<double> list_outgoing_rate_megabit_per_s;
        std::vector<double> list_incoming_rate_megabit_per_s;
        test_run_and_validate_udp_burst_logs(simulation_end_time_ns, temp_dir, schedule, list_outgoing_rate_megabit_per_s, list_incoming_rate_megabit_per_s);

        // They will share the overflow
        ASSERT_EQUAL(list_outgoing_rate_megabit_per_s.size(), 2);
        ASSERT_TRUE(list_outgoing_rate_megabit_per_s.at(0) >= 2.5 && list_outgoing_rate_megabit_per_s.at(0) <= 3.1);
        ASSERT_TRUE(list_outgoing_rate_megabit_per_s.at(1) >= 3.5 && list_outgoing_rate_megabit_per_s.at(1) <= 4.1);

        ASSERT_EQUAL(list_incoming_rate_megabit_per_s.size(), 2);
        ASSERT_TRUE(list_incoming_rate_megabit_per_s.at(0) >= 2.5 && list_incoming_rate_megabit_per_s.at(0) <= 3.1);
        ASSERT_TRUE(list_incoming_rate_megabit_per_s.at(1) >= 3.5 && list_incoming_rate_megabit_per_s.at(1) <= 4.1);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstEndToEndDoubleOverflowTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndDoubleOverflowTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end double-overflow") {};

    void DoRun () {
        prepare_test_dir();

        int64_t simulation_end_time_ns = 5000000000;

        // One-to-one, 5s, 10.0 Mbit/s, 100 microseconds delay
        write_basic_config(simulation_end_time_ns, 123456, "0,1");
        write_single_topology(10.0, 100000);

        // A UDP burst each way
        std::vector<UdpBurstInfo> schedule;
        schedule.push_back(UdpBurstInfo(0, 0, 1, 15, 1000000000, 3000000000, "", "abc"));
        schedule.push_back(UdpBurstInfo(1, 0, 1, 10, 1000000000, 3000000000, "", "abc"));

        // Perform the run
        std::vector<double> list_outgoing_rate_megabit_per_s;
        std::vector<double> list_incoming_rate_megabit_per_s;
        test_run_and_validate_udp_burst_logs(simulation_end_time_ns, temp_dir, schedule, list_outgoing_rate_megabit_per_s, list_incoming_rate_megabit_per_s);

        // They will share the overflow
        ASSERT_EQUAL(list_outgoing_rate_megabit_per_s.size(), 2);
        ASSERT_TRUE(list_outgoing_rate_megabit_per_s.at(0) >= 14.5);
        ASSERT_TRUE(list_outgoing_rate_megabit_per_s.at(1) >= 9.5);

        ASSERT_EQUAL(list_incoming_rate_megabit_per_s.size(), 2);
        ASSERT_TRUE(list_incoming_rate_megabit_per_s.at(0) >= 5.0);
        ASSERT_TRUE(list_incoming_rate_megabit_per_s.at(1) >= 2.5);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstEndToEndLossTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndLossTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end loss") {};

    void DoRun () {
        prepare_test_dir();

        int64_t simulation_end_time_ns = 5000000000;

        // One-to-one, 5s
        write_basic_config(simulation_end_time_ns, 123456, "0,1,2,3,4,5,6");
        double bandwidth_rate = 5.0;

        // Topology with loss
        std::ofstream topology_file;
        topology_file.open (temp_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1,2,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,1-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=" << bandwidth_rate << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=map(0->1: iid_uniform_random_pkt(0.3), 1->0: iid_uniform_random_pkt(1.0), 1->2: none, 2->1: iid_uniform_random_pkt(0.4), 1->3: none, 3->1: none)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // A UDP burst each way
        std::vector<UdpBurstInfo> schedule;
        schedule.push_back(UdpBurstInfo(0, 0, 1, bandwidth_rate / 2.0, 0, 5000000000, "", ""));
        schedule.push_back(UdpBurstInfo(1, 1, 0, bandwidth_rate, 0, 5000000000, "", ""));
        schedule.push_back(UdpBurstInfo(2, 2, 1, bandwidth_rate, 0, 5000000000, "", ""));
        schedule.push_back(UdpBurstInfo(3, 1, 2, bandwidth_rate, 0, 5000000000, "", ""));
        schedule.push_back(UdpBurstInfo(4, 1, 3, bandwidth_rate, 0, 5000000000, "", ""));
        schedule.push_back(UdpBurstInfo(5, 3, 1, bandwidth_rate, 0, 5000000000, "", ""));
        schedule.push_back(UdpBurstInfo(6, 0, 1, bandwidth_rate / 2.0, 0, 5000000000, "", ""));

        // Perform the run
        std::vector<double> list_outgoing_rate_megabit_per_s;
        std::vector<double> list_incoming_rate_megabit_per_s;
        test_run_and_validate_udp_burst_logs(simulation_end_time_ns, temp_dir, schedule, list_outgoing_rate_megabit_per_s, list_incoming_rate_megabit_per_s);

        // Loss enacted is accounted for
        ASSERT_EQUAL(list_outgoing_rate_megabit_per_s.size(), 7);
        ASSERT_EQUAL_APPROX(list_outgoing_rate_megabit_per_s.at(0), bandwidth_rate / 2.0, 0.01);
        ASSERT_EQUAL_APPROX(list_outgoing_rate_megabit_per_s.at(1), bandwidth_rate, 0.01);
        ASSERT_EQUAL_APPROX(list_outgoing_rate_megabit_per_s.at(2), bandwidth_rate, 0.01);
        ASSERT_EQUAL_APPROX(list_outgoing_rate_megabit_per_s.at(3), bandwidth_rate, 0.01);
        ASSERT_EQUAL_APPROX(list_outgoing_rate_megabit_per_s.at(4), bandwidth_rate, 0.01);
        ASSERT_EQUAL_APPROX(list_outgoing_rate_megabit_per_s.at(5), bandwidth_rate, 0.01);
        ASSERT_EQUAL_APPROX(list_outgoing_rate_megabit_per_s.at(6), bandwidth_rate / 2.0, 0.01);

        ASSERT_EQUAL(list_incoming_rate_megabit_per_s.size(), 7);
        ASSERT_EQUAL_APPROX(list_incoming_rate_megabit_per_s.at(0), bandwidth_rate / 2.0 * (1.0 - 0.3), 0.1);
        ASSERT_EQUAL_APPROX(list_incoming_rate_megabit_per_s.at(1), bandwidth_rate * (1.0 - 1.0), 0.1);
        ASSERT_EQUAL_APPROX(list_incoming_rate_megabit_per_s.at(2), bandwidth_rate * (1.0 - 0.4), 0.1);
        ASSERT_EQUAL_APPROX(list_incoming_rate_megabit_per_s.at(3), bandwidth_rate, 0.1);
        ASSERT_EQUAL_APPROX(list_incoming_rate_megabit_per_s.at(4), bandwidth_rate, 0.1);
        ASSERT_EQUAL_APPROX(list_incoming_rate_megabit_per_s.at(5), bandwidth_rate, 0.1);
        ASSERT_EQUAL_APPROX(list_incoming_rate_megabit_per_s.at(6), bandwidth_rate / 2.0 * (1.0 - 0.3), 0.1);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstEndToEndNotEnabledTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndNotEnabledTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end not-enabled") {};

    void DoRun () {

        // Run directory
        prepare_test_dir();

        // Config file
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << 1000000000 << std::endl;
        config_file << "simulation_seed=" << 1111111111 << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_udp_burst_scheduler=false" << std::endl;
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
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology);
        basicSimulation->Run();
        udpBurstScheduler.WriteResults();
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

class UdpBurstEndToEndInvalidLoggingIdTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndInvalidLoggingIdTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end invalid-logging-id") {};

    void DoRun () {

        // Run directory
        prepare_test_dir();

        // Config file
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << 1000000000 << std::endl;
        config_file << "simulation_seed=" << 1111111111 << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_udp_burst_scheduler=true" << std::endl;
        config_file << "udp_burst_schedule_filename=\"udp_burst_schedule.csv\"" << std::endl;
        config_file << "udp_burst_enable_logging_for_udp_burst_ids=set(0,1)" << std::endl; // Invalid entry: 1
        config_file.close();

        // One flow
        std::ofstream schedule_file;
        schedule_file.open (temp_dir + "/udp_burst_schedule.csv");
        schedule_file << "0,0,1,100,0,10000,," << std::endl;
        schedule_file.close();

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
        ASSERT_EXCEPTION(UdpBurstScheduler(basicSimulation, topology));
        basicSimulation->Finalize();

        // Make sure these are removed
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/udp_burst_schedule.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(temp_dir + "/logs_ns3");
        remove_dir_if_exists(temp_dir);

    }

};

////////////////////////////////////////////////////////////////////////////////////////
