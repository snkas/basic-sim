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

#ifndef UDP_BURST_CLIENT_H
#define UDP_BURST_CLIENT_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/exp-util.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/udp-burst-header.h"
#include "ns3/socket-generator.h"

namespace ns3 {

class Socket;
class Packet;

class UdpBurstClient : public Application 
{
public:
  static TypeId GetTypeId (void);
  UdpBurstClient ();
  virtual ~UdpBurstClient ();

  void SetUdpSocketGenerator(Ptr<UdpSocketGenerator> udpSocketGenerator);
  void SetIpTos(uint8_t ipTos);

  uint32_t GetUdpBurstId();
  std::string GetAdditionalParameters();

  uint32_t GetSent();
  uint32_t GetMaxSegmentSizeByte() const;
  uint32_t GetMaxUdpPayloadSizeByte() const;

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);
  void Finish ();
  void ScheduleTransmit (Time dt);
  void Send (void);

  // Parameters
  Address m_localAddress;                //!< Local address (IP, port)
  Address m_remoteAddress;               //!< Remote (server) address (IP, port)
  uint32_t m_udpBurstId;                 //!< Unique UDP burst identifier
  Time m_startTime;                      //!< Start time
  double m_targetRateMegabitPerSec;      //!< Target rate (incl. headers) in Mbit/s
  Time m_duration;                       //!< Duration of the sending
  std::string m_additionalParameters;    //!< Additional parameters (unused; reserved for future use)
  bool m_enableDetailedLoggingToFile;    //!< True iff you want to write detailed logs
  std::string m_baseLogsDir;             //!< Where the logs will be written to: logs_dir/udp_burst_[id]_outgoing.csv
  uint32_t m_maxSegmentSizeByte;         //!< Maximum segment size
  uint32_t m_maxUdpPayloadSizeByte;      //!< Maximum size of UDP payload before it gets fragmented
  Ptr<UdpSocketGenerator> m_udpSocketGenerator;  //!< UDP socket generator

  // State
  Ptr<Socket> m_socket;  //!< Socket
  EventId m_sendEvent;   //!< Event to send the next packet
  uint32_t m_sent;       //!< Counter for sent packets

};

} // namespace ns3

#endif /* UDP_BURST_CLIENT_H */
