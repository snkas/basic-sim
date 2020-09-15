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

#include "ptop-link-queue-tracker.h"

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED (PtopLinkQueueTracker);
    TypeId PtopLinkQueueTracker::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::PtopLinkQueueTracker")
                .SetParent<Object> ()
                .SetGroupName("BasicSim")
        ;
        return tid;
    }

    PtopLinkQueueTracker::PtopLinkQueueTracker(Ptr<PointToPointNetDevice> netDevice) {

        // Register this tracker into the tracing callbacks of the network device
        m_queue = netDevice->GetQueue();
        
        // Logging bytes in the queue
        m_current_num_bytes = 0;
        m_current_num_bytes_timestamp = 0;
        m_queue->TraceConnectWithoutContext("BytesInQueue", MakeCallback(&PtopLinkQueueTracker::NetDeviceBytesInQueueCallback, this));
        
        // Logging number of packets in the queue
        m_current_num_packets = 0;
        m_current_num_packets_timestamp = 0;
        m_queue->TraceConnectWithoutContext("PacketsInQueue", MakeCallback(&PtopLinkQueueTracker::NetDevicePacketsInQueueCallback, this));

    }

    void PtopLinkQueueTracker::NetDeviceBytesInQueueCallback(uint32_t, uint32_t num_bytes) {
        int64_t now_ns = (int64_t) Simulator::Now().GetNanoSeconds();
        if (now_ns != m_current_num_bytes_timestamp) {
            m_log_time_num_bytes.push_back(std::make_tuple(m_current_num_bytes_timestamp, m_current_num_bytes));
        }
        m_current_num_bytes = num_bytes;
        m_current_num_bytes_timestamp = now_ns;
    }

    void PtopLinkQueueTracker::NetDevicePacketsInQueueCallback(uint32_t, uint32_t num_packets) {
        int64_t now_ns = (int64_t) Simulator::Now().GetNanoSeconds();
        if (now_ns != m_current_num_packets_timestamp) {
            m_log_time_num_packets.push_back(std::make_tuple(m_current_num_packets_timestamp, m_current_num_packets));
        }
        m_current_num_packets = num_packets;
        m_current_num_packets_timestamp = now_ns;
    }

    const std::vector<std::tuple<int64_t, int64_t>>& PtopLinkQueueTracker::GetLogTimeNumBytes() {
        // Log the last known value
        m_log_time_num_bytes.push_back(std::make_tuple(m_current_num_bytes_timestamp, m_current_num_bytes));

        return m_log_time_num_bytes;
    }

    const std::vector<std::tuple<int64_t, int64_t>>& PtopLinkQueueTracker::GetLogTimeNumPackets() {
        // Log the last known value
        m_log_time_num_packets.push_back(std::make_tuple(m_current_num_packets_timestamp, m_current_num_packets));

        return m_log_time_num_packets;
    }

}
