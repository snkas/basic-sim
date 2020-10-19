/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/test.h"
#include "ns3/topology-ptop.h"
#include "../test-helpers.h"
#include "ns3/ipv4-arbiter-routing-helper.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/fq-codel-queue-disc.h"
#include "ns3/ptop-link-utilization-tracker-helper.h"
#include "ns3/udp-burst-scheduler.h"

using namespace ns3;





////////////////////////////////////////////////////////////////////////////////////////

class PtopLinkUtilizationBaseTestCase : public TestCase
{
public:
    PtopLinkUtilizationBaseTestCase (std::string s) : TestCase (s) {};
    const std::string ptop_utilization_test_dir = ".tmp-ptop-utilization-test";

    void prepare_test_dir() {
        mkdir_if_not_exists(ptop_utilization_test_dir);
        remove_file_if_exists(ptop_utilization_test_dir + "/config_ns3.properties");
        remove_file_if_exists(ptop_utilization_test_dir + "/topology.properties");
        remove_file_if_exists(ptop_utilization_test_dir + "/udp_burst_schedule.csv");
    }

    void write_basic_config(std::string log_for_links) {
        std::ofstream config_file(ptop_utilization_test_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=1950000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties.temp\"" << std::endl;
        config_file << "enable_link_utilization_tracking=true" << std::endl;
        config_file << "link_utilization_tracking_interval_ns=100000000" << std::endl;
        config_file << "link_utilization_tracking_enable_for_links=" << log_for_links << std::endl;
        config_file << "enable_udp_burst_scheduler=true" << std::endl;
        config_file << "udp_burst_schedule_filename=\"udp_burst_schedule.csv\"" << std::endl;
        config_file.close();
    }

    void write_four_side_topology() {
        std::ofstream topology_file;
        topology_file.open(ptop_utilization_test_dir + "/topology.properties.temp");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-3,0-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10" << std::endl;
        topology_file << "link_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file << "all_nodes_are_endpoints=true" << std::endl;
        topology_file.close();
    }

    void run_default() {

        // Create simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(ptop_utilization_test_dir);

        // Create topology
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // Schedule UDP bursts
        UdpBurstScheduler udpBurstScheduler(basicSimulation, topology);

        // Install utilization trackers
        PtopLinkUtilizationTrackerHelper utilTrackerHelper = PtopLinkUtilizationTrackerHelper(basicSimulation, topology);

        // Run simulation
        basicSimulation->Run();

        // Write utilization results
        utilTrackerHelper.WriteResults();

        // Finalize the simulation
        basicSimulation->Finalize();

    }

    void validate_link_utilization_logs(
            std::vector<std::pair<int64_t, int64_t>> dir_a_b_list,
            int64_t duration_ns,
            int64_t link_utilization_tracking_interval_ns,
            std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>>& link_utilization,
            std::map<std::pair<int64_t, int64_t>, double>& link_overall_utilization_as_fraction
    ) {

        // Expected number of entries per pair
        size_t expected_num_entries_per_pair = (size_t) std::ceil((double) duration_ns / (double) link_utilization_tracking_interval_ns);

        // Sort it
        std::sort(dir_a_b_list.begin(), dir_a_b_list.end());

        // Initialize result
        for (std::pair<int64_t, int64_t> a_b : dir_a_b_list) {
            link_utilization[a_b] = std::vector<std::tuple<int64_t, int64_t, int64_t>>();
            link_overall_utilization_as_fraction[a_b] = 0.0;
        }

        // link_utilization.csv
        std::vector<int64_t> correct_busy_time_ns;
        std::vector<std::string> lines_csv = read_file_direct(ptop_utilization_test_dir + "/logs_ns3/link_utilization.csv");
        ASSERT_EQUAL(lines_csv.size(), expected_num_entries_per_pair * dir_a_b_list.size());
        size_t line_i = 0;
        for (std::pair<int64_t, int64_t> dir_a_b : dir_a_b_list) {
            for (size_t j = 0; j < expected_num_entries_per_pair; j++) {
                std::string line = lines_csv[line_i];

                // Extract values
                std::vector<std::string> comma_split = split_string(line, ",", 5);
                int64_t from_node_id = parse_positive_int64(comma_split[0]);
                int64_t to_node_id = parse_positive_int64(comma_split[1]);
                int64_t interval_start_ns = parse_positive_int64(comma_split[2]);
                int64_t interval_end_ns = parse_positive_int64(comma_split[3]);
                int64_t interval_busy_time_ns = parse_positive_int64(comma_split[4]);

                // Validate values
                ASSERT_EQUAL(dir_a_b.first, from_node_id);
                ASSERT_EQUAL(dir_a_b.second, to_node_id);
                ASSERT_EQUAL(interval_start_ns, (int64_t) (j * link_utilization_tracking_interval_ns));
                ASSERT_EQUAL(interval_end_ns, (int64_t) std::min((int64_t) (j + 1) * link_utilization_tracking_interval_ns, duration_ns));

                // For keeping track later
                correct_busy_time_ns.push_back(interval_busy_time_ns);

                // Result
                link_utilization[std::make_pair(dir_a_b.first, dir_a_b.second)].push_back(std::make_tuple(interval_start_ns, interval_end_ns, interval_busy_time_ns));

                line_i++;
            }
        }

        // Correct amount of lines present
        ASSERT_EQUAL(line_i, expected_num_entries_per_pair * dir_a_b_list.size());
        for (std::pair<int64_t, int64_t> a_b : dir_a_b_list) {
            ASSERT_EQUAL(expected_num_entries_per_pair, link_utilization.at(a_b).size());
        }

        // link_utilization_compressed.csv/txt
        std::vector<std::string> lines_compressed_csv = read_file_direct(ptop_utilization_test_dir + "/logs_ns3/link_utilization_compressed.csv");
        std::vector<std::string> lines_compressed_txt = read_file_direct(ptop_utilization_test_dir + "/logs_ns3/link_utilization_compressed.txt");

        // They are the same, except one is human-readable
        ASSERT_EQUAL(lines_compressed_csv.size(), lines_compressed_txt.size() - 1);

        // Correct exact header
        ASSERT_EQUAL(lines_compressed_txt[0], "From     To       Interval start (ms)   Interval end (ms)     Utilization");

        // Go over all the lines
        int64_t correct_busy_time_idx = 0;
        int64_t counter_ns = 0;
        int64_t prev_interval_end_ns = 0;
        size_t current_edge_idx = 0;
        for (size_t i = 0; i < lines_compressed_csv.size(); i++) {
            std::string line_csv = lines_compressed_csv[i];
            std::string line_txt = lines_compressed_txt[i + 1];

            // Split the CSV line
            std::vector<std::string> line_csv_spl = split_string(line_csv, ",", 5);

            // Split the text line
            std::vector<std::string> line_txt_spl;
            std::istringstream iss(line_txt);
            for (std::string s; iss >> s;) {
                line_txt_spl.push_back(s);
            }

            // Check from-to
            ASSERT_EQUAL(parse_positive_int64(line_csv_spl[0]), dir_a_b_list.at(current_edge_idx).first);
            ASSERT_EQUAL(parse_positive_int64(line_csv_spl[1]), dir_a_b_list.at(current_edge_idx).second);
            ASSERT_EQUAL(parse_positive_int64(line_csv_spl[0]), parse_positive_int64(line_txt_spl[0]));
            ASSERT_EQUAL(parse_positive_int64(line_csv_spl[1]), parse_positive_int64(line_txt_spl[1]));

            // Check interval
            ASSERT_EQUAL(parse_positive_int64(line_csv_spl[2]), prev_interval_end_ns);
            ASSERT_EQUAL_APPROX(parse_positive_int64(line_csv_spl[2]) / 1000000.0, parse_positive_double(line_txt_spl[2]), 0.01);
            ASSERT_EQUAL_APPROX(parse_positive_int64(line_csv_spl[3]) / 1000000.0, parse_positive_double(line_txt_spl[3]), 0.01);

            // Check that utilization matches up
            double util_100_csv = (double) parse_positive_int64(line_csv_spl[4]) / (double) (parse_positive_int64(line_csv_spl[3]) - parse_positive_int64(line_csv_spl[2])) * 100.0;
            double util_100_txt = parse_positive_double(line_txt_spl[4].substr(0, line_txt_spl[4].size() - 1));
            ASSERT_EQUAL_APPROX(
                    util_100_csv,
                    util_100_txt,
                    0.01
            );
            ASSERT_EQUAL(line_txt_spl[4].substr(line_txt_spl[4].size() - 1, line_txt_spl[4].size()), "%");
            prev_interval_end_ns = parse_positive_int64(line_csv_spl[3]);

            // Now finally check that the utilization values are correct
            int64_t expected_sum_busy_time_ns = 0;
            while (counter_ns < parse_positive_int64(line_csv_spl[3])) {
                counter_ns += 100000000;
                expected_sum_busy_time_ns += correct_busy_time_ns.at(correct_busy_time_idx);
                correct_busy_time_idx += 1;
            }
            ASSERT_EQUAL(parse_positive_int64(line_csv_spl[4]), expected_sum_busy_time_ns);

            // If last interval, move onto the next edge
            if (parse_positive_int64(line_csv_spl[3]) == duration_ns) {
                current_edge_idx += 1;
                prev_interval_end_ns = 0;
                counter_ns = 0;
            }

        }
        ASSERT_EQUAL(current_edge_idx, dir_a_b_list.size());
        ASSERT_EQUAL(prev_interval_end_ns, 0);
        ASSERT_EQUAL(counter_ns, 0);
        ASSERT_EQUAL(correct_busy_time_idx, (int64_t) (dir_a_b_list.size() * expected_num_entries_per_pair));

        // link_utilization_summary.txt
        std::vector<std::string> lines_summary_txt = read_file_direct(ptop_utilization_test_dir + "/logs_ns3/link_utilization_summary.txt");
        ASSERT_EQUAL(lines_summary_txt.size(), dir_a_b_list.size() + 1);

        // Correct exact header
        ASSERT_EQUAL(lines_summary_txt[0], "From     To       Utilization");

        // Go over all the lines
        for (size_t i = 0; i < lines_summary_txt.size() - 1; i++) {
            std::string line_txt = lines_summary_txt[i + 1];

            // Split the text line
            std::vector<std::string> line_txt_spl;
            std::istringstream iss(line_txt);
            for (std::string s; iss >> s;) {
                line_txt_spl.push_back(s);
            }

            // Check from-to
            ASSERT_EQUAL(parse_positive_int64(line_txt_spl[0]), dir_a_b_list.at(i).first);
            ASSERT_EQUAL(parse_positive_int64(line_txt_spl[1]), dir_a_b_list.at(i).second);

            // Expected busy fraction = utilization
            int64_t total_busy_time_ns = 0;
            for (size_t j = 0; j < link_utilization.at(dir_a_b_list.at(i)).size(); j++) {
                total_busy_time_ns += std::get<2>(link_utilization.at(dir_a_b_list.at(i)).at(j));
            }
            link_overall_utilization_as_fraction[dir_a_b_list.at(i)] = (double) total_busy_time_ns / (double) duration_ns;

            // Check that utilization matches up
            double util_100_txt = parse_positive_double(line_txt_spl[2].substr(0, line_txt_spl[2].size() - 1));
            ASSERT_EQUAL_APPROX(util_100_txt, link_overall_utilization_as_fraction.at(dir_a_b_list.at(i)) * 100.0, 0.01);
            ASSERT_EQUAL(line_txt_spl[2].substr(line_txt_spl[2].size() - 1, line_txt_spl[2].size()), "%");

        }

    }

    void cleanup() {
        remove_file_if_exists(ptop_utilization_test_dir + "/config_ns3.properties");
        remove_file_if_exists(ptop_utilization_test_dir + "/topology.properties.temp");
        remove_file_if_exists(ptop_utilization_test_dir + "/udp_burst_schedule.csv");
        remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/link_utilization.csv");
        remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/link_utilization_compressed.csv");
        remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/link_utilization_compressed.txt");
        remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/link_utilization_summary.txt");
        remove_dir_if_exists(ptop_utilization_test_dir + "/logs_ns3");
        remove_dir_if_exists(ptop_utilization_test_dir);
    }

};

////////////////////////////////////////////////////////////////////////////////////////

class PtopLinkUtilizationSimpleTestCase : public PtopLinkUtilizationBaseTestCase
{
public:
    PtopLinkUtilizationSimpleTestCase () : PtopLinkUtilizationBaseTestCase ("ptop-link-utilization simple") {};
    void DoRun () {

        prepare_test_dir();
        write_basic_config("all");
        write_four_side_topology();
        std::ofstream udp_burst_schedule_file;
        udp_burst_schedule_file.open (ptop_utilization_test_dir + "/udp_burst_schedule.csv");
        udp_burst_schedule_file << "0,0,1,50,0,500000000,," << std::endl;
        udp_burst_schedule_file << "1,2,3,90,250000000,500000000,," << std::endl;
        udp_burst_schedule_file << "2,3,2,90,250000000,500000000,," << std::endl;
        udp_burst_schedule_file << "3,1,3,90,250000000,5000000000,," << std::endl;
        udp_burst_schedule_file.close();

        // Run it
        run_default();

        // Directed edge list
        std::vector <std::pair<int64_t, int64_t>> dir_a_b_list;
        dir_a_b_list.push_back(std::make_pair(0, 1));
        dir_a_b_list.push_back(std::make_pair(1, 0));
        dir_a_b_list.push_back(std::make_pair(0, 2));
        dir_a_b_list.push_back(std::make_pair(2, 0));
        dir_a_b_list.push_back(std::make_pair(1, 3));
        dir_a_b_list.push_back(std::make_pair(3, 1));
        dir_a_b_list.push_back(std::make_pair(3, 2));
        dir_a_b_list.push_back(std::make_pair(2, 3));

        // Check results
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_utilization;
        std::map<std::pair<int64_t, int64_t>, double> link_overall_utilization_as_fraction;
        validate_link_utilization_logs(dir_a_b_list, 1950000000, 100000000, link_utilization, link_overall_utilization_as_fraction);

        // Specific interval's link utilization
        for (std::pair<int64_t, int64_t> dir_a_b : dir_a_b_list) {
            ASSERT_EQUAL(link_utilization.at(dir_a_b).size(), 20);
            for (int j = 0; j < 20; j++) {
                int64_t busy_time_ns = std::get<2>(link_utilization.at(dir_a_b).at(j));
                if (dir_a_b.first == 0 and dir_a_b.second == 1) {
                    if (j < 5) {
                        ASSERT_TRUE(busy_time_ns >= 50000000);
                    } else if (j == 5) {
                        ASSERT_TRUE(busy_time_ns <= 100000);
                    } else {
                        ASSERT_EQUAL(busy_time_ns, 0);
                    }
                } else if ((dir_a_b.first == 2 and dir_a_b.second == 3) ||
                           (dir_a_b.first == 3 and dir_a_b.second == 2)) {
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
            }
        }

        // Check that overall utilization matches up
        ASSERT_EQUAL_APPROX(link_overall_utilization_as_fraction.at(std::make_pair(0, 1)), 25.0 / 195.0, 0.2);
        ASSERT_EQUAL(link_overall_utilization_as_fraction.at(std::make_pair(1, 0)), 0);
        ASSERT_EQUAL(link_overall_utilization_as_fraction.at(std::make_pair(0, 2)), 0);
        ASSERT_EQUAL(link_overall_utilization_as_fraction.at(std::make_pair(2, 0)), 0);
        ASSERT_EQUAL_APPROX(link_overall_utilization_as_fraction.at(std::make_pair(2, 3)), 45.0 / 195.0, 0.2);
        ASSERT_EQUAL_APPROX(link_overall_utilization_as_fraction.at(std::make_pair(3, 2)), 45.0 / 195.0, 0.2);
        ASSERT_EQUAL_APPROX(link_overall_utilization_as_fraction.at(std::make_pair(1, 3)), 153.0 / 195.0, 0.2);
        ASSERT_EQUAL(link_overall_utilization_as_fraction.at(std::make_pair(3, 1)), 0);

        // Clean up
        cleanup();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PtopLinkUtilizationSpecificLinksTestCase : public PtopLinkUtilizationBaseTestCase
{
public:
    PtopLinkUtilizationSpecificLinksTestCase () : PtopLinkUtilizationBaseTestCase ("ptop-link-utilization specific-links") {};
    void DoRun () {

        prepare_test_dir();
        write_basic_config("set(0->1, 3->1, 2->0)");
        write_four_side_topology();
        std::ofstream udp_burst_schedule_file;
        udp_burst_schedule_file.open (ptop_utilization_test_dir + "/udp_burst_schedule.csv");
        udp_burst_schedule_file << "0,0,1,50,0,500000000,," << std::endl;
        udp_burst_schedule_file << "1,2,3,90,250000000,500000000,," << std::endl;
        udp_burst_schedule_file << "2,3,2,90,250000000,500000000,," << std::endl;
        udp_burst_schedule_file << "3,1,3,90,250000000,5000000000,," << std::endl;
        udp_burst_schedule_file.close();

        // Run it
        run_default();

        // Directed edge list
        std::vector <std::pair<int64_t, int64_t>> dir_a_b_list;
        dir_a_b_list.push_back(std::make_pair(0, 1));
        dir_a_b_list.push_back(std::make_pair(2, 0));
        dir_a_b_list.push_back(std::make_pair(3, 1));

        // Check results
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>> link_utilization;
        std::map<std::pair<int64_t, int64_t>, double> link_overall_utilization_as_fraction;
        validate_link_utilization_logs(dir_a_b_list, 1950000000, 100000000, link_utilization, link_overall_utilization_as_fraction);

        // Specific interval's link utilization
        for (std::pair<int64_t, int64_t> dir_a_b : dir_a_b_list) {
            ASSERT_EQUAL(link_utilization.at(dir_a_b).size(), 20);
            for (int j = 0; j < 20; j++) {
                int64_t busy_time_ns = std::get<2>(link_utilization.at(dir_a_b).at(j));
                if (dir_a_b.first == 0 and dir_a_b.second == 1) {
                    if (j < 5) {
                        ASSERT_TRUE(busy_time_ns >= 50000000);
                    } else if (j == 5) {
                        ASSERT_TRUE(busy_time_ns <= 100000);
                    } else {
                        ASSERT_EQUAL(busy_time_ns, 0);
                    }
                } else {
                    ASSERT_EQUAL(busy_time_ns, 0);
                }
            }
        }

        // Check that overall utilization matches up
        ASSERT_EQUAL_APPROX(link_overall_utilization_as_fraction.at(std::make_pair(0, 1)), 25.0 / 195.0, 0.2);
        ASSERT_EQUAL(link_overall_utilization_as_fraction.at(std::make_pair(2, 0)), 0);
        ASSERT_EQUAL(link_overall_utilization_as_fraction.at(std::make_pair(3, 1)), 0);

        // Clean up
        cleanup();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PtopLinkUtilizationNotEnabledTestCase : public TestCase
{
public:
    PtopLinkUtilizationNotEnabledTestCase () : TestCase ("ptop-link-utilization not-enabled") {};
    const std::string ptop_utilization_test_dir = ".tmp-ptop-utilization-test";
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
        topology_file << "link_device_receive_error_model=none" << std::endl;
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
        PtopLinkUtilizationTrackerHelper utilTrackerHelper = PtopLinkUtilizationTrackerHelper(basicSimulation, topology);

        // Run simulation
        basicSimulation->Run();

        // Write utilization results
        utilTrackerHelper.WriteResults();

        // Finalize the simulation
        basicSimulation->Finalize();

        // Nothing should have been logged
        ASSERT_FALSE(file_exists(ptop_utilization_test_dir + "/logs_ns3/link_utilization.csv"));
        ASSERT_FALSE(file_exists(ptop_utilization_test_dir + "/logs_ns3/link_utilization_compressed.csv"));
        ASSERT_FALSE(file_exists(ptop_utilization_test_dir + "/logs_ns3/link_utilization_compressed.txt"));
        ASSERT_FALSE(file_exists(ptop_utilization_test_dir + "/logs_ns3/link_utilization_summary.txt"));

        // Clean up
        remove_file_if_exists(ptop_utilization_test_dir + "/config_ns3.properties");
        remove_file_if_exists(ptop_utilization_test_dir + "/topology.properties.temp");
        remove_file_if_exists(ptop_utilization_test_dir + "/udp_burst_schedule.csv");
        remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/link_utilization.csv");
        remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/link_utilization_compressed.csv");
        remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/link_utilization_compressed.txt");
        remove_file_if_exists(ptop_utilization_test_dir + "/logs_ns3/link_utilization_summary.txt");
        remove_dir_if_exists(ptop_utilization_test_dir + "/logs_ns3");
        remove_dir_if_exists(ptop_utilization_test_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////
