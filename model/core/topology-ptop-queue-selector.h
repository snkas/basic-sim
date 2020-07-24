#ifndef TOPOLOGY_PTOP_QUEUE_SELECTOR_H
#define TOPOLOGY_PTOP_QUEUE_SELECTOR_H

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

    class TopologyPtopQueueSelector {
    public:

        /**
         * Parse the queue value into a queue object factory.
         *
         * @param value     String value (e.g., "drop_tail(100p)", "drop_tail(100000B)")
         *
         * @return Queue object factory if success, else throws an exception
         */
        static ObjectFactory Parse(std::string value) {

            // First-in-first-out = drop-tail
            if (starts_with(value, "drop_tail(") && ends_with(value, ")")) {

                // Get rid of the drop_tail( and ) part
                std::string fifo_max_queue_size_value = value.substr(10, value.size() - 11);

                // Make sure it is either "100p" or "100000B'
                if (!ends_with(fifo_max_queue_size_value, "p") && !ends_with(fifo_max_queue_size_value, "B")) {
                    throw std::runtime_error(
                            "Invalid maximum drop-tail queue size value: " + fifo_max_queue_size_value);
                }
                parse_positive_int64(fifo_max_queue_size_value.substr(0, fifo_max_queue_size_value.size() - 1));

                // Finally create the queue factory
                ObjectFactory queueFactory;
                queueFactory.SetTypeId("ns3::DropTailQueue<Packet>");
                queueFactory.Set("MaxSize", QueueSizeValue(QueueSize(fifo_max_queue_size_value)));
                return queueFactory;

            } else {
                throw std::runtime_error("Invalid queue value: " + value);
            }

        }

    };

}

#endif //TOPOLOGY_PTOP_QUEUE_SELECTOR_H
