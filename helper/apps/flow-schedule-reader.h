#ifndef FLOW_SCHEDULE_READER_H
#define FLOW_SCHEDULE_READER_H

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

class FlowScheduleEntry
{
public:
    FlowScheduleEntry(
            int64_t flow_id,
            int64_t from_node_id,
            int64_t to_node_id,
            int64_t size_byte,
            int64_t start_time_ns,
            std::string additional_parameters,
            std::string metadata
    );
    int64_t GetFlowId();
    int64_t GetFromNodeId();
    int64_t GetToNodeId();
    int64_t GetSizeByte();
    int64_t GetStartTimeNs();
    std::string GetAdditionalParameters();
    std::string GetMetadata();
private:
    int64_t m_flow_id;
    int64_t m_from_node_id;
    int64_t m_to_node_id;
    int64_t m_size_byte;
    int64_t m_start_time_ns;
    std::string m_additional_parameters;
    std::string m_metadata;
};

std::vector<FlowScheduleEntry> read_flow_schedule(
        const std::string& filename,
        Ptr<Topology> topology,
        const int64_t simulation_end_time_ns
);

}

#endif //FLOW_SCHEDULE_READER_H
