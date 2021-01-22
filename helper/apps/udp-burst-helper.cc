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

#include "udp-burst-helper.h"

namespace ns3 {

UdpBurstServerHelper::UdpBurstServerHelper (Address localAddress, std::string baseLogsDir)
{
  m_factory.SetTypeId (UdpBurstServer::GetTypeId ());
  SetAttribute ("LocalAddress", AddressValue (localAddress));
  SetAttribute ("BaseLogsDir", StringValue (baseLogsDir));
}

void 
UdpBurstServerHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
UdpBurstServerHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

Ptr<Application>
UdpBurstServerHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<UdpBurstServer> ();
  node->AddApplication (app);
  return app;
}

UdpBurstClientHelper::UdpBurstClientHelper (Address localAddress, Address remoteAddress, int64_t udpBurstId, double targetRateMbps, Time duration, std::string additionalParameters, bool enableDetailedLoggingToFile, std::string baseLogsDir)
{
  m_factory.SetTypeId (UdpBurstClient::GetTypeId ());
  SetAttribute ("LocalAddress", AddressValue (localAddress));
  SetAttribute ("RemoteAddress", AddressValue (remoteAddress));
  SetAttribute ("UdpBurstId", UintegerValue (udpBurstId));
  SetAttribute ("TargetRateMbps", DoubleValue (targetRateMbps));
  SetAttribute ("Duration", TimeValue (duration));
  SetAttribute ("AdditionalParameters", StringValue (additionalParameters));
  SetAttribute ("EnableDetailedLoggingToFile", BooleanValue (enableDetailedLoggingToFile));
  SetAttribute ("BaseLogsDir", StringValue (baseLogsDir));
}

void 
UdpBurstClientHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
UdpBurstClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

Ptr<Application>
UdpBurstClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<UdpBurstClient> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
