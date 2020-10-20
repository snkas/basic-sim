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

#ifndef TCP_OPTIMIZER_H
#define TCP_OPTIMIZER_H

#include "ns3/basic-simulation.h"
#include "ns3/ipv4-routing-helper.h"

namespace ns3 {

    class TcpOptimizer
    {
    public:
        static void OptimizeBasic(Ptr<BasicSimulation> basicSimulation);
        static void OptimizeUsingWorstCaseRtt(Ptr<BasicSimulation> basicSimulation, int64_t worst_case_rtt_ns);
    private:
        static void Generic();
    };

} // namespace ns3

#endif /* TCP_OPTIMIZER_H */
