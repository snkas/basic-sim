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
#include "tcp-flow-send-application.h"
#include <fstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpFlowSendApplication");

NS_OBJECT_ENSURE_REGISTERED (TcpFlowSendApplication);

TypeId
TcpFlowSendApplication::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::TcpFlowSendApplication")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<TcpFlowSendApplication>()
            .AddAttribute("SendSize", "The amount of data to send each time.",
                          UintegerValue(100000),
                          MakeUintegerAccessor(&TcpFlowSendApplication::m_sendSize),
                          MakeUintegerChecker<uint32_t>(1))
            .AddAttribute("Remote", "The address of the destination (IPv4 address, port)",
                          AddressValue(),
                          MakeAddressAccessor(&TcpFlowSendApplication::m_peer),
                          MakeAddressChecker())
            .AddAttribute("MaxBytes",
                          "The total number of bytes to send. "
                          "Once these bytes are sent, "
                          "no data  is sent again. The value zero means "
                          "that there is no limit.",
                          UintegerValue(0),
                          MakeUintegerAccessor(&TcpFlowSendApplication::m_maxBytes),
                          MakeUintegerChecker<uint64_t>())
            .AddAttribute("TcpFlowId",
                          "TCP flow identifier",
                          UintegerValue(0),
                          MakeUintegerAccessor(&TcpFlowSendApplication::m_tcpFlowId),
                          MakeUintegerChecker<uint64_t>())
            .AddAttribute("EnableTcpFlowLoggingToFile",
                          "True iff you want to track some aspects (progress, rtt, rto, cwnd, inflated cwnd, "
                          "ssthresh, inflight, state, congestion state) of the TCP flow over time.",
                          BooleanValue(true),
                          MakeBooleanAccessor(&TcpFlowSendApplication::m_enableDetailedLogging),
                          MakeBooleanChecker())
            .AddAttribute ("BaseLogsDir",
                           "Base logging directory (logging will be placed here, "
                           "i.e. logs_dir/tcp_flow_[flow id]_{progress, rtt, rto, cwnd, cwnd_inflated, ssthresh, "
                           "inflight, state, cong_state}.csv",
                           StringValue (""),
                           MakeStringAccessor (&TcpFlowSendApplication::m_baseLogsDir),
                           MakeStringChecker ())
            .AddAttribute ("AdditionalParameters",
                           "Additional parameter string; this might be parsed in another version of this application "
                           "to do slightly different behavior (e.g., set priority on TCP socket etc.)",
                           StringValue (""),
                           MakeStringAccessor (&TcpFlowSendApplication::m_additionalParameters),
                           MakeStringChecker ())
            .AddTraceSource("Tx", "A new packet is created and is sent",
                            MakeTraceSourceAccessor(&TcpFlowSendApplication::m_txTrace),
                            "ns3::Packet::TracedCallback");
    return tid;
}


TcpFlowSendApplication::TcpFlowSendApplication()
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
}

TcpFlowSendApplication::~TcpFlowSendApplication() {
    NS_LOG_FUNCTION(this);
}

void
TcpFlowSendApplication::DoDispose(void) {
    NS_LOG_FUNCTION(this);

    m_socket = 0;
    // chain up
    Application::DoDispose();
}

void TcpFlowSendApplication::StartApplication(void) { // Called at time specified by Start
    NS_LOG_FUNCTION(this);

    // Create the socket if not already
    if (!m_socket) {
       m_socket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());

        // Bind socket
        NS_ABORT_MSG_UNLESS(InetSocketAddress::IsMatchingType(m_peer), "Only IPv4 is supported.");
        if (m_socket->Bind() == -1) {
            throw std::runtime_error("Failed to bind socket");
        }

        // Callbacks
        m_socket->SetConnectCallback(
                MakeCallback(&TcpFlowSendApplication::ConnectionSucceeded, this),
                MakeCallback(&TcpFlowSendApplication::ConnectionFailed, this)
        );
        m_socket->SetSendCallback(MakeCallback(&TcpFlowSendApplication::DataSend, this));
        m_socket->SetCloseCallbacks(
                MakeCallback(&TcpFlowSendApplication::SocketClosedNormal, this),
                MakeCallback(&TcpFlowSendApplication::SocketClosedError, this)
        );
        if (m_enableDetailedLogging) {

            // Progress
            m_log_update_helper_progress_byte = LogUpdateHelper<int64_t>(false, true, m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_progress.csv", m_tcpFlowId), std::to_string(m_tcpFlowId) + ",");
            m_log_update_helper_progress_byte.Update(Simulator::Now().GetNanoSeconds(), GetAckedBytes());

            // Measured RTT (ns)
            m_log_update_helper_rtt_ns = LogUpdateHelper<int64_t>(false, true, m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_rtt.csv", m_tcpFlowId), std::to_string(m_tcpFlowId) + ",");
            // At the socket creation, there is no RTT measurement, so retrieving it early will just yield 0
            // As such there "is" basically no RTT measurement till then, so we are not going to write 0
            m_socket->TraceConnectWithoutContext("RTT", MakeCallback(&TcpFlowSendApplication::RttChange, this));

            // Retransmission timeout (ns)
            m_log_update_helper_rto_ns = LogUpdateHelper<int64_t>(false, true, m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_rto.csv", m_tcpFlowId), std::to_string(m_tcpFlowId) + ",");
            m_socket->TraceConnectWithoutContext("RTO", MakeCallback(&TcpFlowSendApplication::RtoChange, this));

            // Congestion window
            m_log_update_helper_cwnd_byte = LogUpdateHelper<int64_t>(false, true, m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_cwnd.csv", m_tcpFlowId), std::to_string(m_tcpFlowId) + ",");
            // Congestion window is only set upon SYN reception, so retrieving it early will just yield 0
            // As such there "is" basically no congestion window till then, so we are not going to write 0
            m_socket->TraceConnectWithoutContext("CongestionWindow", MakeCallback(&TcpFlowSendApplication::CwndChange, this));

            // Congestion window inflated
            m_log_update_helper_cwnd_inflated_byte = LogUpdateHelper<int64_t>(false, true, m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_cwnd_inflated.csv", m_tcpFlowId), std::to_string(m_tcpFlowId) + ",");
            m_socket->TraceConnectWithoutContext("CongestionWindowInflated", MakeCallback(&TcpFlowSendApplication::CwndInflatedChange, this));

            // Slow-start threshold
            m_log_update_helper_ssthresh_byte = LogUpdateHelper<int64_t>(false, true, m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_ssthresh.csv", m_tcpFlowId), std::to_string(m_tcpFlowId) + ",");
            m_socket->TraceConnectWithoutContext("SlowStartThreshold", MakeCallback(&TcpFlowSendApplication::SsthreshChange, this));

            // In-flight
            m_log_update_helper_inflight_byte = LogUpdateHelper<int64_t>(false, true, m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_inflight.csv", m_tcpFlowId), std::to_string(m_tcpFlowId) + ",");
            m_socket->TraceConnectWithoutContext("BytesInFlight", MakeCallback(&TcpFlowSendApplication::InflightChange, this));

            // State
            m_log_update_helper_state = LogUpdateHelper<std::string>(false, true, m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_state.csv", m_tcpFlowId), std::to_string(m_tcpFlowId) + ",");
            StateChange(TcpSocket::TcpStates_t::CLOSED, TcpSocket::TcpStates_t::CLOSED); // Default at start is CLOSED
            m_socket->TraceConnectWithoutContext("State", MakeCallback(&TcpFlowSendApplication::StateChange, this));

            // CongState
            m_log_update_helper_cong_state = LogUpdateHelper<std::string>(false, true, m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_cong_state.csv", m_tcpFlowId), std::to_string(m_tcpFlowId) + ",");
            CongStateChange(TcpSocketState::TcpCongState_t::CA_OPEN, TcpSocketState::TcpCongState_t::CA_OPEN); // Default at start is CA_OPEN
            m_socket->TraceConnectWithoutContext("CongState", MakeCallback(&TcpFlowSendApplication::CongStateChange, this));

        }

        // Connect
        if (m_socket->Connect(m_peer) == -1) {
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

void TcpFlowSendApplication::StopApplication(void) { // Called at time specified by Stop
    throw std::runtime_error("TCP flow send application is not intended to be stopped after being started.");
    /*
     * Deprecated stop code:
     *
     * NS_LOG_FUNCTION(this);
     * if (m_socket != 0) {
     *    m_socket->Close();
     *    m_connected = false;
     * } else {
     *    NS_LOG_WARN("TcpFlowSendApplication found null socket to close in StopApplication");
     * }
     */
}

void TcpFlowSendApplication::SendData(void) {
    NS_LOG_FUNCTION(this);
    while (m_maxBytes == 0 || m_totBytes < m_maxBytes) { // Time to send more

        // uint64_t to allow the comparison later.
        // the result is in a uint32_t range anyway, because
        // m_sendSize is uint32_t.
        uint64_t toSend = m_sendSize;
        // Make sure we don't send too many
        if (m_maxBytes > 0) {
            toSend = std::min(toSend, m_maxBytes - m_totBytes);
        }

        NS_LOG_LOGIC("sending packet at " << Simulator::Now());
        Ptr <Packet> packet = Create<Packet>(toSend);
        int actual = m_socket->Send(packet);
        if (actual > 0) {
            m_totBytes += actual;
            m_txTrace(packet);
        }
        // We exit this loop when actual < toSend as the send side
        // buffer is full. The "DataSent" callback will pop when
        // some buffer space has freed up.
        if ((unsigned) actual != toSend) {
            break;
        }
    }
    // Check if time to close (all sent)
    if (m_totBytes == m_maxBytes && m_connected) {
        m_socket->Close(); // Close will only happen after send buffer is finished
        m_connected = false;
    }
}

void TcpFlowSendApplication::ConnectionSucceeded(Ptr <Socket> socket) {
    NS_LOG_FUNCTION(this << socket);
    NS_LOG_LOGIC("TcpFlowSendApplication Connection succeeded");
    m_connected = true;
    SendData();
}

void TcpFlowSendApplication::ConnectionFailed(Ptr <Socket> socket) {
    NS_LOG_FUNCTION(this << socket);
    NS_LOG_LOGIC("TcpFlowSendApplication, Connection Failed");
    m_completionTimeNs = Simulator::Now().GetNanoSeconds();
    m_connFailed = true;
    m_closedByError = false;
    m_closedNormally = false;
    m_ackedBytes = 0;
    m_isCompleted = false;
    m_socket = 0;
}

void TcpFlowSendApplication::DataSend(Ptr <Socket>, uint32_t) {
    NS_LOG_FUNCTION(this);
    if (m_connected) { // Only send new data if the connection has completed
        SendData();
    }

    // Log the progress as DataSend() is called anytime space in the transmission buffer frees up
    if (m_enableDetailedLogging) {
        m_log_update_helper_progress_byte.Update(Simulator::Now().GetNanoSeconds (), GetAckedBytes());
    }

}

void TcpFlowSendApplication::SocketClosedNormal(Ptr <Socket> socket) {
    m_completionTimeNs = Simulator::Now().GetNanoSeconds();
    m_connFailed = false;
    m_closedByError = false;
    m_closedNormally = true;
    NS_ABORT_MSG_IF(m_socket->GetObject<TcpSocketBase>()->GetTxBuffer()->Size() != 0, "Socket closed normally but send buffer is not empty");
    m_ackedBytes = m_totBytes - m_socket->GetObject<TcpSocketBase>()->GetTxBuffer()->Size();
    m_isCompleted = m_ackedBytes == m_maxBytes;
    m_socket = 0;
}

void TcpFlowSendApplication::SocketClosedError(Ptr <Socket> socket) {
    m_completionTimeNs = Simulator::Now().GetNanoSeconds();
    m_connFailed = false;
    m_closedByError = true;
    m_closedNormally = false;
    m_ackedBytes = m_totBytes - m_socket->GetObject<TcpSocketBase>()->GetTxBuffer()->Size();
    m_isCompleted = false;
    m_socket = 0;
}

int64_t TcpFlowSendApplication::GetAckedBytes() {
    if (m_connFailed || m_closedNormally || m_closedByError) {
        return m_ackedBytes;
    } else {
        return m_totBytes - m_socket->GetObject<TcpSocketBase>()->GetTxBuffer()->Size();
    }
}

Ptr<Socket> TcpFlowSendApplication::GetSocket() {
    return m_socket;
}

int64_t TcpFlowSendApplication::GetCompletionTimeNs() {
    return m_completionTimeNs;
}

bool TcpFlowSendApplication::IsCompleted() {
    return m_isCompleted;
}

bool TcpFlowSendApplication::IsConnFailed() {
    return m_connFailed;
}

bool TcpFlowSendApplication::IsClosedNormally() {
    return m_closedNormally;
}

bool TcpFlowSendApplication::IsClosedByError() {
    return m_closedByError;
}

void
TcpFlowSendApplication::RttChange (Time oldRtt, Time newRtt)
{
    m_log_update_helper_rtt_ns.Update(Simulator::Now().GetNanoSeconds(), newRtt.GetNanoSeconds());
}

void
TcpFlowSendApplication::RtoChange (Time oldRto, Time newRto)
{
    m_log_update_helper_rto_ns.Update(Simulator::Now().GetNanoSeconds(), newRto.GetNanoSeconds());
}

void
TcpFlowSendApplication::CwndChange(uint32_t, uint32_t newCwnd)
{
    m_log_update_helper_cwnd_byte.Update(Simulator::Now().GetNanoSeconds(), newCwnd);
}

void
TcpFlowSendApplication::CwndInflatedChange(uint32_t, uint32_t newCwndInflated)
{
    m_log_update_helper_cwnd_inflated_byte.Update(Simulator::Now().GetNanoSeconds(), newCwndInflated);
}

void
TcpFlowSendApplication::SsthreshChange(uint32_t oldCwnd, uint32_t newSsthresh)
{
    m_log_update_helper_ssthresh_byte.Update(Simulator::Now().GetNanoSeconds(), newSsthresh);
}

void
TcpFlowSendApplication::InflightChange(uint32_t, uint32_t newInflight)
{
    m_log_update_helper_inflight_byte.Update(Simulator::Now().GetNanoSeconds(), newInflight);
}

void
TcpFlowSendApplication::StateChange(TcpSocket::TcpStates_t, TcpSocket::TcpStates_t newState)
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
TcpFlowSendApplication::CongStateChange(TcpSocketState::TcpCongState_t, TcpSocketState::TcpCongState_t newCongState)
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
TcpFlowSendApplication::FinalizeDetailedLogs() {
    if (m_enableDetailedLogging) {

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

} // Namespace ns3
