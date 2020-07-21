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

#ifndef UDP_BURST_APPLICATION_H
#define UDP_BURST_APPLICATION_H

#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/traced-callback.h"
#include "ns3/seq-ts-header.h"

namespace ns3 {

    class Socket;
    class Packet;

    class UdpBurstApplication : public Application
    {
    public:
        static TypeId GetTypeId (void);
        UdpBurstApplication ();
        virtual ~UdpBurstApplication ();
        void RegisterBurst(
                InetSocketAddress targetAddress,
                int64_t udp_burst_id,
                int64_t rate_byte_per_s,
                int64_t start_time_ns,
                int64_t duration_ns,
                std::string additional_parameters
        );

    protected:
        virtual void DoDispose (void);

    private:
        virtual void StartApplication (void);
        virtual void StopApplication (void);
        void HandleRead (Ptr<Socket> socket);

        uint16_t m_port;      //!< Port on which we listen for incoming packets.
        Ptr<Socket> m_socket; //!< IPv4 Socket
        Address m_local;      //!< local multicast address
//
//        std::vector<Address> node_id_to_remote_address;
//        std::vector<UdpBurstScheduleEntry> m_burst_schedule; // Hops from one burst to the next (Event numero uno)
//        std::vector<UdpBurstScheduleEntry> m_active_bursts; // Each time finds the burst which has the least amount of time remaining, and then plans that (Event numero dos)

    };

} // namespace ns3

#endif /* UDP_BURST_APPLICATION_H */
