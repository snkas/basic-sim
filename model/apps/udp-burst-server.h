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

#ifndef UDP_BURST_SERVER_H
#define UDP_BURST_SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/string.h"
#include "ns3/exp-util.h"
#include "ns3/traced-callback.h"
#include "ns3/udp-burst-header.h"
#include "ns3/socket-generator.h"

namespace ns3 {

class Socket;
class Packet;

class UdpBurstServer : public Application 
{
public:
  static TypeId GetTypeId (void);
  UdpBurstServer ();
  virtual ~UdpBurstServer ();

  void SetUdpSocketGenerator(Ptr<UdpSocketGenerator> udpSocketGenerator);
  void SetIpTos(uint8_t ipTos);

  uint32_t GetMaxSegmentSizeByte() const;
  uint32_t GetMaxUdpPayloadSizeByte() const;
  void RegisterIncomingBurst(int64_t udp_burst_id, bool enable_precise_logging);
  std::vector<std::tuple<int64_t, uint64_t>> GetIncomingBurstsInformation();
  uint64_t GetReceivedCounterOf(int64_t udp_burst_id);

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);
  void HandleRead (Ptr<Socket> socket);

  Address m_localAddress;            //!< Local address (IPv4, port) the server operates out of
  std::string m_baseLogsDir;         //!< Where the logs will be written to: logs_dir/udp_burst_[id]_incoming.csv
  uint32_t m_maxSegmentSizeByte;     //!< Maximum segment size
  uint32_t m_maxUdpPayloadSizeByte;  //!< Maximum size of UDP payload before it gets fragmented
  Ptr<UdpSocketGenerator> m_udpSocketGenerator;  //!< UDP socket generator

  // State
  Ptr<Socket> m_socket;  //!< IPv4 Socket

  // Logging
  std::vector<int64_t> m_incoming_bursts;                               //!< Registered incoming bursts
  std::map<int64_t, uint64_t> m_incoming_bursts_received_counter;       //!< Counter for how many packets received
  std::map<int64_t, uint64_t> m_incoming_bursts_enable_precise_logging; //!< True iff enable precise logging for each burst

};

} // namespace ns3

#endif /* UDP_BURST_SERVER_H */
