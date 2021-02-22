/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Georgia Institute of Technology
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
 * Adapted from BulkSendApplication by:
 * Author: George F. Riley <riley@ece.gatech.edu>
 */

#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/string.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/tcp-tx-buffer.h"
#include "ns3/exp-util.h"
#include "tcp-flow-client.h"
#include <fstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpFlowClient");

NS_OBJECT_ENSURE_REGISTERED (TcpFlowClient);

TypeId
TcpFlowClient::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::TcpFlowClient")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<TcpFlowClient>()
            .AddAttribute("LocalAddress",
                          "The local address on which to bind the client socket (IPv4 address, port)",
                          AddressValue(),
                          MakeAddressAccessor(&TcpFlowClient::m_localAddress),
                          MakeAddressChecker())
            .AddAttribute("RemoteAddress",
                          "The address of the destination server (IPv4 address, port)",
                          AddressValue(),
                          MakeAddressAccessor(&TcpFlowClient::m_remoteAddress),
                          MakeAddressChecker())
            .AddAttribute("TcpFlowId",
                          "TCP flow identifier",
                          UintegerValue(0),
                          MakeUintegerAccessor(&TcpFlowClient::m_tcpFlowId),
                          MakeUintegerChecker<uint64_t>())
            .AddAttribute("FlowSizeByte",
                          "The total number of bytes to send",
                          UintegerValue(1),
                          MakeUintegerAccessor(&TcpFlowClient::m_flowSizeByte),
                          MakeUintegerChecker<uint64_t>(1))
            .AddAttribute ("AdditionalParameters",
                           "Additional parameters (unused; reserved for future use)",
                           StringValue (""),
                           MakeStringAccessor (&TcpFlowClient::m_additionalParameters),
                           MakeStringChecker ())
            .AddAttribute("EnableDetailedLoggingToFile",
                          "True iff you want to track some aspects (progress, rtt, rto, cwnd, inflated cwnd, "
                          "ssthresh, inflight, state, congestion state) of the TCP flow over time.",
                          BooleanValue(false),
                          MakeBooleanAccessor(&TcpFlowClient::m_enableDetailedLoggingToFile),
                          MakeBooleanChecker())
            .AddAttribute ("BaseLogsDir",
                           "Base logging directory (logging will be placed here, "
                           "i.e. logs_dir/tcp_flow_[flow id]_{progress, rtt, rto, cwnd, cwnd_inflated, ssthresh, "
                           "inflight, state, cong_state}.csv",
                           StringValue (""),
                           MakeStringAccessor (&TcpFlowClient::m_baseLogsDir),
                           MakeStringChecker ())
            .AddAttribute("SendStepSize", "The amount of data to send at each iteration step in the send loop.",
                          UintegerValue(100000),
                          MakeUintegerAccessor(&TcpFlowClient::m_sendStepSize),
                          MakeUintegerChecker<uint32_t>(1));
    return tid;
}


TcpFlowClient::TcpFlowClient()
        : m_socket(0),
          m_connected(false),
          m_totBytes(0),
          m_completionTimeNs(-1),
          m_connFailed(false),
          m_closedNormally(false),
          m_closedByError(false),
          m_ackedBytes(0),
          m_isCompleted(false) {
    NS_LOG_FUNCTION(this);
    m_tcpSocketGenerator = CreateObject<TcpSocketGeneratorDefault>();
}

TcpFlowClient::~TcpFlowClient() {
    NS_LOG_FUNCTION(this);
    m_tcpSocketGenerator = 0;
    m_socket = 0;
}

void
TcpFlowClient::DoDispose(void) {
    NS_LOG_FUNCTION(this);

    m_socket = 0;
    // chain up
    Application::DoDispose();
}

void
TcpFlowClient::SetTcpSocketGenerator(Ptr<TcpSocketGenerator> tcpSocketGenerator) {
    m_tcpSocketGenerator = tcpSocketGenerator;
}

void
TcpFlowClient::SetRemotePort(uint16_t remotePort) {
    NS_ABORT_MSG_UNLESS(InetSocketAddress::IsMatchingType(m_remoteAddress), "Only IPv4 is supported.");
    InetSocketAddress newRemoteAddress = InetSocketAddress::ConvertFrom(m_remoteAddress);
    newRemoteAddress.SetPort(remotePort);
    m_remoteAddress = newRemoteAddress;
}

void
TcpFlowClient::SetIpTos(uint8_t ipTos) {
    NS_ABORT_MSG_UNLESS(InetSocketAddress::IsMatchingType(m_localAddress), "Only IPv4 is supported.");
    InetSocketAddress newLocalAddress = InetSocketAddress::ConvertFrom(m_localAddress);
    newLocalAddress.SetTos(ipTos);
    m_localAddress = newLocalAddress;
    NS_ABORT_MSG_UNLESS(InetSocketAddress::IsMatchingType(m_remoteAddress), "Only IPv4 is supported.");
    InetSocketAddress newRemoteAddress = InetSocketAddress::ConvertFrom(m_remoteAddress);
    newRemoteAddress.SetTos(ipTos);
    m_remoteAddress = newRemoteAddress;
}

void TcpFlowClient::StartApplication(void) { // Called at time specified by Start
    NS_LOG_FUNCTION(this);

    // Create the socket if not already
    if (!m_socket) {
        m_socket = m_tcpSocketGenerator->GenerateTcpSocket(TcpFlowClient::GetTypeId(), this);

        // Bind socket
        NS_ABORT_MSG_UNLESS(InetSocketAddress::IsMatchingType(m_localAddress), "Only IPv4 is supported.");
        NS_ABORT_MSG_UNLESS(InetSocketAddress::IsMatchingType(m_remoteAddress), "Only IPv4 is supported.");
        if (m_socket->Bind(m_localAddress) == -1) {
            throw std::runtime_error("Failed to bind socket");
        }

        // Callbacks
        m_socket->SetConnectCallback(
                MakeCallback(&TcpFlowClient::ConnectionSucceeded, this),
                MakeCallback(&TcpFlowClient::ConnectionFailed, this)
        );
        m_socket->SetSendCallback(MakeCallback(&TcpFlowClient::DataSend, this));
        m_socket->SetCloseCallbacks(
                MakeCallback(&TcpFlowClient::SocketClosedNormal, this),
                MakeCallback(&TcpFlowClient::SocketClosedError, this)
        );
        if (m_enableDetailedLoggingToFile) {

            // Progress
            m_log_update_helper_progress_byte = LogUpdateHelper<int64_t>(false, true, m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_progress.csv", m_tcpFlowId), std::to_string(m_tcpFlowId) + ",");
            m_log_update_helper_progress_byte.Update(Simulator::Now().GetNanoSeconds(), GetAckedBytes());
            m_socket->GetObject<TcpSocketBase>()->GetTxBuffer()->TraceConnectWithoutContext("UnackSequence", MakeCallback(&TcpFlowClient::TxBufferUnackSequenceChange, this));

            // Measured RTT (ns)
            m_log_update_helper_rtt_ns = LogUpdateHelper<int64_t>(false, true, m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_rtt.csv", m_tcpFlowId), std::to_string(m_tcpFlowId) + ",");
            // At the socket creation, there is no RTT measurement, so retrieving it early will just yield 0
            // As such there "is" basically no RTT measurement till then, so we are not going to write 0
            m_socket->TraceConnectWithoutContext("RTT", MakeCallback(&TcpFlowClient::RttChange, this));

            // Retransmission timeout (ns)
            m_log_update_helper_rto_ns = LogUpdateHelper<int64_t>(false, true, m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_rto.csv", m_tcpFlowId), std::to_string(m_tcpFlowId) + ",");
            m_socket->TraceConnectWithoutContext("RTO", MakeCallback(&TcpFlowClient::RtoChange, this));

            // Congestion window
            m_log_update_helper_cwnd_byte = LogUpdateHelper<int64_t>(false, true, m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_cwnd.csv", m_tcpFlowId), std::to_string(m_tcpFlowId) + ",");
            // Congestion window is only set upon SYN reception, so retrieving it early will just yield 0
            // As such there "is" basically no congestion window till then, so we are not going to write 0
            m_socket->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&TcpFlowClient::CwndChange, this));

            // Congestion window inflated
            m_log_update_helper_cwnd_inflated_byte = LogUpdateHelper<int64_t>(false, true, m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_cwnd_inflated.csv", m_tcpFlowId), std::to_string(m_tcpFlowId) + ",");
            m_socket->TraceConnectWithoutContext("CongestionWindowInflated", MakeCallback(&TcpFlowClient::CwndInflatedChange, this));

            // Slow-start threshold
            m_log_update_helper_ssthresh_byte = LogUpdateHelper<int64_t>(false, true, m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_ssthresh.csv", m_tcpFlowId), std::to_string(m_tcpFlowId) + ",");
            m_socket->TraceConnectWithoutContext("SlowStartThreshold", MakeCallback(&TcpFlowClient::SsthreshChange, this));

            // In-flight
            m_log_update_helper_inflight_byte = LogUpdateHelper<int64_t>(false, true, m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_inflight.csv", m_tcpFlowId), std::to_string(m_tcpFlowId) + ",");
            m_socket->TraceConnectWithoutContext("BytesInFlight", MakeCallback(&TcpFlowClient::InflightChange, this));

            // State
            m_log_update_helper_state = LogUpdateHelper<std::string>(false, true, m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_state.csv", m_tcpFlowId), std::to_string(m_tcpFlowId) + ",");
            StateChange(TcpSocket::TcpStates_t::CLOSED, TcpSocket::TcpStates_t::CLOSED); // Default at start is CLOSED
            m_socket->TraceConnectWithoutContext("State", MakeCallback(&TcpFlowClient::StateChange, this));

            // CongState
            m_log_update_helper_cong_state = LogUpdateHelper<std::string>(false, true, m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_cong_state.csv", m_tcpFlowId), std::to_string(m_tcpFlowId) + ",");
            CongStateChange(TcpSocketState::TcpCongState_t::CA_OPEN, TcpSocketState::TcpCongState_t::CA_OPEN); // Default at start is CA_OPEN
            m_socket->TraceConnectWithoutContext("CongState", MakeCallback(&TcpFlowClient::CongStateChange, this));

        }

        // Connect
        if (m_socket->Connect(m_remoteAddress) == -1) {
            // If the connection error code is -1, the connection has already failed,
            // typically because of no route being available
            ConnectionFailed(m_socket);
            return;
        }

        // No receiving necessary
        m_socket->ShutdownRecv();

    }
    if (m_connected) {
        SendData();
    }
}

void TcpFlowClient::StopApplication(void) { // Called at time specified by Stop
    throw std::runtime_error("TCP flow client cannot be stopped like a regular application, "
                             "it finished only by the socket closing.");
}

void TcpFlowClient::SendData(void) {
    NS_LOG_FUNCTION(this);
    while (m_totBytes < m_flowSizeByte) { // As long as not everything has been put into the L4 buffer

        // How much to put into the buffer in this iteration
        uint64_t toSend = std::min((uint64_t) m_sendStepSize, m_flowSizeByte - m_totBytes);

        // Create a "packet" with the amount of data and send it to the TCP socket
        // The packet will be taken apart by the TCP socket and put into the buffer in segment size pieces
        // The sending returns the amount it was able to put into the buffer
        NS_LOG_LOGIC("sending packet at " << Simulator::Now());
        Ptr <Packet> packet = Create<Packet>(toSend);
        int actual = m_socket->Send(packet);
        if (actual > 0) {
            m_totBytes += actual;
        }

        // We exit this loop when actual < toSend as the send side
        // buffer is full. The "DataSent" callback will pop when
        // some buffer space has freed up.
        if ((unsigned) actual != toSend) {
            break;
        }

    }

    // Check if it is time to close (all data has been put into the L4 buffer)
    if (m_totBytes == m_flowSizeByte && m_connected) {
        m_socket->Close(); // Close will only happen after send buffer is finished
        m_connected = false;
    }
}

void TcpFlowClient::ConnectionSucceeded(Ptr <Socket> socket) {
    NS_LOG_FUNCTION(this << socket);
    NS_LOG_LOGIC("TcpFlowClient Connection succeeded");
    m_connected = true;
    SendData();
}

void TcpFlowClient::ConnectionFailed(Ptr <Socket> socket) {
    NS_LOG_FUNCTION(this << socket);
    NS_LOG_LOGIC("TcpFlowClient, Connection Failed");
    m_completionTimeNs = Simulator::Now().GetNanoSeconds();
    m_connFailed = true;
    m_closedByError = false;
    m_closedNormally = false;
    m_ackedBytes = 0;
    m_isCompleted = false;
    m_socket = 0;
}

void TcpFlowClient::DataSend(Ptr <Socket>, uint32_t) {
    NS_LOG_FUNCTION(this);
    if (m_connected) { // Only send new data if the connection has completed
        SendData();
    }
}

void TcpFlowClient::SocketClosedNormal(Ptr <Socket> socket) {
    m_completionTimeNs = Simulator::Now().GetNanoSeconds();
    m_connFailed = false;
    m_closedByError = false;
    m_closedNormally = true;
    NS_ABORT_MSG_IF(m_socket->GetObject<TcpSocketBase>()->GetTxBuffer()->Size() != 0, "Socket closed normally but send buffer is not empty");
    m_ackedBytes = m_totBytes - m_socket->GetObject<TcpSocketBase>()->GetTxBuffer()->Size();
    m_isCompleted = m_ackedBytes == m_flowSizeByte;
    m_socket = 0;
}

void TcpFlowClient::SocketClosedError(Ptr <Socket> socket) {
    m_completionTimeNs = Simulator::Now().GetNanoSeconds();
    m_connFailed = false;
    m_closedByError = true;
    m_closedNormally = false;
    m_ackedBytes = m_totBytes - m_socket->GetObject<TcpSocketBase>()->GetTxBuffer()->Size();
    m_isCompleted = false;
    m_socket = 0;
}

int64_t TcpFlowClient::GetAckedBytes() {
    if (m_connFailed || m_closedNormally || m_closedByError) {
        return m_ackedBytes;
    } else {
        return m_totBytes - m_socket->GetObject<TcpSocketBase>()->GetTxBuffer()->Size();
    }
}

Ptr<Socket> TcpFlowClient::GetSocket() {
    return m_socket;
}

int64_t TcpFlowClient::GetCompletionTimeNs() {
    return m_completionTimeNs;
}

bool TcpFlowClient::IsCompleted() {
    return m_isCompleted;
}

bool TcpFlowClient::IsConnFailed() {
    return m_connFailed;
}

bool TcpFlowClient::IsClosedNormally() {
    return m_closedNormally;
}

bool TcpFlowClient::IsClosedByError() {
    return m_closedByError;
}

void TcpFlowClient::TxBufferUnackSequenceChange (SequenceNumber32, SequenceNumber32) {
    // Note that we do not make use of the arguments, as the sequence number
    // can loop around for transmission larger than 4 GiB. As such, we make
    // use of the GetAckedBytes () function instead. GetAckedBytes () works because the
    // Size () of the TcpTxBuffer (m_size) is updated before the sequence number
    // is increased.
    if (m_enableDetailedLoggingToFile) {
        m_log_update_helper_progress_byte.Update(Simulator::Now().GetNanoSeconds (), GetAckedBytes());
    }
}

void
TcpFlowClient::RttChange (Time oldRtt, Time newRtt)
{
    m_log_update_helper_rtt_ns.Update(Simulator::Now().GetNanoSeconds(), newRtt.GetNanoSeconds());
}

void
TcpFlowClient::RtoChange (Time oldRto, Time newRto)
{
    m_log_update_helper_rto_ns.Update(Simulator::Now().GetNanoSeconds(), newRto.GetNanoSeconds());
}

void
TcpFlowClient::CwndChange(uint32_t, uint32_t newCwnd)
{
    m_log_update_helper_cwnd_byte.Update(Simulator::Now().GetNanoSeconds(), newCwnd);
}

void
TcpFlowClient::CwndInflatedChange(uint32_t, uint32_t newCwndInflated)
{
    m_log_update_helper_cwnd_inflated_byte.Update(Simulator::Now().GetNanoSeconds(), newCwndInflated);
}

void
TcpFlowClient::SsthreshChange(uint32_t oldCwnd, uint32_t newSsthresh)
{
    m_log_update_helper_ssthresh_byte.Update(Simulator::Now().GetNanoSeconds(), newSsthresh);
}

void
TcpFlowClient::InflightChange(uint32_t, uint32_t newInflight)
{
    m_log_update_helper_inflight_byte.Update(Simulator::Now().GetNanoSeconds(), newInflight);
}

void
TcpFlowClient::StateChange(TcpSocket::TcpStates_t, TcpSocket::TcpStates_t newState)
{
    std::string newStateString;
    switch (newState) {
        case TcpSocket::TcpStates_t::CLOSED:
            newStateString = "CLOSED";
            break;
        case TcpSocket::TcpStates_t::LISTEN:
            newStateString = "LISTEN";
            break;
        case TcpSocket::TcpStates_t::SYN_SENT:
            newStateString = "SYN_SENT";
            break;
        case TcpSocket::TcpStates_t::SYN_RCVD:
            newStateString = "SYN_RCVD";
            break;
        case TcpSocket::TcpStates_t::ESTABLISHED:
            newStateString = "ESTABLISHED";
            break;
        case TcpSocket::TcpStates_t::CLOSE_WAIT:
            newStateString = "CLOSE_WAIT";
            break;
        case TcpSocket::TcpStates_t::LAST_ACK:
            newStateString = "LAST_ACK";
            break;
        case TcpSocket::TcpStates_t::FIN_WAIT_1:
            newStateString = "FIN_WAIT_1";
            break;
        case TcpSocket::TcpStates_t::FIN_WAIT_2:
            newStateString = "FIN_WAIT_2";
            break;
        case TcpSocket::TcpStates_t::CLOSING:
            newStateString = "CLOSING";
            break;
        case TcpSocket::TcpStates_t::TIME_WAIT:
            newStateString = "TIME_WAIT";
            break;
        case TcpSocket::TcpStates_t::LAST_STATE:
            newStateString = "LAST_STATE";
            break;
    }
    m_log_update_helper_state.Update(Simulator::Now().GetNanoSeconds(), newStateString);
}

void
TcpFlowClient::CongStateChange(TcpSocketState::TcpCongState_t, TcpSocketState::TcpCongState_t newCongState)
{
    std::string newCongStateString;
    switch (newCongState) {
        case TcpSocketState::TcpCongState_t::CA_OPEN:
            newCongStateString = "CA_OPEN";
            break;
        case TcpSocketState::TcpCongState_t::CA_DISORDER:
            newCongStateString = "CA_DISORDER";
            break;
        case TcpSocketState::TcpCongState_t::CA_CWR:
            newCongStateString = "CA_CWR";
            break;
        case TcpSocketState::TcpCongState_t::CA_RECOVERY:
            newCongStateString = "CA_RECOVERY";
            break;
        case TcpSocketState::TcpCongState_t::CA_LOSS:
            newCongStateString = "CA_LOSS";
            break;
        case TcpSocketState::TcpCongState_t::CA_LAST_STATE:
            newCongStateString = "CA_LAST_STATE";
            break;
    }
    m_log_update_helper_cong_state.Update(Simulator::Now().GetNanoSeconds(), newCongStateString);
}

void
TcpFlowClient::FinalizeDetailedLogs() {
    if (m_enableDetailedLoggingToFile) {

        // The following traced variables exist really
        // only when the connection is alive
        int64_t timestamp;
        if (m_connFailed || m_closedByError || m_closedNormally) {
            timestamp = m_completionTimeNs;
        } else {
            timestamp = Simulator::Now().GetNanoSeconds ();
        }
        m_log_update_helper_progress_byte.Finalize(timestamp);
        m_log_update_helper_rtt_ns.Finalize(timestamp);
        m_log_update_helper_rto_ns.Finalize(timestamp);
        m_log_update_helper_cwnd_byte.Finalize(timestamp);
        m_log_update_helper_cwnd_inflated_byte.Finalize(timestamp);
        m_log_update_helper_ssthresh_byte.Finalize(timestamp);
        m_log_update_helper_inflight_byte.Finalize(timestamp);

        // The state effectively is perpetual -- for example, after
        // finishing successfully the state will always be CLOSED
        m_log_update_helper_state.Finalize(Simulator::Now().GetNanoSeconds ());
        m_log_update_helper_cong_state.Finalize(Simulator::Now().GetNanoSeconds ());

    }
}

uint64_t TcpFlowClient::GetTcpFlowId() {
    return m_tcpFlowId;
}

std::string TcpFlowClient::GetAdditionalParameters() {
    return m_additionalParameters;
}

} // Namespace ns3
