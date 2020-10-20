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

#ifndef ARBITER_ECMP_HELPER_H
#define ARBITER_ECMP_HELPER_H

#include "ns3/ipv4-routing-helper.h"
#include "ns3/basic-simulation.h"
#include "ns3/topology-ptop.h"
#include "ns3/ipv4-arbiter-routing.h"
#include "ns3/arbiter-ecmp.h"

namespace ns3 {

    class ArbiterEcmpHelper
    {
    public:
        static void InstallArbiters (Ptr<BasicSimulation> basicSimulation, Ptr<TopologyPtop> topology);
    private:
        static std::vector<std::vector<std::vector<uint32_t>>> CalculateGlobalState(Ptr<TopologyPtop> topology);
    };

} // namespace ns3

#endif /* ARBITER_ECMP_HELPER_H */
