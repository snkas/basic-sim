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

#include "topology-ptop-queue-selector-default.h"

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED (TopologyPtopQueueSelectorDefault);
    TypeId TopologyPtopQueueSelectorDefault::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::TopologyPtopQueueSelectorDefault")
                .SetParent<TopologyPtopQueueSelector> ()
                .SetGroupName("BasicSim")
        ;
        return tid;
    }

    /*
     * Parse the queue value into a queue object factory.
     *
     * @param value     String value (e.g., "drop_tail(100p)", "drop_tail(100000B)")
     *
     * @return Pair of (Queue object factory, Maximum queue size) if success, else throws an exception
     */
    ObjectFactory TopologyPtopQueueSelectorDefault::ParseQueueValue(Ptr<TopologyPtop>, std::string value) {

        // First-in-first-out = drop-tail (in ns-3, it is called "DropTail" for queues)
        if (starts_with(value, "drop_tail(") && ends_with(value, ")")) {

            // Get rid of the drop_tail( and ) part
            std::string max_queue_size_value = value.substr(10, value.size() - 11);

            // Make sure it is either "100p" or "100000B'
            if (!ends_with(max_queue_size_value, "p") && !ends_with(max_queue_size_value, "B")) {
                throw std::runtime_error(
                        "Invalid maximum drop-tail queue size value: " + max_queue_size_value);
            }
            parse_geq_one_int64(max_queue_size_value.substr(0, max_queue_size_value.size() - 1));

            // Finally create the queue factory
            ObjectFactory queueFactory;
            queueFactory.SetTypeId("ns3::DropTailQueue<Packet>");
            queueFactory.Set("MaxSize", QueueSizeValue(QueueSize(max_queue_size_value)));
            return queueFactory;

        } else {
            throw std::runtime_error("Invalid queue value: " + value);
        }

    }

}
