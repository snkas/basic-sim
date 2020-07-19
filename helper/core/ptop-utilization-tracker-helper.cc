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
        std::cout << "SETUP POINT-TO-POINT UTILIZATION TRACKING" << std::endl;

        // Save for writing results later after simulation is done
        m_basicSimulation = basicSimulation;
        m_topology = topology;

        // TODO: Read in basic simulation parameters
        m_utilization_interval_ns = 100000000; // 100ms

        // Go over every edge in the topology
        for (int i = 0; i < topology->GetNumUndirectedEdges(); i++) {

            // Edge a -- b
            std::pair<int64_t, int64_t> edge = topology->GetUndirectedEdges()[i];
            std::pair<uint32_t, uint32_t> edge_if_idxs = topology->GetInterfaceIdxsForEdges()[i];

            // One tracker a -> b
            Ptr<PointToPointNetDevice> networkDevice_a_b = topology->GetNodes().Get(edge.first)->GetObject<Ipv4>()->GetNetDevice(edge_if_idxs.first)->GetObject<PointToPointNetDevice>();
            Ptr<PtopUtilizationTracker> tracker_a_b = CreateObject<PtopUtilizationTracker>(networkDevice_a_b, m_utilization_interval_ns);
            m_utilization_trackers.push_back(tracker_a_b);

            // One tracker b -> a
            Ptr<PointToPointNetDevice> networkDevice_b_a = topology->GetNodes().Get(edge.second)->GetObject<Ipv4>()->GetNetDevice(edge_if_idxs.second)->GetObject<PointToPointNetDevice>();
            Ptr<PtopUtilizationTracker> tracker_b_a = CreateObject<PtopUtilizationTracker>(networkDevice_b_a, m_utilization_interval_ns);
            m_utilization_trackers.push_back(tracker_b_a);

        }
        std::cout << "  > Installed utilization tracker on " << m_utilization_trackers.size() << " point-to-point network devices" << std::endl;

        // TODO: Remove old files

        std::cout << std::endl;

    }

    void PtopUtilizationTrackerHelper::WriteResults() {

        // Open CSV file
        FILE* file_utilization_csv = fopen((m_basicSimulation->GetLogsDir() + "/utilization.csv").c_str(), "w+");
        FILE* file_utilization_compressed_csv = fopen((m_basicSimulation->GetLogsDir() + "/utilization_compressed.csv").c_str(), "w+");
        FILE* file_utilization_compressed_txt = fopen((m_basicSimulation->GetLogsDir() + "/utilization_compressed.txt").c_str(), "w+");
        FILE* file_utilization_summary_txt = fopen((m_basicSimulation->GetLogsDir() + "/utilization_summary.txt").c_str(), "w+");

        // Print headers
        fprintf(file_utilization_compressed_txt, "From     To       Interval start (ms)   Interval end (ms)     Utilization\n");
        fprintf(file_utilization_summary_txt, "From     To       Utilization\n");

        // Go over every tracker
        for (size_t i = 0; i < m_utilization_trackers.size(); i++) {

            // Tracker
            Ptr<PtopUtilizationTracker> tracker = m_utilization_trackers[i];

            // Retrieve the corresponding directed edge
            std::pair<int64_t, int64_t> directed_edge = m_topology->GetUndirectedEdges()[(int) std::floor((double) i / 2.0)];
            if (i % 2 == 1) {
                directed_edge = std::make_pair(directed_edge.second, directed_edge.first);
            }

            // Go over every utilization interval
            const std::vector<std::tuple<int64_t, int64_t, int64_t>> intervals = tracker->FinalizeUtilization();
            int64_t interval_left_side_ns = 0;
            int64_t utilization_busy_sum_ns = 0;
            for (size_t j = 0; j < intervals.size(); j++) {
                utilization_busy_sum_ns += std::get<2>(intervals[j]);

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
                    double util_j = std::get<2>(intervals[j]) / (std::get<1>(intervals[j]) - std::get<0>(intervals[j]));
                    double util_j_next = std::get<2>(intervals[j + 1]) / (std::get<1>(intervals[j + 1]) - std::get<0>(intervals[j + 1]));
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
                            std::get<2>(intervals[j])
                    );

                    // Write nicely formatted to the TXT file
                    fprintf(file_utilization_compressed_txt,
                            "%-8d %-8d %-21.2f %-21.2f %.2f%%\n",
                            (int) directed_edge.first,
                            (int) directed_edge.second,
                            interval_left_side_ns / 1000000.0,
                            std::get<1>(intervals[j]) / 1000000.0,
                            ((double) std::get<2>(intervals[j])) / ((double) m_utilization_interval_ns) * 100.0
                    );

                    interval_left_side_ns = std::get<1>(intervals[j]);

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

        // Close CSV file
        fclose(file_utilization_csv);
        fclose(file_utilization_compressed_csv);
        fclose(file_utilization_compressed_txt);
        fclose(file_utilization_summary_txt);

    }

}
