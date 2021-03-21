/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstEndToEndTestCase : public TestCaseWithLogValidators {
public:
    UdpBurstEndToEndTestCase(std::string s) : TestCaseWithLogValidators(s) {};
    std::string test_run_dir = ".tmp-test-udp-burst-end-to-end";

    void write_basic_config(int64_t simulation_end_time_ns, int64_t simulation_seed, std::string detailed_logging_enabled_for) {
        std::ofstream config_file;
        config_file.open (test_run_dir + "/config_ns3.properties");
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

    void test_run_and_validate_udp_burst_logs(
            int64_t simulation_end_time_ns,
            std::string test_run_dir, 
            std::vector<UdpBurstInfo> write_schedule,
            std::vector<double>& list_outgoing_rate_megabit_per_s,
            std::vector<double>& list_incoming_rate_megabit_per_s
    ) {

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts.csv");

        // Write schedule file
        std::ofstream schedule_file;
        schedule_file.open (test_run_dir + "/udp_burst_schedule.csv");
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
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology);
        basicSimulation->Run();
        udpBurstScheduler.WriteResults();
        basicSimulation->Finalize();

        // Check finished.txt
        std::vector<std::string> finished_lines = read_file_direct(test_run_dir + "/logs_ns3/finished.txt");
        ASSERT_EQUAL(finished_lines.size(), 1);
        ASSERT_EQUAL(finished_lines[0], "Yes");

        // For which logging was enabled (all)
        std::set<int64_t> udp_burst_ids_with_logging;
        for (size_t i = 0; i < write_schedule.size(); i++) {
            udp_burst_ids_with_logging.insert(i);
        }

        // Validate UDP burst logs
        validate_udp_burst_logs(
                simulation_end_time_ns,
                test_run_dir,
                write_schedule,
                udp_burst_ids_with_logging,
                list_outgoing_rate_megabit_per_s,
                list_incoming_rate_megabit_per_s
       );

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/udp_burst_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_outgoing.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_outgoing.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_incoming.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_incoming.txt");
        for (UdpBurstInfo entry : write_schedule) {
            remove_file_if_exists(test_run_dir + "/logs_ns3/udp_burst_" + std::to_string(entry.GetUdpBurstId()) + "_outgoing.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/udp_burst_" + std::to_string(entry.GetUdpBurstId()) + "_incoming.csv");
        }
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }

};

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstEndToEndOneToOneEqualStartTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndOneToOneEqualStartTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end 1-to-1 equal-start") {};

    void DoRun () {
        test_run_dir = ".tmp-test-udp-burst-end-to-end-1-to-1-equal-start";
        prepare_clean_run_dir(test_run_dir);

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
        test_run_and_validate_udp_burst_logs(simulation_end_time_ns, test_run_dir, schedule, list_outgoing_rate_megabit_per_s, list_incoming_rate_megabit_per_s);

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
        test_run_dir = ".tmp-test-udp-burst-end-to-end-overflow";
        prepare_clean_run_dir(test_run_dir);

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
        test_run_and_validate_udp_burst_logs(simulation_end_time_ns, test_run_dir, schedule, list_outgoing_rate_megabit_per_s, list_incoming_rate_megabit_per_s);

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
        test_run_dir = ".tmp-test-udp-burst-end-to-end-double-enough";
        prepare_clean_run_dir(test_run_dir);

        int64_t simulation_end_time_ns = 5000000000;

        // One-to-one, 5s, 10.0 Mbit/s, 100 microseconds delay
        write_basic_config(simulation_end_time_ns, 123456, "0,1");
        write_single_topology(10.0, 100000);

        // A UDP burst each way
        std::vector<UdpBurstInfo> schedule;
        schedule.push_back(UdpBurstInfo(0, 0, 1, 3, 1000000000, 3000000000, "xyz1", "abc"));
        schedule.push_back(UdpBurstInfo(1, 0, 1, 4, 1000000000, 3000000000, "xyz2", "abc"));

        // Perform the run
        std::vector<double> list_outgoing_rate_megabit_per_s;
        std::vector<double> list_incoming_rate_megabit_per_s;
        test_run_and_validate_udp_burst_logs(simulation_end_time_ns, test_run_dir, schedule, list_outgoing_rate_megabit_per_s, list_incoming_rate_megabit_per_s);

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
        test_run_dir = ".tmp-test-udp-burst-end-to-end-double-overflow";
        prepare_clean_run_dir(test_run_dir);

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
        test_run_and_validate_udp_burst_logs(simulation_end_time_ns, test_run_dir, schedule, list_outgoing_rate_megabit_per_s, list_incoming_rate_megabit_per_s);

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
        test_run_dir = ".tmp-test-udp-burst-end-to-end-loss";
        prepare_clean_run_dir(test_run_dir);

        int64_t simulation_end_time_ns = 5000000000;

        // One-to-one, 5s
        write_basic_config(simulation_end_time_ns, 123456, "0,1,2,3,4,5,6");
        double bandwidth_rate = 5.0;

        // Topology with loss
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
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
        test_run_and_validate_udp_burst_logs(simulation_end_time_ns, test_run_dir, schedule, list_outgoing_rate_megabit_per_s, list_incoming_rate_megabit_per_s);

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
        ASSERT_EQUAL_APPROX(list_incoming_rate_megabit_per_s.at(6), bandwidth_rate / 2.0 * (1.0 - 0.3), 0.15);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstEndToEndNotEnabledTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndNotEnabledTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end not-enabled") {};

    void DoRun () {

        // Run directory
        test_run_dir = ".tmp-test-udp-burst-end-to-end-not-enabled";
        prepare_clean_run_dir(test_run_dir);

        // Config file
        std::ofstream config_file;
        config_file.open (test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << 1000000000 << std::endl;
        config_file << "simulation_seed=" << 1111111111 << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_udp_burst_scheduler=false" << std::endl;
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
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology);
        basicSimulation->Run();
        udpBurstScheduler.WriteResults();
        basicSimulation->Finalize();

        // Make sure these are removed
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

class UdpBurstEndToEndInvalidLoggingIdTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndInvalidLoggingIdTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end invalid-logging-id") {};

    void DoRun () {

        // Run directory
        test_run_dir = ".tmp-test-udp-burst-end-to-end-invalid-logging-id";
        prepare_clean_run_dir(test_run_dir);

        // Config file
        std::ofstream config_file;
        config_file.open (test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << 1000000000 << std::endl;
        config_file << "simulation_seed=" << 1111111111 << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_udp_burst_scheduler=true" << std::endl;
        config_file << "udp_burst_schedule_filename=\"udp_burst_schedule.csv\"" << std::endl;
        config_file << "udp_burst_enable_logging_for_udp_burst_ids=set(0,1)" << std::endl; // Invalid entry: 1
        config_file.close();

        // One flow
        std::ofstream schedule_file;
        schedule_file.open (test_run_dir + "/udp_burst_schedule.csv");
        schedule_file << "0,0,1,100,0,10000,," << std::endl;
        schedule_file.close();

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
        ASSERT_EXCEPTION(UdpBurstScheduler(basicSimulation, topology));
        basicSimulation->Finalize();

        // Make sure these are removed
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

class UdpBurstEndToEndLoggingSpecificTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndLoggingSpecificTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end logging-specific") {};

    void DoRun () {
        test_run_dir = ".tmp-test-udp-burst-end-to-end-logging-specific";
        prepare_clean_run_dir(test_run_dir);

        int64_t simulation_end_time_ns = 100000000;
        int64_t simulation_seed = 1839582950225;

        // topology.properties
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,0-2,1-3,2-3)" << std::endl;
        topology_file << "all_nodes_are_endpoints=true" << std::endl;
        topology_file << "link_channel_delay_ns=100000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=20" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=iid_uniform_random_pkt(0.0)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // config_ns3.properties
        std::ofstream config_file;
        config_file.open (test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << simulation_end_time_ns << std::endl;
        config_file << "simulation_seed=" << simulation_seed << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_udp_burst_scheduler=true" << std::endl;
        config_file << "udp_burst_schedule_filename=\"udp_burst_schedule.csv\"" << std::endl;
        config_file << "udp_burst_enable_logging_for_udp_burst_ids=set(2, 4, 5)" << std::endl;
        config_file.close();

        // Couple of UDP bursts
        std::vector<UdpBurstInfo> write_schedule;
        write_schedule.push_back(UdpBurstInfo(0, 0, 3, 10.0, 0, 131525125, "", ""));
        write_schedule.push_back(UdpBurstInfo(1, 1, 2, 6.0, 2525, 1351515151, "", ""));
        write_schedule.push_back(UdpBurstInfo(2, 3, 1, 7.0, 2525, 2223515, "", ""));
        write_schedule.push_back(UdpBurstInfo(3, 2, 3, 13.45, 2525, 22222, "", ""));
        write_schedule.push_back(UdpBurstInfo(4, 1, 2, 8.2, 3255235, 3525, "", ""));
        write_schedule.push_back(UdpBurstInfo(5, 1, 0, 1.463, 25253552, 422462, "", ""));

        // Write schedule file
        std::ofstream schedule_file;
        schedule_file.open (test_run_dir + "/udp_burst_schedule.csv");
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
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology);
        basicSimulation->Run();
        udpBurstScheduler.WriteResults();
        basicSimulation->Finalize();

        // For which logging was enabled
        std::set<int64_t> udp_burst_ids_with_logging;
        udp_burst_ids_with_logging.insert(2);
        udp_burst_ids_with_logging.insert(4);
        udp_burst_ids_with_logging.insert(5);

        // For results (in this case, we don't check this, as we just care about logging)
        std::vector<double> list_outgoing_rate_megabit_per_s;
        std::vector<double> list_incoming_rate_megabit_per_s;

        // Validate UDP burst logs
        validate_udp_burst_logs(
                simulation_end_time_ns,
                test_run_dir,
                write_schedule,
                udp_burst_ids_with_logging,
                list_outgoing_rate_megabit_per_s,
                list_incoming_rate_megabit_per_s
        );

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/udp_burst_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_outgoing.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_outgoing.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_incoming.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_incoming.txt");
        for (int64_t i : udp_burst_ids_with_logging) {
            remove_file_if_exists(test_run_dir + "/logs_ns3/udp_burst_" + std::to_string(i) + "_outgoing.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/udp_burst_" + std::to_string(i) + "_incoming.csv");
        }
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstEndToEndLoggingAllTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndLoggingAllTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end logging-all") {};

    void DoRun () {
        test_run_dir = ".tmp-test-udp-burst-end-to-end-logging-all";
        prepare_clean_run_dir(test_run_dir);

        int64_t simulation_end_time_ns = 100000000;
        int64_t simulation_seed = 1839582950225;

        // topology.properties
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,0-2,1-3,2-3)" << std::endl;
        topology_file << "all_nodes_are_endpoints=true" << std::endl;
        topology_file << "link_channel_delay_ns=100000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=20" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=iid_uniform_random_pkt(0.0)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // config_ns3.properties
        std::ofstream config_file;
        config_file.open (test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << simulation_end_time_ns << std::endl;
        config_file << "simulation_seed=" << simulation_seed << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_udp_burst_scheduler=true" << std::endl;
        config_file << "udp_burst_schedule_filename=\"udp_burst_schedule.csv\"" << std::endl;
        config_file << "udp_burst_enable_logging_for_udp_burst_ids=all" << std::endl;
        config_file.close();

        // Couple of UDP bursts
        std::vector<UdpBurstInfo> write_schedule;
        write_schedule.push_back(UdpBurstInfo(0, 0, 3, 10.0, 0, 131525125, "", ""));
        write_schedule.push_back(UdpBurstInfo(1, 1, 2, 6.0, 2525, 1351515151, "", ""));
        write_schedule.push_back(UdpBurstInfo(2, 3, 1, 7.0, 2525, 2223515, "", ""));
        write_schedule.push_back(UdpBurstInfo(3, 2, 3, 13.45, 2525, 22222, "", ""));
        write_schedule.push_back(UdpBurstInfo(4, 1, 2, 8.2, 3255235, 3525, "", ""));
        write_schedule.push_back(UdpBurstInfo(5, 1, 0, 1.463, 25253552, 422462, "", ""));

        // Write schedule file
        std::ofstream schedule_file;
        schedule_file.open (test_run_dir + "/udp_burst_schedule.csv");
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
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology);
        basicSimulation->Run();
        udpBurstScheduler.WriteResults();
        basicSimulation->Finalize();

        // For which logging was enabled (all)
        std::set<int64_t> udp_burst_ids_with_logging;
        for (size_t i = 0; i < write_schedule.size(); i++) {
            udp_burst_ids_with_logging.insert(i);
        }

        // For results (in this case, we don't check this, as we just care about logging)
        std::vector<double> list_outgoing_rate_megabit_per_s;
        std::vector<double> list_incoming_rate_megabit_per_s;

        // Validate UDP burst logs
        validate_udp_burst_logs(
                simulation_end_time_ns,
                test_run_dir,
                write_schedule,
                udp_burst_ids_with_logging,
                list_outgoing_rate_megabit_per_s,
                list_incoming_rate_megabit_per_s
        );

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/udp_burst_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_outgoing.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_outgoing.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_incoming.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_incoming.txt");
        for (int64_t i : udp_burst_ids_with_logging) {
            remove_file_if_exists(test_run_dir + "/logs_ns3/udp_burst_" + std::to_string(i) + "_outgoing.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/udp_burst_" + std::to_string(i) + "_incoming.csv");
        }
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstEndToEndMultiPathTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndMultiPathTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end multi-path") {};

    void DoRun () {
        test_run_dir = ".tmp-test-udp-burst-end-to-end-multi-path";
        prepare_clean_run_dir(test_run_dir);

        int64_t simulation_end_time_ns = 250000000;
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
        topology_file << "link_channel_delay_ns=100000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=50" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=iid_uniform_random_pkt(0.0)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // config_ns3.properties
        std::ofstream config_file;
        config_file.open (test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << simulation_end_time_ns << std::endl;
        config_file << "simulation_seed=" << simulation_seed << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_udp_burst_scheduler=true" << std::endl;
        config_file << "udp_burst_schedule_filename=\"udp_burst_schedule.csv\"" << std::endl;
        config_file << "udp_burst_enable_logging_for_udp_burst_ids=all" << std::endl;
        config_file << "enable_link_net_device_utilization_tracking=true" << std::endl;
        config_file << "link_net_device_utilization_tracking_interval_ns=100000000" << std::endl;
        config_file << "link_net_device_utilization_tracking_enable_for_links=all" << std::endl;
        config_file.close();

        // Many UDP bursts
        std::vector<UdpBurstInfo> write_schedule;
        int num_bursts = 100;
        for (int i = 0; i < num_bursts; i++) {
            write_schedule.push_back(UdpBurstInfo(i, 4, 3, 0.5, 0, simulation_end_time_ns, "", ""));
        }

        // Write schedule file
        std::ofstream schedule_file;
        schedule_file.open (test_run_dir + "/udp_burst_schedule.csv");
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
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology);
        PtopLinkNetDeviceUtilizationTracking utilTrackerHelper = PtopLinkNetDeviceUtilizationTracking(basicSimulation, topology);
        basicSimulation->Run();
        udpBurstScheduler.WriteResults();
        utilTrackerHelper.WriteResults();
        basicSimulation->Finalize();

        // For which logging was enabled (all)
        std::set<int64_t> udp_burst_ids_with_logging;
        for (size_t i = 0; i < write_schedule.size(); i++) {
            udp_burst_ids_with_logging.insert(i);
        }

        // For results (in this case, we don't check this, as we just care about logging)
        std::vector<double> list_outgoing_rate_megabit_per_s;
        std::vector<double> list_incoming_rate_megabit_per_s;

        // Validate UDP burst logs
        validate_udp_burst_logs(
                simulation_end_time_ns,
                test_run_dir,
                write_schedule,
                udp_burst_ids_with_logging,
                list_outgoing_rate_megabit_per_s,
                list_incoming_rate_megabit_per_s
        );

        // Directed edge list
        // 0-4,1-4,2-4,0-8,0-6,1-7,2-5,3-8,3-6,3-7,3-5
        std::vector <std::pair<int64_t, int64_t>> dir_a_b_list;
        dir_a_b_list.push_back(std::make_pair(0, 4));
        dir_a_b_list.push_back(std::make_pair(1, 4));
        dir_a_b_list.push_back(std::make_pair(2, 4));
        dir_a_b_list.push_back(std::make_pair(0, 8));
        dir_a_b_list.push_back(std::make_pair(0, 6));
        dir_a_b_list.push_back(std::make_pair(1, 7));
        dir_a_b_list.push_back(std::make_pair(2, 5));
        dir_a_b_list.push_back(std::make_pair(8, 3));
        dir_a_b_list.push_back(std::make_pair(6, 3));
        dir_a_b_list.push_back(std::make_pair(7, 3));
        dir_a_b_list.push_back(std::make_pair(5, 3));
        dir_a_b_list.push_back(std::make_pair(4, 0));
        dir_a_b_list.push_back(std::make_pair(4, 1));
        dir_a_b_list.push_back(std::make_pair(4, 2));
        dir_a_b_list.push_back(std::make_pair(8, 0));
        dir_a_b_list.push_back(std::make_pair(6, 0));
        dir_a_b_list.push_back(std::make_pair(7, 1));
        dir_a_b_list.push_back(std::make_pair(5, 2));
        dir_a_b_list.push_back(std::make_pair(3, 8));
        dir_a_b_list.push_back(std::make_pair(3, 6));
        dir_a_b_list.push_back(std::make_pair(3, 7));
        dir_a_b_list.push_back(std::make_pair(3, 5));

        // Validate link utilization logs
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_net_device_utilization;
        std::map<std::pair<int64_t, int64_t>, double> link_overall_utilization_as_fraction;
        validate_link_net_device_utilization_logs(test_run_dir, dir_a_b_list, simulation_end_time_ns, 100000000, link_net_device_utilization, link_overall_utilization_as_fraction);

        // To print the actual utilization
        std::cout << "5-3: " << link_overall_utilization_as_fraction.at(std::make_pair(5, 3)) << std::endl;
        std::cout << "6-3: " << link_overall_utilization_as_fraction.at(std::make_pair(6, 3)) << std::endl;
        std::cout << "7-3: " << link_overall_utilization_as_fraction.at(std::make_pair(7, 3)) << std::endl;
        std::cout << "8-3: " << link_overall_utilization_as_fraction.at(std::make_pair(8, 3)) << std::endl;

        // The utilization should be 33% for (6-3, 8-3), 33% for 5-3 and 33% for 7-3
        ASSERT_TRUE(link_overall_utilization_as_fraction.at(std::make_pair(5, 3)) >= 0.25);
        ASSERT_TRUE(link_overall_utilization_as_fraction.at(std::make_pair(6, 3)) >= 0.125);
        ASSERT_TRUE(link_overall_utilization_as_fraction.at(std::make_pair(7, 3)) >= 0.25);
        ASSERT_TRUE(link_overall_utilization_as_fraction.at(std::make_pair(8, 3)) >= 0.125);
        ASSERT_TRUE(
            link_overall_utilization_as_fraction.at(std::make_pair(5, 3)) +
            link_overall_utilization_as_fraction.at(std::make_pair(6, 3)) +
            link_overall_utilization_as_fraction.at(std::make_pair(7, 3)) +
            link_overall_utilization_as_fraction.at(std::make_pair(8, 3))
            <= 1.1
        );

        // Make sure these are removed
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
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_outgoing.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_outgoing.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_incoming.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/udp_bursts_incoming.txt");
        for (int64_t i : udp_burst_ids_with_logging) {
            remove_file_if_exists(test_run_dir + "/logs_ns3/udp_burst_" + std::to_string(i) + "_outgoing.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/udp_burst_" + std::to_string(i) + "_incoming.csv");
        }
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstEndToEndHeaderTestCase : public UdpBurstEndToEndTestCase
{
public:
    UdpBurstEndToEndHeaderTestCase () : UdpBurstEndToEndTestCase ("udp-burst-end-to-end header") {};

    void DoRun () {
        UdpBurstHeader header;
        header.SetId(55);
        header.SetSeq(66);
        ASSERT_EQUAL(header.GetId(), 55);
        ASSERT_EQUAL(header.GetSeq(), 66);
        ASSERT_EQUAL(header.GetSerializedSize(), 16);
        std::ostringstream stream;
        header.Print(stream);
        ASSERT_EQUAL(stream.str(), "(id=55, seq=66)");
    }
};

////////////////////////////////////////////////////////////////////////////////////////
