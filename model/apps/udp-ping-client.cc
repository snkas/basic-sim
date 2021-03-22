/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
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
 */

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "udp-ping-client.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpPingClient");

NS_OBJECT_ENSURE_REGISTERED (UdpPingClient);

TypeId
UdpPingClient::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::UdpPingClient")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<UdpPingClient>()
            .AddAttribute("LocalAddress",
                          "The local address (IPv4 address, port). Setting the IPv4 address will enable "
                          "proper ECMP routing (as else it forces an early lookup with only destination IP). "
                          "Setting the port is not necessary, as it will be assigned an ephemeral one.",
                          AddressValue(),
                          MakeAddressAccessor(&UdpPingClient::m_localAddress),
                          MakeAddressChecker())
            .AddAttribute("RemoteAddress",
                          "The address of the destination server (IPv4 address, port)",
                          AddressValue(),
                          MakeAddressAccessor(&UdpPingClient::m_remoteAddress),
                          MakeAddressChecker())
            .AddAttribute("UdpPingId",
                          "Unique UDP ping identifier",
                          UintegerValue(0),
                          MakeUintegerAccessor(&UdpPingClient::m_udpPingId),
                          MakeUintegerChecker<uint64_t>())
            .AddAttribute("Interval",
                          "The time to wait between sending pings",
                          TimeValue(Seconds(1.0)),
                          MakeTimeAccessor(&UdpPingClient::m_interval),
                          MakeTimeChecker())
            .AddAttribute("Duration",
                          "How long to do the pinging",
                          TimeValue(Seconds(10.0)),
                          MakeTimeAccessor(&UdpPingClient::m_duration),
                          MakeTimeChecker())
            .AddAttribute("WaitAfterwards",
                          "How long to wait after the duration is over",
                          TimeValue(Seconds(1.0)),
                          MakeTimeAccessor(&UdpPingClient::m_waitAfterwards),
                          MakeTimeChecker())
            .AddAttribute ("AdditionalParameters",
                           "Additional parameters (unused; reserved for future use)",
                           StringValue (""),
                           MakeStringAccessor (&UdpPingClient::m_additionalParameters),
                           MakeStringChecker());
    return tid;
}

UdpPingClient::UdpPingClient() {
    NS_LOG_FUNCTION(this);
    m_udpSocketGenerator = CreateObject<UdpSocketGeneratorDefault>();
    m_socket = 0;
    m_sendEvent = EventId();
    m_waitForFinishEvent = EventId();
    m_sent = 0;
}

UdpPingClient::~UdpPingClient() {
    NS_LOG_FUNCTION(this);
    m_udpSocketGenerator = 0;
    m_socket = 0;
}

void
UdpPingClient::DoDispose(void) {
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
}

void
UdpPingClient::SetUdpSocketGenerator(Ptr<UdpSocketGenerator> udpSocketGenerator) {
    m_udpSocketGenerator = udpSocketGenerator;
}

void
UdpPingClient::SetIpTos(uint8_t ipTos) {
    NS_ABORT_MSG_UNLESS(InetSocketAddress::IsMatchingType(m_localAddress), "Only IPv4 is supported.");
    InetSocketAddress newLocalAddress = InetSocketAddress::ConvertFrom(m_localAddress);
    newLocalAddress.SetTos(ipTos);
    m_localAddress = newLocalAddress;
    NS_ABORT_MSG_UNLESS(InetSocketAddress::IsMatchingType(m_remoteAddress), "Only IPv4 is supported.");
    InetSocketAddress newRemoteAddress = InetSocketAddress::ConvertFrom(m_remoteAddress);
    newRemoteAddress.SetTos(ipTos);
    m_remoteAddress = newRemoteAddress;
}

void
UdpPingClient::StartApplication(void) {
    NS_LOG_FUNCTION(this);
    if (m_socket == 0) {
        m_socket = m_udpSocketGenerator->GenerateUdpSocket(UdpPingClient::GetTypeId(), this);

        // Bind socket
        NS_ABORT_MSG_UNLESS(InetSocketAddress::IsMatchingType(m_localAddress), "Only IPv4 is supported.");
        if (m_socket->Bind(m_localAddress) == -1) {
            throw std::runtime_error("Failed to bind socket");
        }

        // Connect
        NS_ABORT_MSG_UNLESS(InetSocketAddress::IsMatchingType(m_remoteAddress), "Only IPv4 is supported.");
        m_socket->Connect(m_remoteAddress);

    }
    m_startTime = Simulator::Now();
    m_socket->SetRecvCallback(MakeCallback(&UdpPingClient::HandleRead, this));
    m_socket->SetAllowBroadcast(false);
    ScheduleTransmit(Seconds(0.));
}

void
UdpPingClient::StopApplication() { // Called at time specified by Stop
    throw std::runtime_error(
            "UDP ping client cannot be stopped like a regular application, only of its own volition."
    );
}

void
UdpPingClient::Finish() {
    NS_LOG_FUNCTION(this);
    NS_ABORT_UNLESS(m_sendEvent.IsExpired());
    NS_ABORT_UNLESS(m_waitForFinishEvent.IsExpired());
    if (m_socket != 0) {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback < void, Ptr < Socket > > ());
        m_socket = 0;
    }
}

void
UdpPingClient::ScheduleTransmit(Time dt) {
    NS_LOG_FUNCTION(this << dt);
    m_sendEvent = Simulator::Schedule(dt, &UdpPingClient::Send, this);
}

void
UdpPingClient::Send(void) {
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_sendEvent.IsExpired());

    // Current time
    uint64_t now_ns = Simulator::Now().GetNanoSeconds();

    // Packet with timestamp
    Ptr<Packet> p = Create<Packet>();
    UdpPingHeader pingHeader;
    pingHeader.SetId(m_udpPingId);
    pingHeader.SetSeq(m_sent);
    pingHeader.SetTs(now_ns);
    p->AddHeader(pingHeader);

    // Timestamps
    m_sendRequestTimestamps.push_back(now_ns);
    m_replyTimestamps.push_back(-1);
    m_receiveReplyTimestamps.push_back(-1);
    m_sent++;

    // Send out
    m_socket->Send(p);

    // Schedule next transmit, or wait to close
    if (now_ns + m_interval.GetNanoSeconds() < (uint64_t) (m_startTime.GetNanoSeconds() + m_duration.GetNanoSeconds())) {
        ScheduleTransmit(m_interval);
    } else {
        m_waitForFinishEvent = Simulator::Schedule(
                NanoSeconds(m_startTime.GetNanoSeconds() + m_duration.GetNanoSeconds() - now_ns + m_waitAfterwards.GetNanoSeconds()),
                &UdpPingClient::Finish, this
        );
    }

}

void
UdpPingClient::HandleRead(Ptr <Socket> socket) {
    NS_LOG_FUNCTION(this << socket);
    Ptr <Packet> packet;
    Address from;
    Address localAddress;
    while ((packet = socket->RecvFrom(from))) {

        // Receiving header
        UdpPingHeader pingHeader;
        packet->RemoveHeader (pingHeader);
        uint32_t seqNo = pingHeader.GetSeq();

        // It is possible that a client gets assigned the same port as a previously
        // existing client, and as such it is possible that it receives ping
        // replies from the previous occupant. Therefore, we have this check
        // and no exception thrown if it fails, as it is a valid thing which can happen.
        if (pingHeader.GetId() == m_udpPingId) {

            // Sanity checks
            NS_ABORT_MSG_IF(seqNo >= m_sendRequestTimestamps.size(), "Sequence number has not (yet) been sent out.");
            NS_ABORT_MSG_IF(m_replyTimestamps.at(seqNo) != -1 || m_receiveReplyTimestamps.at(seqNo) != -1, "Already got reply for this sequence number.");

            // Update the timestamps for this sequence number
            m_replyTimestamps.at(seqNo) = pingHeader.GetTs();
            m_receiveReplyTimestamps.at(seqNo) = Simulator::Now().GetNanoSeconds();

        }

    }
}

uint32_t UdpPingClient::GetUdpPingId() {
    return m_udpPingId;
}

std::string UdpPingClient::GetAdditionalParameters() {
    return m_additionalParameters;
}

uint32_t UdpPingClient::GetSent() {
    return m_sent;
}

std::vector<int64_t> UdpPingClient::GetSendRequestTimestamps() {
    return m_sendRequestTimestamps;
}

std::vector<int64_t> UdpPingClient::GetReplyTimestamps() {
    return m_replyTimestamps;
}

std::vector<int64_t> UdpPingClient::GetReceiveReplyTimestamps() {
    return m_receiveReplyTimestamps;
}

} // Namespace ns3
