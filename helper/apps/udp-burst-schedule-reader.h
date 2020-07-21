#ifndef UDP_BURST_SCHEDULE_READER_H
#define UDP_BURST_SCHEDULE_READER_H

#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <cstring>
#include <fstream>
#include <cinttypes>
#include <algorithm>
#include <regex>
#include "ns3/exp-util.h"
#include "ns3/topology.h"

namespace ns3 {

class UdpBurstScheduleEntry
{
public:
    UdpBurstScheduleEntry(
            int64_t udp_burst_id,
            int64_t from_node_id,
            int64_t to_node_id,
            int64_t rate_byte_per_sec,
            int64_t start_time_ns,
            int64_t duration_ns,
            std::string additional_parameters,
            std::string metadata
    );
    int64_t GetUdpBurstId();
    int64_t GetFromNodeId();
    int64_t GetToNodeId();
    int64_t GetRateBytePerSec();
    int64_t GetStartTimeNs();
    int64_t GetDurationNs();
    std::string GetAdditionalParameters();
    std::string GetMetadata();
private:
    int64_t m_udp_burst_id;
    int64_t m_from_node_id;
    int64_t m_to_node_id;
    int64_t m_rate_byte_per_sec;
    int64_t m_start_time_ns;
    int64_t m_duration_ns;
    std::string m_additional_parameters;
    std::string m_metadata;
};

std::vector<UdpBurstScheduleEntry> read_udp_burst_schedule(
        const std::string& filename,
        Ptr<Topology> topology,
        const int64_t simulation_end_time_ns
);

}

#endif //UDP_BURST_SCHEDULE_READER_H
