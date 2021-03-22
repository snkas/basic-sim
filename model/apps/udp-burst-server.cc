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
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"

#include "udp-burst-server.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpBurstServer");

NS_OBJECT_ENSURE_REGISTERED (UdpBurstServer);

TypeId
UdpBurstServer::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::UdpBurstServer")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<UdpBurstServer>()
            .AddAttribute("LocalAddress",
                          "The local address (IPv4 address, port). Setting the IPv4 address will enable "
                          "proper ECMP routing (as else it forces an early lookup with only destination IP). "
                          "Setting the port is handy as it is a server, so it's good to be reachable.",
                          AddressValue(),
                          MakeAddressAccessor(&UdpBurstServer::m_localAddress),
                          MakeAddressChecker())
            .AddAttribute ("BaseLogsDir",
                           "Base logging directory (logging will be placed here, i.e. logs_dir/udp_burst_[id]_incoming.csv",
                           StringValue (""),
                           MakeStringAccessor (&UdpBurstServer::m_baseLogsDir),
                           MakeStringChecker ())
            .AddAttribute("MaxSegmentSizeByte",
                          "Maximum segment size (byte), in other words: "
                          "the maximum total packet size before it gets fragmented.",
                          UintegerValue(1500), // 1500 (= point-to-point default)
                          MakeUintegerAccessor (&UdpBurstServer::m_maxSegmentSizeByte),
                          MakeUintegerChecker<uint32_t>())
            .AddAttribute("MaxUdpPayloadSizeByte",
                          "Maximum payload size after deducting all headers (IPv4/IPv6, UDP, any other tunneling, etc.).",
                          UintegerValue(1472), // Default: 1500 (point-to-point default) - 20 (IPv4) - 8 (UDP) = 1472
                          MakeUintegerAccessor (&UdpBurstServer::m_maxUdpPayloadSizeByte),
                          MakeUintegerChecker<uint32_t>());
    return tid;
}

UdpBurstServer::UdpBurstServer() {
    NS_LOG_FUNCTION(this);
    m_udpSocketGenerator = CreateObject<UdpSocketGeneratorDefault>();
    m_socket = 0;
}

UdpBurstServer::~UdpBurstServer() {
    NS_LOG_FUNCTION(this);
    m_udpSocketGenerator = 0;
    m_socket = 0;
}

void
UdpBurstServer::DoDispose(void) {
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
}

void
UdpBurstServer::SetUdpSocketGenerator(Ptr<UdpSocketGenerator> udpSocketGenerator) {
    m_udpSocketGenerator = udpSocketGenerator;
}

void
UdpBurstServer::SetIpTos(uint8_t ipTos) {
    NS_ABORT_MSG_UNLESS(InetSocketAddress::IsMatchingType(m_localAddress), "Only IPv4 is supported.");
    InetSocketAddress newLocalAddress = InetSocketAddress::ConvertFrom(m_localAddress);
    newLocalAddress.SetTos(ipTos);
    m_localAddress = newLocalAddress;
}

uint32_t
UdpBurstServer::GetMaxSegmentSizeByte() const {
    return m_maxSegmentSizeByte;
}

uint32_t
UdpBurstServer::GetMaxUdpPayloadSizeByte() const {
    return m_maxUdpPayloadSizeByte;
}

void
UdpBurstServer::RegisterIncomingBurst(int64_t udp_burst_id, bool enable_precise_logging) {
    m_incoming_bursts.push_back(udp_burst_id);
    m_incoming_bursts_received_counter.insert(std::make_pair(udp_burst_id, 0));
    m_incoming_bursts_enable_precise_logging.insert(std::make_pair(udp_burst_id, enable_precise_logging));
    if (enable_precise_logging) {
        std::ofstream ofs;
        ofs.open(m_baseLogsDir + "/" + format_string("udp_burst_%" PRIu64 "_incoming.csv", udp_burst_id));
        ofs.close();
    }
}

void
UdpBurstServer::StartApplication(void) {
    NS_LOG_FUNCTION(this);

    if (m_socket == 0) {

        // Create socket
        m_socket = m_udpSocketGenerator->GenerateUdpSocket(UdpBurstServer::GetTypeId(), this);

        // Bind to local address
        if (m_socket->Bind(m_localAddress) == -1) {
            throw std::runtime_error("Failed to bind socket");
        }

    }

    m_socket->SetRecvCallback(MakeCallback(&UdpBurstServer::HandleRead, this));
}

void
UdpBurstServer::StopApplication() {
    throw std::runtime_error("UDP burst server is not permitted to be stopped.");
}

void
UdpBurstServer::HandleRead(Ptr <Socket> socket) {
    NS_LOG_FUNCTION(this << socket);
    Ptr <Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom(from))) {

        // What we receive
        UdpBurstHeader incomingBurstHeader;
        packet->RemoveHeader (incomingBurstHeader);

        // Count packets from incoming bursts
        m_incoming_bursts_received_counter.at(incomingBurstHeader.GetId()) += 1;

        // Log precise timestamp received of the sequence packet if needed
        if (m_incoming_bursts_enable_precise_logging.at(incomingBurstHeader.GetId())) {
            std::ofstream ofs;
            ofs.open(m_baseLogsDir + "/" + format_string("udp_burst_%" PRIu64 "_incoming.csv", incomingBurstHeader.GetId()), std::ofstream::out | std::ofstream::app);
            ofs << incomingBurstHeader.GetId() << "," << incomingBurstHeader.GetSeq() << "," << Simulator::Now().GetNanoSeconds() << std::endl;
            ofs.close();
        }

    }
}

std::vector<std::tuple<int64_t, uint64_t>>
UdpBurstServer::GetIncomingBurstsInformation() {
    std::vector<std::tuple<int64_t, uint64_t>> result;
    for (size_t i = 0; i < m_incoming_bursts.size(); i++) {
        result.push_back(std::make_tuple(m_incoming_bursts.at(i), m_incoming_bursts_received_counter.at(m_incoming_bursts.at(i))));
    }
    return result;
}

uint64_t
UdpBurstServer::GetReceivedCounterOf(int64_t udp_burst_id) {
    return m_incoming_bursts_received_counter.at(udp_burst_id);
}

} // Namespace ns3
