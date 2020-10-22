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
     * @param value     String value (e.g., "disabled", "default", "fq_codel_better_rtt")
     *
     * @return Pair(True iff enabled, traffic control helper instance) (or an exception if invalid)
     */
    std::pair<bool, TrafficControlHelper> TopologyPtopTcQdiscSelectorDefault::ParseTcQdiscValue(Ptr<TopologyPtop> topology, std::string value) {
        if (value == "disabled") {
            TrafficControlHelper unusedHelper; // We have to create some instance, but it is not used
            return std::make_pair(false, unusedHelper);

        } else if (value == "default") {
            TrafficControlHelper defaultHelper; // Default is actually fq_codel with some high RTT value
            return std::make_pair(true, defaultHelper);

        } else if (value == "fq_codel_better_rtt") {
            TrafficControlHelper fqCoDelBetterRttHelper;
            std::string interval = format_string("%" PRId64 "ns", topology->GetWorstCaseRttEstimateNs());
            std::string target = format_string("%" PRId64 "ns", topology->GetWorstCaseRttEstimateNs() / 20);
            fqCoDelBetterRttHelper.SetRootQueueDisc(
                    "ns3::FqCoDelQueueDisc",
                    "Interval", StringValue(interval),
                    "Target", StringValue(target)
            );
            return std::make_pair(true, fqCoDelBetterRttHelper);

        } else if (starts_with(value, "fifo(") && ends_with(value, ")")) { // First-in-first-out = drop-tail (in ns-3, it is called "FIFO" for qdiscs)

            // Get rid of the fifo( and ) part
            std::string max_queue_size_value = value.substr(5, value.size() - 6);

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
                    "MaxSize", QueueSizeValue (QueueSize (max_queue_size_value))  // Maximum queue size (packets)
            );
            return std::make_pair(true, fifoHelper);

        } else if (starts_with(value, "simple_red(") && ends_with(value, ")")) {

            // Get rid of the "simple_red("-prefix and ")"-postfix
            std::string inner_part = value.substr(11, value.size() - 12);
            std::vector<std::string> spl = split_string(inner_part, ";", 4);

            // Action
            std::string str_action = spl.at(0);
            if (str_action != "ecn" && str_action != "drop") {
                throw std::invalid_argument("Invalid RED action: " + str_action);
            }
            bool action_is_ecn = str_action == "ecn";

            // Thresholds and maximum size
            int64_t min_th_pkt = parse_positive_int64(spl.at(1));
            int64_t max_th_pkt = parse_positive_int64(spl.at(2));
            int64_t max_size_pkt = parse_positive_int64(spl.at(3));
            if (min_th_pkt > max_th_pkt) {
                throw std::invalid_argument("RED minimum threshold cannot exceed maximum threshold");
            }
            if (max_th_pkt > max_size_pkt) {
                throw std::invalid_argument("RED maximum threshold cannot exceed maximum queue size");
            }

            // Create the traffic control helper
            TrafficControlHelper simpleRedHelper;

            // Set the root queueing discipline to RED with the attributes
            simpleRedHelper.SetRootQueueDisc(
                    "ns3::RedQueueDisc",
                    "QW", DoubleValue (1),                           // Only instantaneous queue size is used to estimate average queue size (thus they are equal)
                    "UseEcn", BooleanValue (action_is_ecn),          // Either: (a) mark with ECN
                    "UseHardDrop", BooleanValue (!action_is_ecn),    //         (b) drop
                    "MeanPktSize", UintegerValue (1500),             // Mean packet size we set to 1500 byte always
                    "MinTh", DoubleValue (min_th_pkt),               // RED minimum threshold (packets)
                    "MaxTh", DoubleValue (max_th_pkt),               // RED maximum threshold (packets)
                    "MaxSize", QueueSizeValue (QueueSize (std::to_string(max_size_pkt) + "p"))  // Maximum queue size (packets)
            );

            // Return traffic control qdisc is enabled and the helper to create it
            return std::make_pair(true, simpleRedHelper);

        } else {
            throw std::runtime_error("Invalid traffic control qdisc value: " + value);
        }
    }

}
