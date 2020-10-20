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

        } else {
            throw std::runtime_error("Invalid traffic control qdisc value: " + value);
        }
    }

}
