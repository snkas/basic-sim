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

#include "ptop-utilization-tracker-helper.h"

namespace ns3 {

    PtopUtilizationTrackerHelper::PtopUtilizationTrackerHelper(Ptr <BasicSimulation> basicSimulation, Ptr <TopologyPtop> topology) {
        std::cout << "POINT-TO-POINT UTILIZATION TRACKING" << std::endl;

        // Save for writing results later after simulation is done
        m_basicSimulation = basicSimulation;
        m_topology = topology;

        // Check if it is enabled explicitly
        m_enabled = parse_boolean(m_basicSimulation->GetConfigParamOrDefault("enable_link_utilization_tracking", "false"));
        if (!m_enabled) {
            std::cout << "  > Not enabled explicitly, so disabled" << std::endl;

        } else {
            std::cout << "  > Enabled for all point-to-point links in the topology" << std::endl;

            // Distributed information
            m_system_id = m_basicSimulation->GetSystemId();
            m_enable_distributed = m_basicSimulation->IsDistributedEnabled();
            m_distributed_node_system_id_assignment = m_basicSimulation->GetDistributedNodeSystemIdAssignment();

            // Read in parameters
            m_utilization_interval_ns = parse_geq_one_int64(m_basicSimulation->GetConfigParamOrFail("link_utilization_tracking_interval_ns"));
            std::cout << "  > Utilization aggregation interval... " << m_utilization_interval_ns << " ns" << std::endl;
            // TODO: Add additional parameter to specifically select links

            // Go over every edge in the topology
            for (int i = 0; i < m_topology->GetNumUndirectedEdges(); i++) {

                // Edge a -- b
                std::pair<int64_t, int64_t> edge = m_topology->GetUndirectedEdges()[i];
                std::pair<uint32_t, uint32_t> edge_if_idxs = m_topology->GetInterfaceIdxsForEdges()[i];

                // One tracker a -> b
                if (!m_enable_distributed || m_distributed_node_system_id_assignment[edge.first] == m_system_id) {
                    Ptr<PointToPointNetDevice> networkDevice_a_b = m_topology->GetNodes().Get(edge.first)->GetObject<Ipv4>()->GetNetDevice(edge_if_idxs.first)->GetObject<PointToPointNetDevice>();
                    Ptr<PtopUtilizationTracker> tracker_a_b = CreateObject<PtopUtilizationTracker>(networkDevice_a_b, m_utilization_interval_ns);
                    m_utilization_trackers.push_back(tracker_a_b);
                    m_installed_edges.push_back(edge);
                }

                // One tracker b -> a
                if (!m_enable_distributed || m_distributed_node_system_id_assignment[edge.second] == m_system_id) {
                    Ptr<PointToPointNetDevice> networkDevice_b_a = m_topology->GetNodes().Get(edge.second)->GetObject<Ipv4>()->GetNetDevice(edge_if_idxs.second)->GetObject<PointToPointNetDevice>();
                    Ptr<PtopUtilizationTracker> tracker_b_a = CreateObject<PtopUtilizationTracker>(networkDevice_b_a, m_utilization_interval_ns);
                    m_utilization_trackers.push_back(tracker_b_a);
                    m_installed_edges.push_back(std::make_pair(edge.second, edge.first));
                }

            }
            std::cout << "  > Tracking utilization on " << m_utilization_trackers.size() << " point-to-point network devices" << std::endl;
            m_basicSimulation->RegisterTimestamp("Install utilization trackers");

            // Determine filenames
            if (m_enable_distributed) {
                m_filename_utilization_csv = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_utilization.csv";
                m_filename_utilization_compressed_csv = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_utilization_compressed.csv";
                m_filename_utilization_compressed_txt = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_utilization_compressed.txt";
                m_filename_utilization_summary_txt = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_utilization_summary.txt";
            } else {
                m_filename_utilization_csv = m_basicSimulation->GetLogsDir() + "/utilization.csv";
                m_filename_utilization_compressed_csv = m_basicSimulation->GetLogsDir() + "/utilization_compressed.csv";
                m_filename_utilization_compressed_txt = m_basicSimulation->GetLogsDir() + "/utilization_compressed.txt";
                m_filename_utilization_summary_txt = m_basicSimulation->GetLogsDir() + "/utilization_summary.txt";
            }

            // Remove files if they are there
            remove_file_if_exists(m_filename_utilization_csv);
            remove_file_if_exists(m_filename_utilization_compressed_csv);
            remove_file_if_exists(m_filename_utilization_compressed_txt);
            remove_file_if_exists(m_filename_utilization_summary_txt);

            printf("  > Removed previous utilization tracking files if present\n");
            m_basicSimulation->RegisterTimestamp("Remove previous utilization tracking log files");

        }

        std::cout << std::endl;
    }

    void PtopUtilizationTrackerHelper::WriteResults() {
        std::cout << "UTILIZATION TRACKING RESULTS" << std::endl;

        // Check if it is enabled explicitly
        if (!m_enabled) {
            std::cout << "  > Not enabled, so no results are written" << std::endl;

        } else {

            // Open CSV file
            std::cout << "  > Opening utilization log files:" << std::endl;
            FILE* file_utilization_csv = fopen(m_filename_utilization_csv.c_str(), "w+");
            std::cout << "    >> Opened: " << m_filename_utilization_csv << std::endl;
            FILE* file_utilization_compressed_csv = fopen(m_filename_utilization_compressed_csv.c_str(), "w+");
            std::cout << "    >> Opened: " << m_filename_utilization_compressed_csv << std::endl;
            FILE* file_utilization_compressed_txt = fopen(m_filename_utilization_compressed_txt.c_str(), "w+");
            std::cout << "    >> Opened: " << m_filename_utilization_compressed_txt << std::endl;
            FILE* file_utilization_summary_txt = fopen(m_filename_utilization_summary_txt.c_str(), "w+");
            std::cout << "    >> Opened: " << m_filename_utilization_summary_txt << std::endl;

            // Print headers
            std::cout << "  > Writing utilization compressed TXT header" << std::endl;
            fprintf(file_utilization_compressed_txt, "From     To       Interval start (ms)   Interval end (ms)     Utilization\n");
            std::cout << "  > Writing utilization summary TXT header" << std::endl;
            fprintf(file_utilization_summary_txt, "From     To       Utilization\n");

            // Go over every tracker
            std::cout << "  > Writing utilization log files" << std::endl;
            for (size_t i = 0; i < m_utilization_trackers.size(); i++) {

                // Tracker
                Ptr<PtopUtilizationTracker> tracker = m_utilization_trackers[i];

                // Retrieve the corresponding directed edge
                std::pair<int64_t, int64_t> directed_edge = m_installed_edges[i];

                // Go over every utilization interval
                const std::vector<std::tuple<int64_t, int64_t, int64_t>> intervals = tracker->FinalizeUtilization();
                int64_t interval_left_side_ns = 0;
                int64_t utilization_busy_sum_ns = 0;
                int64_t running_busy_sum_ns = 0;
                for (size_t j = 0; j < intervals.size(); j++) {
                    utilization_busy_sum_ns += std::get<2>(intervals[j]);
                    running_busy_sum_ns += std::get<2>(intervals[j]);

                    // Write plain to the uncompressed CSV file:
                    // <from>,<to>,<interval start (ns)>,<interval end (ns)>,<amount of busy in this interval (ns)>
                    fprintf(file_utilization_csv,
                            "%d,%d,%" PRId64 ",%" PRId64 ",%" PRId64 "\n",
                            (int) directed_edge.first,
                            (int) directed_edge.second,
                            std::get<0>(intervals[j]),
                            std::get<1>(intervals[j]),
                            std::get<2>(intervals[j])
                    );

                    // Compressed version:
                    // Only write if it is the last one, or if the utilization is sufficiently different from the next
                    bool print_compressed_line = (j == intervals.size() - 1);
                    if (!print_compressed_line) {
                        double util_j = ((double) std::get<2>(intervals[j])) / (double) (std::get<1>(intervals[j]) - std::get<0>(intervals[j]));
                        double util_j_next = ((double) std::get<2>(intervals[j + 1])) / (double) (std::get<1>(intervals[j + 1]) - std::get<0>(intervals[j + 1]));
                        if (std::abs(util_j - util_j_next) >= UTILIZATION_TRACKER_COMPRESSION_APPROXIMATELY_NOT_EQUAL) { // Approximately not equal
                            print_compressed_line = true;
                        }
                    }
                    if (print_compressed_line) {

                        // Write plain to the compressed CSV file:
                        // <from>,<to>,<interval start (ns)>,<interval end (ns)>,<amount of busy in this interval (ns)>
                        fprintf(file_utilization_compressed_csv,
                                "%d,%d,%" PRId64 ",%" PRId64 ",%" PRId64 "\n",
                                (int) directed_edge.first,
                                (int) directed_edge.second,
                                interval_left_side_ns,
                                std::get<1>(intervals[j]),
                                running_busy_sum_ns
                        );

                        // Write nicely formatted to the TXT file
                        fprintf(file_utilization_compressed_txt,
                                "%-8d %-8d %-21.2f %-21.2f %.2f%%\n",
                                (int) directed_edge.first,
                                (int) directed_edge.second,
                                interval_left_side_ns / 1000000.0,
                                std::get<1>(intervals[j]) / 1000000.0,
                                ((double) running_busy_sum_ns) / ((double) (std::get<1>(intervals[j]) - interval_left_side_ns)) * 100.0
                        );

                        interval_left_side_ns = std::get<1>(intervals[j]);
                        running_busy_sum_ns = 0;

                    }
                }

                // Write nicely formatted to the summary TXT file
                fprintf(file_utilization_summary_txt,
                        "%-8d %-8d %.2f%%\n",
                        (int) directed_edge.first,
                        (int) directed_edge.second,
                        ((double) utilization_busy_sum_ns) / (std::get<1>(intervals[intervals.size() - 1])) * 100.0
                );

            }

            // Close log files
            std::cout << "  > Closing utilization log files:" << std::endl;
            fclose(file_utilization_csv);
            std::cout << "    >> Closed: " << m_filename_utilization_csv << std::endl;
            fclose(file_utilization_compressed_csv);
            std::cout << "    >> Closed: " << m_filename_utilization_compressed_csv << std::endl;
            fclose(file_utilization_compressed_txt);
            std::cout << "    >> Closed: " << m_filename_utilization_compressed_txt << std::endl;
            fclose(file_utilization_summary_txt);
            std::cout << "    >> Closed: " << m_filename_utilization_summary_txt << std::endl;

            // Register completion
            std::cout << "  > Utilization log files have been written" << std::endl;
            m_basicSimulation->RegisterTimestamp("Write utilization log files");

        }

        std::cout << std::endl;
    }

}
