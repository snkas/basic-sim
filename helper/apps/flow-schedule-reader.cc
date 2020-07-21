#include "flow-schedule-reader.h"

namespace ns3 {

FlowScheduleEntry::FlowScheduleEntry(
        int64_t flow_id,
        int64_t from_node_id,
        int64_t to_node_id,
        int64_t size_byte,
        int64_t start_time_ns,
        std::string additional_parameters,
        std::string metadata
) {
    m_flow_id = flow_id;
    m_from_node_id = from_node_id;
    m_to_node_id = to_node_id;
    m_size_byte = size_byte;
    m_start_time_ns = start_time_ns;
    m_additional_parameters = additional_parameters;
    m_metadata = metadata;
}

int64_t FlowScheduleEntry::GetFlowId() {
    return m_flow_id;
}

int64_t FlowScheduleEntry::GetFromNodeId() {
    return m_from_node_id;
}

int64_t FlowScheduleEntry::GetToNodeId() {
    return m_to_node_id;
}

int64_t FlowScheduleEntry::GetSizeByte() {
    return m_size_byte;
}

int64_t FlowScheduleEntry::GetStartTimeNs() {
    return m_start_time_ns;
}

std::string FlowScheduleEntry::GetAdditionalParameters() {
    return m_additional_parameters;
}

std::string FlowScheduleEntry::GetMetadata() {
    return m_metadata;
}

/**
 * Read in the flow schedule.
 *
 * @param filename                  File name of the schedule.csv
 * @param topology                  Topology
 * @param simulation_end_time_ns    Simulation end time (ns) : all flows must start less than this value
*/
std::vector<FlowScheduleEntry> read_flow_schedule(const std::string& filename, Ptr<Topology> topology, const int64_t simulation_end_time_ns) {

    // Schedule to put in the data
    std::vector<FlowScheduleEntry> schedule;

    // Check that the file exists
    if (!file_exists(filename)) {
        throw std::runtime_error(format_string("File %s does not exist.", filename.c_str()));
    }

    // Open file
    std::string line;
    std::ifstream schedule_file(filename);
    if (schedule_file) {

        // Go over each line
        size_t line_counter = 0;
        int64_t prev_start_time_ns = 0;
        while (getline(schedule_file, line)) {

            // Split on ,
            std::vector<std::string> comma_split = split_string(line, ",", 7);

            // Fill entry
            int64_t flow_id = parse_positive_int64(comma_split[0]);
            if (flow_id != (int64_t) line_counter) {
                throw std::invalid_argument(format_string("Flow ID is not ascending by one each line (violation: %" PRId64 ")\n", flow_id));
            }
            int64_t from_node_id = parse_positive_int64(comma_split[1]);
            int64_t to_node_id = parse_positive_int64(comma_split[2]);
            int64_t size_byte = parse_positive_int64(comma_split[3]);
            int64_t start_time_ns = parse_positive_int64(comma_split[4]);
            std::string additional_parameters = comma_split[5];
            std::string metadata = comma_split[6];

            // Must be weakly ascending start time
            if (prev_start_time_ns > start_time_ns) {
                throw std::invalid_argument(format_string("Start time is not weakly ascending (on line with flow ID: %" PRId64 ", violation: %" PRId64 ")\n", flow_id, start_time_ns));
            }
            prev_start_time_ns = start_time_ns;

            // Check node IDs
            if (from_node_id == to_node_id) {
                throw std::invalid_argument(format_string("Flow to itself at node ID: %" PRId64 ".", to_node_id));
            }

            // Check endpoint validity
            if (!topology->IsValidEndpoint(from_node_id)) {
                throw std::invalid_argument(format_string("Invalid from-endpoint for a schedule entry based on topology: %d", from_node_id));
            }
            if (!topology->IsValidEndpoint(to_node_id)) {
                throw std::invalid_argument(format_string("Invalid to-endpoint for a schedule entry based on topology: %d", to_node_id));
            }

            // Check start time
            if (start_time_ns >= simulation_end_time_ns) {
                throw std::invalid_argument(format_string(
                        "Flow %" PRId64 " has invalid start time %" PRId64 " >= %" PRId64 ".",
                        flow_id, start_time_ns, simulation_end_time_ns
                ));
            }

            // Put into schedule
            schedule.push_back(FlowScheduleEntry(flow_id, from_node_id, to_node_id, size_byte, start_time_ns, additional_parameters, metadata));

            // Next line
            line_counter++;

        }

        // Close file
        schedule_file.close();

    } else {
        throw std::runtime_error(format_string("File %s could not be read.", filename.c_str()));
    }

    return schedule;

}

}
