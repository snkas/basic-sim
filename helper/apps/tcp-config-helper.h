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

#ifndef TCP_CONFIG_HELPER_H
#define TCP_CONFIG_HELPER_H

#include "ns3/basic-simulation.h"
#include "ns3/ipv4-routing-helper.h"
#include "ns3/initial-helpers.h"

namespace ns3 {

    class TcpConfigHelper
    {
    public:
        static void Configure(Ptr<BasicSimulation> basicSimulation);
    private:
        static void ConfigureBasic();
        static void ConfigureCustom(Ptr<BasicSimulation> basicSimulation);
        static void SetTcpAttributes(
                int64_t clock_granularity_ns,
                int64_t init_cwnd_pkt,
                int64_t snd_buf_size_byte,
                int64_t rcv_buf_size_byte,
                int64_t segment_size_byte,
                bool opt_timestamp_enabled,
                bool opt_sack_enabled,
                bool opt_win_scaling_enabled,
                bool pacing_enabled,
                int64_t delayed_ack_packet_count,
                bool no_delay,
                int64_t max_seg_lifetime_ns,
                int64_t min_rto_ns,
                int64_t initial_rtt_estimate_ns,
                int64_t connection_timeout_ns,
                int64_t delayed_ack_timeout_ns,
                int64_t persist_timeout_ns
        );
    };

} // namespace ns3

#endif /* TCP_CONFIG_HELPER_H */
