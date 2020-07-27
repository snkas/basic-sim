#include <utility>
#include "ns3/core-module.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/topology.h"
#include "ns3/exp-util.h"
#include "ns3/basic-simulation.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/command-line.h"
#include "ns3/traffic-control-helper.h"
#include "ns3/point-to-point-ab-helper.h"
#include "ns3/object-factory.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"
#include "topology-ptop.h"

namespace ns3 {

    /**
     * Parse the traffic control queueing discipline value into a traffic control helper which
     * can generate that.
     *
     * @param topology  Handle to the topology (might have relevant information)
     * @param value     String value (e.g., "disabled", "default", "fq_codel_better_rtt")
     *
     * @return Pair(True iff enabled, traffic control helper instance) (or an exception if invalid)
     */
    std::pair<bool, TrafficControlHelper> TopologyPtop::ParseTcQdiscValue(std::string value) {
        if (value == "disabled") {
            TrafficControlHelper unusedHelper; // We have to create some instance, but it is not used
            return std::make_pair(false, unusedHelper);

        } else if (value == "default") {
            TrafficControlHelper defaultHelper; // Default is actually fq_codel with some high RTT value
            return std::make_pair(true, defaultHelper);

        } else if (value == "fq_codel_better_rtt") {
            TrafficControlHelper fqCoDelBetterRttHelper;
            std::string interval = format_string("%" PRId64 "ns", m_worst_case_rtt_estimate_ns);
            std::string target = format_string("%" PRId64 "ns", m_worst_case_rtt_estimate_ns / 20);
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
