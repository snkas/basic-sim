/*
 * Copyright (c) 2021 ETH Zurich
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
 */

#include "socket-generator.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TcpSocketGenerator);
TypeId TcpSocketGenerator::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::TcpSocketGenerator")
            .SetParent<Object> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

NS_OBJECT_ENSURE_REGISTERED (TcpSocketGeneratorDefault);
TypeId TcpSocketGeneratorDefault::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::TcpSocketGeneratorDefault")
            .SetParent<TcpSocketGenerator> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

Ptr<Socket> TcpSocketGeneratorDefault::GenerateTcpSocket(TypeId appTypeId, Ptr<Application> app) {
    return Socket::CreateSocket(app->GetNode(), TcpSocketFactory::GetTypeId());
}

NS_OBJECT_ENSURE_REGISTERED (UdpSocketGenerator);
TypeId UdpSocketGenerator::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::UdpSocketGenerator")
            .SetParent<Object> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

NS_OBJECT_ENSURE_REGISTERED (UdpSocketGeneratorDefault);
TypeId UdpSocketGeneratorDefault::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::UdpSocketGeneratorDefault")
            .SetParent<UdpSocketGenerator> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

Ptr<Socket> UdpSocketGeneratorDefault::GenerateUdpSocket(TypeId appTypeId, Ptr<Application> app) {
    return Socket::CreateSocket(app->GetNode(), UdpSocketFactory::GetTypeId());
}

}
