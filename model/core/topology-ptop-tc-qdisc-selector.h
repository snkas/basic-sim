#ifndef TOPOLOGY_PTOP_TC_QDISC_SELECTOR_H
#define TOPOLOGY_PTOP_TC_QDISC_SELECTOR_H

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

namespace ns3 {

    class TopologyPtopTcQdiscSelector {
    public:

        /**
         * Validate that the traffic control queueing discipline is permitted.
         * Only the values "default" and "disabled" are permitted.
         *
         * @param value     String value (e.g., "disabled", "default")
         *
         * @return The same value if the parsing was successful, else it will have thrown an exception.
         */
        static std::string Validate(std::string value) {
            if (value == "default") {
                return value;
            } else if (value == "disabled") {
                return value;
            } else {
                throw std::runtime_error("Invalid traffic control qdisc value: " + value);
            }
        }

        /**
         * Parse the traffic control queueing discipline value into a traffic control helper which
         * can generate that.
         *
         * @param value     String value (e.g., "default")
         *
         * @return Traffic control helper (or an exception if invalid)
         */
        static TrafficControlHelper Parse(std::string value) {
            if (value == "default") {
                TrafficControlHelper defaultHelper;
                return defaultHelper;
            } else {
                throw std::runtime_error("Invalid traffic control qdisc value: " + value);
            }
        }

    };

}

#endif //TOPOLOGY_PTOP_TC_QDISC_SELECTOR_H
