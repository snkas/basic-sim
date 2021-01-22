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
#include "udp-burst-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpBurstHeader");

NS_OBJECT_ENSURE_REGISTERED (UdpBurstHeader);

TypeId
UdpBurstHeader::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::UdpBurstHeader")
            .SetParent<Header> ()
            .SetGroupName("BasicSim")
            .AddConstructor<UdpBurstHeader> ()
    ;
    return tid;
}

UdpBurstHeader::UdpBurstHeader ()
  : m_id (0),
    m_seq (0)
{
  NS_LOG_FUNCTION (this);
}

void
UdpBurstHeader::SetId (uint64_t id)
{
    NS_LOG_FUNCTION (this << id);
    m_id = id;
}

uint64_t
UdpBurstHeader::GetId (void) const
{
    NS_LOG_FUNCTION (this);
    return m_id;
}

void
UdpBurstHeader::SetSeq (uint64_t seq)
{
  NS_LOG_FUNCTION (this << seq);
  m_seq = seq;
}

uint64_t
UdpBurstHeader::GetSeq (void) const
{
  NS_LOG_FUNCTION (this);
  return m_seq;
}

TypeId
UdpBurstHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
UdpBurstHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  os << "(id=" << m_id << ", seq=" << m_seq << ")";
}

uint32_t
UdpBurstHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  return 8+8;
}

void
UdpBurstHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteHtonU64 (m_id);
  i.WriteHtonU64 (m_seq);
}

uint32_t
UdpBurstHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  m_id = i.ReadNtohU64 ();
  m_seq = i.ReadNtohU64 ();
  return GetSerializedSize ();
}

} // namespace ns3
