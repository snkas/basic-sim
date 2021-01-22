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
 * Adapted from BulkSendHelper by:
 * Author: Geoge Riley <riley@ece.gatech.edu>
 * Adapted from OnOffHelper by:
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "tcp-flow-helper.h"

namespace ns3 {

TcpFlowServerHelper::TcpFlowServerHelper (Address address)
{
    m_factory.SetTypeId ("ns3::TcpFlowServer");
    SetAttribute ("Local", AddressValue (address));
}

void
TcpFlowServerHelper::SetAttribute (std::string name, const AttributeValue &value)
{
    m_factory.Set (name, value);
}

ApplicationContainer
TcpFlowServerHelper::Install (Ptr<Node> node) const
{
    return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
TcpFlowServerHelper::Install (NodeContainer c) const
{
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
        apps.Add (InstallPriv (*i));
    }

    return apps;
}

Ptr<Application>
TcpFlowServerHelper::InstallPriv (Ptr<Node> node) const
{
    Ptr<Application> app = m_factory.Create<Application> ();
    node->AddApplication (app);

    return app;
}

TcpFlowClientHelper::TcpFlowClientHelper (Address address, uint64_t maxBytes, int64_t flowId, bool enableFlowLoggingToFile, std::string baseLogsDir, std::string additionalParameters)
{
  m_factory.SetTypeId ("ns3::TcpFlowClient");
  m_factory.Set ("Remote", AddressValue (address));
  m_factory.Set ("MaxBytes", UintegerValue (maxBytes));
  m_factory.Set ("TcpFlowId", UintegerValue (flowId));
  m_factory.Set ("EnableTcpFlowLoggingToFile", BooleanValue (enableFlowLoggingToFile));
  m_factory.Set ("BaseLogsDir", StringValue (baseLogsDir));
  m_factory.Set ("AdditionalParameters", StringValue (additionalParameters));
}

ApplicationContainer
TcpFlowClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

Ptr<Application>
TcpFlowClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Application> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
