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

#include "udp-ping-server.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("UdpPingServer");

    NS_OBJECT_ENSURE_REGISTERED (UdpPingServer);

    TypeId
    UdpPingServer::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::UdpPingServer")
                .SetParent<Application>()
                .SetGroupName("Applications")
                .AddConstructor<UdpPingServer>()
                .AddAttribute("LocalAddress",
                              "The local address (IPv4 address, port). Setting the IPv4 address will enable"
                              "proper ECMP routing (as else it forces an early lookup with only destination IP). "
                              "Setting the port is handy as it is a server, so it's good to be reachable.",
                              AddressValue(),
                              MakeAddressAccessor(&UdpPingServer::m_localAddress),
                              MakeAddressChecker());
        return tid;
    }

    UdpPingServer::UdpPingServer() {
        NS_LOG_FUNCTION(this);
    }

    UdpPingServer::~UdpPingServer() {
        NS_LOG_FUNCTION(this);
        m_socket = 0;
    }

    void
    UdpPingServer::DoDispose(void) {
        NS_LOG_FUNCTION(this);
        Application::DoDispose();
    }

    void
    UdpPingServer::StartApplication(void) {
        NS_LOG_FUNCTION(this);

        if (m_socket == 0) {

            // Create socket
            TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
            m_socket = Socket::CreateSocket(GetNode(), tid);

            // Bind to local address
            if (m_socket->Bind(m_localAddress) == -1) {
                throw std::runtime_error("Failed to bind socket");
            }

        }

        m_socket->SetRecvCallback(MakeCallback(&UdpPingServer::HandleRead, this));
    }

    void
    UdpPingServer::StopApplication() {
        throw std::runtime_error("UDP RTT server is not permitted to be stopped.");
    }

    void
    UdpPingServer::HandleRead(Ptr <Socket> socket) {
        NS_LOG_FUNCTION(this << socket);
        Ptr <Packet> packet;
        Address from;
        while ((packet = socket->RecvFrom(from))) {

            // What we receive
            UdpPingHeader incomingPingHeader;
            packet->RemoveHeader (incomingPingHeader);

            // Add header
            UdpPingHeader outgoingPingHeader;
            outgoingPingHeader.SetId(incomingPingHeader.GetId());
            outgoingPingHeader.SetSeq(incomingPingHeader.GetSeq());
            outgoingPingHeader.SetTs(Simulator::Now().GetNanoSeconds());
            packet->AddHeader(outgoingPingHeader);

            // Remove any tags
            packet->RemoveAllPacketTags();
            packet->RemoveAllByteTags();

            // Send back with the reply header as payload
            socket->SendTo(packet, 0, from);

        }
    }

} // Namespace ns3
