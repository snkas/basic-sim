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
#include "udp-burst-client.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpBurstClient");

NS_OBJECT_ENSURE_REGISTERED (UdpBurstClient);

TypeId
UdpBurstClient::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::UdpBurstClient")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<UdpBurstClient>()
            .AddAttribute("LocalAddress",
                          "The local address (IPv4 address, port). Setting the IPv4 address will enable "
                          "proper ECMP routing (as else it forces an early lookup with only destination IP). "
                          "Setting the port is not necessary, as it will be assigned an ephemeral one.",
                          AddressValue(),
                          MakeAddressAccessor(&UdpBurstClient::m_localAddress),
                          MakeAddressChecker())
            .AddAttribute("RemoteAddress",
                          "The address of the destination server (IPv4 address, port)",
                          AddressValue(),
                          MakeAddressAccessor(&UdpBurstClient::m_remoteAddress),
                          MakeAddressChecker())
            .AddAttribute("UdpBurstId",
                          "Unique UDP burst identifier",
                          UintegerValue(0),
                          MakeUintegerAccessor(&UdpBurstClient::m_udpBurstId),
                          MakeUintegerChecker<uint64_t>())
            .AddAttribute("TargetRateMbps",
                          "Target rate (incl. headers) in Mbit/s",
                          DoubleValue(10.0),
                          MakeDoubleAccessor(&UdpBurstClient::m_targetRateMegabitPerSec),
                          MakeDoubleChecker<double>())
            .AddAttribute("Duration",
                          "How long to do the bursting",
                          TimeValue(Seconds(10.0)),
                          MakeTimeAccessor(&UdpBurstClient::m_duration),
                          MakeTimeChecker())
            .AddAttribute ("AdditionalParameters",
                           "Additional parameters (unused; reserved for future use)",
                           StringValue (""),
                           MakeStringAccessor (&UdpBurstClient::m_additionalParameters),
                           MakeStringChecker())
            .AddAttribute("EnableDetailedLoggingToFile",
                          "True iff you want to have the exact send time stamps logged to file: logs_dir/udp_burst_[id]_outgoing.csv",
                          BooleanValue(false),
                          MakeBooleanAccessor(&UdpBurstClient::m_enableDetailedLoggingToFile),
                          MakeBooleanChecker())
            .AddAttribute ("BaseLogsDir",
                           "Base logging directory (logging will be placed here, i.e. logs_dir/udp_burst_[id]_outgoing.csv",
                           StringValue (""),
                           MakeStringAccessor (&UdpBurstClient::m_baseLogsDir),
                           MakeStringChecker ())
            .AddAttribute("MaxSegmentSizeByte",
                          "Maximum segment size (byte), in other words: "
                          "the maximum total packet size before it gets fragmented.",
                          UintegerValue(1500), // 1500 (= point-to-point default)
                          MakeUintegerAccessor (&UdpBurstClient::m_maxSegmentSizeByte),
                          MakeUintegerChecker<uint32_t>())
            .AddAttribute("MaxUdpPayloadSizeByte",
                          "Maximum payload size after deducting all headers (IPv4/IPv6, UDP, any other tunneling, etc.).",
                          UintegerValue(1472), // Default: 1500 (point-to-point default) - 20 (IPv4) - 8 (UDP) = 1472
                          MakeUintegerAccessor (&UdpBurstClient::m_maxUdpPayloadSizeByte),
                          MakeUintegerChecker<uint32_t>());
    return tid;
}

UdpBurstClient::UdpBurstClient() {
    NS_LOG_FUNCTION(this);
    m_udpSocketGenerator = CreateObject<UdpSocketGeneratorDefault>();
    m_socket = 0;
    m_sent = 0;
    m_sendEvent = EventId();
}

UdpBurstClient::~UdpBurstClient() {
    NS_LOG_FUNCTION(this);
    m_udpSocketGenerator = 0;
    m_socket = 0;
}

void
UdpBurstClient::DoDispose(void) {
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
}

void
UdpBurstClient::SetUdpSocketGenerator(Ptr<UdpSocketGenerator> udpSocketGenerator) {
    m_udpSocketGenerator = udpSocketGenerator;
}

void
UdpBurstClient::SetIpTos(uint8_t ipTos) {
    NS_ABORT_MSG_UNLESS(InetSocketAddress::IsMatchingType(m_localAddress), "Only IPv4 is supported.");
    InetSocketAddress newLocalAddress = InetSocketAddress::ConvertFrom(m_localAddress);
    newLocalAddress.SetTos(ipTos);
    m_localAddress = newLocalAddress;
    NS_ABORT_MSG_UNLESS(InetSocketAddress::IsMatchingType(m_remoteAddress), "Only IPv4 is supported.");
    InetSocketAddress newRemoteAddress = InetSocketAddress::ConvertFrom(m_remoteAddress);
    newRemoteAddress.SetTos(ipTos);
    m_remoteAddress = newRemoteAddress;
}

uint32_t
UdpBurstClient::GetMaxSegmentSizeByte() const {
    return m_maxSegmentSizeByte;
}

uint32_t
UdpBurstClient::GetMaxUdpPayloadSizeByte() const {
    return m_maxUdpPayloadSizeByte;
}

void
UdpBurstClient::StartApplication(void) {
    NS_LOG_FUNCTION(this);
    if (m_socket == 0) {
        m_socket = m_udpSocketGenerator->GenerateUdpSocket(UdpBurstClient::GetTypeId(), this);

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
    m_socket->SetAllowBroadcast(false);
    ScheduleTransmit(Seconds(0.));
}

void
UdpBurstClient::StopApplication() { // Called at time specified by Stop
    throw std::runtime_error(
            "UDP burst client cannot be stopped like a regular application, it finished only when it is done sending."
    );
}

void
UdpBurstClient::ScheduleTransmit(Time dt) {
    NS_LOG_FUNCTION(this << dt);
    m_sendEvent = Simulator::Schedule(dt, &UdpBurstClient::Send, this);
}

void
UdpBurstClient::Send(void) {
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_sendEvent.IsExpired());

    // Full payload packet
    UdpBurstHeader burstHeader;
    burstHeader.SetId(m_udpBurstId);
    burstHeader.SetSeq(m_sent);
    Ptr<Packet> p = Create<Packet>(m_maxUdpPayloadSizeByte - burstHeader.GetSerializedSize());
    p->AddHeader(burstHeader);

    // Sent out
    m_sent++;

    // Log precise timestamp sent away of the sequence packet if needed
    if (m_enableDetailedLoggingToFile) {
        std::ofstream ofs;
        ofs.open(m_baseLogsDir + "/" + format_string("udp_burst_%" PRIu64 "_outgoing.csv", burstHeader.GetId()), std::ofstream::out | std::ofstream::app);
        ofs << burstHeader.GetId() << "," << burstHeader.GetSeq() << "," << Simulator::Now().GetNanoSeconds() << std::endl;
        ofs.close();
    }

    // Send out
    m_socket->Send(p);

    // Schedule next transmit, or wait to close
    uint64_t now_ns = Simulator::Now().GetNanoSeconds();
    uint64_t packet_gap_nanoseconds = std::ceil((double) m_maxSegmentSizeByte / (m_targetRateMegabitPerSec / 8000.0));
    if (now_ns + packet_gap_nanoseconds < (uint64_t) (m_startTime.GetNanoSeconds() + m_duration.GetNanoSeconds())) {
        ScheduleTransmit(NanoSeconds(packet_gap_nanoseconds));
    }

}

uint32_t UdpBurstClient::GetUdpBurstId() {
    return m_udpBurstId;
}

std::string UdpBurstClient::GetAdditionalParameters() {
    return m_additionalParameters;
}

uint32_t UdpBurstClient::GetSent() {
    return m_sent;
}

} // Namespace ns3
