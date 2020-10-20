/*
 * Copyright (c) 2020 ETH Zurich
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

#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include "ns3/core-module.h"
#include "ns3/node-container.h"

namespace ns3 {

class Topology : public Object
{
public:
    static TypeId GetTypeId (void);
    virtual ~Topology() {}
    virtual const NodeContainer& GetNodes() = 0;
    virtual int64_t GetNumNodes() = 0;
    virtual bool IsValidEndpoint(int64_t node_id) = 0;
    virtual const std::set<int64_t>& GetEndpoints() = 0;
};

}

#endif //TOPOLOGY_H
