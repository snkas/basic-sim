/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/test.h"
#include "ns3/topology-ptop.h"
#include "test-helpers.h"
#include "ns3/ipv4-arbiter-routing-helper.h"
#include "ns3/traffic-control-layer.h"
#include "ns3/fq-codel-queue-disc.h"
#include "ns3/ptop-link-queue-tracker-helper.h"
#include "ns3/udp-burst-scheduler.h"

using namespace ns3;

class TestCaseWithLogValidators : public TestCase
{
public:
    TestCaseWithLogValidators (std::string s) : TestCase (s) {};

    /**
     * Validation of link queue logs.
     * 
     * @param run_dir           In:  run directory
     * @param dir_a_b_list      In:  list of edges for which logging was done
     * @param link_queue_pkt    Out: mapping of pair to a vector of intervals <from, to, queue size in byte>
     * @param link_queue_byte   Out: mapping of pair to a vector of intervals <from, to, queue size in byte>
     */
    void validate_link_queue_logs(
            std::string run_dir,
            std::vector<std::pair<int64_t, int64_t>> dir_a_b_list,
            std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>>& link_queue_pkt,
            std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>>& link_queue_byte
    ) {

        // Sort it
        std::sort(dir_a_b_list.begin(), dir_a_b_list.end());

        // Initialize result
        for (std::pair<int64_t, int64_t> a_b : dir_a_b_list) {
            link_queue_pkt[a_b] = std::vector<std::tuple<int64_t, int64_t, int64_t>>();
            link_queue_byte[a_b] = std::vector<std::tuple<int64_t, int64_t, int64_t>>();
        }

        // Both link queue files need to be checked, and their checking is nearly identical
        for (size_t file_choice = 0;  file_choice < 2; file_choice++) {

            // Packets or bytes, all the checks are the same basically
            std::vector<std::string> lines_csv;
            if (file_choice == 0) {
                lines_csv = read_file_direct(run_dir + "/logs_ns3/link_queue_pkt.csv");
            } else {
                lines_csv = read_file_direct(run_dir + "/logs_ns3/link_queue_byte.csv");
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
                    link_queue_pkt[std::make_pair(from_node_id, to_node_id)].push_back(std::make_tuple(interval_start_ns, interval_end_ns, num_packets_or_bytes));
                } else {
                    link_queue_byte[std::make_pair(from_node_id, to_node_id)].push_back(std::make_tuple(interval_start_ns, interval_end_ns, num_packets_or_bytes));
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
        ASSERT_EQUAL(link_queue_byte.size(), dir_a_b_list.size());
        ASSERT_EQUAL(link_queue_byte.size(), link_queue_pkt.size());

        // Their size and intervals must be equal of the two files
        for (std::pair <int64_t, int64_t> a_b : dir_a_b_list) {
            ASSERT_EQUAL(link_queue_byte.at(a_b).size(), link_queue_pkt.at(a_b).size());
            for (uint32_t i = 0; i < link_queue_byte.at(a_b).size(); i++) {

                // Interval start
                ASSERT_EQUAL(
                        std::get<0>(link_queue_byte.at(a_b)[i]),
                        std::get<0>(link_queue_pkt.at(a_b)[i])
                );

                // Interval end
                ASSERT_EQUAL(
                        std::get<1>(link_queue_byte.at(a_b)[i]),
                        std::get<1>(link_queue_pkt.at(a_b)[i])
                );

                // Packets vs. bytes
                ASSERT_EQUAL(
                        std::get<2>(link_queue_byte.at(a_b)[i]),
                        std::get<2>(link_queue_pkt.at(a_b)[i]) * 1502
                );

            }
        }

    }
    
};
