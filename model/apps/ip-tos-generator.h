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

#ifndef IP_TOS_GENERATOR_H
#define IP_TOS_GENERATOR_H

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

class IpTosGenerator : public Object {
public:
    static TypeId GetTypeId(void);
    IpTosGenerator() {};
    virtual ~IpTosGenerator() {};

    /*
     * Generates the 8-bit value to set for the IP Type of Service (TOS) field.
     * Be aware that if the socket it is used for is a STREAM, the two least significant
     * bit will not be used as they are reserved for ECN setting by the socket.
     *
     * @param appTypeId   Application type identifier
     * @param app         Application
     *
     * @return IP TOS 8-bit value
     */
    virtual uint8_t GenerateIpTos(TypeId appTypeId, Ptr<Application> app) = 0;
};

class IpTosGeneratorDefault : public IpTosGenerator {
public:
    static TypeId GetTypeId(void);
    uint8_t GenerateIpTos(TypeId appTypeId, Ptr<Application> app);
};

}

#endif //IP_TOS_GENERATOR_H
