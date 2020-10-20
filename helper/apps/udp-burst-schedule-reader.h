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
