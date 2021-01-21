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

#ifndef UDP_PING_CLIENT_H
#define UDP_PING_CLIENT_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/string.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/udp-ping-header.h"

namespace ns3 {

class Socket;
class Packet;

class UdpPingClient : public Application 
{
public:
  static TypeId GetTypeId (void);
  UdpPingClient ();
  virtual ~UdpPingClient ();

  uint32_t GetUdpPingId();
  uint32_t GetSent();
  std::vector<int64_t> GetSendRequestTimestamps();
  std::vector<int64_t> GetReplyTimestamps();
  std::vector<int64_t> GetReceiveReplyTimestamps();

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);
  void Finish ();
  void ScheduleTransmit (Time dt);
  void Send (void);
  void HandleRead (Ptr<Socket> socket);

  Time m_start_time; //!< Start time
  Time m_interval; //!< Packet inter-send time
  Time m_duration; //!< Duration of the sending
  Time m_wait_afterwards; //!< How long to wait after the duration is over before closing socket
  std::string m_additionalParameters; //!< Additional parameters
  Ptr<Socket> m_socket; //!< Socket
  Address m_localAddress; //!< Local address (IP, port)
  Address m_peerAddress;  //!< Remote peer address (IP, port)
  EventId m_sendEvent; //!< Event to send the next packet
  EventId m_waitForFinishEvent; //!< Event to wait to finish the client

  uint32_t m_udpPingId; //!< Unique UDP ping identifier
  uint32_t m_sent; //!< Counter for sent packets

  std::vector<int64_t> m_sendRequestTimestamps;
  std::vector<int64_t> m_replyTimestamps;
  std::vector<int64_t> m_receiveReplyTimestamps;

};

} // namespace ns3

#endif /* UDP_PING_CLIENT_H */
