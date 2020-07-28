/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/test.h"
#include "ns3/topology-ptop.h"
#include "../test-helpers.h"
#include "ns3/ipv4-arbiter-routing-helper.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/fq-codel-queue-disc.h"
#include "ns3/ptop-utilization-tracker-helper.h"
#include "ns3/udp-burst-scheduler.h"

using namespace ns3;

const std::string ptop_utilization_test_dir = ".tmp-ptop-utilization-test";

void cleanup_ptop_utilization_test() {
    remove_file_if_exists(ptop_utilization_test_dir + "/config_ns3.properties");
    remove_file_if_exists(ptop_utilization_test_dir + "/topology.properties.temp");
    remove_file_if_exists(ptop_utilization_test_dir + "/udp_burst_schedule.csv");
    remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/finished.txt");
    remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/timing_results.txt");
    remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/utilization.csv");
    remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/utilization_compressed.csv");
    remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/utilization_compressed.txt");
    remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/utilization_summary.txt");
    remove_dir_if_exists(ptop_utilization_test_dir + "/logs_ns3");
    remove_dir_if_exists(ptop_utilization_test_dir);
}

////////////////////////////////////////////////////////////////////////////////////////

class PtopUtilizationSimpleTestCase : public TestCase
{
public:
    PtopUtilizationSimpleTestCase () : TestCase ("ptop-utilization simple") {};
    void DoRun () {

        mkdir_if_not_exists(ptop_utilization_test_dir);
        std::ofstream config_file(ptop_utilization_test_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=1950000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties.temp\"" << std::endl;
        config_file << "enable_link_utilization_tracking=true" << std::endl;
        config_file << "link_utilization_tracking_interval_ns=100000000" << std::endl;
        config_file << "enable_udp_burst_scheduler=true" << std::endl;
        config_file << "udp_burst_schedule_filename=\"udp_burst_schedule.csv\"" << std::endl;
        config_file.close();

        std::ofstream udp_burst_schedule_file;
        udp_burst_schedule_file.open (ptop_utilization_test_dir + "/udp_burst_schedule.csv");
        udp_burst_schedule_file << "0,0,1,50,0,500000000,," << std::endl;
        udp_burst_schedule_file << "1,2,3,90,250000000,500000000,," << std::endl;
        udp_burst_schedule_file << "2,3,2,90,250000000,500000000,," << std::endl;
        udp_burst_schedule_file << "3,1,3,90,250000000,5000000000,," << std::endl;
        udp_burst_schedule_file.close();

        std::ofstream topology_file;
        topology_file.open (ptop_utilization_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-3,0-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file << "all_nodes_are_endpoints=true" << std::endl;
        topology_file.close();

        // Create simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(ptop_utilization_test_dir);

        // Create topology
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // Schedule UDP bursts
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology);
        
        // Install utilization trackers
        PtopUtilizationTrackerHelper utilTrackerHelper = PtopUtilizationTrackerHelper(basicSimulation, topology);

        // Run simulation
        basicSimulation->Run();

        // Write utilization results
        utilTrackerHelper.WriteResults();

        // Finalize the simulation
        basicSimulation->Finalize();

        // utilization.csv
        std::vector<std::string> lines_csv = read_file_direct(ptop_utilization_test_dir + "/logs_ns3/utilization.csv");
        int i = 0;
        for (std::pair<int64_t, int64_t> a_b : topology->GetUndirectedEdges()) {
            std::vector<std::pair<int64_t, int64_t>> dir_a_b_list;
            dir_a_b_list.push_back(a_b);
            dir_a_b_list.push_back(std::make_pair(a_b.second, a_b.first));
            for (std::pair<int64_t, int64_t> dir_a_b : dir_a_b_list) {
                for (int j = 0; j < 20; j++) {
                    std::string line = lines_csv[i];
                    std::vector<std::string> comma_split = split_string(line, ",", 5);
                    ASSERT_EQUAL(parse_positive_int64(comma_split[0]), dir_a_b.first);
                    ASSERT_EQUAL(parse_positive_int64(comma_split[1]), dir_a_b.second);
                    ASSERT_EQUAL(parse_positive_int64(comma_split[2]), (int64_t) (j * 100000000));
                    int64_t expected_end_time_ns = (j + 1) * 100000000;
                    if (expected_end_time_ns > 1950000000) {
                        expected_end_time_ns = 1950000000;
                    }
                    ASSERT_EQUAL(parse_positive_int64(comma_split[3]), expected_end_time_ns);
                    int64_t busy_time_ns = parse_positive_int64(comma_split[4]);
                    if (dir_a_b.first == 0 and dir_a_b.second == 1) {
                        if (j < 5) {
                            ASSERT_TRUE(busy_time_ns >= 50000000);
                        } else if (j == 5) {
                            ASSERT_TRUE(busy_time_ns <= 100000);
                        } else {
                            ASSERT_EQUAL(busy_time_ns, 0);
                        }
                    } else if ((dir_a_b.first == 2 and dir_a_b.second == 3) || (dir_a_b.first == 3 and dir_a_b.second == 2)) {
                        if (j == 2 or j == 7) {
                            ASSERT_TRUE(busy_time_ns >= 45000000);
                        } else if (j > 2 && j < 7) {
                            ASSERT_TRUE(busy_time_ns >= 90000000);
                        } else {
                            ASSERT_EQUAL(busy_time_ns, 0);
                        }
                    } else if (dir_a_b.first == 1 and dir_a_b.second == 3) {
                        if (j == 2 || j == 19) {
                            ASSERT_TRUE(busy_time_ns >= 45000000);
                        } else if (j > 2) {
                            ASSERT_TRUE(busy_time_ns >= 90000000);
                        } else {
                            ASSERT_EQUAL(busy_time_ns, 0);
                        }
                    } else {
                        ASSERT_EQUAL(busy_time_ns, 0);
                    }
                    i++;
                }
            }
        }

        cleanup_ptop_utilization_test();
    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PtopUtilizationNotEnabledTestCase : public TestCase
{
public:
    PtopUtilizationNotEnabledTestCase () : TestCase ("ptop-utilization not-enabled") {};
    void DoRun () {

        mkdir_if_not_exists(ptop_utilization_test_dir);
        std::ofstream config_file(ptop_utilization_test_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=1950000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties.temp\"" << std::endl;
        config_file << "enable_link_utilization_tracking=false" << std::endl;
        config_file << "enable_udp_burst_scheduler=true" << std::endl;
        config_file << "udp_burst_schedule_filename=\"udp_burst_schedule.csv\"" << std::endl;
        config_file.close();

        std::ofstream udp_burst_schedule_file;
        udp_burst_schedule_file.open (ptop_utilization_test_dir + "/udp_burst_schedule.csv");
        udp_burst_schedule_file << "0,0,1,50,0,500000000,," << std::endl;
        udp_burst_schedule_file.close();

        std::ofstream topology_file;
        topology_file.open (ptop_utilization_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-3,0-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file << "all_nodes_are_endpoints=true" << std::endl;
        topology_file.close();

        // Create simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(ptop_utilization_test_dir);

        // Create topology
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // Schedule UDP bursts
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology);

        // Install utilization trackers
        PtopUtilizationTrackerHelper utilTrackerHelper = PtopUtilizationTrackerHelper(basicSimulation, topology);

        // Run simulation
        basicSimulation->Run();

        // Write utilization results
        utilTrackerHelper.WriteResults();

        // Finalize the simulation
        basicSimulation->Finalize();

        // Nothing should have been logged
        ASSERT_FALSE(file_exists(ptop_utilization_test_dir + "/logs_ns3/utilization.csv"));
        ASSERT_FALSE(file_exists(ptop_utilization_test_dir + "/logs_ns3/utilization_compressed.csv"));
        ASSERT_FALSE(file_exists(ptop_utilization_test_dir + "/logs_ns3/utilization_compressed.txt"));
        ASSERT_FALSE(file_exists(ptop_utilization_test_dir + "/logs_ns3/utilization_summary.txt"));

        cleanup_ptop_utilization_test();
    }
};

////////////////////////////////////////////////////////////////////////////////////////
