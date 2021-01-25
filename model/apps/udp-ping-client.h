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
#include "ns3/socket-generator.h"

namespace ns3 {

class Socket;
class Packet;

class UdpPingClient : public Application 
{
public:
  static TypeId GetTypeId (void);
  UdpPingClient ();
  virtual ~UdpPingClient ();

  void SetUdpSocketGenerator(Ptr<UdpSocketGenerator> udpSocketGenerator);
  void SetIpTos(uint8_t ipTos);

  uint32_t GetUdpPingId();
  std::string GetAdditionalParameters();

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

  // Parameters
  Address m_localAddress;                //!< Local address (IP, port)
  Address m_remoteAddress;               //!< Remote (server) address (IP, port)
  uint32_t m_udpPingId;                  //!< Unique UDP ping identifier
  Time m_interval;                       //!< Packet inter-send time
  Time m_duration;                       //!< Duration of the sending
  Time m_waitAfterwards;                 //!< How long to wait after the duration is over before closing socket
  std::string m_additionalParameters;    //!< Additional parameters (unused; reserved for future use)
  Ptr<UdpSocketGenerator> m_udpSocketGenerator;  //!< UDP socket generator

  // State
  Time m_startTime;             //!< Start time
  Ptr<Socket> m_socket;         //!< Socket
  EventId m_sendEvent;          //!< Event to send the next packet
  EventId m_waitForFinishEvent; //!< Event to wait to finish the client
  uint32_t m_sent;              //!< Counter for sent packets

  // Logging
  std::vector<int64_t> m_sendRequestTimestamps;
  std::vector<int64_t> m_replyTimestamps;
  std::vector<int64_t> m_receiveReplyTimestamps;

};

} // namespace ns3

#endif /* UDP_PING_CLIENT_H */
