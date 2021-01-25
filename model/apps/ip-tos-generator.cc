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

#include "ip-tos-generator.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (IpTosGenerator);
TypeId IpTosGenerator::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::IpTosGenerator")
            .SetParent<Object> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

NS_OBJECT_ENSURE_REGISTERED (IpTosGeneratorDefault);
TypeId IpTosGeneratorDefault::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::IpTosGeneratorDefault")
            .SetParent<IpTosGenerator> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

uint8_t IpTosGeneratorDefault::GenerateIpTos(TypeId appTypeId, Ptr<Application> app) {
    return 0;
}

}
