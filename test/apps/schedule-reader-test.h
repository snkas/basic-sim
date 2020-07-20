/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/test.h"
#include "ns3/exp-util.h"
#include "ns3/schedule-reader.h"
#include "ns3/topology-ptop.h"
#include "ns3/ipv4-arbiter-routing-helper.h"
#include "../test-helpers.h"

using namespace ns3;

const std::string schedule_reader_test_dir = ".tmp-scheduler-reader-test";

void prepare_schedule_reader_test_config() {
    mkdir_if_not_exists(schedule_reader_test_dir);
}

void cleanup_schedule_reader_test() {
    remove_file_if_exists(schedule_reader_test_dir + "/config_ns3.properties");
    remove_file_if_exists(schedule_reader_test_dir + "/topology.properties");
    remove_file_if_exists(schedule_reader_test_dir + "/schedule.csv");
    remove_file_if_exists(schedule_reader_test_dir + "/logs_ns3/finished.txt");
    remove_file_if_exists(schedule_reader_test_dir + "/logs_ns3/timing_results.txt");
    remove_dir_if_exists(schedule_reader_test_dir + "/logs_ns3");
    remove_dir_if_exists(schedule_reader_test_dir);
}

////////////////////////////////////////////////////////////////////////////////////////

class ScheduleReaderNormalTestCase : public TestCase
{
public:
    ScheduleReaderNormalTestCase () : TestCase ("schedule-reader normal") {};

    void DoRun () {
        prepare_schedule_reader_test_config();

        // Normal

        std::ofstream config_file(schedule_reader_test_dir + "/config_ns3.properties");
        config_file << "topology_filename=\"topology.properties\"" << std::endl;
        config_file << "flow_schedule_filename=\"schedule.csv\"" << std::endl;
        config_file << "simulation_end_time_ns=10000000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_link_data_rate_megabit_per_s=100.0" << std::endl;
        config_file << "topology_link_delay_ns=10000" << std::endl;
        config_file << "topology_max_queue_size_pkt=100" << std::endl;
        config_file << "topology_disable_traffic_control_endpoint_tors_xor_servers=true" << std::endl;
        config_file << "topology_disable_traffic_control_non_endpoint_switches=true" << std::endl;
        config_file.close();

        std::ofstream schedule_file(schedule_reader_test_dir + "/schedule.csv");
        schedule_file << "0,0,1,100000,47327,a=b,test" << std::endl;
        schedule_file << "1,7,3,7488338,1356567,a=b2," << std::endl;
        schedule_file.close();

        std::ofstream topology_file;
        topology_file.open (schedule_reader_test_dir + "/topology.properties");
        topology_file << "num_nodes=8" << std::endl;
        topology_file << "num_undirected_edges=7" << std::endl;
        topology_file << "switches=set(0,1,2,3,4,5,6,7)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1,2,3,4,5,6,7)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3,3-4,4-5,5-6,6-7)" << std::endl;
        topology_file.close();

        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(schedule_reader_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        std::vector<schedule_entry_t> schedule = read_schedule(schedule_reader_test_dir + "/schedule.csv", topology, 10000000000);

        ASSERT_EQUAL(schedule.size(), 2);

        ASSERT_EQUAL(schedule[0].flow_id, 0);
        ASSERT_EQUAL(schedule[0].from_node_id, 0);
        ASSERT_EQUAL(schedule[0].to_node_id, 1);
        ASSERT_EQUAL(schedule[0].size_byte, 100000);
        ASSERT_EQUAL(schedule[0].start_time_ns, 47327);
        ASSERT_EQUAL(schedule[0].additional_parameters, "a=b");
        ASSERT_EQUAL(schedule[0].metadata, "test");

        ASSERT_EQUAL(schedule[1].flow_id, 1);
        ASSERT_EQUAL(schedule[1].from_node_id, 7);
        ASSERT_EQUAL(schedule[1].to_node_id, 3);
        ASSERT_EQUAL(schedule[1].size_byte, 7488338);
        ASSERT_EQUAL(schedule[1].start_time_ns, 1356567);
        ASSERT_EQUAL(schedule[1].additional_parameters, "a=b2");
        ASSERT_EQUAL(schedule[1].metadata, "");

        // Empty

        std::ofstream schedule_file_empty(schedule_reader_test_dir + "/schedule.csv");
        schedule_file_empty.close();
        std::vector<schedule_entry_t> schedule_empty = read_schedule(schedule_reader_test_dir + "/schedule.csv", topology, 10000000);
        ASSERT_EQUAL(schedule_empty.size(), 0);

        basicSimulation->Finalize();
        cleanup_schedule_reader_test();

    }
};

class ScheduleReaderInvalidTestCase : public TestCase
{
public:
    ScheduleReaderInvalidTestCase () : TestCase ("schedule-reader invalid") {};

    void DoRun () {
        prepare_schedule_reader_test_config();

        std::ofstream schedule_file;
        std::vector<schedule_entry_t> schedule;

        std::ofstream config_file(schedule_reader_test_dir + "/config_ns3.properties");
        config_file << "topology_filename=\"topology.properties\"" << std::endl;
        config_file << "flow_schedule_filename=\"schedule.csv\"" << std::endl;
        config_file << "simulation_end_time_ns=10000000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_link_data_rate_megabit_per_s=100.0" << std::endl;
        config_file << "topology_link_delay_ns=10000" << std::endl;
        config_file << "topology_max_queue_size_pkt=100" << std::endl;
        config_file << "topology_disable_traffic_control_endpoint_tors_xor_servers=true" << std::endl;
        config_file << "topology_disable_traffic_control_non_endpoint_switches=true" << std::endl;
        config_file.close();

        std::ofstream topology_file;
        topology_file.open (schedule_reader_test_dir + "/topology.properties");
        topology_file << "num_nodes=5" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3,4)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1,3,4)" << std::endl; // Only 2 cannot be endpoint
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3,3-4)" << std::endl;
        topology_file.close();

        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(schedule_reader_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Non-existent file
        ASSERT_EXCEPTION(read_schedule("does-not-exist-temp.file", topology, 10000000));
        
        // Normal
        schedule_file = std::ofstream(schedule_reader_test_dir + "/schedule.csv");
        schedule_file << "0,0,4,100000,1356567,a=b,test" << std::endl;
        schedule_file.close();
        schedule = read_schedule(schedule_reader_test_dir + "/schedule.csv", topology, 10000000);

        // Source = Destination
        schedule_file = std::ofstream(schedule_reader_test_dir + "/schedule.csv");
        schedule_file << "0,3,3,100000,1356567,a=b,test" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_schedule(schedule_reader_test_dir + "/schedule.csv", topology, 10000000));

        // Invalid source (out of range)
        schedule_file = std::ofstream(schedule_reader_test_dir + "/schedule.csv");
        schedule_file << "0,9,0,100000,1356567,a=b,test" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_schedule(schedule_reader_test_dir + "/schedule.csv", topology, 10000000));

        // Invalid destination (out of range)
        schedule_file = std::ofstream(schedule_reader_test_dir + "/schedule.csv");
        schedule_file << "0,3,6,100000,1356567,a=b,test" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_schedule(schedule_reader_test_dir + "/schedule.csv", topology, 10000000));

        // Invalid source (not a ToR)
        schedule_file = std::ofstream(schedule_reader_test_dir + "/schedule.csv");
        schedule_file << "0,2,4,100000,1356567,a=b,test" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_schedule(schedule_reader_test_dir + "/schedule.csv", topology, 10000000));

        // Invalid destination (not a ToR)
        schedule_file = std::ofstream(schedule_reader_test_dir + "/schedule.csv");
        schedule_file << "0,4,2,100000,1356567,a=b,test" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_schedule(schedule_reader_test_dir + "/schedule.csv", topology, 10000000));

        // Not ascending flow ID
        schedule_file = std::ofstream(schedule_reader_test_dir + "/schedule.csv");
        schedule_file << "1,3,4,100000,1356567,a=b,test" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_schedule(schedule_reader_test_dir + "/schedule.csv", topology, 10000000));

        // Negative flow size
        schedule_file = std::ofstream(schedule_reader_test_dir + "/schedule.csv");
        schedule_file << "0,3,4,-6,1356567,a=b,test" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_schedule(schedule_reader_test_dir + "/schedule.csv", topology, 10000000));

        // Not enough values
        schedule_file = std::ofstream(schedule_reader_test_dir + "/schedule.csv");
        schedule_file << "0,3,4,7778,1356567,a=b" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_schedule(schedule_reader_test_dir + "/schedule.csv", topology, 10000000));

        // Negative time
        schedule_file = std::ofstream(schedule_reader_test_dir + "/schedule.csv");
        schedule_file << "0,3,4,86959,-7,a=b,test" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(schedule = read_schedule(schedule_reader_test_dir + "/schedule.csv", topology, 10000000));

        // Just normal ordered with equal start time
        schedule_file = std::ofstream(schedule_reader_test_dir + "/schedule.csv");
        schedule_file << "0,3,4,86959,9999,a=b,test" << std::endl;
        schedule_file << "1,3,4,86959,9999,a=b,test" << std::endl;
        schedule_file.close();
        schedule = read_schedule(schedule_reader_test_dir + "/schedule.csv", topology, 10000000);

        // Not ordered time
        schedule_file = std::ofstream(schedule_reader_test_dir + "/schedule.csv");
        schedule_file << "0,3,4,86959,10000,a=b,test" << std::endl;
        schedule_file << "1,3,4,86959,9999,a=b,test" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_schedule(schedule_reader_test_dir + "/schedule.csv", topology, 10000000));

        // Exceeding time
        schedule_file = std::ofstream(schedule_reader_test_dir + "/schedule.csv");
        schedule_file << "0,3,4,86959,10000000,a=b,test" << std::endl;
        schedule_file.close();
        ASSERT_EXCEPTION(read_schedule(schedule_reader_test_dir + "/schedule.csv", topology, 10000000));

        basicSimulation->Finalize();
        cleanup_schedule_reader_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////
