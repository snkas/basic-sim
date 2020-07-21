#include "udp-burst-schedule-reader.h"

namespace ns3 {

UdpBurstScheduleEntry::UdpBurstScheduleEntry(
        int64_t udp_burst_id,
        int64_t from_node_id,
        int64_t to_node_id,
        int64_t rate_byte_per_sec,
        int64_t start_time_ns,
        int64_t duration_ns,
        std::string additional_parameters,
        std::string metadata
) {
    m_udp_burst_id = udp_burst_id;
    m_from_node_id = from_node_id;
    m_to_node_id = to_node_id;
    m_rate_byte_per_sec = rate_byte_per_sec;
    m_start_time_ns = start_time_ns;
    m_duration_ns = duration_ns;
    m_additional_parameters = additional_parameters;
    m_metadata = metadata;
}

int64_t UdpBurstScheduleEntry::GetUdpBurstId() {
    return m_udp_burst_id;
}

int64_t UdpBurstScheduleEntry::GetFromNodeId() {
    return m_from_node_id;
}

int64_t UdpBurstScheduleEntry::GetToNodeId() {
    return m_to_node_id;
}

int64_t UdpBurstScheduleEntry::GetRateBytePerSec() {
    return m_rate_byte_per_sec;
}

int64_t UdpBurstScheduleEntry::GetStartTimeNs() {
    return m_start_time_ns;
}

int64_t UdpBurstScheduleEntry::GetDurationNs() {
    return m_duration_ns;
}

std::string UdpBurstScheduleEntry::GetAdditionalParameters() {
    return m_additional_parameters;
}

std::string UdpBurstScheduleEntry::GetMetadata() {
    return m_metadata;
}

/**
 * Read in the UDP burst schedule.
 *
 * @param filename                  File name of the udp_burst_schedule.csv
 * @param topology                  Topology
 * @param simulation_end_time_ns    Simulation end time (ns) : all UDP bursts must start less than this value
*/
std::vector<UdpBurstScheduleEntry> read_udp_burst_schedule(const std::string& filename, Ptr<Topology> topology, const int64_t simulation_end_time_ns) {

    // Schedule to put in the data
    std::vector<UdpBurstScheduleEntry> schedule;

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
            std::vector<std::string> comma_split = split_string(line, ",", 8);

            // Fill entry
            int64_t udp_burst_id = parse_positive_int64(comma_split[0]);
            if (udp_burst_id != (int64_t) line_counter) {
                throw std::invalid_argument(format_string("UDP burst ID is not ascending by one each line (violation: %" PRId64 ")\n", udp_burst_id));
            }
            int64_t from_node_id = parse_positive_int64(comma_split[1]);
            int64_t to_node_id = parse_positive_int64(comma_split[2]);
            int64_t rate_byte_per_sec = parse_positive_int64(comma_split[3]);
            int64_t start_time_ns = parse_positive_int64(comma_split[4]);
            int64_t duration_ns = parse_positive_int64(comma_split[5]);
            std::string additional_parameters = comma_split[6];
            std::string metadata = comma_split[7];

            // Must be weakly ascending start time
            if (prev_start_time_ns > start_time_ns) {
                throw std::invalid_argument(format_string("Start time is not weakly ascending (on line with flow ID: %" PRId64 ", violation: %" PRId64 ")\n", udp_burst_id, start_time_ns));
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
                        udp_burst_id, start_time_ns, simulation_end_time_ns
                ));
            }

            // Put into schedule
            schedule.push_back(UdpBurstScheduleEntry(udp_burst_id, from_node_id, to_node_id, rate_byte_per_sec, start_time_ns, duration_ns, additional_parameters, metadata));

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
