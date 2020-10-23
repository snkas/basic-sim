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
 * Author: Simon
 */

#include "ptop-link-interface-tc-qdisc-queue-tracking.h"

namespace ns3 {

    PtopLinkInterfaceTcQdiscQueueTracking::PtopLinkInterfaceTcQdiscQueueTracking(Ptr <BasicSimulation> basicSimulation, Ptr <TopologyPtop> topology) {
        std::cout << "POINT-TO-POINT LINK INTERFACE TC QDISC QUEUE TRACKING" << std::endl;

        // Save for writing results later after simulation is done
        m_basicSimulation = basicSimulation;
        m_topology = topology;

        // Exit if not enabled
        m_enabled = parse_boolean(m_basicSimulation->GetConfigParamOrDefault("enable_link_interface_tc_qdisc_queue_tracking", "false"));
        if (!m_enabled) {
            std::cout << "  > Not enabled explicitly, so disabled" << std::endl;
            std::cout << std::endl;
            return;
        }

        // Distributed information
        m_enable_distributed = m_basicSimulation->IsDistributedEnabled();

        // Check to enable for which links
        std::string enable_for_links_str = basicSimulation->GetConfigParamOrDefault("link_interface_tc_qdisc_queue_tracking_enable_for_links", "all");
        std::set<std::pair<int64_t, int64_t>> enable_for_links_set;
        if (enable_for_links_str == "all") {
            // For all links
            enable_for_links_set = m_topology->GetLinksSet();
        } else {
            // Only between select links
            enable_for_links_set = parse_set_directed_pair_positive_int64(enable_for_links_str);
        }

        // Enable it for links in the set
        for (std::pair<int64_t, int64_t> p : enable_for_links_set) {
            if (!m_enable_distributed || m_basicSimulation->IsNodeAssignedToThisSystem(p.first)) {
                Ptr<QueueDisc> queueDisc = m_topology->GetNodes().Get(p.first)->GetObject<TrafficControlLayer>()->GetRootQueueDiscOnDevice(m_topology->GetSendingNetDeviceForLink(p));
                if (queueDisc == 0) {
                    throw std::invalid_argument(format_string(
                            "Cannot enable traffic-control qdisc queue tracking on an interface which does not have a qdisc (%d -> %d).",
                            p.first, p.second
                    ));
                }
                Ptr<QdiscQueueTracker> tracker_a_b = CreateObject<QdiscQueueTracker>(queueDisc);
                m_qdisc_queue_trackers.push_back(std::make_pair(p, tracker_a_b));
            }
        }
        std::cout << "  > Tracking traffic-control qdisc queue on " << m_qdisc_queue_trackers.size() << " point-to-point interfaces" << std::endl;
        m_basicSimulation->RegisterTimestamp("Install link (interface traffic-control) qdisc queue trackers");

        // Determine filenames
        if (m_enable_distributed) {
            m_filename_link_interface_tc_qdisc_queue_pkt_csv = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_basicSimulation->GetSystemId()) + "_link_interface_tc_qdisc_queue_pkt.csv";
            m_filename_link_interface_tc_qdisc_queue_byte_csv = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_basicSimulation->GetSystemId()) + "_link_interface_tc_qdisc_queue_byte.csv";
        } else {
            m_filename_link_interface_tc_qdisc_queue_pkt_csv = m_basicSimulation->GetLogsDir() + "/link_interface_tc_qdisc_queue_pkt.csv";
            m_filename_link_interface_tc_qdisc_queue_byte_csv = m_basicSimulation->GetLogsDir() + "/link_interface_tc_qdisc_queue_byte.csv";
        }

        // Remove files if they are there
        remove_file_if_exists(m_filename_link_interface_tc_qdisc_queue_pkt_csv);
        remove_file_if_exists(m_filename_link_interface_tc_qdisc_queue_byte_csv);

        printf("  > Removed previous link (interface traffic-control) qdisc queue tracking files if present\n");
        m_basicSimulation->RegisterTimestamp("Remove previous link (interface traffic-control) qdisc queue tracking log files");

        std::cout << std::endl;
    }

    void PtopLinkInterfaceTcQdiscQueueTracking::WriteResults() {
        std::cout << "POINT-TO-POINT LINK INTERFACE TC QDISC QUEUE TRACKING RESULTS" << std::endl;

        // Exit if not enabled
        if (!m_enabled) {
            std::cout << "  > Not enabled, so no results are written" << std::endl;
            std::cout << std::endl;
            return;
        }

        // Open CSV files
        std::cout << "  > Opening link (interface traffic-control) qdisc queue log files:" << std::endl;
        FILE* file_link_interface_tc_qdisc_queue_pkt_csv = fopen(m_filename_link_interface_tc_qdisc_queue_pkt_csv.c_str(), "w+");
        std::cout << "    >> Opened: " << m_filename_link_interface_tc_qdisc_queue_pkt_csv << std::endl;
        FILE* file_link_interface_tc_qdisc_queue_byte_csv = fopen(m_filename_link_interface_tc_qdisc_queue_byte_csv.c_str(), "w+");
        std::cout << "    >> Opened: " << m_filename_link_interface_tc_qdisc_queue_byte_csv << std::endl;

        // Sort
        struct ascending_by_directed_link
        {
            inline bool operator() (const std::pair<std::pair<int64_t, int64_t>, Ptr<QdiscQueueTracker>>& a, const std::pair<std::pair<int64_t, int64_t>, Ptr<QdiscQueueTracker>>& b)
            {
                return (a.first.first == b.first.first ? a.first.second < b.first.second : a.first.first < b.first.first);
            }
        };
        std::sort(m_qdisc_queue_trackers.begin(), m_qdisc_queue_trackers.end(), ascending_by_directed_link());

        // Go over every tracker
        std::cout << "  > Writing link (interface traffic-control) qdisc queue log files" << std::endl;
        for (size_t i = 0; i < m_qdisc_queue_trackers.size(); i++) {

            // Retrieve the corresponding directed edge
            std::pair<int64_t, int64_t> directed_edge = m_qdisc_queue_trackers.at(i).first;

            // Tracker
            Ptr<QdiscQueueTracker> tracker = m_qdisc_queue_trackers.at(i).second;

            // Queue size in packets
            const std::vector<std::tuple<int64_t, int64_t, int64_t>> log_entries_pkt = tracker->GetIntervalsNumPackets();
            for (size_t j = 0; j < log_entries_pkt.size(); j++) {

                // Write plain to the CSV file:
                // <from>,<to>,<interval start (ns)>,<interval end (ns)>,<number of packets>
                fprintf(file_link_interface_tc_qdisc_queue_pkt_csv,
                        "%d,%d,%" PRId64 ",%" PRId64 ",%" PRId64 "\n",
                        (int) directed_edge.first,
                        (int) directed_edge.second,
                        std::get<0>(log_entries_pkt.at(j)),
                        std::get<1>(log_entries_pkt.at(j)),
                        std::get<2>(log_entries_pkt.at(j))
                );
            }

            // Queue size in byte
            const std::vector<std::tuple<int64_t, int64_t, int64_t>> log_entries_byte = tracker->GetIntervalsNumBytes();
            for (size_t j = 0; j < log_entries_byte.size(); j++) {

                // Write plain to the CSV file:
                // <from>,<to>,<interval start (ns)>,<interval end (ns)>,<number of bytes>
                fprintf(file_link_interface_tc_qdisc_queue_byte_csv,
                        "%d,%d,%" PRId64 ",%" PRId64 ",%" PRId64 "\n",
                        (int) directed_edge.first,
                        (int) directed_edge.second,
                        std::get<0>(log_entries_byte.at(j)),
                        std::get<1>(log_entries_byte.at(j)),
                        std::get<2>(log_entries_byte.at(j))
                );
            }


        }

        // Close log files
        std::cout << "  > Closing link (interface traffic-control) qdisc queue log files:" << std::endl;
        fclose(file_link_interface_tc_qdisc_queue_pkt_csv);
        std::cout << "    >> Closed: " << m_filename_link_interface_tc_qdisc_queue_pkt_csv << std::endl;
        fclose(file_link_interface_tc_qdisc_queue_byte_csv);
        std::cout << "    >> Closed: " << m_filename_link_interface_tc_qdisc_queue_byte_csv << std::endl;

        // Register completion
        std::cout << "  > Link (interface traffic-control) qdisc queue log files have been written" << std::endl;
        m_basicSimulation->RegisterTimestamp("Write link (interface traffic-control) qdisc queue log files");

        std::cout << std::endl;
    }

}
