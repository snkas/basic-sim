/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

////////////////////////////////////////////////////////////////////////////////////////

class UdpPingScheduleReaderTestCase : public TestCaseWithLogValidators
{
public:
    UdpPingScheduleReaderTestCase (std::string s) : TestCaseWithLogValidators (s) {};
    std::string test_run_dir = ".tmp-test-udp-ping-schedule-reader";

    void cleanup_udp_ping_schedule_reader_test() {
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

class UdpPingScheduleReaderNormalTestCase : public UdpPingScheduleReaderTestCase
{
public:
    UdpPingScheduleReaderNormalTestCase () : UdpPingScheduleReaderTestCase ("udp-ping-schedule-reader normal") {};

    void DoRun () {
        test_run_dir = ".tmp-test-udp-ping-schedule-reader-normal";
        prepare_clean_run_dir(test_run_dir);

        // Normal

        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=10000000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "udp_ping_schedule_filename=\"udp_ping_schedule.csv\"" << std::endl;
        config_file.close();

        std::ofstream schedule_file(test_run_dir + "/udp_ping_schedule.csv");
        // From 0 to 1 every 10ms starting at 500ns for 1 second send a ping and wait afterwards 50ms before exiting
        schedule_file << "0,0,1,10000000,500,1000000000,50000000,a=c,test14" << std::endl;
        // From 7 to 3 every 5ms starting at 6666ns for 0.5 second send a ping and wait afterwards 0ms before exiting
        schedule_file << "1,7,3,5000000,6666,500000000,0,a=b2," << std::endl;
        schedule_file.close();

        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=8" << std::endl;
        topology_file << "num_undirected_edges=7" << std::endl;
        topology_file << "switches=set(0,1,2,3,4,5,6,7)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1,2,3,4,5,6,7)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3,3-4,4-5,5-6,6-7)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        std::vector<UdpPingInfo> schedule = read_udp_ping_schedule(test_run_dir + "/udp_ping_schedule.csv", topology, 10000000000);

        ASSERT_EQUAL(schedule.size(), 2);

        ASSERT_EQUAL(schedule[0].GetUdpPingId(), 0);
        ASSERT_EQUAL(schedule[0].GetFromNodeId(), 0);
        ASSERT_EQUAL(schedule[0].GetToNodeId(), 1);
        ASSERT_EQUAL(schedule[0].GetIntervalNs(), 10000000);
        ASSERT_EQUAL(schedule[0].GetStartTimeNs(), 500);
        ASSERT_EQUAL(schedule[0].GetDurationNs(), 1000000000);
        ASSERT_EQUAL(schedule[0].GetWaitAfterwardsNs(), 50000000);
        ASSERT_EQUAL(schedule[0].GetAdditionalParameters(), "a=c");
        ASSERT_EQUAL(schedule[0].GetMetadata(), "test14");

        ASSERT_EQUAL(schedule[1].GetUdpPingId(), 1);
        ASSERT_EQUAL(schedule[1].GetFromNodeId(), 7);
        ASSERT_EQUAL(schedule[1].GetToNodeId(), 3);
        ASSERT_EQUAL(schedule[1].GetIntervalNs(), 5000000);
        ASSERT_EQUAL(schedule[1].GetStartTimeNs(), 6666);
        ASSERT_EQUAL(schedule[1].GetDurationNs(), 500000000);
        ASSERT_EQUAL(schedule[1].GetWaitAfterwardsNs(), 0);
        ASSERT_EQUAL(schedule[1].GetAdditionalParameters(), "a=b2");
        ASSERT_EQUAL(schedule[1].GetMetadata(), "");

        // Empty

        std::ofstream schedule_file_empty(test_run_dir + "/udp_ping_schedule.csv");
        schedule_file_empty.close();
        std::vector<UdpPingInfo> schedule_empty = read_udp_ping_schedule(test_run_dir + "/udp_ping_schedule.csv", topology, 10000000);
        ASSERT_EQUAL(schedule_empty.size(), 0);

        basicSimulation->Finalize();
        cleanup_udp_ping_schedule_reader_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpPingScheduleReaderInvalidTestCase : public UdpPingScheduleReaderTestCase
{
public:
    UdpPingScheduleReaderInvalidTestCase () : UdpPingScheduleReaderTestCase ("udp-ping-schedule-reader invalid") {};

    void DoRun () {
        test_run_dir = ".tmp-test-udp-ping-schedule-reader-invalid";
        prepare_clean_run_dir(test_run_dir);

        std::ofstream schedule_file;
        std::vector<UdpPingInfo> schedule;

        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=10000000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "udp_ping_schedule_filename=\"udp_ping_schedule.csv\"" << std::endl;
        config_file.close();

        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=5" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3,4)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1,3,4)" << std::endl; // Only 2 cannot be endpoint
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3,3-4)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Non-existent file
        ASSERT_EXCEPTION(read_udp_ping_schedule("does-not-exist-temp.file", topology, 10000000));
        
        // Normal
        schedule_file = std::ofstream(test_run_dir + "/udp_ping_schedule.csv");
        schedule_file << "0,3,1,1000,100002,2000009,33,xyz,abc" << std::endl;
        schedule_file.close();
        schedule = read_udp_ping_schedule(test_run_dir + "/udp_ping_schedule.csv", topology, 10000000);

        // Source = Destination
        schedule_file = std::ofstream(test_run_dir + "/udp_ping_schedule.csv");
        schedule_file << "0,3,3,1000,100002,2000009,33,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_ping_schedule(test_run_dir + "/udp_ping_schedule.csv", topology, 10000000));

        // Invalid source (out of range)
        schedule_file = std::ofstream(test_run_dir + "/udp_ping_schedule.csv");
        schedule_file << "0,9,1,1000,100002,2000009,33,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_ping_schedule(test_run_dir + "/udp_ping_schedule.csv", topology, 10000000));

        // Invalid destination (out of range)
        schedule_file = std::ofstream(test_run_dir + "/udp_ping_schedule.csv");
        schedule_file << "0,3,9,1000,100002,2000009,33,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_ping_schedule(test_run_dir + "/udp_ping_schedule.csv", topology, 10000000));

        // Invalid source (not a ToR)
        schedule_file = std::ofstream(test_run_dir + "/udp_ping_schedule.csv");
        schedule_file << "0,2,3,1000,100002,2000009,33,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_ping_schedule(test_run_dir + "/udp_ping_schedule.csv", topology, 10000000));

        // Invalid destination (not a ToR)
        schedule_file = std::ofstream(test_run_dir + "/udp_ping_schedule.csv");
        schedule_file << "0,3,2,1000,100002,2000009,33,xyz,abcc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_ping_schedule(test_run_dir + "/udp_ping_schedule.csv", topology, 10000000));

        // Not ascending UDP ping ID
        schedule_file = std::ofstream(test_run_dir + "/udp_ping_schedule.csv");
        schedule_file << "1,3,1,1000,100002,2000009,33,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_ping_schedule(test_run_dir + "/udp_ping_schedule.csv", topology, 10000000));

        // Negative UDP ping interval
        schedule_file = std::ofstream(test_run_dir + "/udp_ping_schedule.csv");
        schedule_file << "0,3,1,-1,100002,2000009,33,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_ping_schedule(test_run_dir + "/udp_ping_schedule.csv", topology, 10000000));

        // Zero UDP ping rate
        schedule_file = std::ofstream(test_run_dir + "/udp_ping_schedule.csv");
        schedule_file << "0,3,1,0,100002,2000009,33,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_ping_schedule(test_run_dir + "/udp_ping_schedule.csv", topology, 10000000));

        // Not enough values
        schedule_file = std::ofstream(test_run_dir + "/udp_ping_schedule.csv");
        schedule_file << "0,3,1,1000,100002,2000009,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_ping_schedule(test_run_dir + "/udp_ping_schedule.csv", topology, 10000000));

        // Negative time
        schedule_file = std::ofstream(test_run_dir + "/udp_ping_schedule.csv");
        schedule_file << "0,3,1,1000,-100,2000009,33,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(schedule = read_udp_ping_schedule(test_run_dir + "/udp_ping_schedule.csv", topology, 10000000));

        // Just normal ordered with equal start time
        schedule_file = std::ofstream(test_run_dir + "/udp_ping_schedule.csv");
        schedule_file << "0,3,1,1000,100002,2000009,33,xyz,abc" << std::endl;
        schedule_file << "1,3,1,1000,100002,2000009,33,xyz,abc" << std::endl;
        schedule_file.close();
        schedule = read_udp_ping_schedule(test_run_dir + "/udp_ping_schedule.csv", topology, 10000000);

        // Not ordered time
        schedule_file = std::ofstream(test_run_dir + "/udp_ping_schedule.csv");
        schedule_file << "0,3,1,1000,100002,2000009,33,xyz,abc" << std::endl;
        schedule_file << "1,3,1,1000,100001,2000009,33,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION_MATCH_WHAT(
                read_udp_ping_schedule(test_run_dir + "/udp_ping_schedule.csv", topology, 10000000),
                "Start time is not weakly ascending (on line with UDP ping ID: 1, violation: 100001)"
        );

        // Exceeding time
        schedule_file = std::ofstream(test_run_dir + "/udp_ping_schedule.csv");
        schedule_file << "0,3,1,1000,10000000,2000009,33,xyz,abc" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_udp_ping_schedule(test_run_dir + "/udp_ping_schedule.csv", topology, 10000000));

        basicSimulation->Finalize();
        cleanup_udp_ping_schedule_reader_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////
