/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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

#include "queue-tracker.h"

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED (QueueTracker);
    TypeId QueueTracker::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::QueueTracker")
                .SetParent<Object> ()
                .SetGroupName("BasicSim")
        ;
        return tid;
    }

    // Register this tracker into the tracing callbacks of the queue
    QueueTracker::QueueTracker(Ptr<Queue<Packet>> queue) {

        // Save queue pointer
        m_queue = queue;

        // Logging number of packets in the queue
        m_log_update_helper_queue_pkt = LogUpdateHelper<int64_t>();
        m_log_update_helper_queue_pkt.Update(0, 0);
        m_queue->TraceConnectWithoutContext("PacketsInQueue", MakeCallback(&QueueTracker::PacketsInQueueCallback, this));
        
        // Logging bytes in the queue
        m_log_update_helper_queue_byte = LogUpdateHelper<int64_t>();
        m_log_update_helper_queue_byte.Update(0, 0);
        m_queue->TraceConnectWithoutContext("BytesInQueue", MakeCallback(&QueueTracker::BytesInQueueCallback, this));

    }

    void QueueTracker::PacketsInQueueCallback(uint32_t, uint32_t num_packets) {
        m_log_update_helper_queue_pkt.Update(
                (int64_t) Simulator::Now().GetNanoSeconds(),
                num_packets
        );
    }

    void QueueTracker::BytesInQueueCallback(uint32_t, uint32_t num_bytes) {
        m_log_update_helper_queue_byte.Update(
                (int64_t) Simulator::Now().GetNanoSeconds(),
                num_bytes
        );
    }

    const std::vector<std::tuple<int64_t, int64_t, int64_t>>& QueueTracker::GetIntervalsNumPackets() {
        return m_log_update_helper_queue_pkt.Finalize((int64_t) Simulator::Now().GetNanoSeconds());
    }

    const std::vector<std::tuple<int64_t, int64_t, int64_t>>& QueueTracker::GetIntervalsNumBytes() {
        return m_log_update_helper_queue_byte.Finalize((int64_t) Simulator::Now().GetNanoSeconds());
    }

}
