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

#ifndef TCP_FLOW_SERVER_H
#define TCP_FLOW_SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/traced-callback.h"
#include "ns3/address.h"
#include "ns3/socket-generator.h"

namespace ns3 {

class Address;
class Socket;
class Packet;

class TcpFlowServer : public Application
{
public:
  static TypeId GetTypeId (void);
  TcpFlowServer ();
  virtual ~TcpFlowServer ();

  void SetTcpSocketGenerator(Ptr<TcpSocketGenerator> tcpSocketGenerator);
  void SetIpTos(uint8_t ipTos);

  uint64_t GetTotalRx();
 
protected:
  virtual void DoDispose (void);

private:
    
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop
  
  void HandleRead (Ptr<Socket> socket);
  void HandleAccept (Ptr<Socket> socket, const Address& from);
  void HandlePeerClose (Ptr<Socket> socket);
  void HandlePeerError (Ptr<Socket> socket);
  void CleanUp (Ptr<Socket> socket);

  // Parameters
  Address m_localAddress; //!< Local address to bind to

  // State
  Ptr<Socket> m_socket;                 //!< Listening socket (upon connection request, forks off a socket)
  std::list<Ptr<Socket> > m_socketList; //!< List of accepted sockets (are removed upon closed)
  uint64_t m_totalRx;                   //!< Total (payload) bytes received
  Ptr<TcpSocketGenerator> m_tcpSocketGenerator;  //!< TCP socket generator

};

} // namespace ns3

#endif /* TCP_FLOW_SERVER_H */
