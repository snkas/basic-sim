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

#ifndef UDP_PING_SCHEDULE_READER_H
#define UDP_PING_SCHEDULE_READER_H

#include <fstream>
#include <cinttypes>
#include "ns3/exp-util.h"
#include "ns3/topology.h"

namespace ns3 {

class UdpPingInfo
{
public:
    UdpPingInfo(
            int64_t udp_ping_id,
            int64_t from_node_id,
            int64_t to_node_id,
            int64_t interval_ns,
            int64_t start_time_ns,
            int64_t duration_ns,
            int64_t wait_afterwards_ns,
            std::string additional_parameters,
            std::string metadata
    );
    int64_t GetUdpPingId() const;
    int64_t GetFromNodeId();
    int64_t GetToNodeId();
    int64_t GetIntervalNs();
    int64_t GetStartTimeNs();
    int64_t GetDurationNs();
    int64_t GetWaitAfterwardsNs();
    std::string GetAdditionalParameters();
    std::string GetMetadata();
private:
    int64_t m_udp_ping_id;
    int64_t m_from_node_id;
    int64_t m_to_node_id;
    int64_t m_interval_ns;
    int64_t m_start_time_ns;
    int64_t m_duration_ns;
    int64_t m_wait_afterwards_ns;
    std::string m_additional_parameters;
    std::string m_metadata;
};

std::vector<UdpPingInfo> read_udp_ping_schedule(
        const std::string& filename,
        Ptr<Topology> topology,
        const int64_t simulation_end_time_ns
);

}

#endif //UDP_PING_SCHEDULE_READER_H
