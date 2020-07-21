/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Georgia Institute of Technology
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
 * Adapted from BulkSendApplication by:
 * Author: George F. Riley <riley@ece.gatech.edu>
 */

#ifndef FLOW_SEND_APPLICATION_H
#define FLOW_SEND_APPLICATION_H

#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/string.h"
#include "ns3/traced-callback.h"

namespace ns3 {

class Address;
class Socket;

class FlowSendApplication : public Application
{
public:
  static TypeId GetTypeId (void);

  FlowSendApplication ();

  virtual ~FlowSendApplication ();

  int64_t GetAckedBytes();
  int64_t GetCompletionTimeNs();
  bool IsCompleted();
  bool IsConnFailed();
  bool IsClosedByError();
  bool IsClosedNormally();

protected:
  virtual void DoDispose (void);
private:
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop

  /**
   * Send data until the L4 transmission buffer is full.
   */
  void SendData ();

  Ptr<Socket>     m_socket;       //!< Associated socket
  Address         m_peer;         //!< Peer address
  bool            m_connected;    //!< True if connected
  uint32_t        m_sendSize;     //!< Size of data to send each time
  uint64_t        m_maxBytes;     //!< Limit total number of bytes sent
  uint64_t        m_flowId;       //!< Flow identifier
  uint64_t        m_totBytes;     //!< Total bytes sent so far
  TypeId          m_tid;          //!< The type of protocol to use.
  int64_t         m_completionTimeNs; //!< Completion time in nanoseconds
  bool            m_connFailed;       //!< Whether the connection failed
  bool            m_closedNormally;   //!< Whether the connection closed normally
  bool            m_closedByError;    //!< Whether the connection closed by error
  uint64_t        m_ackedBytes;       //!< Amount of acknowledged bytes cached after close of the socket
  bool            m_isCompleted;      //!< True iff the flow is completed fully AND closed normally

  // Flow logging
  bool m_enableFlowLoggingToFile;          //!< True iff you want to write flow logs
  std::string m_baseLogsDir;               //!< Where the flow logs will be written to:
                                           //!<   logs_dir/flow-[id]-{progress, cwnd, rtt}.csv
  std::string m_additionalParameters;      //!< Not used in this version of the application
  TracedCallback<Ptr<const Packet> > m_txTrace;

private:
  void ConnectionSucceeded (Ptr<Socket> socket);
  void ConnectionFailed (Ptr<Socket> socket);
  void DataSend (Ptr<Socket>, uint32_t);
  void SocketClosedNormal(Ptr<Socket> socket);
  void SocketClosedError(Ptr<Socket> socket);
  void RttChange(Time oldRtt, Time newRtt);
  void CwndChange(uint32_t oldCwnd, uint32_t newCwnd);

};

} // namespace ns3

#endif /* FLOW_SEND_APPLICATION_H */
