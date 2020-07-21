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
                              MakeUintegerChecker<uint16_t>());
        return tid;
    }

    UdpBurstApplication::UdpBurstApplication() {
        NS_LOG_FUNCTION(this);
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

            // What we receive
            SeqTsHeader incomingSeqTs;
            packet->RemoveHeader (incomingSeqTs);

            // Add header
            SeqTsHeader outgoingSeqTs; // Creates one with the current timestamp
            outgoingSeqTs.SetSeq(incomingSeqTs.GetSeq());
            packet->AddHeader(outgoingSeqTs);

            // Remove any tags
            packet->RemoveAllPacketTags();
            packet->RemoveAllByteTags();

            // Send back with the new timestamp on it
            socket->SendTo(packet, 0, from);

        }
    }

} // Namespace ns3
