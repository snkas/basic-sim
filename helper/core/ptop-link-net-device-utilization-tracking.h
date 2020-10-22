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

#ifndef PTOP_LINK_NET_DEVICE_UTILIZATION_TRACKER_HELPER_H
#define PTOP_LINK_NET_DEVICE_UTILIZATION_TRACKER_HELPER_H

#define UTILIZATION_TRACKER_COMPRESSION_APPROXIMATELY_NOT_EQUAL 0.000001

#include "ns3/basic-simulation.h"
#include "ns3/topology-ptop.h"
#include "ns3/net-device-utilization-tracker.h"

namespace ns3 {

    class PtopLinkNetDeviceUtilizationTracking
    {

    public:
        PtopLinkNetDeviceUtilizationTracking(Ptr<BasicSimulation> basicSimulation, Ptr<TopologyPtop> topology);
        void WriteResults();

    private:
        std::vector<std::pair<std::pair<int64_t, int64_t>, Ptr<NetDeviceUtilizationTracker>>> m_utilization_trackers;
        Ptr<BasicSimulation> m_basicSimulation;
        Ptr<TopologyPtop> m_topology;
        int64_t m_utilization_interval_ns;
        bool m_enabled;

        std::string m_filename_net_device_utilization_csv;
        std::string m_filename_net_device_utilization_compressed_csv;
        std::string m_filename_net_device_utilization_compressed_txt;
        std::string m_filename_net_device_utilization_summary_txt;

        bool m_enable_distributed;

    };


} // namespace ns3

#endif /* PTOP_LINK_NET_DEVICE_UTILIZATION_TRACKER_HELPER_H */
