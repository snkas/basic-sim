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

#include "udp-burst-application.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("UdpBurstApplication");

    NS_OBJECT_ENSURE_REGISTERED (UdpBurstApplication);

    TypeId
    UdpBurstApplication::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::UdpBurstApplication")
                .SetParent<Application>()
                .SetGroupName("Applications")
                .AddConstructor<UdpBurstApplication>()
                .AddAttribute("Port", "Port on which we listen for incoming packets.",
                              UintegerValue(9),
                              MakeUintegerAccessor(&UdpBurstApplication::m_port),
                              MakeUintegerChecker<uint16_t>())
                .AddAttribute("MaxUdpPayloadSizeByte", "Total UDP payload size (byte) before it gets fragmented.",
                              UintegerValue(1472), // 1500 (point-to-point default) - 20 (IP) - 8 (UDP) = 1472
                              MakeUintegerAccessor(&UdpBurstApplication::m_max_udp_payload_size_byte),
                              MakeUintegerChecker<uint32_t>());
        return tid;
    }

    UdpBurstApplication::UdpBurstApplication() {
        NS_LOG_FUNCTION(this);
        m_next_internal_burst_idx = 0;
    }

    UdpBurstApplication::~UdpBurstApplication() {
        NS_LOG_FUNCTION(this);
        m_socket = 0;
    }

    void
    UdpBurstApplication::RegisterBurst(UdpBurstInfo burstInfo, InetSocketAddress targetAddress) {
        if (m_bursts.size() >= 1 && burstInfo.GetStartTimeNs() < std::get<0>(m_bursts[m_bursts.size() - 1]).GetStartTimeNs()) {
            throw std::runtime_error("Bursts must be added weakly ascending on start time");
        }
        m_bursts.push_back(std::make_tuple(burstInfo, targetAddress));
        m_bursts_packets_sent_counter.push_back(0);
    }

    void
    UdpBurstApplication::DoDispose(void) {
        NS_LOG_FUNCTION(this);
        Application::DoDispose();
    }

    void
    UdpBurstApplication::StartApplication(void) {
        NS_LOG_FUNCTION(this);

        if (m_socket == 0) {

            // Bind to UDP port
            TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
            m_socket = Socket::CreateSocket(GetNode(), tid);
            InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
            if (m_socket->Bind(local) == -1) {
                NS_FATAL_ERROR("Failed to bind socket");
            }

            // No multi-cast
            if (addressUtils::IsMulticast(m_local)) {
                throw std::runtime_error("Multi-cast is not supported.");
            }

        }

        // Receive of packets
        m_socket->SetRecvCallback(MakeCallback(&UdpBurstApplication::HandleRead, this));

        // First process call is for the start of the first burst
        if (m_bursts.size() > 0) {
            Simulator::Schedule(NanoSeconds(std::get<0>(m_bursts[0]).GetStartTimeNs()), &UdpBurstApplication::StartNextBurst, this);
        }

    }

    void
    UdpBurstApplication::StartNextBurst()
    {
        int64_t now_ns = Simulator::Now().GetNanoSeconds();

        // If this function is called, there must be a next burst
        if (m_next_internal_burst_idx >= m_bursts.size() || std::get<0>(m_bursts[m_next_internal_burst_idx]).GetStartTimeNs() != now_ns) {
            throw std::runtime_error("No next burst available; this function should not have been called.");
        }

        // Start the self-calling (and self-ending) process of sending out packets of the burst
        BurstSendOut(m_next_internal_burst_idx);

        // Schedule the start of the next burst if there are more
        m_next_internal_burst_idx += 1;
        if (m_next_internal_burst_idx < m_bursts.size()) {
            Simulator::Schedule(NanoSeconds(std::get<0>(m_bursts[m_next_internal_burst_idx]).GetStartTimeNs() - now_ns), &UdpBurstApplication::StartNextBurst, this);
        }

    }

    void
    UdpBurstApplication::BurstSendOut(size_t internal_burst_idx)
    {
        // Send out the packet as desired
        TransmitFullPacket(internal_burst_idx);

        // For now, let's not send out one-by-one, just to verify correctness
        // int64_t next_packet_send_ns = 1000;
        // Simulator::Schedule(NanoSeconds(next_packet_send_ns), &UdpBurstApplication::BurstSendOut, this, internal_burst_idx);
    }

    void
    UdpBurstApplication::TransmitFullPacket(size_t internal_burst_idx) {

        // Header with (udp_burst_id, seq_no)
        IdSeqHeader idSeq;
        idSeq.SetId(std::get<0>(m_bursts[internal_burst_idx]).GetUdpBurstId());
        idSeq.SetSeq(m_bursts_packets_sent_counter[internal_burst_idx]);

        // One more packet will be sent out
        m_bursts_packets_sent_counter[internal_burst_idx] += 1;

        // A full payload packet
        Ptr<Packet> p = Create<Packet>(m_max_udp_payload_size_byte - idSeq.GetSerializedSize());
        p->AddHeader(idSeq);

        // Send out the packet to the target address
        m_socket->SendTo(p, 0, std::get<1>(m_bursts[internal_burst_idx]));

    }

    void
    UdpBurstApplication::StopApplication() {
        NS_LOG_FUNCTION(this);
        if (m_socket != 0) {
            m_socket->Close();
            m_socket->SetRecvCallback(MakeNullCallback < void, Ptr < Socket > > ());
        }
    }

    void
    UdpBurstApplication::HandleRead(Ptr <Socket> socket) {
        NS_LOG_FUNCTION(this << socket);
        Ptr <Packet> packet;
        Address from;
        while ((packet = socket->RecvFrom(from))) {

            // Extract burst identifier and packet sequence number
            IdSeqHeader incomingIdSeq;
            packet->RemoveHeader (incomingIdSeq);

            // Print
            std::cout << "Burst " << incomingIdSeq.GetId() << " sequence " << incomingIdSeq.GetSeq() << std::endl;

        }
    }

} // Namespace ns3
