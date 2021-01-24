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

#ifndef SOCKET_GENERATOR_H
#define SOCKET_GENERATOR_H

#include "ns3/core-module.h"
#include "ns3/node-container.h"
#include "ns3/topology.h"
#include "ns3/exp-util.h"
#include "ns3/basic-simulation.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/traffic-control-helper.h"
#include "ns3/point-to-point-ab-helper.h"

namespace ns3 {
    
class TcpSocketGenerator : public Object {
public:
    static TypeId GetTypeId(void);
    TcpSocketGenerator() {};
    virtual ~TcpSocketGenerator() {};

    /*
     * Generate a TCP socket for an application that it would correspond to.
     *
     * @param appTypeId   Application type identifier
     * @param app         Application
     *
     * @return TCP socket pointer
     */
    virtual Ptr<Socket> GenerateTcpSocket(TypeId appTypeId, Ptr<Application> app) = 0;
};

class TcpSocketGeneratorDefault : public TcpSocketGenerator {
public:
    static TypeId GetTypeId(void);
    Ptr<Socket> GenerateTcpSocket(TypeId appTypeId, Ptr<Application> app);
};

class UdpSocketGenerator : public Object {
public:
    static TypeId GetTypeId(void);
    UdpSocketGenerator() {};
    virtual ~UdpSocketGenerator() {};

    /*
     * Generate a UDP socket for an application that it would correspond to.
     *
     * @param appTypeId   Application type identifier
     * @param app         Application
     *
     * @return UDP socket pointer
     */
    virtual Ptr<Socket> GenerateUdpSocket(TypeId appTypeId, Ptr<Application> app) = 0;
};

class UdpSocketGeneratorDefault : public UdpSocketGenerator {
public:
    static TypeId GetTypeId(void);
    Ptr<Socket> GenerateUdpSocket(TypeId appTypeId, Ptr<Application> app);
};

}

#endif //SOCKET_GENERATOR_H
