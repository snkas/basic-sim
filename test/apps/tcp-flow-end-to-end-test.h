/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

////////////////////////////////////////////////////////////////////////////////////////

class BeforeRunOperation {
public:
    virtual void operation(Ptr<TopologyPtop> topology) = 0;
};

class BeforeRunOperationNothing : public BeforeRunOperation {
public:
    void operation(Ptr<TopologyPtop> topology) {
        // Nothing
    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndTestCase : public TestCaseWithLogValidators {
public:
    TcpFlowEndToEndTestCase(std::string s) : TestCaseWithLogValidators(s) {};
    std::string test_run_dir = ".tmp-test-tcp-flow-end-to-end";

    void write_basic_config(int64_t simulation_end_time_ns, int64_t simulation_seed, uint32_t num_tcp_flows) {
        std::ofstream config_file;
        config_file.open (test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << simulation_end_time_ns << std::endl;
        config_file << "simulation_seed=" << simulation_seed << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "tcp_config=basic" << std::endl;
        config_file << "enable_tcp_flow_scheduler=true" << std::endl;
        config_file << "tcp_flow_schedule_filename=\"tcp_flow_schedule.csv\"" << std::endl;
        std::string ids = "";
        for (size_t i = 0; i < num_tcp_flows; i++) {
            if (i != 0) {
                ids = ids + ",";
            }
            ids = ids + std::to_string(i);
        }
        config_file << "tcp_flow_enable_logging_for_tcp_flow_ids=set(" << ids << ")" << std::endl;
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
        topology_file << "link_net_device_receive_error_model=iid_uniform_random_pkt(0.0)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
    }

    void test_run_and_validate_tcp_flow_logs(int64_t simulation_end_time_ns, std::string test_run_dir, std::vector<TcpFlowScheduleEntry> write_schedule, std::vector<int64_t>& end_time_ns_list, std::vector<int64_t>& sent_byte_list, std::vector<std::string>& finished_list, BeforeRunOperation* beforeRunOperation) {

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.csv");

        // Write schedule file
        std::ofstream schedule_file;
        schedule_file.open (test_run_dir + "/tcp_flow_schedule.csv");
        for (TcpFlowScheduleEntry entry : write_schedule) {
            schedule_file
                    << entry.GetTcpFlowId() << ","
                    << entry.GetFromNodeId() << ","
                    << entry.GetToNodeId() << ","
                    << entry.GetSizeByte() << ","
                    << entry.GetStartTimeNs() << ","
                    << entry.GetAdditionalParameters() << ","
                    << entry.GetMetadata()
                    << std::endl;
        }
        schedule_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        TcpConfigHelper::Configure(basicSimulation);
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology);
        beforeRunOperation->operation(topology);
        basicSimulation->Run();
        tcpFlowScheduler.WriteResults();
        basicSimulation->Finalize();

        // For which logging was enabled
        std::set<int64_t> tcp_flow_ids_with_logging;
        for (size_t i = 0; i < write_schedule.size(); i++) {
            tcp_flow_ids_with_logging.insert(i);
        }

        // Validate TCP flow logs
        validate_tcp_flow_logs(
                simulation_end_time_ns,
                test_run_dir,
                write_schedule,
                tcp_flow_ids_with_logging,
                end_time_ns_list,
                sent_byte_list,
                finished_list
        );

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/tcp_flow_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.txt");
        for (size_t i = 0; i < write_schedule.size(); i++) {
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_progress.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_rtt.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_rto.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cwnd.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cwnd_inflated.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_ssthresh.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_inflight.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_state.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cong_state.csv");
        }
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }

};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndOneToOneEqualStartTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndOneToOneEqualStartTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end 1-to-1 equal-start") {};

    void DoRun () {
        test_run_dir = ".tmp-test-tcp-flow-end-to-end-1-to-1-equal-start";
        prepare_clean_run_dir(test_run_dir);

        int64_t simulation_end_time_ns = 5000000000;

        // One-to-one, 5s, 10.0 Mbit/s, 100 microseconds delay
        write_basic_config(simulation_end_time_ns, 123456, 2);
        write_single_topology(10.0, 100000);

        // A flow each way
        std::vector<TcpFlowScheduleEntry> schedule;
        schedule.push_back(TcpFlowScheduleEntry(0, 0, 1, 1000000, 0, "zyx1", "abc"));
        schedule.push_back(TcpFlowScheduleEntry(1, 1, 0, 1000000, 0, "zyx2", ""));

        // Perform the run
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;
        BeforeRunOperationNothing op;
        test_run_and_validate_tcp_flow_logs(simulation_end_time_ns, test_run_dir, schedule, end_time_ns_list, sent_byte_list, finished_list, &op);

        // As they are started at the same point and should have the same behavior, progress should be equal
        ASSERT_EQUAL(end_time_ns_list[0], end_time_ns_list[1]);
        ASSERT_EQUAL(sent_byte_list[0], sent_byte_list[1]);
        ASSERT_EQUAL(finished_list[0], "YES");
        ASSERT_EQUAL(finished_list[1], "YES");

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndOneToOneSimpleStartTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndOneToOneSimpleStartTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end 1-to-1 simple") {};

    void DoRun () {
        test_run_dir = ".tmp-test-tcp-flow-end-to-end-1-to-1-simple";
        prepare_clean_run_dir(test_run_dir);

        int64_t simulation_end_time_ns = 5000000000;

        // One-to-one, 5s, 100.0 Mbit/s, 10 microseconds delay
        write_basic_config(simulation_end_time_ns, 123456, 1);
        write_single_topology(100.0, 10000);

        // One flow
        std::vector<TcpFlowScheduleEntry> schedule;
        schedule.push_back(TcpFlowScheduleEntry(0, 0, 1, 300, 0, "", ""));

        // Perform the run
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;

        BeforeRunOperationNothing op;
        test_run_and_validate_tcp_flow_logs(simulation_end_time_ns, test_run_dir, schedule, end_time_ns_list, sent_byte_list, finished_list, &op);

        // As they are started at the same point and should have the same behavior, progress should be equal
        int expected_end_time_ns = 0;
        expected_end_time_ns += 4 * 10000; // SYN, SYN+ACK, ACK+DATA, FIN (last ACK+FIN is not acknowledged, and all the data is already ACKed)
        // 1 byte / 100 Mbit/s = 80 ns to transmit 1 byte
        expected_end_time_ns += 58 * 80;  // SYN = 2 (P2P) + 20 (IP) + 20 (TCP basic) + 16 (TCP SYN options: wndscale: 3, timestamp: 10, SACK-perm: 2 -> in 4 byte steps so padded to 16) = 58 bytes
        expected_end_time_ns += 58 * 80;  // SYN+ACK = 2 (P2P) + 20 (IP) + 20 (TCP basic) + 16 (TCP SYN options) = 58 bytes
        expected_end_time_ns += 54 * 80;  // ACK = 2 (P2P) + 20 (IP) + 20 (TCP basic) + 12 (TCP option: timestamp: 10 -> in 4 byte steps so padded to 12) = 54 bytes
        expected_end_time_ns += 354 * 80; // ACK = 2 (P2P) + 20 (IP) + 20 (TCP basic) + 12 (TCP option) + Data (300) = 354 bytes
        expected_end_time_ns += 54 * 80;  // ACK = 2 (P2P) + 20 (IP) + 20 (TCP basic) + 12 (TCP option) = 54 bytes
        expected_end_time_ns += 54 * 80;  // FIN+ACK = 2 (P2P) + 20 (IP) + 20 (TCP basic) + 12 (TCP option)  = 54 bytes
        ASSERT_EQUAL_APPROX(end_time_ns_list[0], expected_end_time_ns, 6); // 6 transmissions for which the rounding can be down
        ASSERT_EQUAL(300, sent_byte_list[0]);
        ASSERT_EQUAL(finished_list[0], "YES");

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndOneToOneApartStartTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndOneToOneApartStartTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end 1-to-1 apart-start") {};

    void DoRun () {
        test_run_dir = ".tmp-test-tcp-flow-end-to-end-1-to-1-apart-start";
        prepare_clean_run_dir(test_run_dir);

        int64_t simulation_end_time_ns = 10000000000;

        // One-to-one, 10s, 10.0 Mbit/s, 100 microseconds delay
        write_basic_config(simulation_end_time_ns, 654321, 2);
        write_single_topology(10.0, 100000);

        // A flow each way
        std::vector<TcpFlowScheduleEntry> schedule;
        schedule.push_back(TcpFlowScheduleEntry(0, 0, 1, 1000000, 0, "", "first"));
        schedule.push_back(TcpFlowScheduleEntry(1, 0, 1, 1000000, 5000000000, "", "second"));

        // Perform the run
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;
        BeforeRunOperationNothing op;
        test_run_and_validate_tcp_flow_logs(simulation_end_time_ns, test_run_dir, schedule, end_time_ns_list, sent_byte_list, finished_list, &op);

        // As they are started without any interference, should have same completion
        ASSERT_EQUAL(end_time_ns_list[0], end_time_ns_list[1] - 5000000000);
        ASSERT_EQUAL(sent_byte_list[0], sent_byte_list[1]);
        ASSERT_EQUAL(finished_list[0], "YES");
        ASSERT_EQUAL(finished_list[1], "YES");

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndEcmpSimpleTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndEcmpSimpleTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end ecmp-simple") {};

    void DoRun () {
        test_run_dir = ".tmp-test-tcp-flow-end-to-end-ecmp-simple";
        prepare_clean_run_dir(test_run_dir);

        int64_t simulation_end_time_ns = 100000000;

        // One-to-one, 5s, 30.0 Mbit/s, 200 microsec delay
        write_basic_config(simulation_end_time_ns, 123456, 20);
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,0-2,1-3,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=200000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=30.0" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=fq_codel(100000000; 2000000000; 10240p)" << std::endl;
        topology_file.close();

        // A flow each way
        std::vector<TcpFlowScheduleEntry> schedule;
        for (int i = 0; i < 20; i++) {
            schedule.push_back(TcpFlowScheduleEntry(i, 0, 3, 1000000000, 0, "", ""));
        }

        // Perform the run
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;
        BeforeRunOperationNothing op;
        test_run_and_validate_tcp_flow_logs(simulation_end_time_ns, test_run_dir, schedule, end_time_ns_list, sent_byte_list, finished_list, &op);

        // All are too large to end, and they must consume bandwidth of both links
        int64_t byte_sum = 0;
        for (int i = 0; i < 20; i++) {
            ASSERT_EQUAL(end_time_ns_list[i], simulation_end_time_ns);
            ASSERT_EQUAL(finished_list[i], "NO_ONGOING");
            byte_sum += sent_byte_list[i];
        }
        ASSERT_TRUE(byte_to_megabit(byte_sum) / nanosec_to_sec(simulation_end_time_ns) >= 45.0); // At least 45 Mbit/s, given that probability of all of them going up is 0.5^8

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndEcmpRemainTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndEcmpRemainTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end ecmp-remain") {};

    void DoRun () {
        test_run_dir = ".tmp-test-tcp-flow-end-to-end-ecmp-remain";
        prepare_clean_run_dir(test_run_dir);

        int64_t simulation_end_time_ns = 100000000;

        // One-to-one, 5s, 30.0 Mbit/s, 200 microsec delay
        write_basic_config(simulation_end_time_ns, 123456, 1);
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,0-2,1-3,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=200000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=30" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=iid_uniform_random_pkt(0.0)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // A flow each way
        std::vector<TcpFlowScheduleEntry> schedule;
        schedule.push_back(TcpFlowScheduleEntry(0, 0, 3, 1000000000, 0, "", ""));

        // Perform the run
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;
        BeforeRunOperationNothing op;
        test_run_and_validate_tcp_flow_logs(simulation_end_time_ns, test_run_dir, schedule, end_time_ns_list, sent_byte_list, finished_list, &op);

        // Can only consume the bandwidth of one path, the one it is hashed to
        ASSERT_EQUAL(end_time_ns_list[0], simulation_end_time_ns);
        double rate_Mbps = byte_to_megabit(sent_byte_list[0]) / nanosec_to_sec(end_time_ns_list[0]);
        ASSERT_TRUE(rate_Mbps >= 25.0 && rate_Mbps <= 30.0); // Somewhere between 25 and 30
        ASSERT_EQUAL(finished_list[0], "NO_ONGOING");

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndLoggingSpecificTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndLoggingSpecificTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end logging-specific") {};

    void DoRun () {
        test_run_dir = ".tmp-test-tcp-flow-end-to-end-logging-specific";
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
        config_file << "tcp_config=basic" << std::endl;
        config_file << "enable_tcp_flow_scheduler=true" << std::endl;
        config_file << "tcp_flow_schedule_filename=\"tcp_flow_schedule.csv\"" << std::endl;
        config_file << "tcp_flow_enable_logging_for_tcp_flow_ids=set(2, 4, 5)" << std::endl;
        config_file.close();

        // Couple of flows
        std::vector<TcpFlowScheduleEntry> write_schedule;
        write_schedule.push_back(TcpFlowScheduleEntry(0, 0, 3, 454848, 1024, "", ""));
        write_schedule.push_back(TcpFlowScheduleEntry(1, 1, 2, 24373, 64848, "", ""));
        write_schedule.push_back(TcpFlowScheduleEntry(2, 3, 2, 22644737, 64848, "", ""));
        write_schedule.push_back(TcpFlowScheduleEntry(3, 3, 2, 235626, 64848, "", ""));
        write_schedule.push_back(TcpFlowScheduleEntry(4, 1, 2, 5773377, 4632637, "", ""));
        write_schedule.push_back(TcpFlowScheduleEntry(5, 0, 1, 23262622226, 36373733, "", ""));

        // Write schedule file
        std::ofstream schedule_file;
        schedule_file.open (test_run_dir + "/tcp_flow_schedule.csv");
        for (TcpFlowScheduleEntry entry : write_schedule) {
            schedule_file
                    << entry.GetTcpFlowId() << ","
                    << entry.GetFromNodeId() << ","
                    << entry.GetToNodeId() << ","
                    << entry.GetSizeByte() << ","
                    << entry.GetStartTimeNs() << ","
                    << entry.GetAdditionalParameters() << ","
                    << entry.GetMetadata()
                    << std::endl;
        }
        schedule_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        TcpConfigHelper::Configure(basicSimulation);
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology);
        basicSimulation->Run();
        tcpFlowScheduler.WriteResults();
        basicSimulation->Finalize();

        // For which logging was enabled
        std::set<int64_t> tcp_flow_ids_with_logging;
        tcp_flow_ids_with_logging.insert(2);
        tcp_flow_ids_with_logging.insert(4);
        tcp_flow_ids_with_logging.insert(5);

        // For results (in this case, we don't check this, as we just care about logging)
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;

        // Validate TCP flow logs
        validate_tcp_flow_logs(
                simulation_end_time_ns,
                test_run_dir,
                write_schedule,
                tcp_flow_ids_with_logging,
                end_time_ns_list,
                sent_byte_list,
                finished_list
        );

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/tcp_flow_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.txt");
        for (int64_t i : tcp_flow_ids_with_logging) {
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_progress.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_rtt.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_rto.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cwnd.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cwnd_inflated.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_ssthresh.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_inflight.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_state.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cong_state.csv");
        }
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndLoggingAllTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndLoggingAllTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end logging-all") {};

    void DoRun () {
        test_run_dir = ".tmp-test-tcp-flow-end-to-end-logging-all";
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
        config_file << "tcp_config=basic" << std::endl;
        config_file << "enable_tcp_flow_scheduler=true" << std::endl;
        config_file << "tcp_flow_schedule_filename=\"tcp_flow_schedule.csv\"" << std::endl;
        config_file << "tcp_flow_enable_logging_for_tcp_flow_ids=all" << std::endl;
        config_file.close();

        // Couple of flows
        std::vector<TcpFlowScheduleEntry> write_schedule;
        write_schedule.push_back(TcpFlowScheduleEntry(0, 0, 3, 454848, 1024, "", ""));
        write_schedule.push_back(TcpFlowScheduleEntry(1, 1, 2, 24373, 64848, "", ""));
        write_schedule.push_back(TcpFlowScheduleEntry(2, 3, 2, 22644737, 64848, "", ""));
        write_schedule.push_back(TcpFlowScheduleEntry(3, 3, 2, 235626, 64848, "", ""));
        write_schedule.push_back(TcpFlowScheduleEntry(4, 1, 2, 5773377, 4632637, "", ""));
        write_schedule.push_back(TcpFlowScheduleEntry(5, 0, 1, 23262622226, 36373733, "", ""));

        // Write schedule file
        std::ofstream schedule_file;
        schedule_file.open (test_run_dir + "/tcp_flow_schedule.csv");
        for (TcpFlowScheduleEntry entry : write_schedule) {
            schedule_file
                    << entry.GetTcpFlowId() << ","
                    << entry.GetFromNodeId() << ","
                    << entry.GetToNodeId() << ","
                    << entry.GetSizeByte() << ","
                    << entry.GetStartTimeNs() << ","
                    << entry.GetAdditionalParameters() << ","
                    << entry.GetMetadata()
                    << std::endl;
        }
        schedule_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        TcpConfigHelper::Configure(basicSimulation);
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology);
        basicSimulation->Run();
        tcpFlowScheduler.WriteResults();
        basicSimulation->Finalize();

        // For which logging was enabled (all)
        std::set<int64_t> tcp_flow_ids_with_logging;
        for (size_t i = 0; i < write_schedule.size(); i++) {
            tcp_flow_ids_with_logging.insert(i);
        }

        // For results (in this case, we don't check this, as we just care about logging)
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;

        // Validate TCP flow logs
        validate_tcp_flow_logs(
                simulation_end_time_ns,
                test_run_dir,
                write_schedule,
                tcp_flow_ids_with_logging,
                end_time_ns_list,
                sent_byte_list,
                finished_list
        );

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/tcp_flow_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.txt");
        for (int64_t i : tcp_flow_ids_with_logging) {
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_progress.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_rtt.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_rto.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cwnd.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cwnd_inflated.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_ssthresh.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_inflight.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_state.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cong_state.csv");
        }
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class ArbiterSpecificDrop: public ArbiterPtop
{
public:
    int m_counter = 0;
    int m_drop_threshold;

    ArbiterSpecificDrop(Ptr<Node> this_node, NodeContainer nodes, Ptr<TopologyPtop> topology, int drop_threshold) : ArbiterPtop(this_node, nodes, topology) {
        // Empty
        m_drop_threshold = drop_threshold;
    }

    int32_t TopologyPtopDecide(
            int32_t source_node_id,
            int32_t target_node_id,
            const std::set<int64_t>& neighbor_node_ids,
            ns3::Ptr<const ns3::Packet> pkt,
            ns3::Ipv4Header const &ipHeader,
            bool is_socket_request_for_source_ip
    ) {
        if (source_node_id == 1) {
            if (m_counter >= m_drop_threshold) {
                return -1;
            } else {
                m_counter += 1;
                if (target_node_id == 3) {
                    return 3;
                } else {
                    throw std::runtime_error("Not possible");
                }
            }
        } else {
            if (source_node_id == 3 && target_node_id == 1) {
                return 1;
            } else if (source_node_id == 0 && target_node_id == 3) {
                return 3;
            } else if (source_node_id == 3 && target_node_id == 0) {
                return 0;
            } else {
                throw std::runtime_error("Not possible");
            }
        }
    }

    std::string StringReprOfForwardingState() {
        return "";
    }

};

////////////////////////////////////////////////////////////////////////////////////////

class BeforeRunOperationDropper : public BeforeRunOperation {
public:
    int m_drop_threshold;
    BeforeRunOperationDropper(int drop_threshold) {
        m_drop_threshold = drop_threshold;
    }
    void operation(Ptr<TopologyPtop> topology) {
        Ptr<ArbiterSpecificDrop> dropper = CreateObject<ArbiterSpecificDrop>(topology->GetNodes().Get(2), topology->GetNodes(), topology, m_drop_threshold);
        topology->GetNodes().Get(2)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(dropper);
    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndOneDropOneNotTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndOneDropOneNotTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end one-drop-one-not") {};

    void DoRun () {
        test_run_dir = ".tmp-test-tcp-flow-end-to-end-one-drop-one-not";
        prepare_clean_run_dir(test_run_dir);

        int64_t simulation_end_time_ns = 100000000;

        // One-to-one, 5s, 30.0 Mbit/s, 200 microsec delay
        write_basic_config(simulation_end_time_ns, 123456, 2);
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

        // Two flows
        std::vector<TcpFlowScheduleEntry> schedule;
        schedule.push_back(TcpFlowScheduleEntry(0, 0, 3, 1000000000, 0, "", ""));
        schedule.push_back(TcpFlowScheduleEntry(1, 1, 3, 1000000000, 0, "", ""));

        // Perform the run
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;
        BeforeRunOperationDropper op(0);
        test_run_and_validate_tcp_flow_logs(simulation_end_time_ns, test_run_dir, schedule, end_time_ns_list, sent_byte_list, finished_list, &op);

        // Can only consume the bandwidth of one path, the one it is hashed to
        ASSERT_EQUAL(end_time_ns_list[0], simulation_end_time_ns);
        ASSERT_EQUAL(end_time_ns_list[1], simulation_end_time_ns);
        double rate_Mbps = byte_to_megabit(sent_byte_list[0]) / nanosec_to_sec(end_time_ns_list[0]);
        ASSERT_TRUE(rate_Mbps >= 25.0 && rate_Mbps <= 30.0); // Somewhere between 25 and 30
        ASSERT_EQUAL(sent_byte_list[1], 0);
        ASSERT_EQUAL(finished_list[0], "NO_ONGOING");
        ASSERT_EQUAL(finished_list[1], "NO_ONGOING");

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndConnFailTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndConnFailTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end conn-fail") {};

    void DoRun () {
        test_run_dir = ".tmp-test-tcp-flow-end-to-end-conn-fail";
        prepare_clean_run_dir(test_run_dir);

        // One-to-one, 10s, 30.0 Mbit/s, 200 microsec delay
        int64_t simulation_end_time_ns = 200000000000;
        write_basic_config(simulation_end_time_ns, 123456, 1);
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
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // Two flows
        std::vector<TcpFlowScheduleEntry> schedule;
        schedule.push_back(TcpFlowScheduleEntry(0, 1, 3, 1000000000, 0, "", ""));

        // Perform the run
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;
        BeforeRunOperationDropper op(0);
        test_run_and_validate_tcp_flow_logs(simulation_end_time_ns, test_run_dir, schedule, end_time_ns_list, sent_byte_list, finished_list, &op);

        // Should fail after some point
        ASSERT_EQUAL(end_time_ns_list[0], simulation_end_time_ns);
        ASSERT_EQUAL(sent_byte_list[0], 0);
        ASSERT_EQUAL(finished_list[0], "NO_CONN_FAIL");

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndPrematureCloseTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndPrematureCloseTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end premature-close") {};

    void DoRun () {

        for (int num_packets_allowed = 1; num_packets_allowed < 5; num_packets_allowed++) {

            test_run_dir = ".tmp-test-tcp-flow-end-to-end-premature-close";
            prepare_clean_run_dir(test_run_dir);

            // One-to-one, 10s, 30.0 Mbit/s, 200 microsec delay
            int64_t simulation_end_time_ns = 200000000000;
            write_basic_config(simulation_end_time_ns, 123456, 1);
            std::ofstream topology_file;
            topology_file.open(test_run_dir + "/topology.properties");
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

            // Two flows
            std::vector <TcpFlowScheduleEntry> schedule;
            schedule.push_back(TcpFlowScheduleEntry(0, 1, 3, 1000000000, 0, "", ""));

            // Perform the run
            std::vector <int64_t> end_time_ns_list;
            std::vector <int64_t> sent_byte_list;
            std::vector <std::string> finished_list;
            BeforeRunOperationDropper op(num_packets_allowed);
            test_run_and_validate_tcp_flow_logs(simulation_end_time_ns, test_run_dir, schedule, end_time_ns_list,
                                                sent_byte_list, finished_list, &op);

            // Can only consume the bandwidth of one path, the one it is hashed to
            ASSERT_EQUAL(end_time_ns_list[0], simulation_end_time_ns);
            ASSERT_EQUAL(sent_byte_list[0], std::max(num_packets_allowed - 2, 0) * 1380);
            ASSERT_EQUAL(finished_list[0], "NO_ERR_CLOSE");

        }

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class BeforeRunOperationScheduleClose : public BeforeRunOperation {
public:
    Ptr<TopologyPtop> m_topology;

    BeforeRunOperationScheduleClose() { }

    void operation(Ptr<TopologyPtop> topology) {
        m_topology = topology;
        // In 10 milliseconds, perform call close on the socket unexpectedly
        Simulator::Schedule(NanoSeconds(10000000), &BeforeRunOperationScheduleClose::PerformBadClose, this);
    }

    void PerformBadClose() {
        m_topology->GetNodes().Get(0)->GetApplication(1)->GetObject<TcpFlowClient>()->GetSocket()->GetObject<TcpSocketBase>()->Close();
    }

};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndBadCloseTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndBadCloseTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end bad-close") {};

    void DoRun () {
        test_run_dir = ".tmp-test-tcp-flow-end-to-end-bad-close";
        prepare_clean_run_dir(test_run_dir);

        // One-to-one, 0.1s, 10.0 Mbit/s, 1 microsec delay
        int64_t simulation_end_time_ns = 100000000;
        write_basic_config(simulation_end_time_ns, 123456, 1);
        std::ofstream topology_file;
        topology_file.open(test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(2)" << std::endl;
        topology_file << "switches_which_are_tors=set(2)" << std::endl;
        topology_file << "servers=set(0, 1, 3)" << std::endl;
        topology_file << "undirected_edges=set(0-2,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=1000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=10" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(50p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=iid_uniform_random_pkt(0.0)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=fifo(50p)" << std::endl;
        topology_file.close();

        // Two flows
        std::vector <TcpFlowScheduleEntry> write_schedule;
        write_schedule.push_back(TcpFlowScheduleEntry(0, 0, 1, 1000000000, 0, "", ""));

        // Perform the run
        std::vector <int64_t> end_time_ns_list;
        std::vector <int64_t> sent_byte_list;
        std::vector <std::string> finished_list;
        BeforeRunOperationScheduleClose beforeRunOperation;

        // Write schedule file
        std::ofstream schedule_file;
        schedule_file.open (test_run_dir + "/tcp_flow_schedule.csv");
        for (TcpFlowScheduleEntry entry : write_schedule) {
            schedule_file
                    << entry.GetTcpFlowId() << ","
                    << entry.GetFromNodeId() << ","
                    << entry.GetToNodeId() << ","
                    << entry.GetSizeByte() << ","
                    << entry.GetStartTimeNs() << ","
                    << entry.GetAdditionalParameters() << ","
                    << entry.GetMetadata()
                    << std::endl;
        }
        schedule_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        TcpConfigHelper::Configure(basicSimulation);
        Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(100000)); // This is set exactly to the send-size to force the buffer to go empty after sending it out
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology);
        beforeRunOperation.operation(topology);
        basicSimulation->Run();
        tcpFlowScheduler.WriteResults();
        basicSimulation->Finalize();

        // For which logging was enabled
        std::set<int64_t> tcp_flow_ids_with_logging;
        for (size_t i = 0; i < write_schedule.size(); i++) {
            tcp_flow_ids_with_logging.insert(i);
        }

        // Validate TCP flow logs
        validate_tcp_flow_logs(
                simulation_end_time_ns,
                test_run_dir,
                write_schedule,
                tcp_flow_ids_with_logging,
                end_time_ns_list,
                sent_byte_list,
                finished_list
        );

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/tcp_flow_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.txt");
        for (size_t i = 0; i < write_schedule.size(); i++) {
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_progress.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_rtt.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_rto.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cwnd.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cwnd_inflated.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_ssthresh.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_inflight.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_state.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cong_state.csv");
        }
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

        // It must have closed bad
        ASSERT_EQUAL(end_time_ns_list[0], simulation_end_time_ns);
        ASSERT_EQUAL(sent_byte_list[0], 100000);
        ASSERT_EQUAL(finished_list[0], "NO_BAD_CLOSE");

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndNonExistentRunDirTestCase : public TestCase
{
public:
    TcpFlowEndToEndNonExistentRunDirTestCase () : TestCase ("tcp-flow-end-to-end non-existent-run-dir") {};

    void DoRun () {
        ASSERT_EXCEPTION(BasicSimulation simulation("path/to/non/existent/run/dir"));
    }

};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowEndToEndNotEnabledTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndNotEnabledTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end not-enabled") {};

    void DoRun () {

        // Run directory
        test_run_dir = ".tmp-test-tcp-flow-end-to-end-not-enabled";
        prepare_clean_run_dir(test_run_dir);

        // Config file
        std::ofstream config_file;
        config_file.open (test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << 1000000000 << std::endl;
        config_file << "simulation_seed=" << 1111111111 << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_tcp_flow_scheduler=false" << std::endl;
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
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology);
        basicSimulation->Run();
        tcpFlowScheduler.WriteResults();
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

class TcpFlowEndToEndInvalidLoggingIdTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndInvalidLoggingIdTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end invalid-logging-id") {};

    void DoRun () {
        test_run_dir = ".tmp-test-tcp-flow-end-to-end-invalid-logging-id";
        prepare_clean_run_dir(test_run_dir);

        int64_t simulation_end_time_ns = 100000000;

        // One-to-one, 5s, 30.0 Mbit/s, 200 microsec delay
        write_basic_config(simulation_end_time_ns, 123456, 2); // One too many
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,0-2,1-3,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=200000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=30" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=iid_uniform_random_pkt(0.0)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // Write schedule file
        std::ofstream schedule_file;
        schedule_file.open (test_run_dir + "/tcp_flow_schedule.csv");
        schedule_file << "0,0,3,1000000000,0,," << std::endl;
        schedule_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ASSERT_EXCEPTION(TcpFlowScheduler(basicSimulation, topology));
        basicSimulation->Finalize();

        // Make sure these are removed
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

class TcpFlowEndToEndMultiPathTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndMultiPathTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end multi-path") {};

    void DoRun () {
        test_run_dir = ".tmp-test-tcp-flow-end-to-end-multi-path";
        prepare_clean_run_dir(test_run_dir);

        int64_t simulation_end_time_ns = 200000000;
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
        topology_file << "link_net_device_data_rate_megabit_per_s=map(0->4:20,1->4:20,2->4:20,0->8:20,0->6:20,1->7:20,2->5:20,8->3:20,6->3:20,7->3:20,5->3:20,4->0:20,4->1:20,4->2:20,8->0:20,6->0:20,7->1:20,5->2:20,3->8:20,3->6:20,3->7:20,3->5:20)" << std::endl;
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
        config_file << "tcp_config=basic" << std::endl;
        config_file << "enable_tcp_flow_scheduler=true" << std::endl;
        config_file << "tcp_flow_schedule_filename=\"tcp_flow_schedule.csv\"" << std::endl;
        config_file << "tcp_flow_enable_logging_for_tcp_flow_ids=all" << std::endl;
        config_file << "enable_link_net_device_utilization_tracking=true" << std::endl;
        config_file << "link_net_device_utilization_tracking_interval_ns=100000000" << std::endl;
        config_file << "link_net_device_utilization_tracking_enable_for_links=all" << std::endl;
        config_file.close();

        // Many TCP flows
        std::vector <TcpFlowScheduleEntry> write_schedule;
        int num_flows = 12;
        for (int i = 0; i < num_flows; i++) {
            write_schedule.push_back(TcpFlowScheduleEntry(i, 4, 3, 1000000000, 0, "", ""));
        }

        // Write schedule file
        std::ofstream schedule_file;
        schedule_file.open (test_run_dir + "/tcp_flow_schedule.csv");
        for (TcpFlowScheduleEntry entry : write_schedule) {
            schedule_file
                    << entry.GetTcpFlowId() << ","
                    << entry.GetFromNodeId() << ","
                    << entry.GetToNodeId() << ","
                    << entry.GetSizeByte() << ","
                    << entry.GetStartTimeNs() << ","
                    << entry.GetAdditionalParameters() << ","
                    << entry.GetMetadata()
                    << std::endl;
        }
        schedule_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        TcpConfigHelper::Configure(basicSimulation);
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology);
        PtopLinkNetDeviceUtilizationTracking utilTrackerHelper = PtopLinkNetDeviceUtilizationTracking(basicSimulation, topology);
        basicSimulation->Run();
        tcpFlowScheduler.WriteResults();
        utilTrackerHelper.WriteResults();
        basicSimulation->Finalize();

        // For which logging was enabled
        std::set<int64_t> tcp_flow_ids_with_logging;
        for (int i = 0; i < num_flows; i++) {
            tcp_flow_ids_with_logging.insert(i);
        }

        // For results
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;

        // Validate TCP flow logs
        validate_tcp_flow_logs(
                simulation_end_time_ns,
                test_run_dir,
                write_schedule,
                tcp_flow_ids_with_logging,
                end_time_ns_list,
                sent_byte_list,
                finished_list
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

        // Make sure they are spread

        // Data packets
        ASSERT_TRUE(link_overall_utilization_as_fraction.at(std::make_pair(5, 3)) >= 0.8);
        ASSERT_TRUE(link_overall_utilization_as_fraction.at(std::make_pair(6, 3)) >= 0.25);
        ASSERT_TRUE(link_overall_utilization_as_fraction.at(std::make_pair(7, 3)) >= 0.8);
        ASSERT_TRUE(link_overall_utilization_as_fraction.at(std::make_pair(8, 3)) >= 0.25);

        // Acknowledgement packets
        ASSERT_TRUE(link_overall_utilization_as_fraction.at(std::make_pair(3, 5)) >= 0.0001);
        ASSERT_TRUE(link_overall_utilization_as_fraction.at(std::make_pair(3, 6)) >= 0.0001);
        ASSERT_TRUE(link_overall_utilization_as_fraction.at(std::make_pair(3, 7)) >= 0.0001);
        ASSERT_TRUE(link_overall_utilization_as_fraction.at(std::make_pair(3, 8)) >= 0.0001);

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/tcp_flow_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_utilization.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_utilization_compressed.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_utilization_compressed.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_utilization_summary.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.txt");
        for (int64_t i : tcp_flow_ids_with_logging) {
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_progress.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_rtt.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_rto.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cwnd.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cwnd_inflated.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_ssthresh.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_inflight.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_state.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cong_state.csv");
        }
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

namespace ns3 {

    class TcpSocketGeneratorEnableEcn : public TcpSocketGenerator {
    public:
        static TypeId GetTypeId(void);
        Ptr<Socket> GenerateTcpSocket(TypeId appTypeId, Ptr<Application> app);
    };

    NS_OBJECT_ENSURE_REGISTERED (TcpSocketGeneratorEnableEcn);
    TypeId TcpSocketGeneratorEnableEcn::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::TcpSocketGeneratorEnableEcn")
                .SetParent<TcpSocketGenerator> ()
                .SetGroupName("BasicSim")
        ;
        return tid;
    }

    Ptr<Socket> TcpSocketGeneratorEnableEcn::GenerateTcpSocket(TypeId appTypeId, Ptr<Application> app) {
        Ptr<Socket> socket = Socket::CreateSocket(app->GetNode(), TcpSocketFactory::GetTypeId());
        socket->GetObject<TcpSocketBase>()->SetUseEcn(TcpSocketState::UseEcn_t::On);
        return socket;
    }
    
}

class TcpFlowEndToEndOneToOneEcnTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndOneToOneEcnTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end 1-to-1 ecn") {};

    void DoRun () {
        test_run_dir = ".tmp-test-tcp-flow-end-to-end-1-to-1-ecn";
        prepare_clean_run_dir(test_run_dir);

        int64_t simulation_end_time_ns = 2000000000; // 5 seconds
        int64_t simulation_seed = 1839582950225;

        // Topology: 0 -- 1
        // 10 Mbit/s, 10 us delay, 100p link net-device queue,
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=2" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=10.0" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=iid_uniform_random_pkt(0.0)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=map(0->1: simple_red(ecn; 1500; 1.0; 250; 250; 1000p; 0.1; no_wait; not_gentle), 1->0: fifo(100p))" << std::endl;
        topology_file.close();

        // config_ns3.properties
        std::ofstream config_file;
        config_file.open (test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << simulation_end_time_ns << std::endl;
        config_file << "simulation_seed=" << simulation_seed << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "tcp_config=basic" << std::endl;
        config_file << "enable_link_net_device_queue_tracking=true" << std::endl;
        config_file << "link_net_device_queue_tracking_enable_for_links=all" << std::endl;
        config_file << "enable_link_interface_tc_qdisc_queue_tracking=true" << std::endl;
        config_file << "link_interface_tc_qdisc_queue_tracking_enable_for_links=all" << std::endl;
        config_file << "enable_tcp_flow_scheduler=true" << std::endl;
        config_file << "tcp_flow_schedule_filename=\"tcp_flow_schedule.csv\"" << std::endl;
        config_file << "tcp_flow_enable_logging_for_tcp_flow_ids=all" << std::endl;
        config_file.close();

        // Couple of flows
        std::vector<TcpFlowScheduleEntry> write_schedule;
        write_schedule.push_back(TcpFlowScheduleEntry(0, 0, 1, 10000000, 0, "", ""));

        // Write schedule file
        std::ofstream schedule_file;
        schedule_file.open (test_run_dir + "/tcp_flow_schedule.csv");
        for (TcpFlowScheduleEntry entry : write_schedule) {
            schedule_file
                    << entry.GetTcpFlowId() << ","
                    << entry.GetFromNodeId() << ","
                    << entry.GetToNodeId() << ","
                    << entry.GetSizeByte() << ","
                    << entry.GetStartTimeNs() << ","
                    << entry.GetAdditionalParameters() << ","
                    << entry.GetMetadata()
                    << std::endl;
        }
        schedule_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        TcpConfigHelper::Configure(basicSimulation);
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology, {888}, CreateObject<ClientRemotePortSelectorDefault>(888), CreateObject<TcpSocketGeneratorEnableEcn>(), CreateObject<IpTosGeneratorDefault>());
        PtopLinkNetDeviceQueueTracking netDeviceQueueTracking = PtopLinkNetDeviceQueueTracking(basicSimulation, topology);
        PtopLinkInterfaceTcQdiscQueueTracking tcQdiscQueueTracking = PtopLinkInterfaceTcQdiscQueueTracking(basicSimulation, topology);
        basicSimulation->Run();
        tcpFlowScheduler.WriteResults();
        netDeviceQueueTracking.WriteResults();
        tcQdiscQueueTracking.WriteResults();
        basicSimulation->Finalize();

        // For which logging was enabled (all)
        std::set<int64_t> tcp_flow_ids_with_logging;
        tcp_flow_ids_with_logging.insert(0);

        // For results (in this case, we don't check this, as we just care about logging)
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;

        // Validate TCP flow logs
        validate_tcp_flow_logs(
                simulation_end_time_ns,
                test_run_dir,
                write_schedule,
                tcp_flow_ids_with_logging,
                end_time_ns_list,
                sent_byte_list,
                finished_list
        );

        // Links
        std::vector<std::pair<int64_t, int64_t>> links;
        links.push_back(std::make_pair(0, 1));
        links.push_back(std::make_pair(1, 0));

        // Get the link net-device queue development
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_net_device_queue_pkt;
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_net_device_queue_byte;
        validate_link_net_device_queue_logs(test_run_dir, links, link_net_device_queue_pkt, link_net_device_queue_byte);

        // Get the link interface traffic-control qdisc queue development
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_interface_tc_qdisc_queue_pkt;
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_interface_tc_qdisc_queue_byte;
        validate_link_interface_tc_qdisc_queue_logs(test_run_dir, links, link_interface_tc_qdisc_queue_pkt, link_interface_tc_qdisc_queue_byte);

        // Now let's find the peak
        size_t max_size_qdisc_pkt = 0;
        for (std::tuple<int64_t, int64_t, int64_t> fr_to_size : link_interface_tc_qdisc_queue_pkt.at(std::make_pair(0, 1))) {
            max_size_qdisc_pkt = std::max((size_t) std::get<2>(fr_to_size), max_size_qdisc_pkt);
        }
        // std::cout << "Maximum size of the qdisc queue: " << max_size_qdisc_pkt << " packets" << std::endl;

        // BDP = 10 Mbit/s * 10 microsecond = 12.5 byte = negligible
        // Link net-device queue size = 100 packets
        // Link qdisc queue size before marking = 250 packets
        // So, once there are 350 packets it will start marking
        // A congestion window of: 350 * 1.5 = 525 will be reached before it cuts the window
        ASSERT_EQUAL_APPROX(max_size_qdisc_pkt, 525 - 100, 5); // 100 subtracted because of link net-device queue of 100

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/tcp_flow_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_queue_byte.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_queue_pkt.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_byte.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_pkt.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.txt");
        for (int64_t i : tcp_flow_ids_with_logging) {
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_progress.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_rtt.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_rto.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cwnd.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cwnd_inflated.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_ssthresh.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_inflight.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_state.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cong_state.csv");
        }
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);
    }
};

////////////////////////////////////////////////////////////////////////////////////////

namespace ns3 {

    class ClientRemotePortSelectorTwo : public ClientRemotePortSelector {
    public:
        static TypeId GetTypeId(void);
        uint16_t SelectRemotePort(TypeId appTypeId, Ptr<Application> app);
    };

    NS_OBJECT_ENSURE_REGISTERED (ClientRemotePortSelectorTwo);
    TypeId ClientRemotePortSelectorTwo::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::ClientRemotePortSelectorTwo")
                .SetParent<ClientRemotePortSelector> ()
                .SetGroupName("BasicSim")
        ;
        return tid;
    }

    uint16_t ClientRemotePortSelectorTwo::SelectRemotePort(TypeId appTypeId, Ptr<Application> app) {
        if (app->GetObject<TcpFlowClient>()->GetTcpFlowId() == 0) {
            return 1000;
        } else {
            return 888;
        }
    }

}

class TcpFlowEndToEndOneToOneTwoServersTestCase : public TcpFlowEndToEndTestCase
{
public:
    TcpFlowEndToEndOneToOneTwoServersTestCase () : TcpFlowEndToEndTestCase ("tcp-flow-end-to-end 1-to-1 two-servers") {};

    void DoRun () {
        test_run_dir = ".tmp-test-tcp-flow-end-to-end-1-to-1-two-servers";
        prepare_clean_run_dir(test_run_dir);

        int64_t simulation_end_time_ns = 2000000000; // 5 seconds
        int64_t simulation_seed = 1839582950225;

        // Topology: 0 -- 1
        // 10 Mbit/s, 10 us delay, 100p link net-device queue,
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=2" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=10.0" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=iid_uniform_random_pkt(0.0)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=map(0->1: simple_red(ecn; 1500; 1.0; 250; 250; 1000p; 0.1; no_wait; not_gentle), 1->0: fifo(100p))" << std::endl;
        topology_file.close();

        // config_ns3.properties
        std::ofstream config_file;
        config_file.open (test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=" << simulation_end_time_ns << std::endl;
        config_file << "simulation_seed=" << simulation_seed << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "tcp_config=basic" << std::endl;
        config_file << "enable_link_net_device_queue_tracking=true" << std::endl;
        config_file << "link_net_device_queue_tracking_enable_for_links=all" << std::endl;
        config_file << "enable_link_interface_tc_qdisc_queue_tracking=true" << std::endl;
        config_file << "link_interface_tc_qdisc_queue_tracking_enable_for_links=all" << std::endl;
        config_file << "enable_tcp_flow_scheduler=true" << std::endl;
        config_file << "tcp_flow_schedule_filename=\"tcp_flow_schedule.csv\"" << std::endl;
        config_file << "tcp_flow_enable_logging_for_tcp_flow_ids=all" << std::endl;
        config_file.close();

        // Couple of flows
        std::vector<TcpFlowScheduleEntry> write_schedule;
        write_schedule.push_back(TcpFlowScheduleEntry(0, 0, 1, 100000, 0, "", ""));
        write_schedule.push_back(TcpFlowScheduleEntry(1, 0, 1, 110000, 0, "", ""));

        // Write schedule file
        std::ofstream schedule_file;
        schedule_file.open (test_run_dir + "/tcp_flow_schedule.csv");
        for (TcpFlowScheduleEntry entry : write_schedule) {
            schedule_file
                    << entry.GetTcpFlowId() << ","
                    << entry.GetFromNodeId() << ","
                    << entry.GetToNodeId() << ","
                    << entry.GetSizeByte() << ","
                    << entry.GetStartTimeNs() << ","
                    << entry.GetAdditionalParameters() << ","
                    << entry.GetMetadata()
                    << std::endl;
        }
        schedule_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        TcpConfigHelper::Configure(basicSimulation);
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology, {888, 1000}, CreateObject<ClientRemotePortSelectorTwo>(), CreateObject<TcpSocketGeneratorDefault>(), CreateObject<IpTosGeneratorDefault>());
        PtopLinkNetDeviceQueueTracking netDeviceQueueTracking = PtopLinkNetDeviceQueueTracking(basicSimulation, topology);
        PtopLinkInterfaceTcQdiscQueueTracking tcQdiscQueueTracking = PtopLinkInterfaceTcQdiscQueueTracking(basicSimulation, topology);
        basicSimulation->Run();
        tcpFlowScheduler.WriteResults();
        netDeviceQueueTracking.WriteResults();
        tcQdiscQueueTracking.WriteResults();
        NodeContainer nodes = topology->GetNodes();
        Ptr<Node> node1 = nodes.Get(1);
        ASSERT_EQUAL(node1->GetNApplications(), 2);
        Ptr<TcpFlowServer> tcpFlowServer0 = node1->GetApplication(0)->GetObject<TcpFlowServer>();
        Ptr<TcpFlowServer> tcpFlowServer1 = node1->GetApplication(1)->GetObject<TcpFlowServer>();
        AddressValue addressValue0;
        tcpFlowServer0->GetAttribute ("LocalAddress", addressValue0);
        InetSocketAddress address0 = InetSocketAddress::ConvertFrom(addressValue0.Get());
        AddressValue addressValue1;
        tcpFlowServer1->GetAttribute ("LocalAddress", addressValue1);
        InetSocketAddress address1 = InetSocketAddress::ConvertFrom(addressValue1.Get());
        ASSERT_TRUE(
                (address0.GetPort() == 888 && address1.GetPort() == 1000) ||
                (address0.GetPort() == 1000 && address1.GetPort() == 888)
        );
        if (address0.GetPort() == 888 && address1.GetPort() == 1000) {
            ASSERT_EQUAL(tcpFlowServer0->GetTotalRx(), 110000);
            ASSERT_EQUAL(tcpFlowServer1->GetTotalRx(), 100000);
        } else {
            ASSERT_EQUAL(tcpFlowServer0->GetTotalRx(), 100000);
            ASSERT_EQUAL(tcpFlowServer1->GetTotalRx(), 110000);
        }

        basicSimulation->Finalize();

        // For which logging was enabled (all)
        std::set<int64_t> tcp_flow_ids_with_logging;
        tcp_flow_ids_with_logging.insert(0);
        tcp_flow_ids_with_logging.insert(1);

        // For results (in this case, we don't check this, as we just care about logging)
        std::vector<int64_t> end_time_ns_list;
        std::vector<int64_t> sent_byte_list;
        std::vector<std::string> finished_list;

        // Validate TCP flow logs
        validate_tcp_flow_logs(
                simulation_end_time_ns,
                test_run_dir,
                write_schedule,
                tcp_flow_ids_with_logging,
                end_time_ns_list,
                sent_byte_list,
                finished_list
        );

        // Check they both have finished
        ASSERT_EQUAL(finished_list.at(0), "YES");
        ASSERT_EQUAL(finished_list.at(1), "YES");

        // Make sure these are removed
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/tcp_flow_schedule.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_queue_byte.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_net_device_queue_pkt.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_byte.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_pkt.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.csv");
        remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flows.txt");
        for (int64_t i : tcp_flow_ids_with_logging) {
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_progress.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_rtt.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_rto.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cwnd.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cwnd_inflated.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_ssthresh.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_inflight.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_state.csv");
            remove_file_if_exists(test_run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cong_state.csv");
        }
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);
    }
};

////////////////////////////////////////////////////////////////////////////////////////
