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

#ifndef UDP_PING_SCHEDULER_H
#define UDP_PING_SCHEDULER_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/basic-simulation.h"
#include "ns3/exp-util.h"
#include "ns3/topology.h"
#include "ns3/udp-ping-helper.h"
#include "ns3/udp-ping-schedule-reader.h"
#include "ns3/socket-generator.h"
#include "ns3/ip-tos-generator.h"

namespace ns3 {

class UdpPingScheduler
{

public:
    UdpPingScheduler(Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology);
    UdpPingScheduler(Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology, Ptr<UdpSocketGenerator> udpSocketGenerator, Ptr<IpTosGenerator> ipTosGenerator);
    void StartNextUdpPing(int i);
    void WriteResults();

protected:
    Ptr<BasicSimulation> m_basicSimulation;
    int64_t m_simulation_end_time_ns;
    Ptr<Topology> m_topology = nullptr;
    Ptr<UdpSocketGenerator> m_udpSocketGenerator;
    Ptr<IpTosGenerator> m_ipTosGenerator;
    bool m_enabled;

    NodeContainer m_nodes;
    std::vector<UdpPingInfo> m_schedule;
    std::vector<ApplicationContainer> m_apps;
    bool m_enable_distributed;
    std::string m_udp_pings_csv_filename;
    std::string m_udp_pings_txt_filename;
};

}

#endif /* UDP_PING_SCHEDULER_H */
