/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "test-case-with-log-validators.h"

TestCaseWithLogValidators::TestCaseWithLogValidators (std::string s) : TestCase (s) {
    // Left empty intentionally
};

/**
 * If the run directory exists, it clears all files from the run directory and its logs_ns3 directory.
 * After it is all empty, it removes the logs_ns3 directory and then the run directory.
 * If there are other unexpected files present in the run directory or its logs_ns3,
 * this function will fail with an exception as the directory it would try to remove would be non-empty.
 *
 * Finally, it will create the run directory (again).
 *
 * @param run_dir   Run directory to create (or if its exists, clear, delete, and re-create)
 */
void TestCaseWithLogValidators::prepare_clean_run_dir(std::string run_dir) {

    if (dir_exists(run_dir)) {

        // Run directory
        remove_file_if_exists(run_dir + "/config_ns3.properties");
        remove_file_if_exists(run_dir + "/topology.properties");
        remove_file_if_exists(run_dir + "/tcp_flow_schedule.csv");
        remove_file_if_exists(run_dir + "/udp_burst_schedule.csv");
        remove_file_if_exists(run_dir + "/udp_ping_schedule.csv");

        // Logs directory

        // Basic simulation
        remove_file_if_exists(run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(run_dir + "/logs_ns3/timing_results.csv");

        // TCP flow remnants
        remove_file_if_exists(run_dir + "/logs_ns3/tcp_flows.txt");
        remove_file_if_exists(run_dir + "/logs_ns3/tcp_flows.csv");
        for (size_t i = 0; i < 1000; i++) {
            if (!file_exists(run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_progress.csv")) {
                break; // We stop this long loop once there is one not present
            }
            remove_file_if_exists(run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_progress.csv");
            remove_file_if_exists(run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_rtt.csv");
            remove_file_if_exists(run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_rto.csv");
            remove_file_if_exists(run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cwnd.csv");
            remove_file_if_exists(run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cwnd_inflated.csv");
            remove_file_if_exists(run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_ssthresh.csv");
            remove_file_if_exists(run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_inflight.csv");
            remove_file_if_exists(run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_state.csv");
            remove_file_if_exists(run_dir + "/logs_ns3/tcp_flow_" + std::to_string(i) + "_cong_state.csv");
        }

        // UDP burst remnants
        remove_file_if_exists(run_dir + "/logs_ns3/udp_bursts_outgoing.txt");
        remove_file_if_exists(run_dir + "/logs_ns3/udp_bursts_outgoing.csv");
        remove_file_if_exists(run_dir + "/logs_ns3/udp_bursts_incoming.txt");
        remove_file_if_exists(run_dir + "/logs_ns3/udp_bursts_incoming.csv");
        for (size_t i = 0; i < 1000; i++) {
            if (!file_exists(run_dir + "/logs_ns3/udp_burst_" + std::to_string(i) + "_outgoing.csv")) {
                break; // We stop this long loop once there is one not present
            }
            remove_file_if_exists(run_dir + "/logs_ns3/udp_burst_" + std::to_string(i) + "_outgoing.csv");
            remove_file_if_exists(run_dir + "/logs_ns3/udp_burst_" + std::to_string(i) + "_incoming.csv");
        }

        // UDP ping remnants
        remove_file_if_exists(run_dir + "/logs_ns3/udp_pings.txt");
        remove_file_if_exists(run_dir + "/logs_ns3/udp_pings.csv");

        // Topology link interface traffic control queueing discipline queue
        remove_file_if_exists(run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_pkt.csv");
        remove_file_if_exists(run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_byte.csv");

        // Topology link net-device queue
        remove_file_if_exists(run_dir + "/logs_ns3/link_net_device_queue_pkt.csv");
        remove_file_if_exists(run_dir + "/logs_ns3/link_net_device_queue_byte.csv");

        // Topology link net-device utilization
        remove_file_if_exists(run_dir + "/logs_ns3/link_net_device_utilization.csv");
        remove_file_if_exists(run_dir + "/logs_ns3/link_net_device_utilization_compressed.csv");
        remove_file_if_exists(run_dir + "/logs_ns3/link_net_device_utilization_compressed.txt");
        remove_file_if_exists(run_dir + "/logs_ns3/link_net_device_utilization_summary.txt");

        // Finally, remove the directories themselves (should be empty)
        remove_dir_if_exists(run_dir + "/logs_ns3");
        remove_dir_if_exists(run_dir);

    }

    // Create the run directory
    mkdir_if_not_exists(run_dir);

}

/**
 * Validation of finished.txt
 *
 * @param run_dir           In:  run directory
 */
void TestCaseWithLogValidators::validate_finished(std::string run_dir) {

    // Check finished.txt
    std::vector<std::string> finished_lines = read_file_direct(run_dir + "/logs_ns3/finished.txt");
    ASSERT_EQUAL(finished_lines.size(), 1);
    ASSERT_EQUAL(finished_lines[0], "Yes");

}

/**
 * Validation of link net-device queue logs.
 *
 * @param run_dir           In:  run directory
 * @param dir_a_b_list      In:  list of edges for which logging was done
 * @param link_net_device_queue_pkt    Out: mapping of pair to a vector of intervals <from, to, queue size in byte>
 * @param link_net_device_queue_byte   Out: mapping of pair to a vector of intervals <from, to, queue size in byte>
 */
void TestCaseWithLogValidators::validate_link_net_device_queue_logs(
        std::string run_dir,
        std::vector<std::pair<int64_t, int64_t>> dir_a_b_list,
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>>& link_net_device_queue_pkt,
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>>& link_net_device_queue_byte
) {

    // Logs are only valid if the run is finished
    validate_finished(run_dir);

    // Sort it
    std::sort(dir_a_b_list.begin(), dir_a_b_list.end());

    // Initialize result
    for (std::pair<int64_t, int64_t> a_b : dir_a_b_list) {
        link_net_device_queue_pkt[a_b] = std::vector<std::tuple<int64_t, int64_t, int64_t>>();
        link_net_device_queue_byte[a_b] = std::vector<std::tuple<int64_t, int64_t, int64_t>>();
    }

    // Both link net-device queue files need to be checked, and their checking is nearly identical
    for (size_t file_choice = 0;  file_choice < 2; file_choice++) {

        // Packets or bytes, all the checks are the same basically
        std::vector<std::string> lines_csv;
        if (file_choice == 0) {
            lines_csv = read_file_direct(run_dir + "/logs_ns3/link_net_device_queue_pkt.csv");
        } else {
            lines_csv = read_file_direct(run_dir + "/logs_ns3/link_net_device_queue_byte.csv");
        }

        // State variables when checking
        int64_t current_from_to_idx = -1;
        std::pair<int64_t, int64_t> current_from_to = std::make_pair(-1, -1);
        int64_t previous_interval_end_ns = 0;
        int64_t prev_num_packets_or_bytes = -1;

        // Go over every line
        for (size_t line_i = 0; line_i < lines_csv.size(); line_i++) {
            std::string line = lines_csv[line_i];
            std::vector<std::string> comma_split = split_string(line, ",", 5);
            int64_t from_node_id = parse_positive_int64(comma_split[0]);
            int64_t to_node_id = parse_positive_int64(comma_split[1]);
            int64_t interval_start_ns = parse_positive_int64(comma_split[2]);
            int64_t interval_end_ns = parse_positive_int64(comma_split[3]);
            int64_t num_packets_or_bytes = parse_positive_int64(comma_split[4]);

            // Put into result
            if (file_choice == 0) {
                link_net_device_queue_pkt[std::make_pair(from_node_id, to_node_id)].push_back(std::make_tuple(interval_start_ns, interval_end_ns, num_packets_or_bytes));
            } else {
                link_net_device_queue_byte[std::make_pair(from_node_id, to_node_id)].push_back(std::make_tuple(interval_start_ns, interval_end_ns, num_packets_or_bytes));
            }

            // From-to has to be ordered and matching
            if (from_node_id != current_from_to.first || to_node_id != current_from_to.second) {
                previous_interval_end_ns = 0;
                prev_num_packets_or_bytes = -1;
                current_from_to_idx += 1;
                current_from_to = dir_a_b_list[current_from_to_idx];
                ASSERT_EQUAL(interval_start_ns, 0); // The first entry must have zero time
            }
            ASSERT_TRUE(from_node_id == current_from_to.first);
            ASSERT_TRUE(to_node_id == current_from_to.second);

            // Time intervals must match up
            ASSERT_EQUAL(interval_start_ns, previous_interval_end_ns);
            previous_interval_end_ns = interval_end_ns;

            // Value must be different
            ASSERT_NOT_EQUAL(prev_num_packets_or_bytes, num_packets_or_bytes);
            prev_num_packets_or_bytes = num_packets_or_bytes;

        }

    }

    // Only the pairs must be in there
    ASSERT_EQUAL(link_net_device_queue_byte.size(), dir_a_b_list.size());
    ASSERT_EQUAL(link_net_device_queue_byte.size(), link_net_device_queue_pkt.size());

    // Their size and intervals must be equal of the two files
    for (std::pair <int64_t, int64_t> a_b : dir_a_b_list) {
        ASSERT_EQUAL(link_net_device_queue_byte.at(a_b).size(), link_net_device_queue_pkt.at(a_b).size());
        for (uint32_t i = 0; i < link_net_device_queue_byte.at(a_b).size(); i++) {

            // Interval start
            ASSERT_EQUAL(
                    std::get<0>(link_net_device_queue_byte.at(a_b)[i]),
                    std::get<0>(link_net_device_queue_pkt.at(a_b)[i])
            );

            // Interval end
            ASSERT_EQUAL(
                    std::get<1>(link_net_device_queue_byte.at(a_b)[i]),
                    std::get<1>(link_net_device_queue_pkt.at(a_b)[i])
            );

            // Packets vs. bytes
            ASSERT_TRUE(
                    std::get<2>(link_net_device_queue_byte.at(a_b)[i]) <= std::get<2>(link_net_device_queue_pkt.at(a_b)[i]) * 1502
            );

        }
    }

}

/**
 * Validation of link net-device utilization logs.
 *
 * @param run_dir           In:  run directory
 * @param dir_a_b_list      In:  list of edges for which logging was done
 * @param duration_ns       In:  duration in nanoseconds
 * @param link_net_device_utilization_tracking_interval_ns  In: Tracking interval in nanoseconds
 * @param link_net_device_utilization           Out: mapping of pair to a vector of intervals <from, to, busy time (ns)>
 * @param link_overall_utilization_as_fraction  Out: mapping of pair to its overall utilization fraction during the entire run (in [0.0, 1.0])
 */
void TestCaseWithLogValidators::validate_link_net_device_utilization_logs(
        std::string run_dir,
        std::vector<std::pair<int64_t, int64_t>> dir_a_b_list,
        int64_t duration_ns,
        int64_t link_net_device_utilization_tracking_interval_ns,
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>>& link_net_device_utilization,
        std::map<std::pair<int64_t, int64_t>, double>& link_overall_utilization_as_fraction
) {

    // Logs are only valid if the run is finished
    validate_finished(run_dir);

    // Expected number of entries per pair
    size_t expected_num_entries_per_pair = (size_t) std::ceil((double) duration_ns / (double) link_net_device_utilization_tracking_interval_ns);

    // Sort it
    std::sort(dir_a_b_list.begin(), dir_a_b_list.end());

    // Initialize result
    for (std::pair<int64_t, int64_t> a_b : dir_a_b_list) {
        link_net_device_utilization[a_b] = std::vector<std::tuple<int64_t, int64_t, int64_t>>();
        link_overall_utilization_as_fraction[a_b] = 0.0;
    }

    // link_net_device_utilization.csv
    std::vector<int64_t> correct_busy_time_ns;
    std::vector<std::string> lines_csv = read_file_direct(run_dir + "/logs_ns3/link_net_device_utilization.csv");
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
            ASSERT_EQUAL(interval_start_ns, (int64_t) (j * link_net_device_utilization_tracking_interval_ns));
            ASSERT_EQUAL(interval_end_ns, (int64_t) std::min((int64_t) (j + 1) * link_net_device_utilization_tracking_interval_ns, duration_ns));

            // For keeping track later
            correct_busy_time_ns.push_back(interval_busy_time_ns);

            // Result
            link_net_device_utilization[std::make_pair(dir_a_b.first, dir_a_b.second)].push_back(std::make_tuple(interval_start_ns, interval_end_ns, interval_busy_time_ns));

            line_i++;
        }
    }

    // Correct amount of lines present
    ASSERT_EQUAL(line_i, expected_num_entries_per_pair * dir_a_b_list.size());
    for (std::pair<int64_t, int64_t> a_b : dir_a_b_list) {
        ASSERT_EQUAL(expected_num_entries_per_pair, link_net_device_utilization.at(a_b).size());
    }

    // link_net_device_utilization_compressed.csv/txt
    std::vector<std::string> lines_compressed_csv = read_file_direct(run_dir + "/logs_ns3/link_net_device_utilization_compressed.csv");
    std::vector<std::string> lines_compressed_txt = read_file_direct(run_dir + "/logs_ns3/link_net_device_utilization_compressed.txt");

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

    // link_net_device_utilization_summary.txt
    std::vector<std::string> lines_summary_txt = read_file_direct(run_dir + "/logs_ns3/link_net_device_utilization_summary.txt");
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
        for (size_t j = 0; j < link_net_device_utilization.at(dir_a_b_list.at(i)).size(); j++) {
            total_busy_time_ns += std::get<2>(link_net_device_utilization.at(dir_a_b_list.at(i)).at(j));
        }
        link_overall_utilization_as_fraction[dir_a_b_list.at(i)] = (double) total_busy_time_ns / (double) duration_ns;

        // Check that utilization matches up
        double util_100_txt = parse_positive_double(line_txt_spl[2].substr(0, line_txt_spl[2].size() - 1));
        ASSERT_EQUAL_APPROX(util_100_txt, link_overall_utilization_as_fraction.at(dir_a_b_list.at(i)) * 100.0, 0.01);
        ASSERT_EQUAL(line_txt_spl[2].substr(line_txt_spl[2].size() - 1, line_txt_spl[2].size()), "%");

    }

}

/**
 * Validation of link interface qdisc queue logs.
 *
 * @param run_dir           In:  run directory
 * @param dir_a_b_list      In:  list of edges for which logging was done
 * @param link_interface_tc_qdisc_queue_pkt    Out: mapping of pair to a vector of intervals <from, to, queue size in byte>
 * @param link_interface_tc_qdisc_queue_byte   Out: mapping of pair to a vector of intervals <from, to, queue size in byte>
 */
void TestCaseWithLogValidators::validate_link_interface_tc_qdisc_queue_logs(
        std::string run_dir,
        std::vector<std::pair<int64_t, int64_t>> dir_a_b_list,
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>>& link_interface_tc_qdisc_queue_pkt,
        std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>>& link_interface_tc_qdisc_queue_byte
) {

    // Logs are only valid if the run is finished
    validate_finished(run_dir);

    // Sort it
    std::sort(dir_a_b_list.begin(), dir_a_b_list.end());

    // Initialize result
    for (std::pair<int64_t, int64_t> a_b : dir_a_b_list) {
        link_interface_tc_qdisc_queue_pkt[a_b] = std::vector<std::tuple<int64_t, int64_t, int64_t>>();
        link_interface_tc_qdisc_queue_byte[a_b] = std::vector<std::tuple<int64_t, int64_t, int64_t>>();
    }

    // Both link net-device queue files need to be checked, and their checking is nearly identical
    for (size_t file_choice = 0;  file_choice < 2; file_choice++) {

        // Packets or bytes, all the checks are the same basically
        std::vector<std::string> lines_csv;
        if (file_choice == 0) {
            lines_csv = read_file_direct(run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_pkt.csv");
        } else {
            lines_csv = read_file_direct(run_dir + "/logs_ns3/link_interface_tc_qdisc_queue_byte.csv");
        }

        // State variables when checking
        int64_t current_from_to_idx = -1;
        std::pair<int64_t, int64_t> current_from_to = std::make_pair(-1, -1);
        int64_t previous_interval_end_ns = 0;
        int64_t prev_num_packets_or_bytes = -1;

        // Go over every line
        for (size_t line_i = 0; line_i < lines_csv.size(); line_i++) {
            std::string line = lines_csv[line_i];
            std::vector<std::string> comma_split = split_string(line, ",", 5);
            int64_t from_node_id = parse_positive_int64(comma_split[0]);
            int64_t to_node_id = parse_positive_int64(comma_split[1]);
            int64_t interval_start_ns = parse_positive_int64(comma_split[2]);
            int64_t interval_end_ns = parse_positive_int64(comma_split[3]);
            int64_t num_packets_or_bytes = parse_positive_int64(comma_split[4]);

            // Put into result
            if (file_choice == 0) {
                link_interface_tc_qdisc_queue_pkt[std::make_pair(from_node_id, to_node_id)].push_back(std::make_tuple(interval_start_ns, interval_end_ns, num_packets_or_bytes));
            } else {
                link_interface_tc_qdisc_queue_byte[std::make_pair(from_node_id, to_node_id)].push_back(std::make_tuple(interval_start_ns, interval_end_ns, num_packets_or_bytes));
            }

            // From-to has to be ordered and matching
            if (from_node_id != current_from_to.first || to_node_id != current_from_to.second) {
                previous_interval_end_ns = 0;
                prev_num_packets_or_bytes = -1;
                current_from_to_idx += 1;
                current_from_to = dir_a_b_list[current_from_to_idx];
                ASSERT_EQUAL(interval_start_ns, 0); // The first entry must have zero time
            }
            ASSERT_TRUE(from_node_id == current_from_to.first);
            ASSERT_TRUE(to_node_id == current_from_to.second);

            // Time intervals must match up
            ASSERT_EQUAL(interval_start_ns, previous_interval_end_ns);
            previous_interval_end_ns = interval_end_ns;

            // Value must be different
            ASSERT_NOT_EQUAL(prev_num_packets_or_bytes, num_packets_or_bytes);
            prev_num_packets_or_bytes = num_packets_or_bytes;

        }

    }

    // Only the pairs must be in there
    ASSERT_EQUAL(link_interface_tc_qdisc_queue_byte.size(), dir_a_b_list.size());
    ASSERT_EQUAL(link_interface_tc_qdisc_queue_byte.size(), link_interface_tc_qdisc_queue_pkt.size());

    // Their size and intervals must be equal of the two files
    for (std::pair <int64_t, int64_t> a_b : dir_a_b_list) {
        ASSERT_EQUAL(link_interface_tc_qdisc_queue_byte.at(a_b).size(), link_interface_tc_qdisc_queue_pkt.at(a_b).size());
        for (uint32_t i = 0; i < link_interface_tc_qdisc_queue_byte.at(a_b).size(); i++) {

            // Interval start
            ASSERT_EQUAL(
                    std::get<0>(link_interface_tc_qdisc_queue_byte.at(a_b)[i]),
                    std::get<0>(link_interface_tc_qdisc_queue_pkt.at(a_b)[i])
            );

            // Interval end
            ASSERT_EQUAL(
                    std::get<1>(link_interface_tc_qdisc_queue_byte.at(a_b)[i]),
                    std::get<1>(link_interface_tc_qdisc_queue_pkt.at(a_b)[i])
            );

            // Packets vs. bytes
            ASSERT_TRUE(
                    std::get<2>(link_interface_tc_qdisc_queue_byte.at(a_b)[i]) <= std::get<2>(link_interface_tc_qdisc_queue_pkt.at(a_b)[i]) * 1500
            );

        }
    }

}

/**
 * Validation of TCP flow logs.
 *
 * @param simulation_end_time_ns           In:  duration of simulation
 * @param run_dir                          In:  run directory
 * @param tcp_flow_schedule                In:  TCP flow schedule
 * @param tcp_flow_ids_with_logging        In:  TCP flow IDs for which there are detailed logs
 * @param end_time_ns_list                 Out: vector of TCP flows' ending time in ns
 * @param sent_byte_list                   Out: vector of TCP flows' amount of acknowledged sent data amount in byte
 * @param finished_list                    Out: vector of TCP flows' finished status
 */
void TestCaseWithLogValidators::validate_tcp_flow_logs(
        int64_t simulation_end_time_ns,
        std::string run_dir,
        std::vector<TcpFlowScheduleEntry> tcp_flow_schedule,
        std::set<int64_t> tcp_flow_ids_with_logging,
        std::vector<int64_t>& end_time_ns_list,
        std::vector<int64_t>& sent_byte_list,
        std::vector<std::string>& finished_list
) {

    // Logs are only valid if the run is finished
    validate_finished(run_dir);

    // Check finished.txt
    std::vector<std::string> finished_lines = read_file_direct(run_dir + "/logs_ns3/finished.txt");
    ASSERT_EQUAL(finished_lines.size(), 1);
    ASSERT_EQUAL(finished_lines[0], "Yes");

    // Check tcp_flows.csv
    std::vector<std::string> lines_csv = read_file_direct(run_dir + "/logs_ns3/tcp_flows.csv");
    ASSERT_EQUAL(lines_csv.size(), tcp_flow_schedule.size());
    int i = 0;
    for (std::string line : lines_csv) {
        std::vector<std::string> line_spl = split_string(line, ",");
        ASSERT_EQUAL(line_spl.size(), 10);
        ASSERT_EQUAL(parse_positive_int64(line_spl[0]), i);
        ASSERT_EQUAL(parse_positive_int64(line_spl[1]), tcp_flow_schedule[i].GetFromNodeId());
        ASSERT_EQUAL(parse_positive_int64(line_spl[2]), tcp_flow_schedule[i].GetToNodeId());
        ASSERT_EQUAL(parse_positive_int64(line_spl[3]), tcp_flow_schedule[i].GetSizeByte());
        ASSERT_EQUAL(parse_positive_int64(line_spl[4]), tcp_flow_schedule[i].GetStartTimeNs());
        int64_t end_time_ns = parse_positive_int64(line_spl[5]);
        ASSERT_TRUE(end_time_ns >= 0 && end_time_ns <= simulation_end_time_ns);
        ASSERT_EQUAL(parse_positive_int64(line_spl[6]), end_time_ns - tcp_flow_schedule[i].GetStartTimeNs());
        int64_t sent_byte = parse_positive_int64(line_spl[7]);
        ASSERT_TRUE(sent_byte >= 0 && sent_byte <= simulation_end_time_ns);
        ASSERT_TRUE(line_spl[8] == "YES" || line_spl[8] == "NO_ONGOING" || line_spl[8] == "NO_CONN_FAIL" || line_spl[8] == "NO_ERR_CLOSE" || line_spl[8] == "NO_BAD_CLOSE");
        ASSERT_EQUAL(line_spl[9], tcp_flow_schedule[i].GetMetadata());
        end_time_ns_list.push_back(end_time_ns);
        sent_byte_list.push_back(sent_byte);
        finished_list.push_back(line_spl[8]);
        i++;
    }

    // Check tcp_flows.txt
    std::vector<std::string> lines_txt = read_file_direct(run_dir + "/logs_ns3/tcp_flows.txt");
    ASSERT_EQUAL(lines_txt.size(), tcp_flow_schedule.size() + 1);
    i = 0;
    for (std::string line : lines_txt) {
        if (i == 0) {
            ASSERT_EQUAL(
                    line,
                    "TCP flow ID     Source    Target    Size            Start time (ns)   End time (ns)     Duration        Sent            Progress     Avg. rate       Finished?     Metadata"
            );
        } else {
            int j = i - 1;
            std::vector<std::string> line_spl;
            std::istringstream iss(line);
            for (std::string s; iss >> s;) {
                line_spl.push_back(s);
            }
            ASSERT_TRUE(line_spl.size() == 15 || line_spl.size() == 16);
            ASSERT_EQUAL(parse_positive_int64(line_spl[0]), j);
            ASSERT_EQUAL(parse_positive_int64(line_spl[1]), tcp_flow_schedule[j].GetFromNodeId());
            ASSERT_EQUAL(parse_positive_int64(line_spl[2]), tcp_flow_schedule[j].GetToNodeId());
            ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[3]), byte_to_megabit(tcp_flow_schedule[j].GetSizeByte()), 0.01);
            ASSERT_EQUAL(line_spl[4], "Mbit");
            ASSERT_EQUAL(parse_positive_int64(line_spl[5]), tcp_flow_schedule[j].GetStartTimeNs());
            ASSERT_EQUAL(parse_positive_int64(line_spl[6]), end_time_ns_list[j]);
            ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[7]), nanosec_to_millisec(end_time_ns_list[j] - tcp_flow_schedule[j].GetStartTimeNs()), 0.01);
            ASSERT_EQUAL(line_spl[8], "ms");
            ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[9]), byte_to_megabit(sent_byte_list[j]), 0.01);
            ASSERT_EQUAL(line_spl[10], "Mbit");
            ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[11].substr(0, line_spl[11].size() - 1)), sent_byte_list[j] * 100.0 / tcp_flow_schedule[j].GetSizeByte(), 0.1);
            ASSERT_EQUAL(line_spl[11].substr(line_spl[11].size() - 1, line_spl[11].size()), "%");
            ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[12]), byte_to_megabit(sent_byte_list[j]) / nanosec_to_sec(end_time_ns_list[j] - tcp_flow_schedule[j].GetStartTimeNs()), 0.1);
            ASSERT_EQUAL(line_spl[13], "Mbit/s");
            ASSERT_TRUE(line_spl[14] == "YES" || line_spl[14] == "NO_ONGOING" || line_spl[14] == "NO_CONN_FAIL" || line_spl[14] == "NO_ERR_CLOSE" || line_spl[14] == "NO_BAD_CLOSE");
            ASSERT_EQUAL(line_spl[14], finished_list[j]);
            if (line_spl.size() == 16) {
                ASSERT_EQUAL(line_spl[15], tcp_flow_schedule[j].GetMetadata());
            }
        }
        i++;
    }

    // Now go over all the detailed logs
    for (TcpFlowScheduleEntry& entry : tcp_flow_schedule) {
        if (tcp_flow_ids_with_logging.find(entry.GetTcpFlowId()) != tcp_flow_ids_with_logging.end()) {
            int64_t prev_timestamp_ns = 0;

            // TCP connection progress
            // tcp_flow_[id]_progress.csv
            std::vector<std::string> lines_progress_csv = read_file_direct(run_dir + "/logs_ns3/tcp_flow_" + std::to_string(entry.GetTcpFlowId()) + "_progress.csv");
            prev_timestamp_ns = 0;
            int64_t prev_progress_byte = 0;
            for (size_t i = 0; i < lines_progress_csv.size(); i++) {
                std::vector<std::string> line_spl = split_string(lines_progress_csv.at(i), ",");
                ASSERT_EQUAL(line_spl.size(), 3);

                // Correct TCP flow ID
                ASSERT_EQUAL(parse_positive_int64(line_spl[0]), entry.GetTcpFlowId());

                // Weakly ascending timestamp
                int64_t timestamp_ns = parse_positive_int64(line_spl[1]);
                ASSERT_TRUE(timestamp_ns >= prev_timestamp_ns);
                prev_timestamp_ns = timestamp_ns;

                // Progress has to be positive and ascending
                int64_t progress_byte = parse_positive_int64(line_spl[2]);
                ASSERT_TRUE(progress_byte >= prev_progress_byte);
                prev_progress_byte = progress_byte;
            }
            ASSERT_EQUAL(prev_progress_byte, sent_byte_list.at(entry.GetTcpFlowId())); // Progress must be equal to the sent amount reported in the end

            // TCP RTT
            // tcp_flow_[id]_rtt.csv
            std::vector<std::string> lines_rtt_csv = read_file_direct(run_dir + "/logs_ns3/tcp_flow_" + std::to_string(entry.GetTcpFlowId()) + "_rtt.csv");
            prev_timestamp_ns = 0;
            int64_t prev_rtt_ns = -1;
            for (size_t i = 0; i < lines_rtt_csv.size(); i++) {
                std::vector<std::string> line_spl = split_string(lines_rtt_csv.at(i), ",");
                ASSERT_EQUAL(line_spl.size(), 3);

                // Correct TCP flow ID
                ASSERT_EQUAL(parse_positive_int64(line_spl[0]), entry.GetTcpFlowId());

                // Weakly ascending timestamp
                int64_t timestamp_ns = parse_positive_int64(line_spl[1]);
                ASSERT_TRUE(timestamp_ns >= prev_timestamp_ns);
                prev_timestamp_ns = timestamp_ns;

                // RTT has to be positive and different
                int64_t rtt_ns = parse_positive_int64(line_spl[2]);
                ASSERT_TRUE(rtt_ns >= 0);
                ASSERT_TRUE(rtt_ns != prev_rtt_ns || (i == lines_rtt_csv.size() - 1 && rtt_ns == prev_rtt_ns));
                prev_rtt_ns = rtt_ns;
            }

            // TCP RTO
            // tcp_flow_[id]_rto.csv
            std::vector<std::string> lines_rto_csv = read_file_direct(run_dir + "/logs_ns3/tcp_flow_" + std::to_string(entry.GetTcpFlowId()) + "_rto.csv");
            prev_timestamp_ns = 0;
            int64_t prev_rto_ns = -1;
            for (size_t i = 0; i < lines_rto_csv.size(); i++) {
                std::vector<std::string> line_spl = split_string(lines_rto_csv.at(i), ",");
                ASSERT_EQUAL(line_spl.size(), 3);

                // Correct TCP flow ID
                ASSERT_EQUAL(parse_positive_int64(line_spl[0]), entry.GetTcpFlowId());

                // Weakly ascending timestamp
                int64_t timestamp_ns = parse_positive_int64(line_spl[1]);
                ASSERT_TRUE(timestamp_ns >= prev_timestamp_ns);
                prev_timestamp_ns = timestamp_ns;

                // RTO has to be positive and different
                int64_t rto_ns = parse_positive_int64(line_spl[2]);
                ASSERT_TRUE(rto_ns >= 0);
                ASSERT_TRUE(rto_ns != prev_rto_ns || (i == lines_rto_csv.size() - 1 && rto_ns == prev_rto_ns));
                prev_rto_ns = rto_ns;
            }

            // TCP congestion window
            // tcp_flow_[id]_cwnd.csv
            std::vector<std::string> lines_cwnd_csv = read_file_direct(run_dir + "/logs_ns3/tcp_flow_" + std::to_string(entry.GetTcpFlowId()) + "_cwnd.csv");
            prev_timestamp_ns = 0;
            int64_t prev_cwnd_byte = -1;
            for (size_t i = 0; i < lines_cwnd_csv.size(); i++) {
                std::vector<std::string> line_spl = split_string(lines_cwnd_csv.at(i), ",");
                ASSERT_EQUAL(line_spl.size(), 3);

                // Correct TCP flow ID
                ASSERT_EQUAL(parse_positive_int64(line_spl[0]), entry.GetTcpFlowId());

                // Weakly ascending timestamp
                int64_t timestamp_ns = parse_positive_int64(line_spl[1]);
                ASSERT_TRUE(timestamp_ns >= prev_timestamp_ns);
                prev_timestamp_ns = timestamp_ns;

                // Congestion window has to be positive and different
                int64_t cwnd_byte = parse_positive_int64(line_spl[2]);
                ASSERT_TRUE(cwnd_byte != prev_cwnd_byte || (i == lines_cwnd_csv.size() - 1 && cwnd_byte == prev_cwnd_byte));
                prev_cwnd_byte = cwnd_byte;
            }

            // TCP congestion window inflated
            // tcp_flow_[id]_cwnd_inflated.csv
            std::vector<std::string> lines_cwnd_inflated_csv = read_file_direct(run_dir + "/logs_ns3/tcp_flow_" + std::to_string(entry.GetTcpFlowId()) + "_cwnd_inflated.csv");
            prev_timestamp_ns = 0;
            int64_t prev_cwnd_inflated_byte = -1;
            for (size_t i = 0; i < lines_cwnd_inflated_csv.size(); i++) {
                std::vector<std::string> line_spl = split_string(lines_cwnd_inflated_csv.at(i), ",");
                ASSERT_EQUAL(line_spl.size(), 3);

                // Correct TCP flow ID
                ASSERT_EQUAL(parse_positive_int64(line_spl[0]), entry.GetTcpFlowId());

                // Weakly ascending timestamp
                int64_t timestamp_ns = parse_positive_int64(line_spl[1]);
                ASSERT_TRUE(timestamp_ns >= prev_timestamp_ns);
                prev_timestamp_ns = timestamp_ns;

                // Congestion window inflated has to be positive and different
                int64_t cwnd_inflated_byte = parse_positive_int64(line_spl[2]);
                ASSERT_TRUE(cwnd_inflated_byte != prev_cwnd_inflated_byte || (i == lines_cwnd_inflated_csv.size() - 1 && cwnd_inflated_byte == prev_cwnd_inflated_byte));
                prev_cwnd_inflated_byte = cwnd_inflated_byte;
            }

            // TCP slow-start threshold
            // tcp_flow_[id]_ssthresh.csv
            std::vector<std::string> lines_ssthresh_csv = read_file_direct(run_dir + "/logs_ns3/tcp_flow_" + std::to_string(entry.GetTcpFlowId()) + "_ssthresh.csv");
            prev_timestamp_ns = 0;
            int64_t prev_ssthresh_byte = -1;
            for (size_t i = 0; i < lines_ssthresh_csv.size(); i++) {
                std::vector<std::string> line_spl = split_string(lines_ssthresh_csv.at(i), ",");
                ASSERT_EQUAL(line_spl.size(), 3);

                // Correct TCP flow ID
                ASSERT_EQUAL(parse_positive_int64(line_spl[0]), entry.GetTcpFlowId());

                // Weakly ascending timestamp
                int64_t timestamp_ns = parse_positive_int64(line_spl[1]);
                ASSERT_TRUE(timestamp_ns >= prev_timestamp_ns);
                prev_timestamp_ns = timestamp_ns;

                // Slow-start threshold has to be positive and different
                int64_t ssthresh_byte = parse_positive_int64(line_spl[2]);
                ASSERT_TRUE(ssthresh_byte != prev_ssthresh_byte || (i == lines_ssthresh_csv.size() - 1 && ssthresh_byte == prev_ssthresh_byte));
                prev_ssthresh_byte = ssthresh_byte;
            }

            // TCP in-flight
            // tcp_flow_[id]_inflight.csv
            std::vector<std::string> lines_inflight_csv = read_file_direct(run_dir + "/logs_ns3/tcp_flow_" + std::to_string(entry.GetTcpFlowId()) + "_inflight.csv");
            prev_timestamp_ns = 0;
            int64_t prev_inflight_byte = -1;
            for (size_t i = 0; i < lines_inflight_csv.size(); i++) {
                std::vector<std::string> line_spl = split_string(lines_inflight_csv.at(i), ",");
                ASSERT_EQUAL(line_spl.size(), 3);

                // Correct TCP flow ID
                ASSERT_EQUAL(parse_positive_int64(line_spl[0]), entry.GetTcpFlowId());

                // Weakly ascending timestamp
                int64_t timestamp_ns = parse_positive_int64(line_spl[1]);
                ASSERT_TRUE(timestamp_ns >= prev_timestamp_ns);
                prev_timestamp_ns = timestamp_ns;

                // In-flight has to be positive and different
                int64_t inflight_byte = parse_positive_int64(line_spl[2]);
                ASSERT_TRUE(inflight_byte != prev_inflight_byte || (i == lines_inflight_csv.size() - 1 && inflight_byte == prev_inflight_byte));
                prev_inflight_byte = inflight_byte;
            }

            // TCP state
            // tcp_flow_[id]_state.csv
            std::vector<std::string> lines_state_csv = read_file_direct(run_dir + "/logs_ns3/tcp_flow_" + std::to_string(entry.GetTcpFlowId()) + "_state.csv");
            prev_timestamp_ns = 0;
            std::string prev_state = "";
            for (size_t i = 0; i < lines_state_csv.size(); i++) {
                std::vector<std::string> line_spl = split_string(lines_state_csv.at(i), ",");
                ASSERT_EQUAL(line_spl.size(), 3);

                // Correct TCP flow ID
                ASSERT_EQUAL(parse_positive_int64(line_spl[0]), entry.GetTcpFlowId());

                // Weakly ascending timestamp
                int64_t timestamp_ns = parse_positive_int64(line_spl[1]);
                ASSERT_TRUE(timestamp_ns >= prev_timestamp_ns);
                prev_timestamp_ns = timestamp_ns;

                // State has to be one of the permitted states and different from the last
                std::string state = line_spl[2];
                ASSERT_TRUE(
                        state == "CLOSED" ||
                        state == "LISTEN" ||
                        state == "SYN_SENT" ||
                        state == "SYN_RCVD" ||
                        state == "ESTABLISHED" ||
                        state == "CLOSE_WAIT" ||
                        state == "LAST_ACK" ||
                        state == "FIN_WAIT_1" ||
                        state == "FIN_WAIT_2" ||
                        state == "CLOSING" ||
                        state == "TIME_WAIT" ||
                        state == "LAST_STATE"
                );
                ASSERT_TRUE(state != prev_state || (i == lines_state_csv.size() - 1 && state == prev_state));
                prev_state = state;
            }

            // TCP congestion state
            // tcp_flow_[id]_cong_state.csv
            std::vector<std::string> lines_cong_state_csv = read_file_direct(run_dir + "/logs_ns3/tcp_flow_" + std::to_string(entry.GetTcpFlowId()) + "_cong_state.csv");
            prev_timestamp_ns = 0;
            std::string prev_cong_state = "";
            for (size_t i = 0; i < lines_cong_state_csv.size(); i++) {
                std::vector<std::string> line_spl = split_string(lines_cong_state_csv.at(i), ",");
                ASSERT_EQUAL(line_spl.size(), 3);

                // Correct TCP flow ID
                ASSERT_EQUAL(parse_positive_int64(line_spl[0]), entry.GetTcpFlowId());

                // Weakly ascending timestamp
                int64_t timestamp_ns = parse_positive_int64(line_spl[1]);
                ASSERT_TRUE(timestamp_ns >= prev_timestamp_ns);
                prev_timestamp_ns = timestamp_ns;

                // Congestion state has to be one of the permitted states and different from the last
                std::string cong_state = line_spl[2];
                ASSERT_TRUE(
                        cong_state == "CA_OPEN" ||
                        cong_state == "CA_DISORDER" ||
                        cong_state == "CA_CWR" ||
                        cong_state == "CA_RECOVERY" ||
                        cong_state == "CA_LOSS" ||
                        cong_state == "CA_LAST_STATE"
                );
                ASSERT_TRUE(cong_state != prev_cong_state || (i == lines_cong_state_csv.size() - 1 && cong_state == prev_cong_state));
                prev_cong_state = cong_state;
            }

        }
    }

}

/**
 * Validation of UDP burst logs.
 *
 * @param simulation_end_time_ns             In:  duration of simulation
 * @param run_dir                            In:  run directory
 * @param udp_burst_schedule                 In:  UDP burst schedule
 * @param udp_burst_ids_with_logging         In:  UDP burst IDs for which there are detailed logs
 * @param list_outgoing_rate_megabit_per_s   Out: vector of UDP bursts' outgoing rates in megabit/s
 * @param list_incoming_rate_megabit_per_s   Out: vector of UDP bursts' incoming rates in megabit/s
 */
void TestCaseWithLogValidators::validate_udp_burst_logs(
        int64_t simulation_end_time_ns,
        std::string run_dir,
        std::vector<UdpBurstInfo> udp_burst_schedule,
        std::set<int64_t> udp_burst_ids_with_logging,
        std::vector<double>& list_outgoing_rate_megabit_per_s,
        std::vector<double>& list_incoming_rate_megabit_per_s
) {

    // Logs are only valid if the run is finished
    validate_finished(run_dir);

    // For checking the sent / received amount
    std::vector<int64_t> udp_burst_sent_amount;
    std::vector<int64_t> udp_burst_received_amount;

    // Check udp_bursts_outgoing.csv
    std::vector<std::string> lines_outgoing_csv = read_file_direct(run_dir + "/logs_ns3/udp_bursts_outgoing.csv");
    ASSERT_EQUAL(lines_outgoing_csv.size(), udp_burst_schedule.size());
    int i = 0;
    for (std::string line : lines_outgoing_csv) {
        std::vector<std::string> line_spl = split_string(line, ",");
        ASSERT_EQUAL(line_spl.size(), 12);
        ASSERT_EQUAL(parse_positive_int64(line_spl[0]), i);
        ASSERT_EQUAL(parse_positive_int64(line_spl[1]), udp_burst_schedule[i].GetFromNodeId());
        ASSERT_EQUAL(parse_positive_int64(line_spl[2]), udp_burst_schedule[i].GetToNodeId());
        ASSERT_EQUAL(parse_positive_double(line_spl[3]), udp_burst_schedule[i].GetTargetRateMegabitPerSec());
        ASSERT_EQUAL(parse_positive_int64(line_spl[4]), udp_burst_schedule[i].GetStartTimeNs());
        ASSERT_EQUAL(parse_positive_int64(line_spl[5]), udp_burst_schedule[i].GetDurationNs());
        double outgoing_rate_incl_headers_megabit_per_s = parse_positive_double(line_spl[6]);
        double outgoing_rate_payload_megabit_per_s = parse_positive_double(line_spl[7]);
        int64_t sent_packets = parse_positive_int64(line_spl[8]);
        udp_burst_sent_amount.push_back(sent_packets);
        int64_t outgoing_sent_incl_headers_byte = parse_positive_int64(line_spl[9]);
        int64_t outgoing_sent_payload_only_byte = parse_positive_int64(line_spl[10]);
        ASSERT_EQUAL(outgoing_sent_payload_only_byte, sent_packets * 1472);
        ASSERT_EQUAL(outgoing_sent_incl_headers_byte, sent_packets * 1500);
        ASSERT_TRUE(outgoing_rate_payload_megabit_per_s >= outgoing_rate_incl_headers_megabit_per_s * 0.9);
        ASSERT_TRUE(outgoing_rate_incl_headers_megabit_per_s >= outgoing_rate_payload_megabit_per_s);
        ASSERT_EQUAL(line_spl[11], udp_burst_schedule[i].GetMetadata());
        list_outgoing_rate_megabit_per_s.push_back(outgoing_rate_incl_headers_megabit_per_s);
        i++;
    }

    // Check udp_bursts_incoming.csv
    std::vector<std::string> lines_incoming_csv = read_file_direct(run_dir + "/logs_ns3/udp_bursts_incoming.csv");
    ASSERT_EQUAL(lines_incoming_csv.size(), udp_burst_schedule.size());
    i = 0;
    for (std::string line : lines_incoming_csv) {
        std::vector<std::string> line_spl = split_string(line, ",");
        ASSERT_EQUAL(line_spl.size(), 12);
        ASSERT_EQUAL(parse_positive_int64(line_spl[0]), i);
        ASSERT_EQUAL(parse_positive_int64(line_spl[1]), udp_burst_schedule[i].GetFromNodeId());
        ASSERT_EQUAL(parse_positive_int64(line_spl[2]), udp_burst_schedule[i].GetToNodeId());
        ASSERT_EQUAL(parse_positive_double(line_spl[3]), udp_burst_schedule[i].GetTargetRateMegabitPerSec());
        ASSERT_EQUAL(parse_positive_int64(line_spl[4]), udp_burst_schedule[i].GetStartTimeNs());
        ASSERT_EQUAL(parse_positive_int64(line_spl[5]), udp_burst_schedule[i].GetDurationNs());
        double incoming_rate_incl_headers_megabit_per_s = parse_positive_double(line_spl[6]);
        double incoming_rate_payload_megabit_per_s = parse_positive_double(line_spl[7]);
        int64_t received_packets = parse_positive_int64(line_spl[8]);
        udp_burst_received_amount.push_back(received_packets);
        int64_t incoming_received_incl_headers_byte = parse_positive_int64(line_spl[9]);
        int64_t incoming_received_payload_only_byte = parse_positive_int64(line_spl[10]);
        ASSERT_EQUAL(incoming_received_payload_only_byte, received_packets * 1472);
        ASSERT_EQUAL(incoming_received_incl_headers_byte, received_packets * 1500);
        ASSERT_TRUE(incoming_rate_payload_megabit_per_s >= incoming_rate_incl_headers_megabit_per_s * 0.9);
        ASSERT_TRUE(incoming_rate_incl_headers_megabit_per_s >= incoming_rate_payload_megabit_per_s);
        ASSERT_EQUAL(line_spl[11], udp_burst_schedule[i].GetMetadata());
        list_incoming_rate_megabit_per_s.push_back(incoming_rate_incl_headers_megabit_per_s);
        i++;
    }

    // Check udp_bursts_outgoing.txt
    std::vector<std::string> lines_outgoing_txt = read_file_direct(run_dir + "/logs_ns3/udp_bursts_outgoing.txt");
    ASSERT_EQUAL(lines_outgoing_txt.size(), udp_burst_schedule.size() + 1);
    i = 0;
    for (std::string line : lines_outgoing_txt) {
        if (i == 0) {
            ASSERT_EQUAL(
                    line,
                    "UDP burst ID    From      To        Target rate         Start time      Duration        Outgoing rate (w/ headers)  Outgoing rate (payload)     Packets sent    Data sent (w/headers)       Data sent (payload)         Metadata"
            );
        } else {
            int j = i - 1;
            std::vector<std::string> line_spl;
            std::istringstream iss(line);
            for (std::string s; iss >> s;) {
                line_spl.push_back(s);
            }
            ASSERT_TRUE(line_spl.size() == 18 || line_spl.size() == 19);
            ASSERT_EQUAL(parse_positive_int64(line_spl[0]), j);
            ASSERT_EQUAL(parse_positive_int64(line_spl[1]), udp_burst_schedule[j].GetFromNodeId());
            ASSERT_EQUAL(parse_positive_int64(line_spl[2]), udp_burst_schedule[j].GetToNodeId());
            ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[3]), udp_burst_schedule[j].GetTargetRateMegabitPerSec(), 0.01);
            ASSERT_EQUAL(line_spl[4], "Mbit/s");
            ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[5]), nanosec_to_millisec(udp_burst_schedule[j].GetStartTimeNs()), 0.01);
            ASSERT_EQUAL(line_spl[6], "ms");
            ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[7]), nanosec_to_millisec(udp_burst_schedule[j].GetDurationNs()), 0.01);
            ASSERT_EQUAL(line_spl[8], "ms");
            double outgoing_rate_incl_headers_megabit_per_s = parse_positive_double(line_spl[9]);
            ASSERT_EQUAL(line_spl[10], "Mbit/s");
            double outgoing_rate_payload_megabit_per_s = parse_positive_double(line_spl[11]);
            ASSERT_EQUAL(line_spl[12], "Mbit/s");
            ASSERT_EQUAL_APPROX(list_outgoing_rate_megabit_per_s[j], outgoing_rate_incl_headers_megabit_per_s, 0.01);
            int64_t packets_sent = parse_positive_int64(line_spl[13]);
            ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[14]), byte_to_megabit(packets_sent * 1500), 0.01);
            ASSERT_EQUAL(line_spl[15], "Mbit");
            ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[16]), byte_to_megabit(packets_sent * 1472), 0.01);
            ASSERT_EQUAL(line_spl[17], "Mbit");
            ASSERT_TRUE(outgoing_rate_payload_megabit_per_s >= outgoing_rate_incl_headers_megabit_per_s * 0.9);
            ASSERT_TRUE(outgoing_rate_incl_headers_megabit_per_s >= outgoing_rate_payload_megabit_per_s);
            if (line_spl.size() == 19) {
                ASSERT_EQUAL(line_spl[18], udp_burst_schedule[j].GetMetadata());
            }
        }
        i++;
    }

    // Check udp_bursts_incoming.txt
    std::vector<std::string> lines_incoming_txt = read_file_direct(run_dir + "/logs_ns3/udp_bursts_incoming.txt");
    ASSERT_EQUAL(lines_incoming_txt.size(), udp_burst_schedule.size() + 1);
    i = 0;
    for (std::string line : lines_incoming_txt) {
        if (i == 0) {
            ASSERT_EQUAL(
                    line,
                    "UDP burst ID    From      To        Target rate         Start time      Duration        Incoming rate (w/ headers)  Incoming rate (payload)     Packets received   Data received (w/headers)   Data received (payload)     Metadata"
            );
        } else {
            int j = i - 1;
            std::vector<std::string> line_spl;
            std::istringstream iss(line);
            for (std::string s; iss >> s;) {
                line_spl.push_back(s);
            }
            ASSERT_TRUE(line_spl.size() == 18 || line_spl.size() == 19);
            ASSERT_EQUAL(parse_positive_int64(line_spl[0]), j);
            ASSERT_EQUAL(parse_positive_int64(line_spl[1]), udp_burst_schedule[j].GetFromNodeId());
            ASSERT_EQUAL(parse_positive_int64(line_spl[2]), udp_burst_schedule[j].GetToNodeId());
            ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[3]), udp_burst_schedule[j].GetTargetRateMegabitPerSec(), 0.01);
            ASSERT_EQUAL(line_spl[4], "Mbit/s");
            ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[5]), nanosec_to_millisec(udp_burst_schedule[j].GetStartTimeNs()), 0.01);
            ASSERT_EQUAL(line_spl[6], "ms");
            ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[7]), nanosec_to_millisec(udp_burst_schedule[j].GetDurationNs()), 0.01);
            ASSERT_EQUAL(line_spl[8], "ms");
            double incoming_rate_incl_headers_megabit_per_s = parse_positive_double(line_spl[9]);
            ASSERT_EQUAL(line_spl[10], "Mbit/s");
            double incoming_rate_payload_megabit_per_s = parse_positive_double(line_spl[11]);
            ASSERT_EQUAL(line_spl[12], "Mbit/s");
            ASSERT_EQUAL_APPROX(list_incoming_rate_megabit_per_s[j], incoming_rate_incl_headers_megabit_per_s, 0.01);
            int64_t packets_received = parse_positive_int64(line_spl[13]);
            ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[14]), byte_to_megabit(packets_received * 1500), 0.01);
            ASSERT_EQUAL(line_spl[15], "Mbit");
            ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[16]), byte_to_megabit(packets_received * 1472), 0.01);
            ASSERT_EQUAL(line_spl[17], "Mbit");
            ASSERT_TRUE(incoming_rate_payload_megabit_per_s >= incoming_rate_incl_headers_megabit_per_s * 0.9);
            ASSERT_TRUE(incoming_rate_incl_headers_megabit_per_s >= incoming_rate_payload_megabit_per_s);
            if (line_spl.size() == 19) {
                ASSERT_EQUAL(line_spl[18], udp_burst_schedule[j].GetMetadata());
            }
        }
        i++;
    }

    // Check the precise outgoing / incoming logs for each burst
    for (UdpBurstInfo entry : udp_burst_schedule) {
        if (udp_burst_ids_with_logging.find(entry.GetUdpBurstId()) != udp_burst_ids_with_logging.end()) {

            // Outgoing
            std::vector<std::string> lines_precise_outgoing_csv = read_file_direct(run_dir + "/logs_ns3/udp_burst_" + std::to_string(entry.GetUdpBurstId()) + "_outgoing.csv");
            ASSERT_EQUAL(lines_precise_outgoing_csv.size(), (size_t) udp_burst_sent_amount.at(entry.GetUdpBurstId()));
            int j = 0;
            for (std::string line : lines_precise_outgoing_csv) {
                std::vector <std::string> line_spl = split_string(line, ",");
                ASSERT_EQUAL(line_spl.size(), 3);
                ASSERT_EQUAL(parse_positive_int64(line_spl[0]), entry.GetUdpBurstId());
                ASSERT_EQUAL(parse_positive_int64(line_spl[1]), j);
                ASSERT_EQUAL(parse_positive_int64(line_spl[2]), entry.GetStartTimeNs() + j * std::ceil(1500.0 / (entry.GetTargetRateMegabitPerSec() / 8000.0)));
                j += 1;
            }

            // Incoming
            std::vector<std::string> lines_precise_incoming_csv = read_file_direct(run_dir + "/logs_ns3/udp_burst_" + std::to_string(entry.GetUdpBurstId()) + "_incoming.csv");
            ASSERT_EQUAL(lines_precise_incoming_csv.size(), (size_t) udp_burst_received_amount.at(entry.GetUdpBurstId()));
            std::set<int64_t> already_seen_seqs;
            int prev_timestamp_ns = 0;
            for (std::string line : lines_precise_incoming_csv) {
                std::vector <std::string> line_spl = split_string(line, ",");
                ASSERT_EQUAL(line_spl.size(), 3);

                // Must be correct UDP burst ID
                ASSERT_EQUAL(parse_positive_int64(line_spl[0]), entry.GetUdpBurstId());

                // We can only check that the sequence number has not arrived before
                int64_t seq = parse_positive_int64(line_spl[1]);
                ASSERT_TRUE(already_seen_seqs.find(seq) == already_seen_seqs.end());
                already_seen_seqs.insert(seq);

                // And that the timestamps are at least weakly ascending
                int64_t timestamp = parse_positive_int64(line_spl[2]);
                ASSERT_TRUE(timestamp >= prev_timestamp_ns);
                prev_timestamp_ns = timestamp;
            }

        }

    }

}

/**
 * Validation of UDP ping logs.
 *
 * @param simulation_end_time_ns    In:  duration of simulation
 * @param run_dir                   In:  run directory
 * @param udp_ping_schedule         In:  vector of UDP ping schedule
 * @param list_latency_there_ns     Out: vector of pings' latency there (ns)
 * @param list_latency_back_ns      Out: vector of pings' latency back (ns)
 * @param list_rtt_ns               Out: vector of pings' RTT (ns)
 */
void TestCaseWithLogValidators::validate_udp_ping_logs(
        int64_t simulation_end_time_ns,
        std::string run_dir,
        std::vector<UdpPingInfo> udp_ping_schedule,
        std::vector<std::vector<int64_t>>& list_latency_there_ns,
        std::vector<std::vector<int64_t>>& list_latency_back_ns,
        std::vector<std::vector<int64_t>>& list_rtt_ns
) {

    // Logs are only valid if the run is finished
    validate_finished(run_dir);

    // Check udp_pings.csv
    std::vector<std::string> lines_csv = read_file_direct(run_dir + "/logs_ns3/udp_pings.csv");
    size_t i = 0;
    int64_t counter = 0;
    int64_t current_udp_ping_id = -1;
    std::vector<std::vector<int64_t>> list_latency_there_ns_valid;
    std::vector<std::vector<int64_t>> list_latency_back_ns_valid;
    std::vector<std::vector<int64_t>> list_rtt_ns_valid;
    for (std::string line : lines_csv) {
        std::vector<std::string> spl = split_string(line, ",", 9);

        // From-to and sequence number must match
        int64_t udp_ping_id = parse_positive_int64(spl[0]);
        int64_t seq_no = parse_positive_int64(spl[1]);
        if (udp_ping_id != current_udp_ping_id) {
            counter = 0;
            current_udp_ping_id = udp_ping_id;
            ASSERT_EQUAL(current_udp_ping_id, udp_ping_schedule.at(i).GetUdpPingId());
            list_latency_there_ns.push_back(std::vector<int64_t>());
            list_latency_back_ns.push_back(std::vector<int64_t>());
            list_rtt_ns.push_back(std::vector<int64_t>());
            list_latency_there_ns_valid.push_back(std::vector<int64_t>());
            list_latency_back_ns_valid.push_back(std::vector<int64_t>());
            list_rtt_ns_valid.push_back(std::vector<int64_t>());
            i += 1;
        }
        ASSERT_EQUAL(seq_no, counter);
        counter++;

        // Timestamps
        int64_t sent_ns = parse_positive_int64(spl[2]);
        int64_t reply_ns = parse_int64(spl[3]);
        int64_t got_reply_ns = parse_int64(spl[4]);
        int64_t way_there_ns = parse_int64(spl[5]);
        int64_t way_back_ns = parse_int64(spl[6]);
        int64_t rtt_ns = parse_int64(spl[7]);
        bool arrived = true;
        if (trim(spl[8]) == "YES") {
            arrived = true;
        } else if (trim(spl[8]) == "LOST") {
            arrived = false;
        } else {
            ASSERT_TRUE(false);
        }
        if (arrived) {
            ASSERT_TRUE(reply_ns >= sent_ns);
            ASSERT_TRUE(got_reply_ns >= reply_ns);
            ASSERT_EQUAL(way_there_ns, reply_ns - sent_ns);
            ASSERT_EQUAL(way_back_ns, got_reply_ns - reply_ns);
            ASSERT_EQUAL(rtt_ns, got_reply_ns - sent_ns);
            list_latency_there_ns_valid.at(i - 1).push_back(way_there_ns);
            list_latency_back_ns_valid.at(i - 1).push_back(way_back_ns);
            list_rtt_ns_valid.at(i - 1).push_back(rtt_ns);
        } else {
            ASSERT_EQUAL(reply_ns, -1);
            ASSERT_EQUAL(got_reply_ns, -1);
            ASSERT_EQUAL(way_there_ns, -1);
            ASSERT_EQUAL(way_back_ns, -1);
            ASSERT_EQUAL(rtt_ns, -1);
        }
        list_latency_there_ns.at(i - 1).push_back(way_there_ns);
        list_latency_back_ns.at(i - 1).push_back(way_back_ns);
        list_rtt_ns.at(i - 1).push_back(rtt_ns);
    }
    ASSERT_EQUAL(i, udp_ping_schedule.size());

    // Check udp_pings.txt
    std::vector<std::string> lines_txt = read_file_direct(run_dir + "/logs_ns3/udp_pings.txt");
    ASSERT_EQUAL(
            lines_txt[0],
            "UDP ping ID     Source    Target    Start time (ns)   End time (ns)     Interval (ns)     Mean latency there    Mean latency back     Min. RTT        Mean RTT        Max. RTT        Smp.std. RTT    Reply arrival"
    );
    i = 0;
    for (i = 1; i < lines_txt.size(); i++) {
        int j = i - 1;
        std::vector<std::string> line_spl;
        std::istringstream iss(lines_txt[i]);
        for (std::string s; iss >> s;) {
            line_spl.push_back(s);
        }
        ASSERT_EQUAL(line_spl.size(), 20);

        // ID
        ASSERT_PAIR_EQUAL(
                parse_positive_int64(line_spl[0]),
                udp_ping_schedule.at(j).GetUdpPingId()
        );

        // From
        ASSERT_EQUAL(
                parse_positive_int64(line_spl[1]),
                udp_ping_schedule.at(j).GetFromNodeId()
        );

        // To
        ASSERT_EQUAL(
                parse_positive_int64(line_spl[2]),
                udp_ping_schedule.at(j).GetToNodeId()
        );

        // Start time
        ASSERT_EQUAL(
                parse_positive_int64(line_spl[3]),
                udp_ping_schedule.at(j).GetStartTimeNs()
        );

        // End time
        ASSERT_EQUAL(
                parse_positive_int64(line_spl[4]),
                udp_ping_schedule.at(j).GetStartTimeNs() + udp_ping_schedule.at(j).GetDurationNs()
        );

        // Interval
        ASSERT_EQUAL(
                parse_positive_int64(line_spl[5]),
                udp_ping_schedule.at(j).GetIntervalNs()
        );

        // Check
        bool any_valid = list_rtt_ns_valid.at(j).size() > 0;

        // Latency there statistics
        double sum_latency_there = 0.0;
        for (int64_t valid_latency : list_latency_there_ns_valid.at(j)) {
            sum_latency_there += valid_latency;
        }
        double expected_mean_latency_there = any_valid ? sum_latency_there / list_latency_there_ns_valid.at(j).size() : -1;

        // Latency back statistics
        double sum_latency_back = 0.0;
        for (int64_t valid_latency : list_latency_back_ns_valid.at(j)) {
            sum_latency_back += valid_latency;
        }
        double expected_mean_latency_back = any_valid ? sum_latency_back / list_latency_back_ns_valid.at(j).size() : -1;

        // RTTs
        int64_t min_rtt_ns = 100000000000000;
        double sum_rtt_ns = -1;
        int64_t max_rtt_ns = -1;
        for (int64_t valid_rtt : list_rtt_ns_valid.at(j)) {
            sum_rtt_ns += valid_rtt;
            min_rtt_ns = std::min(min_rtt_ns, valid_rtt);
            max_rtt_ns = std::max(max_rtt_ns, valid_rtt);
        }
        if (!any_valid) {
            min_rtt_ns = -1;
        }
        double mean_rtt_ns = any_valid ? sum_rtt_ns / list_rtt_ns_valid.at(j).size() : -1;
        double sum_rtt_min_mean_sq_ns = 0.0;
        for (int64_t valid_rtt : list_rtt_ns_valid.at(j)) {
            sum_rtt_min_mean_sq_ns += std::pow(valid_rtt - mean_rtt_ns, 2);
        }
        double sample_std_rtt_ns = any_valid ? (list_rtt_ns_valid.at(j).size() > 1 ? std::sqrt(sum_rtt_min_mean_sq_ns / (list_rtt_ns_valid.at(j).size() - 1)) : 0) : -1;

        // Match log with the above calculated RTTs
        ASSERT_EQUAL_APPROX(parse_double(line_spl[6]), (expected_mean_latency_there == -1 ? -1 : expected_mean_latency_there / 1e6), 0.01);
        ASSERT_EQUAL(line_spl[7], "ms");
        ASSERT_EQUAL_APPROX(parse_double(line_spl[8]), (expected_mean_latency_back == -1 ? -1 : expected_mean_latency_back / 1e6), 0.01);
        ASSERT_EQUAL(line_spl[9], "ms");
        ASSERT_EQUAL_APPROX(parse_double(line_spl[10]), (min_rtt_ns == -1 ? -1 : min_rtt_ns / 1e6), 0.01);
        ASSERT_EQUAL(line_spl[11], "ms");
        ASSERT_EQUAL_APPROX(parse_double(line_spl[12]), (mean_rtt_ns == -1 ? -1 : mean_rtt_ns / 1e6), 0.01);
        ASSERT_EQUAL(line_spl[13], "ms");
        ASSERT_EQUAL_APPROX(parse_double(line_spl[14]), (max_rtt_ns == -1 ? -1 : max_rtt_ns / 1e6), 0.01);
        ASSERT_EQUAL(line_spl[15], "ms");
        ASSERT_EQUAL_APPROX(parse_double(line_spl[16]), (sample_std_rtt_ns == -1 ? -1 : sample_std_rtt_ns / 1e6), 0.01);
        ASSERT_EQUAL(line_spl[17], "ms");
        ASSERT_EQUAL(line_spl[18], std::to_string(list_rtt_ns_valid.at(j).size()) + "/" + std::to_string(list_rtt_ns.at(j).size()));
        ASSERT_EQUAL(line_spl[19], "(" + std::to_string((int) std::round(((double) list_rtt_ns_valid.at(j).size() / (double) list_rtt_ns.at(j).size()) * 100.0)) + "%)");

    }
    ASSERT_EQUAL(i - 1, udp_ping_schedule.size());

}

