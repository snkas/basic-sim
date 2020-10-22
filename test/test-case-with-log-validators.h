/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/test.h"
#include "ns3/topology-ptop.h"
#include "test-helpers.h"
#include "ns3/ipv4-arbiter-routing-helper.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/fq-codel-queue-disc.h"
#include "ns3/ptop-link-net-device-queue-tracking.h"
#include "ns3/udp-burst-scheduler.h"

using namespace ns3;

class TestCaseWithLogValidators : public TestCase
{
public:
    TestCaseWithLogValidators (std::string s) : TestCase (s) {};

    /**
     * Validation of link net-device queue logs.
     * 
     * @param run_dir           In:  run directory
     * @param dir_a_b_list      In:  list of edges for which logging was done
     * @param link_net_device_queue_pkt    Out: mapping of pair to a vector of intervals <from, to, queue size in byte>
     * @param link_net_device_queue_byte   Out: mapping of pair to a vector of intervals <from, to, queue size in byte>
     */
    void validate_link_net_device_queue_logs(
            std::string run_dir,
            std::vector<std::pair<int64_t, int64_t>> dir_a_b_list,
            std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>>& link_net_device_queue_pkt,
            std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>>& link_net_device_queue_byte
    ) {

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
                ASSERT_EQUAL(
                        std::get<2>(link_net_device_queue_byte.at(a_b)[i]),
                        std::get<2>(link_net_device_queue_pkt.at(a_b)[i]) * 1502
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
    void validate_link_net_device_utilization_logs(
            std::string run_dir,
            std::vector<std::pair<int64_t, int64_t>> dir_a_b_list,
            int64_t duration_ns,
            int64_t link_net_device_utilization_tracking_interval_ns,
            std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>>& link_net_device_utilization,
            std::map<std::pair<int64_t, int64_t>, double>& link_overall_utilization_as_fraction
    ) {

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
     * Validation of UDP burst logs.
     *
     * @param simulation_end_time_ns           In:  duration of simulation
     * @param run_dir                          In:  run directory
     * @param udp_schedule                     In:  UDP schedule
     * @param list_outgoing_rate_megabit_per_s   Out: vector of UDP bursts' outgoing rates in megabit/s
     * @param list_incoming_rate_megabit_per_s   Out: vector of UDP bursts' incoming rates in megabit/s
     */
    void validate_udp_burst_logs(
            int64_t simulation_end_time_ns,
            std::string run_dir,
            std::vector<UdpBurstInfo> udp_schedule,
            std::vector<double>& list_outgoing_rate_megabit_per_s,
            std::vector<double>& list_incoming_rate_megabit_per_s
    ) {

        // For checking the sent / received amount
        std::vector<int64_t> udp_burst_sent_amount;
        std::vector<int64_t> udp_burst_received_amount;

        // Check udp_bursts_outgoing.csv
        std::vector<std::string> lines_outgoing_csv = read_file_direct(run_dir + "/logs_ns3/udp_bursts_outgoing.csv");
        ASSERT_EQUAL(lines_outgoing_csv.size(), udp_schedule.size());
        int i = 0;
        for (std::string line : lines_outgoing_csv) {
            std::vector<std::string> line_spl = split_string(line, ",");
            ASSERT_EQUAL(line_spl.size(), 12);
            ASSERT_EQUAL(parse_positive_int64(line_spl[0]), i);
            ASSERT_EQUAL(parse_positive_int64(line_spl[1]), udp_schedule[i].GetFromNodeId());
            ASSERT_EQUAL(parse_positive_int64(line_spl[2]), udp_schedule[i].GetToNodeId());
            ASSERT_EQUAL(parse_positive_double(line_spl[3]), udp_schedule[i].GetTargetRateMegabitPerSec());
            ASSERT_EQUAL(parse_positive_int64(line_spl[4]), udp_schedule[i].GetStartTimeNs());
            ASSERT_EQUAL(parse_positive_int64(line_spl[5]), udp_schedule[i].GetDurationNs());
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
            ASSERT_EQUAL(line_spl[11], udp_schedule[i].GetMetadata());
            list_outgoing_rate_megabit_per_s.push_back(outgoing_rate_incl_headers_megabit_per_s);
            i++;
        }

        // Check udp_bursts_incoming.csv
        std::vector<std::string> lines_incoming_csv = read_file_direct(run_dir + "/logs_ns3/udp_bursts_incoming.csv");
        ASSERT_EQUAL(lines_incoming_csv.size(), udp_schedule.size());
        i = 0;
        for (std::string line : lines_incoming_csv) {
            std::vector<std::string> line_spl = split_string(line, ",");
            ASSERT_EQUAL(line_spl.size(), 12);
            ASSERT_EQUAL(parse_positive_int64(line_spl[0]), i);
            ASSERT_EQUAL(parse_positive_int64(line_spl[1]), udp_schedule[i].GetFromNodeId());
            ASSERT_EQUAL(parse_positive_int64(line_spl[2]), udp_schedule[i].GetToNodeId());
            ASSERT_EQUAL(parse_positive_double(line_spl[3]), udp_schedule[i].GetTargetRateMegabitPerSec());
            ASSERT_EQUAL(parse_positive_int64(line_spl[4]), udp_schedule[i].GetStartTimeNs());
            ASSERT_EQUAL(parse_positive_int64(line_spl[5]), udp_schedule[i].GetDurationNs());
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
            ASSERT_EQUAL(line_spl[11], udp_schedule[i].GetMetadata());
            list_incoming_rate_megabit_per_s.push_back(incoming_rate_incl_headers_megabit_per_s);
            i++;
        }

        // Check udp_bursts_outgoing.txt
        std::vector<std::string> lines_outgoing_txt = read_file_direct(run_dir + "/logs_ns3/udp_bursts_outgoing.txt");
        ASSERT_EQUAL(lines_outgoing_txt.size(), udp_schedule.size() + 1);
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
                ASSERT_EQUAL(parse_positive_int64(line_spl[1]), udp_schedule[j].GetFromNodeId());
                ASSERT_EQUAL(parse_positive_int64(line_spl[2]), udp_schedule[j].GetToNodeId());
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[3]), udp_schedule[j].GetTargetRateMegabitPerSec(), 0.01);
                ASSERT_EQUAL(line_spl[4], "Mbit/s");
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[5]), nanosec_to_millisec(udp_schedule[j].GetStartTimeNs()), 0.01);
                ASSERT_EQUAL(line_spl[6], "ms");
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[7]), nanosec_to_millisec(udp_schedule[j].GetDurationNs()), 0.01);
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
                    ASSERT_EQUAL(line_spl[18], udp_schedule[j].GetMetadata());
                }
            }
            i++;
        }

        // Check udp_bursts_incoming.txt
        std::vector<std::string> lines_incoming_txt = read_file_direct(run_dir + "/logs_ns3/udp_bursts_incoming.txt");
        ASSERT_EQUAL(lines_incoming_txt.size(), udp_schedule.size() + 1);
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
                ASSERT_EQUAL(parse_positive_int64(line_spl[1]), udp_schedule[j].GetFromNodeId());
                ASSERT_EQUAL(parse_positive_int64(line_spl[2]), udp_schedule[j].GetToNodeId());
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[3]), udp_schedule[j].GetTargetRateMegabitPerSec(), 0.01);
                ASSERT_EQUAL(line_spl[4], "Mbit/s");
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[5]), nanosec_to_millisec(udp_schedule[j].GetStartTimeNs()), 0.01);
                ASSERT_EQUAL(line_spl[6], "ms");
                ASSERT_EQUAL_APPROX(parse_positive_double(line_spl[7]), nanosec_to_millisec(udp_schedule[j].GetDurationNs()), 0.01);
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
                    ASSERT_EQUAL(line_spl[18], udp_schedule[j].GetMetadata());
                }
            }
            i++;
        }

        // Check the precise outgoing / incoming logs for each burst
        for (UdpBurstInfo entry : udp_schedule) {

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
    
};
