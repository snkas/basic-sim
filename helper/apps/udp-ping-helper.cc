/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Adapted from UdpEchoHelper by:
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "udp-ping-helper.h"

namespace ns3 {

UdpPingServerHelper::UdpPingServerHelper (Address localAddress)
{
  m_factory.SetTypeId (UdpPingServer::GetTypeId ());
  SetAttribute ("LocalAddress", AddressValue (localAddress));
}

void 
UdpPingServerHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
UdpPingServerHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

Ptr<Application>
UdpPingServerHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<UdpPingServer> ();
  node->AddApplication (app);
  return app;
}

UdpPingClientHelper::UdpPingClientHelper (Address localAddress, Address remoteAddress, int64_t udpPingId, Time interval, Time duration, Time waitAfterwards, std::string additionalParameters)
{
  m_factory.SetTypeId (UdpPingClient::GetTypeId ());
  SetAttribute ("LocalAddress", AddressValue (localAddress));
  SetAttribute ("RemoteAddress", AddressValue (remoteAddress));
  SetAttribute ("UdpPingId", UintegerValue (udpPingId));
  SetAttribute ("Interval", TimeValue (interval));
  SetAttribute ("Duration", TimeValue (duration));
  SetAttribute ("WaitAfterwards", TimeValue (waitAfterwards));
  SetAttribute ("AdditionalParameters", StringValue (additionalParameters));
}

void 
UdpPingClientHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
UdpPingClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

Ptr<Application>
UdpPingClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<UdpPingClient> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
