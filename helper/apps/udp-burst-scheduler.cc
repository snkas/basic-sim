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
 * Originally based on the TcpFlowScheduler (which is authored by Simon, Hussain)
 */

#include "udp-burst-scheduler.h"

namespace ns3 {

void UdpBurstScheduler::StartNextUdpBurst(int i) {

    // Fetch the flow to start
    UdpBurstInfo& entry = m_schedule.at(i);
    int64_t now_ns = Simulator::Now().GetNanoSeconds();
    NS_ABORT_UNLESS(now_ns == entry.GetStartTimeNs());

    // Helper to install the source application
    UdpBurstClientHelper client(
            InetSocketAddress(m_nodes.Get(entry.GetFromNodeId())->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 0), // Port 0 means an ephemeral port will be assigned
            InetSocketAddress(m_nodes.Get(entry.GetToNodeId())->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1025),
            entry.GetUdpBurstId(),
            entry.GetTargetRateMegabitPerSec(),
            NanoSeconds(entry.GetDurationNs()),
            entry.GetAdditionalParameters(),
            m_enable_logging_for_udp_burst_ids.find(entry.GetUdpBurstId()) != m_enable_logging_for_udp_burst_ids.end(),
            m_basicSimulation->GetLogsDir()
    );

    // Install it on the node and start it right now
    ApplicationContainer app = client.Install(m_nodes.Get(entry.GetFromNodeId()));
    Ptr<UdpBurstClient> udpBurstClient = app.Get(0)->GetObject<UdpBurstClient>();
    udpBurstClient->SetUdpSocketGenerator(m_udpSocketGenerator);
    udpBurstClient->SetIpTos(m_ipTosGenerator->GenerateIpTos(UdpBurstClient::GetTypeId(), udpBurstClient));
    app.Start(NanoSeconds(0));

    // Match the entry to the application for logging later
    m_responsible_for_outgoing_bursts.push_back(std::make_pair(entry, app.Get(0)->GetObject<UdpBurstClient>()));

    // If there is a next flow to start, schedule its start
    if (i + 1 != (int) m_schedule.size()) {
        int64_t next_burst_ns = m_schedule.at(i + 1).GetStartTimeNs();
        Simulator::Schedule(
                NanoSeconds(next_burst_ns - now_ns),
                &UdpBurstScheduler::StartNextUdpBurst, this,
                i + 1
        );
    }

}

UdpBurstScheduler::UdpBurstScheduler(Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology) : UdpBurstScheduler(
        basicSimulation,
        topology,
        CreateObject<UdpSocketGeneratorDefault>(),
        CreateObject<IpTosGeneratorDefault>()
) {
    // Left empty intentionally
}

UdpBurstScheduler::UdpBurstScheduler(Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology, Ptr<UdpSocketGenerator> udpSocketGenerator, Ptr<IpTosGenerator> ipTosGenerator) {
    printf("UDP BURST SCHEDULER\n");

    m_basicSimulation = basicSimulation;
    m_topology = topology;
    m_udpSocketGenerator = udpSocketGenerator;
    m_ipTosGenerator = ipTosGenerator;

    // Check if it is enabled explicitly
    m_enabled = parse_boolean(m_basicSimulation->GetConfigParamOrDefault("enable_udp_burst_scheduler", "false"));
    if (!m_enabled) {
        std::cout << "  > Not enabled explicitly, so disabled" << std::endl;

    } else {
        std::cout << "  > UDP burst scheduler is enabled" << std::endl;

        // Properties we will use often
        m_nodes = m_topology->GetNodes();
        m_simulation_end_time_ns = m_basicSimulation->GetSimulationEndTimeNs();

        // Distributed run information
        m_enable_distributed = m_basicSimulation->IsDistributedEnabled();

        // Read schedule
        std::vector<UdpBurstInfo> complete_schedule = read_udp_burst_schedule(
                m_basicSimulation->GetRunDir() + "/" + m_basicSimulation->GetConfigParamOrFail("udp_burst_schedule_filename"),
                m_topology,
                m_simulation_end_time_ns
        );

        // Enable logging for UDP burst IDs
        std::string enable_for_udp_burst_ids_str = basicSimulation->GetConfigParamOrDefault("udp_burst_enable_logging_for_udp_burst_ids", "set()");
        if (enable_for_udp_burst_ids_str == "all") {
            for (size_t udp_burst_id = 0; udp_burst_id < complete_schedule.size(); udp_burst_id++) {
                m_enable_logging_for_udp_burst_ids.insert(udp_burst_id);
            }
        } else {
            m_enable_logging_for_udp_burst_ids = parse_set_positive_int64(enable_for_udp_burst_ids_str);
        }

        // Check that the UDP burst IDs exist in the logging
        for (int64_t udp_burst_id : m_enable_logging_for_udp_burst_ids) {
            if ((size_t) udp_burst_id >= complete_schedule.size()) {
                throw std::invalid_argument("Invalid UDP burst ID in udp_burst_enable_logging_for_udp_burst_ids: " + std::to_string(udp_burst_id));
            }
        }

        // Schedule read
        printf("  > Read schedule (total UDP bursts: %lu)\n", complete_schedule.size());
        m_basicSimulation->RegisterTimestamp("Read UDP burst schedule");

        // Determine filenames
        if (m_enable_distributed) {
            m_udp_bursts_outgoing_csv_filename = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_basicSimulation->GetSystemId()) + "_udp_bursts_outgoing.csv";
            m_udp_bursts_outgoing_txt_filename = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_basicSimulation->GetSystemId()) + "_udp_bursts_outgoing.txt";
            m_udp_bursts_incoming_csv_filename = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_basicSimulation->GetSystemId()) + "_udp_bursts_incoming.csv";
            m_udp_bursts_incoming_txt_filename = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_basicSimulation->GetSystemId()) + "_udp_bursts_incoming.txt";
        } else {
            m_udp_bursts_outgoing_csv_filename = m_basicSimulation->GetLogsDir() + "/udp_bursts_outgoing.csv";
            m_udp_bursts_outgoing_txt_filename = m_basicSimulation->GetLogsDir() + "/udp_bursts_outgoing.txt";
            m_udp_bursts_incoming_csv_filename = m_basicSimulation->GetLogsDir() + "/udp_bursts_incoming.csv";
            m_udp_bursts_incoming_txt_filename = m_basicSimulation->GetLogsDir() + "/udp_bursts_incoming.txt";
        }

        // Remove files if they are there
        remove_file_if_exists(m_udp_bursts_outgoing_csv_filename);
        remove_file_if_exists(m_udp_bursts_outgoing_txt_filename);
        remove_file_if_exists(m_udp_bursts_incoming_csv_filename);
        remove_file_if_exists(m_udp_bursts_incoming_txt_filename);
        printf("  > Removed previous UDP burst log files if present\n");
        m_basicSimulation->RegisterTimestamp("Remove previous UDP burst log files");

        // Endpoints
        std::set<int64_t> endpoints = m_topology->GetEndpoints();

        // Install UDP burst server on each endpoint node
        std::cout << "  > Setting up " << endpoints.size() << " UDP burst servers" << std::endl;
        for (int64_t endpoint : endpoints) {
            if (!m_enable_distributed || m_basicSimulation->IsNodeAssignedToThisSystem(endpoint)) {
                
                // Install the server
                UdpBurstServerHelper burstServerHelper(
                        InetSocketAddress(m_nodes.Get(endpoint)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1025),
                        m_basicSimulation->GetLogsDir()
                );
                ApplicationContainer app = burstServerHelper.Install(m_nodes.Get(endpoint));
                Ptr<UdpBurstServer> udpBurstServer = app.Get(0)->GetObject<UdpBurstServer>();
                udpBurstServer->SetUdpSocketGenerator(m_udpSocketGenerator);
                udpBurstServer->SetIpTos(m_ipTosGenerator->GenerateIpTos(UdpBurstServer::GetTypeId(), udpBurstServer));
                app.Start(Seconds(0.0));

                // Register all incoming bursts of the server
                for (UdpBurstInfo entry : complete_schedule) {
                    if (entry.GetToNodeId() == endpoint) {
                        udpBurstServer->RegisterIncomingBurst(
                                entry.GetUdpBurstId(),
                                m_enable_logging_for_udp_burst_ids.find(entry.GetUdpBurstId()) != m_enable_logging_for_udp_burst_ids.end()
                        );
                        m_responsible_for_incoming_bursts.push_back(std::make_pair(entry, udpBurstServer));
                    }
                }

            }
        }
        m_basicSimulation->RegisterTimestamp("Setup UDP burst servers");

        // Filter the schedule to only have bursts starting at nodes which are part of this system
        if (m_enable_distributed) {
            std::vector<UdpBurstInfo> filtered_schedule;
            for (UdpBurstInfo &entry : complete_schedule) {
                if (m_basicSimulation->IsNodeAssignedToThisSystem(entry.GetFromNodeId())) {
                    filtered_schedule.push_back(entry);
                }
            }
            m_schedule = filtered_schedule;
        } else {
            m_schedule = complete_schedule;
        }

        // Schedule read
        printf("  > Filtered schedule (total UDP bursts: %lu)\n", complete_schedule.size());
        m_basicSimulation->RegisterTimestamp("Filter UDP burst schedule for this system");
        
        // Setup start of first UDP burst client
        std::cout << "  > Schedule start of first UDP burst client" << std::endl;
        if (m_schedule.size() > 0) {
            Simulator::Schedule(
                    NanoSeconds(m_schedule.at(0).GetStartTimeNs()),
                    &UdpBurstScheduler::StartNextUdpBurst, this,
                    0 // First index
            );
        }
        m_basicSimulation->RegisterTimestamp("Schedule start of first UDP burst client");

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
        FILE* file_outgoing_csv = fopen(m_udp_bursts_outgoing_csv_filename.c_str(), "w+");
        std::cout << "    >> Opened: " << m_udp_bursts_outgoing_csv_filename << std::endl;
        FILE* file_outgoing_txt = fopen(m_udp_bursts_outgoing_txt_filename.c_str(), "w+");
        std::cout << "    >> Opened: " << m_udp_bursts_outgoing_txt_filename << std::endl;
        FILE* file_incoming_csv = fopen(m_udp_bursts_incoming_csv_filename.c_str(), "w+");
        std::cout << "    >> Opened: " << m_udp_bursts_incoming_csv_filename << std::endl;
        FILE* file_incoming_txt = fopen(m_udp_bursts_incoming_txt_filename.c_str(), "w+");
        std::cout << "    >> Opened: " << m_udp_bursts_incoming_txt_filename << std::endl;

        // Header
        std::cout << "  > Writing udp_bursts_{incoming, outgoing}.txt headers" << std::endl;
        fprintf(
                file_outgoing_txt, "%-16s%-10s%-10s%-20s%-16s%-16s%-28s%-28s%-16s%-28s%-28s%s\n",
                "UDP burst ID", "From", "To", "Target rate", "Start time", "Duration",
                "Outgoing rate (w/ headers)", "Outgoing rate (payload)", "Packets sent",
                "Data sent (w/headers)", "Data sent (payload)", "Metadata"
        );
        fprintf(
                file_incoming_txt, "%-16s%-10s%-10s%-20s%-16s%-16s%-28s%-28s%-19s%-28s%-28s%s\n",
                "UDP burst ID", "From", "To", "Target rate", "Start time", "Duration",
                "Incoming rate (w/ headers)", "Incoming rate (payload)", "Packets received",
                "Data received (w/headers)", "Data received (payload)", "Metadata"
        );

        // Sort ascending to preserve UDP burst schedule order
        struct ascending_paired_udp_burst_client_id_key
        {
            inline bool operator() (const std::pair<UdpBurstInfo, Ptr<UdpBurstClient>>& a, const std::pair<UdpBurstInfo, Ptr<UdpBurstClient>>& b)
            {
                return (a.first.GetUdpBurstId() < b.first.GetUdpBurstId());
            }
        };
        std::sort(m_responsible_for_outgoing_bursts.begin(), m_responsible_for_outgoing_bursts.end(), ascending_paired_udp_burst_client_id_key());
        struct ascending_paired_udp_burst_server_id_key
        {
            inline bool operator() (const std::pair<UdpBurstInfo, Ptr<UdpBurstServer>>& a, const std::pair<UdpBurstInfo, Ptr<UdpBurstServer>>& b)
            {
                return (a.first.GetUdpBurstId() < b.first.GetUdpBurstId());
            }
        };
        std::sort(m_responsible_for_incoming_bursts.begin(), m_responsible_for_incoming_bursts.end(), ascending_paired_udp_burst_server_id_key());

        // Outgoing bursts
        std::cout << "  > Writing outgoing log files" << std::endl;
        for (std::pair<UdpBurstInfo, Ptr<UdpBurstClient>> p : m_responsible_for_outgoing_bursts) {
            UdpBurstInfo info = p.first;
            Ptr<UdpBurstClient> udpBurstClient = p.second;

            // Fetch data from the application
            uint32_t complete_packet_size = udpBurstClient->GetMaxSegmentSizeByte();
            uint32_t max_udp_payload_size_byte = udpBurstClient->GetMaxUdpPayloadSizeByte();
            uint64_t sent_counter = udpBurstClient->GetSent();

            // Calculate outgoing rate
            int64_t effective_duration_ns = info.GetStartTimeNs() + info.GetDurationNs() >= m_simulation_end_time_ns ? m_simulation_end_time_ns - info.GetStartTimeNs() : info.GetDurationNs();
            double rate_incl_headers_megabit_per_s = byte_to_megabit(sent_counter * complete_packet_size) / nanosec_to_sec(effective_duration_ns);
            double rate_payload_only_megabit_per_s = byte_to_megabit(sent_counter * max_udp_payload_size_byte) / nanosec_to_sec(effective_duration_ns);

            // Write plain to the CSV
            fprintf(
                    file_outgoing_csv, "%" PRId64 ",%" PRId64 ",%" PRId64 ",%f,%" PRId64 ",%" PRId64 ",%f,%f,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%s\n",
                    info.GetUdpBurstId(), info.GetFromNodeId(), info.GetToNodeId(), info.GetTargetRateMegabitPerSec(), info.GetStartTimeNs(),
                    info.GetDurationNs(), rate_incl_headers_megabit_per_s, rate_payload_only_megabit_per_s, sent_counter,
                    sent_counter * complete_packet_size, sent_counter * max_udp_payload_size_byte, info.GetMetadata().c_str()
            );

            // Write nicely formatted to the text
            char str_target_rate[100];
            sprintf(str_target_rate, "%.2f Mbit/s", info.GetTargetRateMegabitPerSec());
            char str_start_time[100];
            sprintf(str_start_time, "%.2f ms", nanosec_to_millisec(info.GetStartTimeNs()));
            char str_duration_ms[100];
            sprintf(str_duration_ms, "%.2f ms", nanosec_to_millisec(info.GetDurationNs()));
            char str_eff_rate_incl_headers[100];
            sprintf(str_eff_rate_incl_headers, "%.2f Mbit/s", rate_incl_headers_megabit_per_s);
            char str_eff_rate_payload_only[100];
            sprintf(str_eff_rate_payload_only, "%.2f Mbit/s", rate_payload_only_megabit_per_s);
            char str_sent_incl_headers[100];
            sprintf(str_sent_incl_headers, "%.2f Mbit", byte_to_megabit(sent_counter * complete_packet_size));
            char str_sent_payload_only[100];
            sprintf(str_sent_payload_only, "%.2f Mbit", byte_to_megabit(sent_counter * max_udp_payload_size_byte));
            fprintf(
                    file_outgoing_txt,
                    "%-16" PRId64 "%-10" PRId64 "%-10" PRId64 "%-20s%-16s%-16s%-28s%-28s%-16" PRIu64 "%-28s%-28s%s\n",
                    info.GetUdpBurstId(),
                    info.GetFromNodeId(),
                    info.GetToNodeId(),
                    str_target_rate,
                    str_start_time,
                    str_duration_ms,
                    str_eff_rate_incl_headers,
                    str_eff_rate_payload_only,
                    sent_counter,
                    str_sent_incl_headers,
                    str_sent_payload_only,
                    info.GetMetadata().c_str()
            );
        }

        // Incoming bursts
        std::cout << "  > Writing incoming log files" << std::endl;
        for (std::pair<UdpBurstInfo, Ptr<UdpBurstServer>> p : m_responsible_for_incoming_bursts) {
            UdpBurstInfo info = p.first;
            Ptr<UdpBurstServer> udpBurstServerIncoming = p.second;

            // Fetch data from the application
            uint32_t complete_packet_size = udpBurstServerIncoming->GetMaxSegmentSizeByte();
            uint32_t max_udp_payload_size_byte = udpBurstServerIncoming->GetMaxUdpPayloadSizeByte();
            uint64_t received_counter = udpBurstServerIncoming->GetReceivedCounterOf(info.GetUdpBurstId());

            // Calculate incoming rate
            int64_t effective_duration_ns = info.GetStartTimeNs() + info.GetDurationNs() >= m_simulation_end_time_ns ? m_simulation_end_time_ns - info.GetStartTimeNs() : info.GetDurationNs();
            double rate_incl_headers_megabit_per_s = byte_to_megabit(received_counter * complete_packet_size) / nanosec_to_sec(effective_duration_ns);
            double rate_payload_only_megabit_per_s = byte_to_megabit(received_counter * max_udp_payload_size_byte) / nanosec_to_sec(effective_duration_ns);

            // Write plain to the CSV
            fprintf(
                    file_incoming_csv, "%" PRId64 ",%" PRId64 ",%" PRId64 ",%f,%" PRId64 ",%" PRId64 ",%f,%f,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%s\n",
                    info.GetUdpBurstId(), info.GetFromNodeId(), info.GetToNodeId(), info.GetTargetRateMegabitPerSec(), info.GetStartTimeNs(),
                    info.GetDurationNs(), rate_incl_headers_megabit_per_s, rate_payload_only_megabit_per_s, received_counter,
                    received_counter * complete_packet_size, received_counter * max_udp_payload_size_byte, info.GetMetadata().c_str()
            );

            // Write nicely formatted to the text
            char str_target_rate[100];
            sprintf(str_target_rate, "%.2f Mbit/s", info.GetTargetRateMegabitPerSec());
            char str_start_time[100];
            sprintf(str_start_time, "%.2f ms", nanosec_to_millisec(info.GetStartTimeNs()));
            char str_duration_ms[100];
            sprintf(str_duration_ms, "%.2f ms", nanosec_to_millisec(info.GetDurationNs()));
            char str_eff_rate_incl_headers[100];
            sprintf(str_eff_rate_incl_headers, "%.2f Mbit/s", rate_incl_headers_megabit_per_s);
            char str_eff_rate_payload_only[100];
            sprintf(str_eff_rate_payload_only, "%.2f Mbit/s", rate_payload_only_megabit_per_s);
            char str_received_incl_headers[100];
            sprintf(str_received_incl_headers, "%.2f Mbit", byte_to_megabit(received_counter * complete_packet_size));
            char str_received_payload_only[100];
            sprintf(str_received_payload_only, "%.2f Mbit", byte_to_megabit(received_counter * max_udp_payload_size_byte));
            fprintf(
                    file_incoming_txt,
                    "%-16" PRId64 "%-10" PRId64 "%-10" PRId64 "%-20s%-16s%-16s%-28s%-28s%-19" PRIu64 "%-28s%-28s%s\n",
                    info.GetUdpBurstId(),
                    info.GetFromNodeId(),
                    info.GetToNodeId(),
                    str_target_rate,
                    str_start_time,
                    str_duration_ms,
                    str_eff_rate_incl_headers,
                    str_eff_rate_payload_only,
                    received_counter,
                    str_received_incl_headers,
                    str_received_payload_only,
                    info.GetMetadata().c_str()
            );

        }

        // Close files
        std::cout << "  > Closing UDP burst log files:" << std::endl;
        fclose(file_outgoing_csv);
        std::cout << "    >> Closed: " << m_udp_bursts_outgoing_csv_filename << std::endl;
        fclose(file_outgoing_txt);
        std::cout << "    >> Closed: " << m_udp_bursts_outgoing_txt_filename << std::endl;
        fclose(file_incoming_csv);
        std::cout << "    >> Closed: " << m_udp_bursts_incoming_csv_filename << std::endl;
        fclose(file_incoming_txt);
        std::cout << "    >> Closed: " << m_udp_bursts_incoming_txt_filename << std::endl;

        // Register completion
        std::cout << "  > UDP burst log files have been written" << std::endl;
        m_basicSimulation->RegisterTimestamp("Write UDP burst log files");

    }

    std::cout << std::endl;
}

}
