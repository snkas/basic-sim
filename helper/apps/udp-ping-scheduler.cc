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

#include "udp-ping-scheduler.h"

namespace ns3 {

void UdpPingScheduler::StartNextUdpPing(int i) {

    // Fetch the flow to start
    UdpPingInfo& entry = m_schedule.at(i);
    int64_t now_ns = Simulator::Now().GetNanoSeconds();
    NS_ABORT_UNLESS(now_ns == entry.GetStartTimeNs());

    // Helper to install the source application
    UdpPingClientHelper client(
            InetSocketAddress(m_nodes.Get(entry.GetFromNodeId())->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 0), // Port 0 means an ephemeral port will be assigned
            InetSocketAddress(m_nodes.Get(entry.GetToNodeId())->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1026),
            entry.GetUdpPingId(),
            NanoSeconds(entry.GetIntervalNs()),
            NanoSeconds(entry.GetDurationNs()),
            NanoSeconds(entry.GetWaitAfterwardsNs()),
            entry.GetAdditionalParameters()
    );

    // Install it on the node and start it right now
    ApplicationContainer app = client.Install(m_nodes.Get(entry.GetFromNodeId()));
    Ptr<UdpPingClient> udpPingClient = app.Get(0)->GetObject<UdpPingClient>();
    udpPingClient->SetUdpSocketGenerator(m_udpSocketGenerator);
    udpPingClient->SetIpTos(m_ipTosGenerator->GenerateIpTos(UdpPingClient::GetTypeId(), udpPingClient));
    app.Start(NanoSeconds(0));
    m_apps.push_back(app);

    // If there is a next flow to start, schedule its start
    if (i + 1 != (int) m_schedule.size()) {
        int64_t next_ping_ns = m_schedule.at(i + 1).GetStartTimeNs();
        Simulator::Schedule(
                NanoSeconds(next_ping_ns - now_ns),
                &UdpPingScheduler::StartNextUdpPing, this,
                i + 1
        );
    }

}

UdpPingScheduler::UdpPingScheduler(Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology) : UdpPingScheduler(
        basicSimulation,
        topology,
        CreateObject<UdpSocketGeneratorDefault>(),
        CreateObject<IpTosGeneratorDefault>()
) {
    // Left empty intentionally
}

UdpPingScheduler::UdpPingScheduler(Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology, Ptr<UdpSocketGenerator> udpSocketGenerator, Ptr<IpTosGenerator> ipTosGenerator) {
    printf("UDP PING SCHEDULER\n");

    m_basicSimulation = basicSimulation;
    m_topology = topology;
    m_udpSocketGenerator = udpSocketGenerator;
    m_ipTosGenerator = ipTosGenerator;

    // Check if it is enabled explicitly
    m_enabled = parse_boolean(m_basicSimulation->GetConfigParamOrDefault("enable_udp_ping_scheduler", "false"));
    if (!m_enabled) {
        std::cout << "  > Not enabled explicitly, so disabled" << std::endl;

    } else {
        std::cout << "  > UDP ping scheduler is enabled" << std::endl;

        // Properties we will use often
        m_nodes = m_topology->GetNodes();
        m_simulation_end_time_ns = m_basicSimulation->GetSimulationEndTimeNs();

        // Distributed run information
        m_enable_distributed = m_basicSimulation->IsDistributedEnabled();

        // Read schedule
        std::vector<UdpPingInfo> complete_schedule = read_udp_ping_schedule(
                m_basicSimulation->GetRunDir() + "/" + m_basicSimulation->GetConfigParamOrFail("udp_ping_schedule_filename"),
                m_topology,
                m_simulation_end_time_ns
        );

        // Filter the schedule to only have applications starting at nodes which are part of this system
        if (m_enable_distributed) {
            std::vector<UdpPingInfo> filtered_schedule;
            for (UdpPingInfo &entry : complete_schedule) {
                if (m_basicSimulation->IsNodeAssignedToThisSystem(entry.GetFromNodeId())) {
                    filtered_schedule.push_back(entry);
                }
            }
            m_schedule = filtered_schedule;
        } else {
            m_schedule = complete_schedule;
        }

        // Schedule read
        printf("  > Read schedule (total UDP pings: %lu)\n", m_schedule.size());
        m_basicSimulation->RegisterTimestamp("Read UDP ping schedule");

        // Determine filenames
        if (m_enable_distributed) {
            m_udp_pings_csv_filename = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_basicSimulation->GetSystemId()) + "_udp_pings.csv";
            m_udp_pings_txt_filename = m_basicSimulation->GetLogsDir() + "/system_" + std::to_string(m_basicSimulation->GetSystemId()) + "_udp_pings.txt";
        } else {
            m_udp_pings_csv_filename = m_basicSimulation->GetLogsDir() + "/udp_pings.csv";
            m_udp_pings_txt_filename = m_basicSimulation->GetLogsDir() + "/udp_pings.txt";
        }

        // Remove files if they are there
        remove_file_if_exists(m_udp_pings_csv_filename);
        remove_file_if_exists(m_udp_pings_txt_filename);
        printf("  > Removed previous UDP ping log files if present\n");
        m_basicSimulation->RegisterTimestamp("Remove previous UDP ping log files");
        
        // Endpoints
        std::set<int64_t> endpoints = m_topology->GetEndpoints();

        // Install UDP server on each node
        std::cout << "  > Setting up " << endpoints.size() << " UDP ping servers" << std::endl;
        for (int64_t i : endpoints) {
            if (!m_enable_distributed || m_basicSimulation->IsNodeAssignedToThisSystem(i)) {
                UdpPingServerHelper pingServerHelper(
                    InetSocketAddress(m_nodes.Get(i)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1026)
                );
                ApplicationContainer app = pingServerHelper.Install(m_nodes.Get(i));
                Ptr<UdpPingServer> udpPingServer = app.Get(0)->GetObject<UdpPingServer>();
                udpPingServer->SetUdpSocketGenerator(m_udpSocketGenerator);
                udpPingServer->SetIpTos(m_ipTosGenerator->GenerateIpTos(UdpPingServer::GetTypeId(), udpPingServer));
                app.Start(Seconds(0.0));
            }
        }
        m_basicSimulation->RegisterTimestamp("Setup UDP ping servers");

        // Setup start of first client application
        std::cout << "  > Schedule start of first UDP ping client" << std::endl;
        if (m_schedule.size() > 0) {
            Simulator::Schedule(
                    NanoSeconds(m_schedule.at(0).GetStartTimeNs()),
                    &UdpPingScheduler::StartNextUdpPing, this,
                    0 // First index
            );
        }
        m_basicSimulation->RegisterTimestamp("Schedule start of first UDP ping client");

    }

    std::cout << std::endl;
}

void UdpPingScheduler::WriteResults() {
    std::cout << "STORE UDP PING RESULTS" << std::endl;

    // Check if it is enabled explicitly
    if (!m_enabled) {
        std::cout << "  > Not enabled, so no UDP ping results are written" << std::endl;

    } else {

        // Open files
        std::cout << "  > Opening UDP ping log files:" << std::endl;
        FILE* file_csv = fopen(m_udp_pings_csv_filename.c_str(), "w+");
        std::cout << "    >> Opened: " << m_udp_pings_csv_filename << std::endl;
        FILE* file_txt = fopen(m_udp_pings_txt_filename.c_str(), "w+");
        std::cout << "    >> Opened: " << m_udp_pings_txt_filename << std::endl;

        // Header
        std::cout << "  > Writing udp_pings.txt header" << std::endl;
        fprintf(file_txt, "%-16s%-10s%-10s%-18s%-18s%-18s%-22s%-22s%-16s%-16s%-16s%-16s%s\n",
                "UDP ping ID", "Source", "Target", "Start time (ns)", "End time (ns)", "Interval (ns)",
                "Mean latency there", "Mean latency back", "Min. RTT", "Mean RTT", "Max. RTT", "Smp.std. RTT", "Reply arrival");

        // Go over the applications, write each ping's result
        for (uint32_t i = 0; i < m_apps.size(); i++) {
            UdpPingInfo info = m_schedule.at(i);
            Ptr<UdpPingClient> client = m_apps.at(i).Get(0)->GetObject<UdpPingClient>();

            // Data about this pair
            int64_t from_node_id = info.GetFromNodeId();
            int64_t to_node_id = info.GetToNodeId();
            uint32_t sent = client->GetSent();
            std::vector<int64_t> sendRequestTimestamps = client->GetSendRequestTimestamps();
            std::vector<int64_t> replyTimestamps = client->GetReplyTimestamps();
            std::vector<int64_t> receiveReplyTimestamps = client->GetReceiveReplyTimestamps();

            int total = 0;
            double sum_latency_to_there_ns = 0.0;
            double sum_latency_from_there_ns = 0.0;
            std::vector<int64_t> rtts_ns;
            for (uint32_t j = 0; j < sent; j++) {

                // Outcome
                bool reply_arrived = replyTimestamps.at(j) != -1;
                std::string reply_arrived_str = reply_arrived ? "YES" : "LOST";

                // Latencies
                int64_t latency_to_there_ns = reply_arrived ? replyTimestamps.at(j) - sendRequestTimestamps.at(j) : -1;
                int64_t latency_from_there_ns = reply_arrived ? receiveReplyTimestamps.at(j) - replyTimestamps.at(j) : -1;
                int64_t rtt_ns = reply_arrived ? latency_to_there_ns + latency_from_there_ns : -1;

                // Write plain to the csv
                fprintf(
                        file_csv,
                        "%" PRId64 ",%u,%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%" PRId64 ",%s\n",
                        info.GetUdpPingId(), j, sendRequestTimestamps.at(j), replyTimestamps.at(j), receiveReplyTimestamps.at(j),
                        latency_to_there_ns, latency_from_there_ns, rtt_ns, reply_arrived_str.c_str()
                );

                // Add to statistics
                if (reply_arrived) {
                    total++;
                    sum_latency_to_there_ns += latency_to_there_ns;
                    sum_latency_from_there_ns += latency_from_there_ns;
                    rtts_ns.push_back(rtt_ns);
                }

            }

            // Finalize the statistics
            int64_t min_rtt_ns = 10000000000000; // 10000s should be sufficiently high
            int64_t max_rtt_ns = -1;
            double mean_rtt_ns = (sum_latency_to_there_ns + sum_latency_from_there_ns) / total;
            double sum_sq = 0.0;
            for (uint32_t j = 0; j < rtts_ns.size(); j++) {
                min_rtt_ns = std::min(min_rtt_ns, rtts_ns.at(j));
                max_rtt_ns = std::max(max_rtt_ns, rtts_ns.at(j));
                sum_sq += std::pow(rtts_ns.at(j) - mean_rtt_ns, 2);
            }
            double sample_std_rtt_ns;
            double mean_latency_to_there_ns;
            double mean_latency_from_there_ns;
            if (rtts_ns.size() == 0) { // If no measurements came through, it should all be -1
                mean_latency_to_there_ns = -1;
                mean_latency_from_there_ns = -1;
                min_rtt_ns = -1;
                max_rtt_ns = -1;
                mean_rtt_ns = -1;
                sample_std_rtt_ns = -1;
            } else {
                mean_latency_to_there_ns = sum_latency_to_there_ns / total;
                mean_latency_from_there_ns = sum_latency_from_there_ns / total;
                sample_std_rtt_ns = rtts_ns.size() > 1 ? std::sqrt((1.0 / (rtts_ns.size() - 1)) * sum_sq) : 0.0;
            }

            // Write nicely formatted to the text
            char str_latency_to_there_ms[100];
            sprintf(str_latency_to_there_ms, "%.2f ms", mean_latency_to_there_ns == -1 ? -1 : nanosec_to_millisec(mean_latency_to_there_ns));
            char str_latency_from_there_ms[100];
            sprintf(str_latency_from_there_ms, "%.2f ms", mean_latency_from_there_ns == -1 ? -1 : nanosec_to_millisec(mean_latency_from_there_ns));
            char str_min_rtt_ms[100];
            sprintf(str_min_rtt_ms, "%.2f ms", min_rtt_ns == -1 ? -1 : nanosec_to_millisec(min_rtt_ns));
            char str_mean_rtt_ms[100];
            sprintf(str_mean_rtt_ms, "%.2f ms", mean_rtt_ns == -1 ? -1 : nanosec_to_millisec(mean_rtt_ns));
            char str_max_rtt_ms[100];
            sprintf(str_max_rtt_ms, "%.2f ms", max_rtt_ns == -1 ? -1 : nanosec_to_millisec(max_rtt_ns));
            char str_sample_std_rtt_ms[100];
            sprintf(str_sample_std_rtt_ms, "%.2f ms", sample_std_rtt_ns == -1 ? -1 : nanosec_to_millisec(sample_std_rtt_ns));
            fprintf(
                    file_txt, "%-16" PRId64 "%-10" PRId64 "%-10" PRId64 "%-18" PRId64 "%-18" PRId64 "%-18" PRId64 "%-22s%-22s%-16s%-16s%-16s%-16s%d/%d (%d%%)\n",
                    info.GetUdpPingId(), from_node_id, to_node_id, info.GetStartTimeNs(), info.GetStartTimeNs() + info.GetDurationNs(),
                    info.GetIntervalNs(), str_latency_to_there_ms, str_latency_from_there_ms, str_min_rtt_ms, str_mean_rtt_ms, str_max_rtt_ms,
                    str_sample_std_rtt_ms, total, sent, (int) std::round(((double) total / (double) sent) * 100.0)
            );

        }

        // Close files
        std::cout << "  > Closing UDP ping log files:" << std::endl;
        fclose(file_csv);
        std::cout << "    >> Closed: " << m_udp_pings_csv_filename << std::endl;
        fclose(file_txt);
        std::cout << "    >> Closed: " << m_udp_pings_txt_filename << std::endl;

        // Register completion
        std::cout << "  > UDP ping log files have been written" << std::endl;
        m_basicSimulation->RegisterTimestamp("Write UDP ping log files");

    }

    std::cout << std::endl;
}

}
