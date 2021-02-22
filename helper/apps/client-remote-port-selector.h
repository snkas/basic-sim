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

#ifndef CLIENT_REMOTE_PORT_SELECTOR_H
#define CLIENT_REMOTE_PORT_SELECTOR_H

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

    class ClientRemotePortSelector : public Object {
    public:
        static TypeId GetTypeId(void);
        ClientRemotePortSelector() {};
        virtual ~ClientRemotePortSelector() {};

        /*
         * Generates the 16-bit remote port.
         *
         * @param appTypeId   Application type identifier
         * @param app         Application
         *
         * @return Port number (16-bit)
         */
        virtual uint16_t SelectRemotePort(TypeId appTypeId, Ptr<Application> app) = 0;
    };

    class ClientRemotePortSelectorDefault : public ClientRemotePortSelector {
    public:
        static TypeId GetTypeId(void);
        ClientRemotePortSelectorDefault(uint16_t remotePort);
        uint16_t SelectRemotePort(TypeId appTypeId, Ptr<Application> app);
    private:
        uint16_t m_remotePort;
    };

}

#endif //CLIENT_REMOTE_PORT_SELECTOR_H
