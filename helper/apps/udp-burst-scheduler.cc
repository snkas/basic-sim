/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2019 ETH Zurich
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
 * Originally based on the FlowScheduler (which is authored by Simon, Hussain)
 */

#include "udp-burst-scheduler.h"

namespace ns3 {

    UdpBurstScheduler::UdpBurstScheduler(Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology) {
        printf("UDP BURST SCHEDULER\n");

        m_basicSimulation = basicSimulation;
        m_topology = topology;

        // Check if it is enabled explicitly
        m_enabled = parse_boolean(m_basicSimulation->GetConfigParamOrDefault("enable_udp_burst_scheduler", "false"));
        if (!m_enabled) {
            std::cout << "  > Not enabled explicitly, so disabled" << std::endl;

        } else {
            std::cout << "  > UDP burst scheduler is enabled" << std::endl;

            // Properties we will use often
            m_nodes = m_topology->GetNodes();
            m_simulation_end_time_ns = m_basicSimulation->GetSimulationEndTimeNs();
            m_enable_logging_for_ids = parse_set_positive_int64(m_basicSimulation->GetConfigParamOrDefault("udp_burst_enable_logging_for_ids", "set()"));

            // Distributed run information
            m_system_id = m_basicSimulation->GetSystemId();
            m_enable_distributed = m_basicSimulation->IsDistributedEnabled();
            m_distributed_node_system_id_assignment = m_basicSimulation->GetDistributedNodeSystemIdAssignment();

            // Read schedule
            std::vector<UdpBurstInfo> complete_schedule = read_udp_burst_schedule(
                    m_basicSimulation->GetRunDir() + "/" + m_basicSimulation->GetConfigParamOrFail("udp_burst_schedule_filename"),
                    m_topology,
                    m_simulation_end_time_ns
            );

            // Filter the schedule to only have UDP bursts from a node this system controls
            if (m_enable_distributed) {
                std::vector<UdpBurstInfo> filtered_schedule;
                for (UdpBurstInfo &entry : complete_schedule) {
                    if (m_distributed_node_system_id_assignment[entry.GetFromNodeId()] == m_system_id) {
                        filtered_schedule.push_back(entry);
                    }
                }
                m_schedule = filtered_schedule;
            } else {
                m_schedule = complete_schedule;
            }

            // Schedule read
            printf("  > Read schedule (total UDP bursts: %lu)\n", m_schedule.size());
            m_basicSimulation->RegisterTimestamp("Read UDP burst schedule");

            // Determine filenames
            if (m_enable_distributed) {
                m_udp_bursts_csv_filename = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_udp_bursts.csv";
                m_udp_bursts_txt_filename = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_system_id) + "_udp_bursts.txt";
            } else {
                m_udp_bursts_csv_filename = m_basicSimulation->GetLogsDir() + "/udp_bursts.csv";
                m_udp_bursts_txt_filename = m_basicSimulation->GetLogsDir() + "/udp_bursts.txt";
            }

            // Remove files if they are there
            remove_file_if_exists(m_udp_bursts_csv_filename);
            remove_file_if_exists(m_udp_bursts_txt_filename);
            printf("  > Removed previous UDP burst log files if present\n");
            m_basicSimulation->RegisterTimestamp("Remove previous UDP burst log files");

            // Install sink on each endpoint node
            std::cout << "  > Setting up UDP burst applications on all endpoint nodes" << std::endl;
            for (int64_t endpoint : m_topology->GetEndpoints()) {
                if (!m_enable_distributed || m_distributed_node_system_id_assignment[endpoint] == m_system_id) {

                    // Setup the application
                    UdpBurstHelper udpBurstHelper(1026);
                    ApplicationContainer app = udpBurstHelper.Install(m_nodes.Get(endpoint));
                    app.Start(Seconds(0.0));

                    // Plan all burst originating from there
                    Ptr<UdpBurstApplication> udpBurstApp = app.Get(0)->GetObject<UdpBurstApplication>();
                    for (UdpBurstInfo entry : m_schedule) {
                        udpBurstApp->RegisterBurst(
                                InetSocketAddress(m_nodes.Get(entry.GetToNodeId())->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1026),
                                entry
                        );
                    }

                }
            }
            m_basicSimulation->RegisterTimestamp("Setup UDP burst applications");

        }

        std::cout << std::endl;
    }

    void UdpBurstScheduler::WriteResults() {
        std::cout << "STORE UDP BURST RESULTS" << std::endl;

        // Check if it is enabled explicitly
        if (!m_enabled) {
            std::cout << "  > Not enabled, so no UDP burst results are written" << std::endl;

        } else {

            // Open files
            std::cout << "  > Opening UDP burst log files:" << std::endl;
            FILE* file_csv = fopen(m_udp_bursts_csv_filename.c_str(), "w+");
            std::cout << "    >> Opened: " << m_udp_bursts_csv_filename << std::endl;
            FILE* file_txt = fopen(m_udp_bursts_txt_filename.c_str(), "w+");
            std::cout << "    >> Opened: " << m_udp_bursts_txt_filename << std::endl;

            // Header
            std::cout << "  > Writing udp_bursts.txt header" << std::endl;
            fprintf(
                    file_txt, "%-16s%-10s%-10s%-16s%-18s%-18s%-16s%s\n",
                    "UDP burst ID", "Source", "Target", "Rate (Mbit/s)", "Start time (ns)",
                    "Duration", "Arrival", "Metadata"
            );

//            // Go over the schedule, write each flow's result
//            std::cout << "  > Writing log files line-by-line" << std::endl;
//            std::vector<ApplicationContainer>::iterator it = m_apps.begin();
//            for (FlowScheduleEntry& entry : m_schedule) {
//
//                // Retrieve statistics
//                ApplicationContainer app = *it;
//                Ptr<FlowSendApplication> flowSendApp = ((it->Get(0))->GetObject<FlowSendApplication>());
//                bool is_completed = flowSendApp->IsCompleted();
//                bool is_conn_failed = flowSendApp->IsConnFailed();
//                bool is_closed_err = flowSendApp->IsClosedByError();
//                bool is_closed_normal = flowSendApp->IsClosedNormally();
//                int64_t sent_byte = flowSendApp->GetAckedBytes();
//                int64_t fct_ns;
//                if (is_completed) {
//                    fct_ns = flowSendApp->GetCompletionTimeNs() - entry.GetStartTimeNs();
//                } else {
//                    fct_ns = m_simulation_end_time_ns - entry.GetStartTimeNs();
//                }
//                std::string finished_state;
//                if (is_completed) {
//                    finished_state = "YES";
//                } else if (is_conn_failed) {
//                    finished_state = "NO_CONN_FAIL";
//                } else if (is_closed_normal) {
//                    finished_state = "NO_BAD_CLOSE";
//                } else if (is_closed_err) {
//                    finished_state = "NO_ERR_CLOSE";
//                } else {
//                    finished_state = "NO_ONGOING";
//                }
//
//                // Write plain to the csv
//                fprintf(
//                        file_csv, "%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%s,%s\n",
//                        entry.GetFlowId(), entry.GetFromNodeId(), entry.GetToNodeId(), entry.GetSizeByte(), entry.GetStartTimeNs(),
//                        entry.GetStartTimeNs() + fct_ns, fct_ns, sent_byte, finished_state.c_str(), entry.GetMetadata().c_str()
//                );
//
//                // Write nicely formatted to the text
//                char str_size_megabit[100];
//                sprintf(str_size_megabit, "%.2f Mbit", byte_to_megabit(entry.GetSizeByte()));
//                char str_duration_ms[100];
//                sprintf(str_duration_ms, "%.2f ms", nanosec_to_millisec(fct_ns));
//                char str_sent_megabit[100];
//                sprintf(str_sent_megabit, "%.2f Mbit", byte_to_megabit(sent_byte));
//                char str_progress_perc[100];
//                sprintf(str_progress_perc, "%.1f%%", ((double) sent_byte) / ((double) entry.GetSizeByte()) * 100.0);
//                char str_avg_rate_megabit_per_s[100];
//                sprintf(str_avg_rate_megabit_per_s, "%.1f Mbit/s", byte_to_megabit(sent_byte) / nanosec_to_sec(fct_ns));
//                fprintf(
//                        file_txt, "%-12" PRId64 "%-10" PRId64 "%-10" PRId64 "%-16s%-18" PRId64 "%-18" PRId64 "%-16s%-16s%-13s%-16s%-14s%s\n",
//                        entry.GetFlowId(), entry.GetFromNodeId(), entry.GetToNodeId(), str_size_megabit, entry.GetStartTimeNs(),
//                        entry.GetStartTimeNs() + fct_ns, str_duration_ms, str_sent_megabit, str_progress_perc, str_avg_rate_megabit_per_s,
//                        finished_state.c_str(), entry.GetMetadata().c_str()
//                );
//
//                // Move on iterator
//                it++;
//
//            }

            // Close files
            std::cout << "  > Closing UDP burst log files:" << std::endl;
            fclose(file_csv);
            std::cout << "    >> Closed: " << m_udp_bursts_csv_filename << std::endl;
            fclose(file_txt);
            std::cout << "    >> Closed: " << m_udp_bursts_txt_filename << std::endl;

            // Register completion
            std::cout << "  > UDP burst log files have been written" << std::endl;
            m_basicSimulation->RegisterTimestamp("Write UDP burst log files");

        }

        std::cout << std::endl;
    }

}
