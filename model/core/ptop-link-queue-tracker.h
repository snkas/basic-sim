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
 * Author: Simon, Hanjing
 */

#ifndef PTOP_LINK_QUEUE_TRACKER_H
#define PTOP_LINK_QUEUE_TRACKER_H

#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <chrono>
#include <stdexcept>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"


namespace ns3 {

    class PtopLinkQueueTracker : public Object {

    private:

        // Parameters
        Ptr<Queue<Packet>> m_queue;

        // State
        int64_t m_current_num_bytes;
        int64_t m_current_num_bytes_timestamp;
        std::vector<std::tuple<int64_t, int64_t>> m_log_time_num_bytes;
        int64_t m_current_num_packets;
        int64_t m_current_num_packets_timestamp;
        std::vector<std::tuple<int64_t, int64_t>> m_log_time_num_packets;

    public:
        static TypeId GetTypeId (void);
        PtopLinkQueueTracker(Ptr<PointToPointNetDevice> netDevice);
        void NetDeviceBytesInQueueCallback(uint32_t, uint32_t num_bytes);
        void NetDevicePacketsInQueueCallback(uint32_t, uint32_t num_packets);
        const std::vector<std::tuple<int64_t, int64_t>>& GetLogTimeNumBytes();
        const std::vector<std::tuple<int64_t, int64_t>>& GetLogTimeNumPackets();
    };

}

#endif // PTOP_LINK_QUEUE_TRACKER_H
