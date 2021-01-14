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

#include "qdisc-queue-tracker.h"

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED (QdiscQueueTracker);
    TypeId QdiscQueueTracker::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::QdiscQueueTracker")
                .SetParent<Object> ()
                .SetGroupName("BasicSim")
        ;
        return tid;
    }

    QdiscQueueTracker::QdiscQueueTracker(Ptr<QueueDisc> qdisc) {

        // Register this tracker into the tracing callbacks of the root queueing discipline
        m_qdisc = qdisc;

        // Logging number of packets in the qdisc
        m_log_update_helper_qdisc_pkt = LogUpdateHelper<int64_t>();
        m_log_update_helper_qdisc_pkt.Update(0, 0);
        m_qdisc->TraceConnectWithoutContext("PacketsInQueue", MakeCallback(&QdiscQueueTracker::QueueDiscPacketsInQueueCallback, this));
        
        // Logging bytes in the qdisc
        m_log_update_helper_qdisc_byte = LogUpdateHelper<int64_t>();
        m_log_update_helper_qdisc_byte.Update(0, 0);
        m_qdisc->TraceConnectWithoutContext("BytesInQueue", MakeCallback(&QdiscQueueTracker::QueueDiscBytesInQueueCallback, this));

    }

    void QdiscQueueTracker::QueueDiscPacketsInQueueCallback(uint32_t, uint32_t num_packets) {
        m_log_update_helper_qdisc_pkt.Update(
                (int64_t) Simulator::Now().GetNanoSeconds(),
                num_packets
        );
    }

    void QdiscQueueTracker::QueueDiscBytesInQueueCallback(uint32_t, uint32_t num_bytes) {
        m_log_update_helper_qdisc_byte.Update(
                (int64_t) Simulator::Now().GetNanoSeconds(),
                num_bytes
        );
    }

    const std::vector<std::tuple<int64_t, int64_t, int64_t>>& QdiscQueueTracker::GetIntervalsNumPackets() {
        return m_log_update_helper_qdisc_pkt.Finalize((int64_t) Simulator::Now().GetNanoSeconds());
    }

    const std::vector<std::tuple<int64_t, int64_t, int64_t>>& QdiscQueueTracker::GetIntervalsNumBytes() {
        return m_log_update_helper_qdisc_byte.Finalize((int64_t) Simulator::Now().GetNanoSeconds());
    }

}
