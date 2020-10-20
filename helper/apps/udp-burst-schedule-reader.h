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

#ifndef UDP_BURST_SCHEDULE_READER_H
#define UDP_BURST_SCHEDULE_READER_H

#include <fstream>
#include <cinttypes>
#include "ns3/exp-util.h"
#include "ns3/topology.h"
#include "ns3/udp-burst-info.h"

namespace ns3 {

std::vector<UdpBurstInfo> read_udp_burst_schedule(
        const std::string& filename,
        Ptr<Topology> topology,
        const int64_t simulation_end_time_ns
);

}

#endif //UDP_BURST_SCHEDULE_READER_H
