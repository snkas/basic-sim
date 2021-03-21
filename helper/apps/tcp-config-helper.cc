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

#include "ns3/tcp-config-helper.h"

namespace ns3 {
    
void TcpConfigHelper::Configure(Ptr<BasicSimulation> basicSimulation) {

    // Heading
    std::cout << "TCP CONFIGURATION" << std::endl;

    // Retrieve configuration type (either "default", "basic" or "custom")
    std::string config_type = basicSimulation->GetConfigParamOrFail("tcp_config");

    // Perform the type's TCP configuration
    std::cout << "  > Type: " << config_type << std::endl;
    if (config_type == "default") {
        std::cout << "  > Not changing TCP configuration as ns-3 default is selected" << std::endl;
    } else if (config_type == "basic") {
        ConfigureBasic();
    } else if (config_type == "custom") {
        ConfigureCustom(basicSimulation);
    } else {
        throw std::runtime_error("Invalid TCP configuration type: " + config_type);
    }

    basicSimulation->RegisterTimestamp("Setup TCP configuration");
    std::cout << std::endl;
}

/**
 * The objective of "basic" is to configure the TCP stack as similar to default Linux as possible
 * combined with the peculiarities of the ns-3 implementation (thus sometimes resulting in deviations).
 */
void TcpConfigHelper::ConfigureBasic() {

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // CLOCK GRANULARITY
    //   > ns-3 default: 1ms
    //   > Linux default: <= 25ms, possible 1ms
    //   > Value chosen: 1ms
    //
    // Linux source:
    // The TCP Minimum RTO Revisited; Ioannis Psaras and Vassilis Tsaoussidis. States Linux <= 25ms.
    // https://tools.ietf.org/html/rfc7323 . States 1 ms to be the lowest option.

    int64_t clock_granularity_ns = 1000000;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // INITIAL CONGESTION WINDOW
    //   > ns-3 default: 10 packets
    //   > Linux default: 10 packets
    //   > Value chosen: 10 packets
    //
    // Linux source:
    // #define TCP_INIT_CWND		10
    // https://github.com/torvalds/linux/blob/master/include/net/tcp.h

    int64_t init_cwnd_pkt = 10;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // BUFFER SIZES
    //   > ns-3 default:
    //     >> Send buffer: 128 KiB = 131072 byte
    //     >> Receive buffer: 128 KiB = 131072 byte
    //   > Linux default:
    //     >> Send buffer: min = 4 KiB, default = 16 KiB, max = 4 MiB
    //     >> Receive buffer: min = 4 KiB, default = 87380 byte, max = 4 MiB
    //   > Ubuntu 20.04 default:
    //     >> Send buffer: min = 4 KiB, default = 16 KiB, max = 4 MiB
    //     >> Receive buffer: min = 4 KiB, default = 128 KiB, max = 6 MiB
    //   > Value chosen:
    //     >> Send buffer: 131072 * 8192 byte = 1 GiB
    //     >> Receive buffer: 131072 * 8192 byte = 1 GiB
    //
    // Linux source:
    // tcp_wmem (since Linux 2.4)
    // tcp_rmem (since Linux 2.4)
    // https://man7.org/linux/man-pages/man7/tcp.7.html
    //
    // Ubuntu source:
    // cat /proc/sys/net/ipv4/tcp_wmem
    // cat /proc/sys/net/ipv4/tcp_rmem
    //
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
    // only when (excl. header overhead in calculation):-
    //
    // For 10 Gbit/s at most with a base RTT of 859ms
    // For 1 Gbit/s at most with a base RTT of 8.59s
    // For 100 Mbit/s at most with a base RTT of 85.9s

    int64_t snd_buf_size_byte = 131072 * 8192;
    int64_t rcv_buf_size_byte = 131072 * 8192;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // SEGMENT SIZE
    //   > ns-3 default: 536
    //   > Linux default: 536
    //   > Value chosen: 1380
    //
    // Linux source:
    // #define TCP_MSS_DEFAULT		 536U	/* IPv4 (RFC1122, RFC2581) */
    // https://github.com/torvalds/linux/blob/master/include/uapi/linux/tcp.h
    //
    // However, we set it higher because we know the MTU is 1500 byte for point-to-point links.
    // IP header size: min. 20 byte, max. 60 byte
    // TCP header size: min. 20 byte, max. 60 byte
    // So, 1500 - 60 - 60 = 1380 would be the safest bet (given we don't do tunneling)
    // This could be increased higher, e.g. as discussed here:
    // https://blog.cloudflare.com/increasing-ipv6-mtu/ (retrieved April 7th, 2020)
    // In past ns-3 simulations, I've observed that the IP + TCP header is generally not larger than 80 bytes.
    // This means it could be potentially set closer to 1400-1420.

    int64_t segment_size_byte = 1380;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // TIMESTAMP OPTION
    //   > ns-3 default: true
    //   > Linux default: true
    //   > Value chosen: true
    //
    // Linux source:
    // tcp_timestamps (integer; default: 1; since Linux 2.2)
    // 1  Enable timestamps as defined in RFC1323
    // https://man7.org/linux/man-pages/man7/tcp.7.html
    //
    // To get an RTT measurement with a resolution of less than 1ms, it needs
    // to be disabled because the fields in the TCP Option are in milliseconds.
    // When disabling it, there are two downsides:
    //  (1) Less protection against wrapped sequence numbers (PAWS)
    //  (2) Algorithm to see if it has entered loss recovery unnecessarily are not as possible (Eiffel)
    // See: https://tools.ietf.org/html/rfc7323#section-3
    //      https://tools.ietf.org/html/rfc3522

    bool opt_timestamp_enabled = true;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // SELECTIVE ACKNOWLEDGMENT OPTION
    //    > ns-3 default: true
    //    > Linux default: true
    //    > Value chosen: true
    //
    // Linux source:
    // tcp_sack (Boolean; default: enabled; since Linux 2.2)
    // https://man7.org/linux/man-pages/man7/tcp.7.html
    //
    // Selective acknowledgment improves TCP performance, however can
    // be time-consuming to simulate because set operations and merges have
    // to be performed.

    bool opt_sack_enabled = true;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // WINDOW SCALING OPTION
    //   > ns-3 default: true
    //   > Linux default: true
    //   > Value chosen: true
    //
    // Linux source:
    // tcp_window_scaling (Boolean; default: enabled; since Linux 2.2)
    // https://man7.org/linux/man-pages/man7/tcp.7.html

    bool opt_win_scaling_enabled = true;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PACING
    //   > ns-3 default: false
    //   > Linux default: false
    //   > Value chosen: false
    //
    // Linux source:
    // sk_pacing enumeration has as zero value SK_PACING_NONE = 0.
    // https://github.com/torvalds/linux/blob/master/include/net/sock.h
    // The socket struct is initialized with zeroes and with SO_MAX_PACING_RATE goes from NONE to NEEDED
    // https://github.com/torvalds/linux/blob/master/net/core/sock.c

    bool pacing_enabled = false;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DELAYED ACK PACKET COUNT
    //   > ns-3 default: 2
    //   > Linux default: 2
    //   > Value chosen: 2
    //
    // Linux source:
    // "TCP_QUICKACK (since Linux 2.4.4)
    // Enable quickack mode if set or disable quickack mode if cleared.  In quickack mode, acks are sent
    // immediately, rather than delayed if needed in accordance to normal TCP operation."
    // -> by default delayed ACKs.
    // https://man7.org/linux/man-pages/man7/tcp.7.html

    int64_t delayed_ack_packet_count = 2;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // NO DELAY
    //   > ns-3 default: true
    //   > Linux default: false
    //   > Value chosen: true (we deviate from Linux because it is an option, and important for small flows)
    //
    // Linux source:
    // "TCP_NODELAY If set, disable the Nagle algorithm." -> Indicating it is not disabled by default.
    // https://man7.org/linux/man-pages/man7/tcp.7.html

    bool no_delay = true;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // MAXIMUM SEGMENT LIFETIME
    //   > ns-3 default: 120s
    //   > Linux default: 60-120s
    //   > Value chosen: 120s (take the maximum defined by Linux, corresponding to the ns-3 default)
    //
    // Linux source:
    // #define TCP_TIMEWAIT_LEN (60*HZ)
    // #define TCP_FIN_TIMEOUT	TCP_TIMEWAIT_LEN
    // #define TCP_FIN_TIMEOUT_MAX (120 * HZ) /* max TCP_LINGER2 value (two minutes) */
    // https://github.com/torvalds/linux/blob/master/include/net/tcp.h

    int64_t max_seg_lifetime_ns = 120000000000;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // MINIMUM RETRANSMISSION TIMEOUT
    //   > ns-3 default: 1s
    //   > Linux default: 200ms
    //   > Value chosen: 200ms
    //
    // Linux source:
    // #define TCP_RTO_MIN	((unsigned)(HZ/5))
    // https://github.com/torvalds/linux/blob/master/include/net/tcp.h

    int64_t min_rto_ns = 200000000;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // INITIAL ROUND-TRIP TIME (RTT) ESTIMATE
    //   > ns-3 default: 1s
    //   > Linux default: 1s
    //   > Value chosen: 1s
    //
    // Linux source: (we use the same as timeout, as ns-3 has it as well)
    // #define TCP_TIMEOUT_INIT ((unsigned)(1*HZ))
    // https://github.com/torvalds/linux/blob/master/include/net/tcp.h

    int64_t initial_rtt_estimate_ns = 1000000000;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // CONNECTION TIMEOUT
    //   > ns-3 default: 3s
    //   > Linux default: 1s
    //   > Value chosen: 1s
    //
    // A TCP connection starts with the initiator sending a SYN and awaiting a SYN+ACK.
    // It sets the retransmission timeout to:  2^(syn_reties) * conn_timeout
    // At each retry, it increments the syn_retries with 1.
    // When 6 SYN retries are reached, TCP gives up.
    // After 1s + 2s + 4s + 8s + 16s + 32s + 64s = 127s it will time out.
    //
    // Linux source:
    // #define TCP_TIMEOUT_INIT ((unsigned)(1*HZ))
    // https://github.com/torvalds/linux/blob/master/include/net/tcp.h

    int64_t connection_timeout_ns = 1000000000;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // DELAYED ACK TIMEOUT
    //   > ns-3 default: 200ms
    //   > Linux default: 200ms
    //   > Value chosen: 200ms
    //
    // Linux source:
    // #define TCP_DELACK_MAX	((unsigned)(HZ/5))	/* maximal time to delay before sending an ACK */
    // https://github.com/torvalds/linux/blob/master/include/net/tcp.h

    int64_t delayed_ack_timeout_ns = 200000000;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    // PERSIST TIMEOUT
    //   > ns-3 default: 6s
    //   > Linux default: Unknown (it might be that Linux uses Keep Alive instead, without the 1-byte)
    //   > Value chosen: 6s
    //
    // Alternative name: zero window probe (https://tools.ietf.org/html/rfc1122#section-4.2.2.17)
    //
    // Persist timeout is for the case where the receiver sends back a zero window size, indicating
    // that it cannot receive more data. The sender will then not be able to send more, until the
    // receiver informs the sender that it again has space via TCP Window Update packets -- which
    // might get lost. This timeout is such that the sender sends a 1-byte data packet to see if
    // the receiver can receive data again.
    //
    // In practice, as long as the receiver is consuming the incoming received data, this timeout would
    // not be triggered.

    int64_t persist_timeout_ns = 6000000000;

    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Configure using the values defined above
    SetTcpAttributes(
            clock_granularity_ns,
            init_cwnd_pkt,
            snd_buf_size_byte,
            rcv_buf_size_byte,
            segment_size_byte,
            opt_timestamp_enabled,
            opt_sack_enabled,
            opt_win_scaling_enabled,
            pacing_enabled,
            delayed_ack_packet_count,
            no_delay,
            max_seg_lifetime_ns,
            min_rto_ns,
            initial_rtt_estimate_ns,
            connection_timeout_ns,
            delayed_ack_timeout_ns,
            persist_timeout_ns
    );
    
}

void TcpConfigHelper::ConfigureCustom(Ptr<BasicSimulation> basicSimulation) {
    std::cout << "CUSTOM TCP CONFIGURATION" << std::endl;

    // Retrieve all values
    int64_t clock_granularity_ns = parse_positive_int64(basicSimulation->GetConfigParamOrFail("tcp_clock_granularity_ns"));
    int64_t init_cwnd_pkt = parse_positive_int64(basicSimulation->GetConfigParamOrFail("tcp_init_cwnd_pkt"));
    int64_t snd_buf_size_byte = parse_positive_int64(basicSimulation->GetConfigParamOrFail("tcp_snd_buf_size_byte"));
    int64_t rcv_buf_size_byte = parse_positive_int64(basicSimulation->GetConfigParamOrFail("tcp_rcv_buf_size_byte"));
    int64_t segment_size_byte = parse_positive_int64(basicSimulation->GetConfigParamOrFail("tcp_segment_size_byte"));
    bool opt_timestamp_enabled = parse_boolean(basicSimulation->GetConfigParamOrFail("tcp_opt_timestamp_enabled"));
    bool opt_sack_enabled = parse_boolean(basicSimulation->GetConfigParamOrFail("tcp_opt_sack_enabled"));
    bool opt_win_scaling_enabled = parse_boolean(basicSimulation->GetConfigParamOrFail("tcp_opt_win_scaling_enabled"));
    bool pacing_enabled = parse_boolean(basicSimulation->GetConfigParamOrFail("tcp_opt_pacing_enabled"));
    int64_t delayed_ack_packet_count = parse_positive_int64(basicSimulation->GetConfigParamOrFail("tcp_delayed_ack_packet_count"));
    bool no_delay = parse_boolean(basicSimulation->GetConfigParamOrFail("tcp_no_delay"));
    int64_t max_seg_lifetime_ns = parse_positive_int64(basicSimulation->GetConfigParamOrFail("tcp_max_seg_lifetime_ns"));
    int64_t min_rto_ns = parse_positive_int64(basicSimulation->GetConfigParamOrFail("tcp_min_rto_ns"));
    int64_t initial_rtt_estimate_ns = parse_positive_int64(basicSimulation->GetConfigParamOrFail("tcp_initial_rtt_estimate_ns"));
    int64_t connection_timeout_ns = parse_positive_int64(basicSimulation->GetConfigParamOrFail("tcp_connection_timeout_ns"));
    int64_t delayed_ack_timeout_ns = parse_positive_int64(basicSimulation->GetConfigParamOrFail("tcp_delayed_ack_timeout_ns"));
    int64_t persist_timeout_ns = parse_positive_int64(basicSimulation->GetConfigParamOrFail("tcp_persist_timeout_ns"));

    // Configure using the values retrieved above
    SetTcpAttributes(
            clock_granularity_ns,
            init_cwnd_pkt,
            snd_buf_size_byte,
            rcv_buf_size_byte,
            segment_size_byte,
            opt_timestamp_enabled,
            opt_sack_enabled,
            opt_win_scaling_enabled,
            pacing_enabled,
            delayed_ack_packet_count,
            no_delay,
            max_seg_lifetime_ns,
            min_rto_ns,
            initial_rtt_estimate_ns,
            connection_timeout_ns,
            delayed_ack_timeout_ns,
            persist_timeout_ns
    );

}

void TcpConfigHelper::SetTcpAttributes(
        int64_t clock_granularity_ns,
        int64_t init_cwnd_pkt,
        int64_t snd_buf_size_byte,
        int64_t rcv_buf_size_byte,
        int64_t segment_size_byte,
        bool opt_timestamp_enabled,
        bool opt_sack_enabled,
        bool opt_win_scaling_enabled,
        bool pacing_enabled,
        int64_t delayed_ack_packet_count,
        bool no_delay,
        int64_t max_seg_lifetime_ns,
        int64_t min_rto_ns,
        int64_t initial_rtt_estimate_ns,
        int64_t connection_timeout_ns,
        int64_t delayed_ack_timeout_ns,
        int64_t persist_timeout_ns
) {

    // Clock granularity (ns-3 default: 1ms)
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocketBase", "ClockGranularity").Get().GetNanoSeconds() != 1000000, "Unexpected default initial default ClockGranularity.");
    Config::SetDefault("ns3::TcpSocketBase::ClockGranularity", TimeValue(NanoSeconds(clock_granularity_ns)));
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocketBase", "ClockGranularity").Get().GetNanoSeconds() != clock_granularity_ns, "Default initial ClockGranularity was not updated.");
    printf("  > Clock granularity.......... %" PRId64 " ns\n", clock_granularity_ns);

    // Initial congestion window (ns-3 default: 10)
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "InitialCwnd") != 10, "Unexpected default initial default InitialCwnd.");
    Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue(init_cwnd_pkt));
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "InitialCwnd") != init_cwnd_pkt, "Default initial InitialCwnd was not updated.");
    printf("  > Initial CWND............... %" PRId64 " packets\n", init_cwnd_pkt);

    // Send buffer size (ns-3 default: 131072 bytes = 128 KiB)
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "SndBufSize") != 131072, "Unexpected default initial default SndBufSize.");
    Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue(snd_buf_size_byte));
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "SndBufSize") != snd_buf_size_byte, "Default initial SndBufSize was not updated.");
    printf("  > Send buffer size........... %.3f MB\n", snd_buf_size_byte / 1e6);

    // Receive buffer size (ns-3 default: 131072 bytes = 128 KiB)
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "RcvBufSize") != 131072, "Unexpected default initial default RcvBufSize.");
    Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue(rcv_buf_size_byte));
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "RcvBufSize") != rcv_buf_size_byte, "Default initial RcvBufSize was not updated.");
    printf("  > Receive buffer size........ %.3f MB\n", rcv_buf_size_byte / 1e6);

    // Segment size (ns-3 default: 536)
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "SegmentSize") != 536, "Unexpected default initial default SegmentSize.");
    Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue(segment_size_byte));
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "SegmentSize") != segment_size_byte, "Default initial SegmentSize was not updated.");
    printf("  > Segment size............... %" PRId64 " byte\n", segment_size_byte);

    // Timestamp option (ns-3 default: true)
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocketBase", "Timestamp") != true, "Unexpected default initial default Timestamp.");
    Config::SetDefault("ns3::TcpSocketBase::Timestamp", BooleanValue(opt_timestamp_enabled));
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocketBase", "Timestamp") != opt_timestamp_enabled, "Default initial Timestamp was not updated.");
    printf("  > Timestamp option........... %s\n", opt_timestamp_enabled ? "enabled" : "disabled");

    // SACK option (ns-3 default: true)
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocketBase", "Sack") != true, "Unexpected default initial default Sack.");
    Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue(opt_sack_enabled));
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocketBase", "Sack") != opt_sack_enabled, "Default initial Sack was not updated.");
    printf("  > SACK option................ %s\n", opt_sack_enabled ? "enabled" : "disabled");

    // Window scaling option (ns-3 default: true)
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocketBase", "WindowScaling") != true, "Unexpected default initial default WindowScaling.");
    Config::SetDefault("ns3::TcpSocketBase::WindowScaling", BooleanValue(opt_win_scaling_enabled));
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocketBase", "WindowScaling") != opt_win_scaling_enabled, "Default initial WindowScaling was not updated.");
    printf("  > Window scaling option...... %s\n", opt_win_scaling_enabled ? "enabled" : "disabled");

    // Pacing (ns-3 default: false)
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocketState", "EnablePacing") != false, "Unexpected default initial default EnablePacing.");
    Config::SetDefault("ns3::TcpSocketState::EnablePacing", BooleanValue(pacing_enabled));
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocketState", "EnablePacing") != pacing_enabled, "Default initial EnablePacing was not updated.");
    printf("  > Pacing..................... %s\n", pacing_enabled ? "enabled" : "disabled");

    // Number of packets to wait before sending a TCP ACK (ns-3 default: 2, we also keep it at 2)
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "DelAckCount") != 2, "Unexpected default initial default DelAckCount.");
    Config::SetDefault("ns3::TcpSocket::DelAckCount", UintegerValue(delayed_ack_packet_count));
    NS_ABORT_MSG_IF(GetInitialUintValue("ns3::TcpSocket", "DelAckCount") != delayed_ack_packet_count, "Default initial DelAckCount was not updated.");
    printf("  > Delayed ACK count.......... %" PRId64 "\n", delayed_ack_packet_count);

    // Whether to enable or disable Nagle's algorithm (ns-3 default: true, we also set to true)
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocket", "TcpNoDelay") != true, "Unexpected default initial default TcpNoDelay.");
    Config::SetDefault("ns3::TcpSocket::TcpNoDelay", BooleanValue(no_delay));
    NS_ABORT_MSG_IF(GetInitialBooleanValue("ns3::TcpSocket", "TcpNoDelay") != no_delay, "Default initial TcpNoDelay was not updated.");
    printf("  > No delay (disable Nagle)... %s\n", no_delay ? "enabled" : "disabled");

    // Maximum segment lifetime (ns-3 default: 120s)
    NS_ABORT_MSG_IF(GetInitialDoubleValue("ns3::TcpSocketBase", "MaxSegLifetime") != 120.0, "Unexpected default initial default MaxSegLifetime.");
    Config::SetDefault("ns3::TcpSocketBase::MaxSegLifetime", DoubleValue(max_seg_lifetime_ns / 1e9));
    NS_ABORT_MSG_IF(GetInitialDoubleValue("ns3::TcpSocketBase", "MaxSegLifetime") != max_seg_lifetime_ns / 1e9, "Default initial MaxSegLifetime was not updated.");
    printf("  > Maximum segment lifetime... %.3f ms\n", max_seg_lifetime_ns / 1e6);

    // Minimum retransmission timeout (ns-3 default: 1s)
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocketBase", "MinRto").Get().GetNanoSeconds() != 1000000000, "Unexpected default initial default MinRto.");
    Config::SetDefault("ns3::TcpSocketBase::MinRto", TimeValue(NanoSeconds(min_rto_ns)));
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocketBase", "MinRto").Get().GetNanoSeconds() != min_rto_ns, "Default initial MinRto was not updated.");
    printf("  > Minimum RTO................ %.3f ms\n", min_rto_ns / 1e6);

    // Initial RTT estimate (ns-3 default: 1s)
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::RttEstimator", "InitialEstimation").Get().GetNanoSeconds() != 1000000000, "Unexpected default initial default InitialEstimation.");
    Config::SetDefault("ns3::RttEstimator::InitialEstimation", TimeValue(NanoSeconds(initial_rtt_estimate_ns)));
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::RttEstimator", "InitialEstimation").Get().GetNanoSeconds() != initial_rtt_estimate_ns, "Default initial InitialEstimation was not updated.");
    printf("  > Initial RTT measurement.... %.3f ms\n", initial_rtt_estimate_ns / 1e6);

    // Connection timeout (ns-3 default: 3s)
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocket", "ConnTimeout").Get().GetNanoSeconds() != 3000000000, "Unexpected default initial default ConnTimeout.");
    Config::SetDefault("ns3::TcpSocket::ConnTimeout", TimeValue(NanoSeconds(connection_timeout_ns)));
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocket", "ConnTimeout").Get().GetNanoSeconds() != connection_timeout_ns, "Default initial ConnTimeout was not updated.");
    printf("  > Connection timeout......... %.3f ms\n", connection_timeout_ns / 1e6);

    // Delayed ACK timeout (ns-3 default: 0.2s)
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocket", "DelAckTimeout").Get().GetNanoSeconds() != 200000000, "Unexpected default initial default DelAckTimeout.");
    Config::SetDefault("ns3::TcpSocket::DelAckTimeout", TimeValue(NanoSeconds(delayed_ack_timeout_ns)));
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocket", "DelAckTimeout").Get().GetNanoSeconds() != delayed_ack_timeout_ns, "Default initial DelAckTimeout was not updated.");
    printf("  > Delayed ACK timeout........ %.3f ms\n", delayed_ack_timeout_ns / 1e6);

    // Persist timeout (ns-3 default: 6s)
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocket", "PersistTimeout").Get().GetNanoSeconds() != 6000000000, "Unexpected default initial default PersistTimeout.");
    Config::SetDefault("ns3::TcpSocket::PersistTimeout", TimeValue(NanoSeconds(persist_timeout_ns)));
    NS_ABORT_MSG_IF(GetInitialTimeValue("ns3::TcpSocket", "PersistTimeout").Get().GetNanoSeconds() != persist_timeout_ns, "Default initial PersistTimeout was not updated.");
    printf("  > Persist timeout............ %.3f ms\n", persist_timeout_ns / 1e6);

}

}
