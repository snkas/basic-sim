/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/flow-scheduler.h"
#include "ns3/tcp-optimizer.h"
#include "ns3/arbiter-ecmp-helper.h"
#include "ns3/test.h"
#include "../test-helpers.h"
#include <iostream>
#include <fstream>
#include "ns3/pingmesh-scheduler.h"

using namespace ns3;

////////////////////////////////////////////////////////////////////////////////////////

class EndToEndPingmeshTestCase : public TestCase {
public:
    EndToEndPingmeshTestCase(std::string s) : TestCase(s) {};
    const std::string temp_dir = ".tmp-end-to-end-pingmesh-test";

    void prepare_test_dir() {
        mkdir_if_not_exists(temp_dir);
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
    }

    void write_basic_config(int64_t simulation_end_time_ns, int64_t pingmesh_interval_ns, std::string pingmesh_endpoint_pairs) {
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        config_file << "filename_topology=\"topology.properties\"" << std::endl;
        config_file << "pingmesh_interval_ns=" << pingmesh_interval_ns << std::endl;
        config_file << "simulation_end_time_ns=5000000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "link_data_rate_megabit_per_s=10000" << std::endl;
        config_file << "link_delay_ns=50000000" << std::endl;
        config_file << "link_max_queue_size_pkts=10000" << std::endl;
        config_file << "disable_qdisc_endpoint_tors_xor_servers=true" << std::endl;
        config_file << "disable_qdisc_non_endpoint_switches=true" << std::endl;
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
        topology_file.close();
    }

    void test_run_and_simple_validate(int64_t simulation_end_time_ns, std::string temp_dir, uint32_t expected_num_pings, std::map<std::pair<int64_t, int64_t>, int64_t> pair_to_expected_latency, int64_t max_delta_more_latency) {

        // Make sure these are removed
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/pingmesh.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/pingmesh.csv");

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        PingmeshScheduler pingmeshScheduler(basicSimulation, topology); // Requires pingmesh_interval_ns to be present in the configuration
        pingmeshScheduler.Schedule();
        basicSimulation->Run();
        pingmeshScheduler.WriteResults();
        basicSimulation->Finalize();

        // Check finished.txt
        std::vector<std::string> finished_lines = read_file_direct(temp_dir + "/logs_ns3/finished.txt");
        ASSERT_EQUAL(finished_lines.size(), 1);
        ASSERT_EQUAL(finished_lines[0], "Yes");

        // Check pingmesh.csv
        std::vector<std::string> lines_csv = read_file_direct(temp_dir + "/logs_ns3/pingmesh.csv");
        std::pair<int64_t, int64_t> current = std::make_pair(-1, -1);
        int64_t counter = 0;
        for (std::string line : lines_csv) {
            std::vector<std::string> spl = split_string(line, ",", 10);
            int64_t from = parse_positive_int64(spl[0]);
            int64_t to = parse_positive_int64(spl[1]);
            int64_t i = parse_positive_int64(spl[2]);
            if (std::make_pair(from, to) != current) {
                counter = 0;
                current = std::make_pair(from, to);
            }
            ASSERT_EQUAL(i, counter);
            counter++;
            int64_t sent = std::stoll(spl[3]);
            int64_t reply = std::stoll(spl[4]);
            int64_t got_reply = std::stoll(spl[5]);
            int64_t way_there = std::stoll(spl[6]);
            int64_t way_back = std::stoll(spl[7]);
            int64_t rtt = std::stoll(spl[8]);
            bool arrived = true;
            if (trim(spl[9]) == "YES") {
                arrived = true;
            } else if (trim(spl[9]) == "LOST") {
                arrived = false;
            } else {
                ASSERT_TRUE(false);
            }
            ASSERT_TRUE(topology->IsValidEndpoint(from));
            ASSERT_TRUE(topology->IsValidEndpoint(to));
            ASSERT_TRUE(i >= 0);
            ASSERT_TRUE(sent >= 0);
            if (arrived) {
                ASSERT_TRUE(reply >= sent);
                ASSERT_TRUE(got_reply >= reply);
                ASSERT_EQUAL(way_there, reply - sent);
                ASSERT_EQUAL(way_back, got_reply - reply);
                ASSERT_EQUAL(rtt, got_reply - sent);
                ASSERT_TRUE(pair_to_expected_latency.find(std::make_pair(from, to)) != pair_to_expected_latency.end());
                int64_t expected_latency = pair_to_expected_latency.find(std::make_pair(from, to))->second;
                ASSERT_TRUE(way_there >= expected_latency);
                ASSERT_TRUE(way_there >= expected_latency);
                ASSERT_TRUE(way_back <= expected_latency + max_delta_more_latency);
                ASSERT_TRUE(way_back <= expected_latency + max_delta_more_latency);
            } else {
                ASSERT_EQUAL(reply, -1);
                ASSERT_EQUAL(got_reply, -1);
                ASSERT_EQUAL(way_there, -1);
                ASSERT_EQUAL(way_back, -1);
                ASSERT_EQUAL(rtt, -1);
            }
        }

        // Make sure these are removed
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/pingmesh.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/pingmesh.txt");
        remove_dir_if_exists(temp_dir + "/logs_ns3");
        remove_dir_if_exists(temp_dir);

    }

};

class EndToEndPingmeshNineAllTestCase : public EndToEndPingmeshTestCase
{
public:
    EndToEndPingmeshNineAllTestCase () : EndToEndPingmeshTestCase ("pingmesh nine-all") {};

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

        // Perform the run
        test_run_and_simple_validate(5000000000, temp_dir, 100, pair_to_expected_latency, 1000);

    }
};

class EndToEndPingmeshNinePairsTestCase : public EndToEndPingmeshTestCase
{
public:
    EndToEndPingmeshNinePairsTestCase () : EndToEndPingmeshTestCase ("pingmesh nine-pairs") {};

    void DoRun () {
        prepare_test_dir();

        // 5 seconds, every 100ms a ping
        write_basic_config(5000000000, 100000000, "set(2-1, 1-2, 0-5, 5-2, 3-1)");
        write_six_topology();
        std::map<std::pair<int64_t, int64_t>, int64_t> pair_to_expected_latency;
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(0, 5), 150000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(1, 2), 50000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(2, 1), 50000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(3, 1), 100000000));
        pair_to_expected_latency.insert(std::make_pair(std::make_pair(5, 2), 50000000));

        // Perform the run
        test_run_and_simple_validate(5000000000, temp_dir, 100, pair_to_expected_latency, 1000);

    }
};

////////////////////////////////////////////////////////////////////////////////////////
