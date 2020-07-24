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
     * Validate that the traffic control queueing discipline is permitted.
     * Only the values "default" and "disabled" are permitted.
     *
     * @param value     String value (e.g., "disabled", "default", "fq_co_del_better_rtt")
     *
     * @return The same value if the parsing was successful, else it will have thrown an exception.
     */
    std::string TopologyPtop::ValidateTcQdiscValue(std::string value) {
        if (value == "default") {
            return value;
        } else if (value == "disabled") {
            return value;
        } else if (value == "fq_co_del_better_rtt") {
            return value;
        } else {
            throw std::runtime_error("Invalid traffic control qdisc value: " + value);
        }
    }

    /**
     * Parse the traffic control queueing discipline value into a traffic control helper which
     * can generate that.
     *
     * @param topology  Handle to the topology (might have relevant information)
     * @param value     String value (e.g., "default", "fq_co_del_better_rtt")
     *
     * @return Traffic control helper (or an exception if invalid)
     */
    TrafficControlHelper TopologyPtop::ParseTcQdiscValue(std::string value) {
        if (value == "default") {
            TrafficControlHelper defaultHelper;
            return defaultHelper;

        } else if (value == "fq_co_del_better_rtt") {
            TrafficControlHelper fqCoDelBetterRttHelper;
            std::string interval = format_string("%" PRId64 "ns", m_worst_case_rtt_estimate_ns);
            std::string target = format_string("%" PRId64 "ns", m_worst_case_rtt_estimate_ns / 20);
            fqCoDelBetterRttHelper.SetRootQueueDisc(
                    "ns3::FqCoDelQueueDisc",
                    "Interval", StringValue(interval),
                    "Target", StringValue(target)
            );
            return fqCoDelBetterRttHelper;

        } else {
            throw std::runtime_error("Invalid traffic control qdisc value: " + value);
        }
    }

}
