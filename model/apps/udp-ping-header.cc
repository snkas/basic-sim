/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 INRIA
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
 * Based on SeqTsHeader by:
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include "udp-ping-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpPingHeader");

NS_OBJECT_ENSURE_REGISTERED (UdpPingHeader);

TypeId
UdpPingHeader::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::UdpPingHeader")
            .SetParent<Header> ()
            .SetGroupName("BasicSim")
            .AddConstructor<UdpPingHeader> ()
    ;
    return tid;
}

UdpPingHeader::UdpPingHeader ()
  : m_id (0),
    m_seq (0),
    m_ts (0)
{
  NS_LOG_FUNCTION (this);
}

void
UdpPingHeader::SetId (uint64_t id)
{
    NS_LOG_FUNCTION (this << id);
    m_id = id;
}

uint64_t
UdpPingHeader::GetId (void) const
{
    NS_LOG_FUNCTION (this);
    return m_id;
}

void
UdpPingHeader::SetSeq (uint64_t seq)
{
  NS_LOG_FUNCTION (this << seq);
  m_seq = seq;
}

uint64_t
UdpPingHeader::GetSeq (void) const
{
  NS_LOG_FUNCTION (this);
  return m_seq;
}

void
UdpPingHeader::SetTs (uint64_t ts)
{
    NS_LOG_FUNCTION (this << ts);
    m_ts = ts;
}

uint64_t
UdpPingHeader::GetTs (void) const
{
    NS_LOG_FUNCTION (this);
    return m_ts;
}

TypeId
UdpPingHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
UdpPingHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "(id=" << m_id << ", seq=" << m_seq << ", ts=" << m_ts << ")";
}

uint32_t
UdpPingHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  return 8+8+8;
}

void
UdpPingHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteHtonU64 (m_id);
  i.WriteHtonU64 (m_seq);
  i.WriteHtonU64 (m_ts);
}

uint32_t
UdpPingHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  m_id = i.ReadNtohU64 ();
  m_seq = i.ReadNtohU64 ();
  m_ts = i.ReadNtohU64 ();
  return GetSerializedSize ();
}

} // namespace ns3
