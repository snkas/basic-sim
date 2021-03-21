/*
 * Copyright (c) 2020 ETH Zurich
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Simon
 */

#include "topology-ptop-tc-qdisc-selector-default.h"

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED (TopologyPtopTcQdiscSelectorDefault);
    TypeId TopologyPtopTcQdiscSelectorDefault::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::TopologyPtopTcQdiscSelectorDefault")
                .SetParent<TopologyPtopTcQdiscSelector> ()
                .SetGroupName("BasicSim")
        ;
        return tid;
    }

    /**
     * Parse the traffic control queueing discipline value into a traffic control helper which
     * can generate that.
     *
     * @param topology  Handle to the topology (might have relevant information)
     * @param value     String value (e.g., "disabled", "default", "fq_codel(...)",
     *                  "fifo(...)", "pfifo_fast(...)", "simple_red(...)")
     *
     * @return Tuple(True iff enabled, traffic control helper instance, queue size) (or an exception if invalid)
     */
    std::pair<bool, TrafficControlHelper> TopologyPtopTcQdiscSelectorDefault::ParseTcQdiscValue(Ptr<TopologyPtop> topology, std::string value) {
        if (value == "disabled") {
            TrafficControlHelper unusedHelper; // We have to create some instance, but it is not used
            return std::make_pair(false, unusedHelper);

        } else if (value == "default") {
            TrafficControlHelper defaultHelper; // Default is actually fq_codel with values for high RTTs
            return std::make_pair(true, defaultHelper);

        } else if (starts_with(value, "fq_codel(") && ends_with(value, ")")) {  // fq_codel(interval_ns; target_ns; max_queue_size)

            // Get rid of the "fq_codel("-prefix and ")"-postfix
            std::string inner_part = value.substr(9, value.size() - 10);
            std::vector<std::string> spl = split_string(inner_part, ";", 3);

            // Interval
            std::string interval = format_string("%" PRId64 "ns", parse_geq_one_int64(spl.at(0)));

            // Target
            std::string target = format_string("%" PRId64 "ns", parse_geq_one_int64(spl.at(1)));

            // Maximum queue size
            // Make sure it is like "100p"
            std::string max_queue_size_value = trim(spl.at(2));
            if (!ends_with(max_queue_size_value, "p")) {
                throw std::runtime_error("Invalid maximum fq_codel queue size value: " + max_queue_size_value);
            }
            parse_geq_one_int64(max_queue_size_value.substr(0, max_queue_size_value.size() - 1));

            // Create the traffic control helper
            TrafficControlHelper fqCoDelHelper;
            fqCoDelHelper.SetRootQueueDisc(
                    "ns3::FqCoDelQueueDisc",
                    "Interval", StringValue(interval),
                    "Target", StringValue(target),
                    "MaxSize", QueueSizeValue (QueueSize (max_queue_size_value))
            );

            // Return result
            return std::make_pair(true, fqCoDelHelper);

        } else if (starts_with(value, "fifo(") && ends_with(value, ")")) { // First-in-first-out = drop-tail (in ns-3, it is called "FIFO" for qdiscs)

            // Get rid of the fifo( and ) part
            std::string max_queue_size_value = trim(value.substr(5, value.size() - 6));

            // Make sure it is either "100p" or "100000B'
            if (!ends_with(max_queue_size_value, "p") && !ends_with(max_queue_size_value, "B")) {
                throw std::runtime_error(
                        "Invalid maximum FIFO queue size value: " + max_queue_size_value);
            }
            parse_geq_one_int64(max_queue_size_value.substr(0, max_queue_size_value.size() - 1));

            // Return FIFO traffic control helper
            TrafficControlHelper fifoHelper;
            fifoHelper.SetRootQueueDisc(
                    "ns3::FifoQueueDisc",
                    "MaxSize", QueueSizeValue (QueueSize (max_queue_size_value))  // Maximum queue size
            );
            return std::make_pair(true, fifoHelper);

        } else if (starts_with(value, "pfifo_fast(") && ends_with(value, ")")) { // Priority first-in-first-out with 3 bands

            // Get rid of the pfifo_fast( and ) part
            std::string max_queue_size_value = trim(value.substr(11, value.size() - 12));

            // Make sure it is either "100p" or "100000B'
            if (!ends_with(max_queue_size_value, "p") && !ends_with(max_queue_size_value, "B")) {
                throw std::runtime_error(
                        "Invalid maximum pfifo_fast queue size value: " + max_queue_size_value);
            }
            parse_geq_one_int64(max_queue_size_value.substr(0, max_queue_size_value.size() - 1));

            // Return pfifo_fast traffic control helper
            TrafficControlHelper pfifoFastHelper;
            pfifoFastHelper.SetRootQueueDisc(
                    "ns3::PfifoFastQueueDisc",
                    "MaxSize", QueueSizeValue (QueueSize (max_queue_size_value))  // Maximum queue size (packets)
            );
            return std::make_pair(true, pfifoFastHelper);

        // simple_red(ecn/drop; mean_pkt_size_byte; queue_weight; min_th_pkt; max_th_pkt; max_size; max_p; wait/no_wait; gentle/not_gentle)
        } else if (starts_with(value, "simple_red(") && ends_with(value, ")")) {

            // Get rid of the "simple_red("-prefix and ")"-postfix
            std::string inner_part = value.substr(11, value.size() - 12);
            std::vector<std::string> spl = split_string(inner_part, ";", 9);

            // Action
            std::string str_action = trim(spl.at(0));
            if (str_action != "ecn" && str_action != "drop") {
                throw std::invalid_argument("Invalid RED action: " + str_action);
            }
            bool action_is_ecn = str_action == "ecn";

            // Mean packet size in byte
            int64_t mean_pkt_size_byte = parse_geq_one_int64(spl.at(1));

            // Thresholds and maximum size
            double queue_weight = parse_double(spl.at(2));
            if (queue_weight <= 0.0 || queue_weight > 1.0) {
                throw std::invalid_argument("Queue weight must be in range (0, 1.0]: " + trim(spl.at(2)));
            }

            // Minimum threshold
            int64_t min_th_pkt = parse_positive_int64(spl.at(3));

            // Maximum threshold
            int64_t max_th_pkt = parse_positive_int64(spl.at(4));

            // Make sure it is either "100p" or "100000B'
            std::string max_size_str = trim(spl.at(5));
            if (!ends_with(max_size_str, "p") && !ends_with(max_size_str, "B")) {
                throw std::runtime_error(
                        "Invalid maximum RED queue size value: " + max_size_str
                );
            }
            bool max_size_is_in_pkt = ends_with(max_size_str, "p");
            int64_t max_size = parse_geq_one_int64(max_size_str.substr(0, max_size_str.size() - 1));
            int64_t max_size_pkt = max_size_is_in_pkt ? max_size : (int64_t) std::ceil((double) max_size / (double) mean_pkt_size_byte);

            // Check thresholds are valid
            if (min_th_pkt > max_th_pkt) {
                throw std::invalid_argument(format_string(
                        "RED minimum threshold (%d) cannot exceed maximum threshold (%d)",
                        min_th_pkt, max_th_pkt
                ));
            }
            if (max_th_pkt > max_size_pkt) {
                throw std::invalid_argument(format_string(
                        "RED maximum threshold (%d) cannot exceed maximum queue size (%d)",
                        max_th_pkt, max_size_pkt
                ));
            }

            // Maximum probability at maximum threshold
            double max_p = parse_double(spl.at(6));
            if (max_p <= 0.0 || max_p > 1.0) {
                throw std::invalid_argument("Maximum probability must be in range (0, 1.0]: " + trim(spl.at(6)));
            }

            // Whether to wait
            std::string str_wait = trim(spl.at(7));
            if (str_wait != "wait" && str_wait != "no_wait") {
                throw std::invalid_argument("Invalid RED wait: " + str_wait);
            }
            bool wait = str_wait == "wait";

            // Whether to be gentle
            std::string str_gentle = trim(spl.at(8));
            if (str_gentle != "gentle" && str_gentle != "not_gentle") {
                throw std::invalid_argument("Invalid RED gentle: " + str_gentle);
            }
            bool gentle = str_gentle == "gentle";

            // Create the traffic control helper
            TrafficControlHelper simpleRedHelper;

            // Set the root queueing discipline to RED with the attributes
            simpleRedHelper.SetRootQueueDisc(
                    "ns3::RedQueueDisc",
                    "UseEcn", BooleanValue (action_is_ecn),               // Either: (a) mark with ECN
                    "UseHardDrop", BooleanValue (!action_is_ecn),         //         (b) drop
                    "MeanPktSize", UintegerValue (mean_pkt_size_byte),    // Mean packet size in byte (1500 byte is recommended)
                    "QW", DoubleValue (queue_weight),                     // EWMA weight of the instantaneous queue size
                                                                          // At QW = 1: Only instantaneous queue size is used to estimate average queue size (thus they are equal)
                                                                          // At QW < 1: The average queue size changes not as quickly
                    "MinTh", DoubleValue (min_th_pkt),                    // RED minimum threshold (packets)
                    "MaxTh", DoubleValue (max_th_pkt),                    // RED maximum threshold (packets)
                    "MaxSize", QueueSizeValue (QueueSize (max_size_str)), // Maximum queue size
                    "LInterm", DoubleValue(1.0 / max_p),                  // The probability at the RED maximum threshold (inverse)
                    "Wait", BooleanValue (wait),                          // Whether to wait between packets to drop
                    "Gentle", BooleanValue (gentle)                       // Whether to start dropping hard at maximum threshold, or scale linearly to 2 * maximum threshold first
            );

            // Return traffic control qdisc is enabled and the helper to create it
            return std::make_pair(true, simpleRedHelper);

        } else {
            throw std::runtime_error("Invalid traffic control qdisc value: " + value);
        }
    }

}
