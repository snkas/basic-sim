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

#include "udp-ping-schedule-reader.h"

namespace ns3 {

UdpPingInfo::UdpPingInfo(
        int64_t udp_ping_id,
        int64_t from_node_id,
        int64_t to_node_id,
        int64_t interval_ns,
        int64_t start_time_ns,
        int64_t duration_ns,
        int64_t wait_afterwards_ns,
        std::string additional_parameters,
        std::string metadata
) {
    m_udp_ping_id = udp_ping_id;
    m_from_node_id = from_node_id;
    m_to_node_id = to_node_id;
    m_interval_ns = interval_ns;
    m_start_time_ns = start_time_ns;
    m_duration_ns = duration_ns;
    m_wait_afterwards_ns = wait_afterwards_ns;
    m_additional_parameters = additional_parameters;
    m_metadata = metadata;
}

int64_t UdpPingInfo::GetUdpPingId() const {
    return m_udp_ping_id;
}

int64_t UdpPingInfo::GetFromNodeId() {
    return m_from_node_id;
}

int64_t UdpPingInfo::GetToNodeId() {
    return m_to_node_id;
}

int64_t UdpPingInfo::GetIntervalNs() {
    return m_interval_ns;
}

int64_t UdpPingInfo::GetStartTimeNs() {
    return m_start_time_ns;
}

int64_t UdpPingInfo::GetDurationNs() {
    return m_duration_ns;
}

int64_t UdpPingInfo::GetWaitAfterwardsNs() {
    return m_wait_afterwards_ns;
}

std::string UdpPingInfo::GetAdditionalParameters() {
    return m_additional_parameters;
}

std::string UdpPingInfo::GetMetadata() {
    return m_metadata;
}

/**
 * Read in the UDP ping schedule.
 *
 * @param filename                  File name of the udp_ping_schedule.csv
 * @param topology                  Topology
 * @param simulation_end_time_ns    Simulation end time (ns) : all UDP pings must start less than this value
*/
std::vector<UdpPingInfo> read_udp_ping_schedule(const std::string& filename, Ptr<Topology> topology, const int64_t simulation_end_time_ns) {

    // Schedule to put in the data
    std::vector<UdpPingInfo> schedule;

    // Check that the file exists
    if (!file_exists(filename)) {
        throw std::runtime_error(format_string("UDP ping schedule file %s does not exist.", filename.c_str()));
    }

    // Open file
    std::string line;
    std::ifstream schedule_file(filename);
    NS_ABORT_MSG_IF(!schedule_file, format_string("UDP ping schedule file %s could not be opened.", filename.c_str()));

    // Go over each line
    size_t line_counter = 0;
    int64_t prev_start_time_ns = 0;
    while (getline(schedule_file, line)) {

        // Split on ,
        std::vector<std::string> comma_split = split_string(line, ",", 9);

        // Fill entry
        int64_t udp_ping_id = parse_positive_int64(comma_split.at(0));
        if (udp_ping_id != (int64_t) line_counter) {
            throw std::invalid_argument(format_string("UDP ping ID is not ascending by one each line (violation: %" PRId64 ")\n", udp_ping_id));
        }
        int64_t from_node_id = parse_positive_int64(comma_split.at(1));
        int64_t to_node_id = parse_positive_int64(comma_split.at(2));
        int64_t interval_ns = parse_positive_int64(comma_split.at(3));
        int64_t start_time_ns = parse_positive_int64(comma_split.at(4));
        int64_t duration_ns = parse_positive_int64(comma_split.at(5));
        int64_t wait_afterwards_ns = parse_positive_int64(comma_split.at(6));
        std::string additional_parameters = comma_split.at(7);
        std::string metadata = comma_split.at(8);

        // Zero ping interval
        if (interval_ns == 0) {
            throw std::invalid_argument("UDP ping interval is zero.");
        }

        // Must be weakly ascending start time
        if (prev_start_time_ns > start_time_ns) {
            throw std::invalid_argument(format_string("Start time is not weakly ascending (on line with UDP ping ID: %" PRId64 ", violation: %" PRId64 ")", udp_ping_id, start_time_ns));
        }
        prev_start_time_ns = start_time_ns;

        // Check node IDs
        if (from_node_id == to_node_id) {
            throw std::invalid_argument(format_string("UDP ping to itself at node ID: %" PRId64 ".", to_node_id));
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
                    "UDP ping %" PRId64 " has invalid start time %" PRId64 " >= %" PRId64 ".",
                    udp_ping_id, start_time_ns, simulation_end_time_ns
            ));
        }

        // Put into schedule
        schedule.push_back(UdpPingInfo(udp_ping_id, from_node_id, to_node_id, interval_ns, start_time_ns, duration_ns, wait_afterwards_ns, additional_parameters, metadata));

        // Next line
        line_counter++;

    }

    // Close file
    schedule_file.close();

    return schedule;

}

}
