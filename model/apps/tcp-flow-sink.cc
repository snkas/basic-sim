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
 *
 * Author: Simon
 * Adapted from PacketSink by:
 * Author: Tom Henderson (tomhend@u.washington.edu)
 */

#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/tcp-socket-factory.h"
#include "tcp-flow-sink.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpFlowSink");

NS_OBJECT_ENSURE_REGISTERED (TcpFlowSink);

TypeId
TcpFlowSink::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::TcpFlowSink")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<TcpFlowSink>()
            .AddAttribute("Local",
                          "The Address on which to Bind the rx socket.",
                          AddressValue(),
                          MakeAddressAccessor(&TcpFlowSink::m_local),
                          MakeAddressChecker())
            ;
    return tid;
}

TcpFlowSink::TcpFlowSink() {
    NS_LOG_FUNCTION(this);
    m_socket = 0;
    m_totalRx = 0;
}

TcpFlowSink::~TcpFlowSink() {
    NS_LOG_FUNCTION(this);
}

void TcpFlowSink::DoDispose(void) {
    NS_LOG_FUNCTION(this);
    m_socket = 0;
    m_socketList.clear();

    // chain up
    Application::DoDispose();
}


void TcpFlowSink::StartApplication() { // Called at time specified by Start
    NS_LOG_FUNCTION(this);

    // Create a socket which is always in LISTEN state
    // As soon as it processes a SYN in ProcessListen(),
    // it forks itself into a new socket, which
    // keeps the accept and close callbacks
    if (!m_socket) {
        m_socket = Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());

        // Socket constraints
        NS_ABORT_MSG_IF(addressUtils::IsMulticast(m_local), "No support for multicast");
        NS_ABORT_MSG_UNLESS(InetSocketAddress::IsMatchingType(m_local), "Only IPv4 is supported.");

        // Bind socket
        //
        // This is a listening socket. Once it gets a SYN request, it does a CompleteFork. This means it
        // creates a new socket and registers its own 5-tuple endpoint based on the exact received IP/TCP header
        // IP addresses / TCP ports.
        //
        // WHEN DOES IT RECEIVE?
        // (1) m_local(0, ANY) -- An ephemeral port is assigned. A SYN towards any IP address on this node + the ephemeral port will match it.
        // (2) m_local(> 0, ANY) -- Same as (1) except you know which port you get
        // (3) m_local(0, IP) -- Same as (1) except that the IP address must match exactly (together with the ephemeral port)
        // (4) m_local(> 0, IP) -- Exactly this port and IP will match, all else will not (like (3) except you know what is the port)
        //
        // Note: it might be possible that another TCP socket becomes more specific (particular IP as in (4)).
        // The more specificity (see Ipv4EndPointDemux) the more absolute it will get matched to.
        //
        if (m_socket->Bind(m_local) == -1) {
            throw std::runtime_error("Failed to bind socket");
        }
        m_socket->Listen();
        m_socket->ShutdownSend();
    }

    // Callbacks
    m_socket->SetRecvCallback(MakeCallback(&TcpFlowSink::HandleRead, this));
    m_socket->SetAcceptCallback(
            MakeNullCallback<bool, Ptr<Socket>,const Address &>(),
            MakeCallback(&TcpFlowSink::HandleAccept, this)
    );

}

void TcpFlowSink::StopApplication() {  // Called at time specified by Stop
    throw std::runtime_error("TCP flow sink is not intended to be stopped after being started.");
    /*
     * Deprecated stop code:
     *
     * NS_LOG_FUNCTION(this);
     * while (!m_socketList.empty()) {
     *     Ptr <Socket> socket = m_socketList.front();
     *     m_socketList.pop_front();
     *     socket->Close();
     * }
     * if (m_socket) {
     *     m_socket->Close();
     * }
    */
}

void TcpFlowSink::HandleAccept(Ptr<Socket> socket, const Address &from) {
    NS_LOG_FUNCTION(this << socket << from);
    socket->SetRecvCallback (MakeCallback (&TcpFlowSink::HandleRead, this));
    socket->SetCloseCallbacks(
            MakeCallback(&TcpFlowSink::HandlePeerClose, this),
            MakeCallback(&TcpFlowSink::HandlePeerError, this)
    );
    m_socketList.push_back(socket);
}

void TcpFlowSink::HandleRead(Ptr<Socket> socket) {
    NS_LOG_FUNCTION (this << socket);

    // Immediately from the socket drain all the packets it has received
    Ptr<Packet> packet;
    Address from;
    while ((packet = socket->RecvFrom(from))) {
        if (packet->GetSize() == 0) { // EOFs
            break;
        }
        m_totalRx += packet->GetSize ();
    }
}

void TcpFlowSink::HandlePeerClose(Ptr<Socket> socket) {
    NS_LOG_FUNCTION(this << socket);
    CleanUp(socket);
}

void TcpFlowSink::HandlePeerError(Ptr<Socket> socket) {
    NS_LOG_FUNCTION(this << socket);
    CleanUp(socket);
}

void TcpFlowSink::CleanUp(Ptr<Socket> socket) {
    NS_LOG_FUNCTION(this << socket);
    // This function can be called 2x if the LAST_ACK retries fail.
    // That would result in first a normal close, and then an error close.
    std::list<Ptr<Socket>>::iterator it;
    for (it = m_socketList.begin(); it != m_socketList.end(); ++it) {
        if (*it == socket) {
            m_socketList.erase(it);
            break;
        }
    }
}

} // Namespace ns3
