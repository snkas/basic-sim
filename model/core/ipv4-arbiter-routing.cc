/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Based on Ipv4StaticRouting
 */

#define NS_LOG_APPEND_CONTEXT                                   \
  if (m_ipv4 && m_ipv4->GetObject<Node> ()) { \
      std::clog << Simulator::Now ().GetSeconds () \
                << " [node " << m_ipv4->GetObject<Node> ()->GetId () << "] "; }

#include <iomanip>
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/simulator.h"
#include "ns3/ipv4-route.h"
#include "ns3/output-stream-wrapper.h"
#include "ipv4-arbiter-routing.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("Ipv4ArbiterRouting");

    NS_OBJECT_ENSURE_REGISTERED (Ipv4ArbiterRouting);

    TypeId
    Ipv4ArbiterRouting::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::Ipv4ArbiterRouting")
                .SetParent<Ipv4RoutingProtocol>()
                .SetGroupName("Internet")
                .AddConstructor<Ipv4ArbiterRouting>();
        return tid;
    }

    Ipv4ArbiterRouting::Ipv4ArbiterRouting() : m_ipv4(0) {
        NS_LOG_FUNCTION(this);
    }

    Ipv4ArbiterRouting::~Ipv4ArbiterRouting() {
        NS_LOG_FUNCTION(this);
    }

    /**
     * Lookup the route towards the destination.
     * Does not handle local delivery.
     *
     * @param dest      Destination IP address
     * @param header    IPv4 header
     * @param p         Packet
     * @param oif       Requested output interface
     *
     * @return Valid Ipv4 route
     */
    Ptr<Ipv4Route>
    Ipv4ArbiterRouting::LookupArbiter (const Ipv4Address& dest, const Ipv4Header &header, Ptr<const Packet> p, Ptr<NetDevice> oif) {

        // Arbiter must be set
        if (m_arbiter == 0) {
            throw std::runtime_error("Arbiter has not been set");
        }

        // Multi-cast not supported
        NS_ABORT_MSG_IF(dest.IsLocalMulticast(), "Multi-cast is not supported");

        // No support for requested output interfaces
        NS_ABORT_MSG_IF(oif != 0, "Requested output interfaces are not supported");

        // A packet with as destination loop-back is likely not desired
        // if it would be: if_idx = 0 and gateway_ip_address = 0 could have been set
        NS_ABORT_MSG_IF(loopbackMask.IsMatch(dest, loopbackIp), "Loop-back as destination is typically not desired");

        // Decide interface index
        // If is not loop-back (filtered out above), so it goes to the arbiter
        // Local delivery has already been handled if it was input
        ArbiterResult result = m_arbiter->BaseDecide(p, header);

        // If it failed, return a zero pointer
        if (result.Failed()) {
            return 0;
        }

        // If it succeeded to find a next hop, extract the output interface index and gateway IP address
        uint32_t if_idx = result.GetOutIfIdx();
        uint32_t gateway_ip_address = result.GetGatewayIpAddress();

        // Create routing entry
        Ptr<Ipv4Route> rtentry = Create<Ipv4Route>();
        rtentry->SetDestination(dest);
        rtentry->SetSource(m_ipv4->SourceAddressSelection(if_idx, dest)); // This is basically the IP of the interface
                                                                          // It is used by a transport layer to
                                                                          // determine its source IP address
        rtentry->SetGateway(Ipv4Address(gateway_ip_address)); // If the network device does not care about ARP resolution,
                                                              // this can be set to 0.0.0.0
        rtentry->SetOutputDevice(m_ipv4->GetNetDevice(if_idx));
        return rtentry;

    }

    /**
     * Get an output route.
     *
     * TCP:
     * (1) RouteOutput gets called once by TCP (tcp-socket-base.cc) with an header of which only the destination
     *     IP address is set in order to determine the source IP address for the socket (function: SetupEndpoint).
     * (2) Subsequently, RouteOutput is called by TCP (tcp-l4-protocol.cc) on the sender node with a proper IP
     *     AND TCP header.
     *
     * UDP:
     * If the UDP Bind() is not called before with an IP address, the following happens:
     * (1) RouteOutput gets called once by UDP (udp-socket-impl.cc) with an header of which only the destination
     *     IP address is set in order to determine the source IP address for the socket (function: DoSendTo).
     * (2) It is **NOT** called subsequently in udp-l4-protocol.cc, as such the first decision which did not take
     *     into account the UDP header is final. This means ECMP load balancing does not work properly in this case.
     * Else, if it did get called with an IP address, the following happens:
     * (1) It will send it to udp-l4-protocol.cc without a route
     * (2) Subsequently, udp-l4-protocol will just pass it onto ipv4-l3-protocol.cc, which will make the proper
     *     RouteOutput call with the entire IP and UDP header available. In this case ECMP load balancing will work
     *     as expected.
     *
     * @param p         Packet
     * @param header    Header
     * @param oif       Requested output interface
     * @param sockerr   (Output) socket error, set if cannot find route
     *
     * @return IPv4 route
     */
    Ptr <Ipv4Route>
    Ipv4ArbiterRouting::RouteOutput(Ptr <Packet> p, const Ipv4Header &header, Ptr <NetDevice> oif, Socket::SocketErrno &sockerr) {
        NS_LOG_FUNCTION(this << p << header << oif << sockerr);
        Ipv4Address destination = header.GetDestination();

        // Multi-cast to multiple interfaces is not supported
        NS_ABORT_MSG_IF(destination.IsMulticast(), "Multi-cast not supported");

        // Perform lookup
        // Info: If no route is found for a packet with the header with source IP = 102.102.102.102,
        //       the TCP socket will conclude there is no route and not even send out SYNs (any real packet).
        //       If source IP is set already, it just gets dropped and the TCP socket sees it as a normal loss somewhere in the network.
        Ptr<Ipv4Route> route = LookupArbiter(destination, header, p, oif);
        if (route == 0) {
            sockerr = Socket::ERROR_NOROUTETOHOST;
        } else {
            sockerr = Socket::ERROR_NOTERROR;
        }
        return route;

    }

    bool
    Ipv4ArbiterRouting::RouteInput(Ptr<const Packet> p, const Ipv4Header &ipHeader, Ptr<const NetDevice> idev,
                                          UnicastForwardCallback ucb, MulticastForwardCallback mcb,
                                          LocalDeliverCallback lcb, ErrorCallback ecb) {
        NS_ASSERT(m_ipv4 != 0);

        // Check if input device supports IP
        NS_ASSERT(m_ipv4->GetInterfaceForDevice(idev) >= 0);
        uint32_t iif = m_ipv4->GetInterfaceForDevice(idev);

        // Multi-cast
        NS_ABORT_MSG_IF(ipHeader.GetDestination().IsMulticast(), "Multi-cast not supported.");

        // Local delivery
        if (m_ipv4->IsDestinationAddress(ipHeader.GetDestination(), iif)) { // WeakESModel is set by default to true,
                                                                            // as such it works for any IP address
                                                                            // on any interface of the node
            NS_ABORT_MSG_IF(lcb.IsNull(), "Local callback cannot be null");

            // Info: If you want to decide that a packet should not be delivered (dropped),
            //       you can decide that here by not calling lcb(), but still returning true.
            lcb(p, ipHeader, iif);
            return true;
        }

        // Check if input device supports IP forwarding
        NS_ABORT_MSG_IF(!m_ipv4->IsForwarding(iif), "Forwarding must be enabled for every interface");

        // Uni-cast delivery
        Ptr<Ipv4Route> route = LookupArbiter(ipHeader.GetDestination(), ipHeader, p);
        if (route == 0) {

            // Lookup failed, so we did not find a route
            // If there are no other routing protocols, this will lead to a drop
            return false;

        } else {

            // Lookup succeeded in producing a route
            // So we perform the unicast callback to forward there
            ucb(route, p, ipHeader);
            return true;

        }

    }

    void
    Ipv4ArbiterRouting::NotifyInterfaceUp(uint32_t i) {

        // One IP address per interface
        NS_ABORT_MSG_IF(m_ipv4->GetNAddresses(i) != 1, "Each interface is permitted exactly one IP address.");

        // Get interface single IP's address and mask
        Ipv4Address if_addr = m_ipv4->GetAddress(i, 0).GetLocal();
        Ipv4Mask if_mask = m_ipv4->GetAddress(i, 0).GetMask();

        // Loopback interface must be 0
        NS_ABORT_MSG_IF(
                i == 0 && (if_addr != Ipv4Address("127.0.0.1") || if_mask != Ipv4Mask("255.0.0.0")),
                "Loopback interface 0 must have IP 127.0.0.1 and mask 255.0.0.0"
        );

        // Towards another interface
        // Check that the subnet mask is maintained
        NS_ABORT_MSG_IF(
                i != 0 &&
                if_mask.Get() != Ipv4Mask("255.255.255.0").Get(),
                "Each interface must have a subnet mask of 255.255.255.0"
        );

    }

    void
    Ipv4ArbiterRouting::NotifyInterfaceDown(uint32_t i) {
        throw std::runtime_error("Interfaces are not permitted to go down.");
    }

    void
    Ipv4ArbiterRouting::NotifyAddAddress(uint32_t interface, Ipv4InterfaceAddress address) {
        if (m_ipv4->IsUp(interface)) {
            throw std::runtime_error("Not permitted to add IP addresses after the interface has gone up.");
        }
    }

    void
    Ipv4ArbiterRouting::NotifyRemoveAddress(uint32_t interface, Ipv4InterfaceAddress address) {
        throw std::runtime_error("Not permitted to remove IP addresses.");
    }

    void
    Ipv4ArbiterRouting::SetIpv4(Ptr <Ipv4> ipv4) {
        NS_LOG_FUNCTION(this << ipv4);
        NS_ASSERT(m_ipv4 == 0 && ipv4 != 0);
        m_ipv4 = ipv4;
        for (uint32_t i = 0; i < m_ipv4->GetNInterfaces(); i++) {
            if (m_ipv4->IsUp(i)) {
                NotifyInterfaceUp(i);
            } else {
                NotifyInterfaceDown(i);
            }
        }
        m_nodeId = m_ipv4->GetObject<Node>()->GetId();
    }

    void 
    Ipv4ArbiterRouting::PrintRoutingTable(Ptr<OutputStreamWrapper> stream, Time::Unit unit) const {
        std::ostream* os = stream->GetStream ();
        *os << m_arbiter->StringReprOfForwardingState();
    }

    void
    Ipv4ArbiterRouting::SetArbiter (Ptr<Arbiter> arbiter) {
        m_arbiter = arbiter;
    }

    Ptr<Arbiter>
    Ipv4ArbiterRouting::GetArbiter () {
        return m_arbiter;
    }

} // namespace ns3
