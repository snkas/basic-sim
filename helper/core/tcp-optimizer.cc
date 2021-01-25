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

#include "ns3/tcp-optimizer.h"

namespace ns3 {

void TcpOptimizer::Generic() {

    // Clock granularity (ns-3 default: 1ms, we set to 1ns)
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocketBase", "ClockGranularity").Get().GetNanoSeconds() != 1000000, "Unexpected default initial default ClockGranularity.");
    Config::SetDefault("ns3::TcpSocketBase::ClockGranularity", TimeValue(NanoSeconds(1)));
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocketBase", "ClockGranularity").Get().GetNanoSeconds() != 1, "Default initial ClockGranularity was not updated.");
    printf("  > Clock granularity.......... 1 ns\n");

    // Initial congestion window (ns-3 default: 10, we also keep it at 10)
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "InitialCwnd") != 10, "Unexpected default initial default InitialCwnd.");
    int64_t init_cwnd_pkt = 10;
    Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(init_cwnd_pkt));
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "InitialCwnd") != init_cwnd_pkt, "Default initial InitialCwnd was not updated.");
    printf("  > Initial CWND............... %" PRId64 " packets\n", init_cwnd_pkt);

    // Buffer size puts an upper limit of the amount of data permitted to be in-flight.
    // It the buffer size limit is reached the line "if (!m_txBuffer->Add (p))" in TcpSocketBase will be true,
    // and as such limit how much an Application can put.
    //
    // This however does not upper limit the growth of the congestion window:
    // upon the receipt of every ACK packet it is still increased as no loss occurs if the
    // in-flight size <= BDP. If this goes on long enough (with slow-start), the CWND
    // uint32 can even overflow.
    //
    // The TCP rate is limited to:
    // tcp-rate = min(send-buffer-size / base RTT, line-rate)
    //
    // The below value of 1 GiB is able to satisfy the line rate (i.e., bandwidth-delay product)
    // only when (excl. header overhead in calculation):
    //
    // For 10 Gbit/s at most with a base RTT of 859ms
    // For 1 Gbit/s at most with a base RTT of 8.59s
    // For 100 Mbit/s at most with a base RTT of 85.9s

    // Send buffer size (ns-3 default: 131072 bytes = 128 KiB, we set to 1 GiB)
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "SndBufSize") != 131072, "Unexpected default initial default SndBufSize.");
    int64_t snd_buf_size_byte = 131072 * 8192;
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(snd_buf_size_byte));
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "SndBufSize") != snd_buf_size_byte, "Default initial SndBufSize was not updated.");
    printf("  > Send buffer size........... %.3f MB\n", snd_buf_size_byte / 1e6);

    // Receive buffer size (ns-3 default: 131072 bytes = 128 KiB, we set to 1 GiB)
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "RcvBufSize") != 131072, "Unexpected default initial default RcvBufSize.");
    int64_t rcv_buf_size_byte = 131072 * 8192;
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(rcv_buf_size_byte));
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "RcvBufSize") != rcv_buf_size_byte, "Default initial RcvBufSize was not updated.");
    printf("  > Receive buffer size........ %.3f MB\n", rcv_buf_size_byte / 1e6);

    // Segment size (ns-3 default is 536, because we know MTU is 1500 byte, we set it to 1380)
    // IP header size: min. 20 byte, max. 60 byte
    // TCP header size: min. 20 byte, max. 60 byte
    // So, 1500 - 60 - 60 = 1380 would be the safest bet (given we don't do tunneling)
    // This could be increased higher, e.g. as discussed here:
    // https://blog.cloudflare.com/increasing-ipv6-mtu/ (retrieved April 7th, 2020)
    // In past ns-3 simulations, I've observed that the IP + TCP header is generally not larger than 80 bytes.
    // This means it could be potentially set closer to 1400-1420.
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "SegmentSize") != 536, "Unexpected default initial default SegmentSize.");
    int64_t segment_size_byte = 1380;
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(segment_size_byte));
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "SegmentSize") != segment_size_byte, "Default initial SegmentSize was not updated.");
    printf("  > Segment size............... %" PRId64 " byte\n", segment_size_byte);

    // Timestamp option (ns-3 default: true, we also keep it at true)
    // To get an RTT measurement with a resolution of less than 1ms, it needs
    // to be disabled because the fields in the TCP Option are in milliseconds.
    // When disabling it, there are two downsides:
    //  (1) Less protection against wrapped sequence numbers (PAWS)
    //  (2) Algorithm to see if it has entered loss recovery unnecessarily are not as possible (Eiffel)
    // See: https://tools.ietf.org/html/rfc7323#section-3
    //      https://tools.ietf.org/html/rfc3522
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocketBase", "Timestamp") != true, "Unexpected default initial default Timestamp.");
    bool opt_timestamp_enabled = true;
    Config::SetDefault("ns3::TcpSocketBase::Timestamp", BooleanValue(opt_timestamp_enabled));
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocketBase", "Timestamp") != opt_timestamp_enabled, "Default initial Timestamp was not updated.");
    printf("  > Timestamp option........... %s\n", opt_timestamp_enabled ? "enabled" : "disabled");

    // SACK option (ns-3 default: true, we also keep it at true)
    // Selective acknowledgment improves TCP performance, however can
    // be time-consuming to simulate because set operations and merges have
    // to be performed.
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocketBase", "Sack") != true, "Unexpected default initial default Sack.");
    bool opt_sack_enabled = true;  // Default: true.
    Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue(opt_sack_enabled));
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocketBase", "Sack") != opt_sack_enabled, "Default initial Sack was not updated.");
    printf("  > SACK option................ %s\n", opt_sack_enabled ? "enabled" : "disabled");

    // Window scaling option (ns-3 default: true, we also keep it at true)
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocketBase", "WindowScaling") != true, "Unexpected default initial default WindowScaling.");
    bool opt_win_scaling_enabled = true;
    Config::SetDefault("ns3::TcpSocketBase::WindowScaling", BooleanValue(opt_win_scaling_enabled));
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocketBase", "WindowScaling") != opt_win_scaling_enabled, "Default initial WindowScaling was not updated.");
    printf("  > Window scaling option...... %s\n", opt_win_scaling_enabled ? "enabled" : "disabled");

    // Pacing (ns-3 default: false, we also keep it at false)
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocketState", "EnablePacing") != false, "Unexpected default initial default EnablePacing.");
    bool pacing_enabled = false;
    Config::SetDefault("ns3::TcpSocketState::EnablePacing", BooleanValue(pacing_enabled));
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocketState", "EnablePacing") != pacing_enabled, "Default initial EnablePacing was not updated.");
    printf("  > Pacing..................... %s\n", pacing_enabled ? "enabled" : "disabled");

    // Number of packets to wait before sending a TCP ACK (ns-3 default: 2, we also keep it at 2)
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "DelAckCount") != 2, "Unexpected default initial default DelAckCount.");
    int64_t delayed_ack_packet_count = 2;
    Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(delayed_ack_packet_count));
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "DelAckCount") != delayed_ack_packet_count, "Default initial DelAckCount was not updated.");
    printf("  > Delayed ACK count.......... %" PRId64 "\n", delayed_ack_packet_count);

    // Whether to enable or disable Nagle's algorithm (ns-3 default: true, we also set to true)
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocket", "TcpNoDelay") != true, "Unexpected default initial default TcpNoDelay.");
    bool no_delay = true;
    Config::SetDefault("ns3::TcpSocket::TcpNoDelay", BooleanValue(no_delay));
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocket", "TcpNoDelay") != no_delay, "Default initial TcpNoDelay was not updated.");
    printf("  > No delay (disable Nagle)... %s\n", no_delay ? "enabled" : "disabled");

}

void TcpOptimizer::OptimizeBasic(Ptr<BasicSimulation> basicSimulation) {
    std::cout << "TCP OPTIMIZATION BASIC" << std::endl;
    Generic();
    std::cout << std::endl;
    basicSimulation->RegisterTimestamp("Setup TCP parameters");
}

void TcpOptimizer::OptimizeUsingWorstCaseRtt(Ptr<BasicSimulation> basicSimulation, int64_t worst_case_rtt_ns) {
    std::cout << "TCP OPTIMIZATION USING WORST-CASE RTT" << std::endl;

    Generic();

    // Maximum segment lifetime (ns-3 default: 120s, we set 5 * worst-case RTT)
    NS_ABORT_MSG_IF(GetInitialDoubleValue("ns3::TcpSocketBase", "MaxSegLifetime") != 120.0, "Unexpected default initial default MaxSegLifetime.");
    int64_t max_seg_lifetime_ns = 5 * worst_case_rtt_ns;
    Config::SetDefault("ns3::TcpSocketBase::MaxSegLifetime", DoubleValue(max_seg_lifetime_ns / 1e9));
    NS_ABORT_MSG_IF(GetInitialDoubleValue("ns3::TcpSocketBase", "MaxSegLifetime") != max_seg_lifetime_ns / 1e9, "Default initial MaxSegLifetime was not updated.");
    printf("  > Maximum segment lifetime... %.3f ms\n", max_seg_lifetime_ns / 1e6);

    // Minimum retransmission timeout (ns-3 default: 1s, Linux default: 200ms, we set 1.5 * worst-case-RTT)
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocketBase", "MinRto").Get().GetNanoSeconds() != 1000000000, "Unexpected default initial default MinRto.");
    int64_t min_rto_ns = (int64_t)(1.5 * worst_case_rtt_ns);  //
    Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(NanoSeconds(min_rto_ns)));
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocketBase", "MinRto").Get().GetNanoSeconds() != min_rto_ns, "Default initial MinRto was not updated.");
    printf("  > Minimum RTO................ %.3f ms\n", min_rto_ns / 1e6);

    // Initial RTT estimate (ns-3 default: 1s, we set to 2 * worst-case-RTT)
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::RttEstimator", "InitialEstimation").Get().GetNanoSeconds() != 1000000000, "Unexpected default initial default InitialEstimation.");
    int64_t initial_rtt_estimate_ns = 2 * worst_case_rtt_ns;
    Config::SetDefault("ns3::RttEstimator::InitialEstimation", TimeValue(NanoSeconds(initial_rtt_estimate_ns)));
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::RttEstimator", "InitialEstimation").Get().GetNanoSeconds() != initial_rtt_estimate_ns, "Default initial InitialEstimation was not updated.");
    printf("  > Initial RTT measurement.... %.3f ms\n", initial_rtt_estimate_ns / 1e6);

    // Connection timeout (ns-3 default: 3s, we set to 2 * worst-case-RTT)
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocket", "ConnTimeout").Get().GetNanoSeconds() != 3000000000, "Unexpected default initial default ConnTimeout.");
    int64_t connection_timeout_ns = 2 * worst_case_rtt_ns;
    Config::SetDefault("ns3::TcpSocket::ConnTimeout", TimeValue(NanoSeconds(connection_timeout_ns)));
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocket", "ConnTimeout").Get().GetNanoSeconds() != connection_timeout_ns, "Default initial ConnTimeout was not updated.");
    printf("  > Connection timeout......... %.3f ms\n", connection_timeout_ns / 1e6);

    // Delayed ACK timeout (ns-3 default: 0.2s, we set to 0.2 * worst-case-RTT)
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocket", "DelAckTimeout").Get().GetNanoSeconds() != 200000000, "Unexpected default initial default DelAckTimeout.");
    int64_t delayed_ack_timeout_ns = 0.2 * worst_case_rtt_ns;  // 0.2s is default
    Config::SetDefault("ns3::TcpSocket::DelAckTimeout", TimeValue(NanoSeconds(delayed_ack_timeout_ns)));
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocket", "DelAckTimeout").Get().GetNanoSeconds() != delayed_ack_timeout_ns, "Default initial DelAckTimeout was not updated.");
    printf("  > Delayed ACK timeout........ %.3f ms\n", delayed_ack_timeout_ns / 1e6);

    // Persist timeout (ns-3 default: 6s, we set to 4 * worst-case-RTT)
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocket", "PersistTimeout").Get().GetNanoSeconds() != 6000000000, "Unexpected default initial default PersistTimeout.");
    int64_t persist_timeout_ns = 4 * worst_case_rtt_ns;
    Config::SetDefault("ns3::TcpSocket::PersistTimeout", TimeValue(NanoSeconds(persist_timeout_ns)));
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocket", "PersistTimeout").Get().GetNanoSeconds() != persist_timeout_ns, "Default initial PersistTimeout was not updated.");
    printf("  > Persist timeout............ %.3f ms\n", persist_timeout_ns / 1e6);

    std::cout << std::endl;
    basicSimulation->RegisterTimestamp("Setup TCP parameters");
}

}
