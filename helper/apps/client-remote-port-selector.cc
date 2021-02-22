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

#include "client-remote-port-selector.h"

namespace ns3 {

    NS_OBJECT_ENSURE_REGISTERED (ClientRemotePortSelector);
    TypeId ClientRemotePortSelector::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::ClientRemotePortSelector")
                .SetParent<Object> ()
                .SetGroupName("BasicSim")
        ;
        return tid;
    }

    NS_OBJECT_ENSURE_REGISTERED (ClientRemotePortSelectorDefault);
    TypeId ClientRemotePortSelectorDefault::GetTypeId (void)
    {
        static TypeId tid = TypeId ("ns3::ClientRemotePortSelectorDefault")
                .SetParent<ClientRemotePortSelector> ()
                .SetGroupName("BasicSim")
        ;
        return tid;
    }

    ClientRemotePortSelectorDefault::ClientRemotePortSelectorDefault(uint16_t remotePort) {
        m_remotePort = remotePort;
    }

    uint16_t ClientRemotePortSelectorDefault::SelectRemotePort(TypeId appTypeId, Ptr<Application> app) {
        return m_remotePort;
    }

}
