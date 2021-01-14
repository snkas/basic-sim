/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
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
 * Based on QueueTracker
 */

#ifndef QDISC_QUEUE_TRACKER_H
#define QDISC_QUEUE_TRACKER_H

#include <vector>
#include <stdexcept>

#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/queue-disc.h"
#include "ns3/log-update-helper.h"


namespace ns3 {

    class QdiscQueueTracker : public Object {

    public:
        static TypeId GetTypeId (void);
        QdiscQueueTracker(Ptr<QueueDisc> qdisc);
        void QueueDiscPacketsInQueueCallback(uint32_t, uint32_t num_packets);
        void QueueDiscBytesInQueueCallback(uint32_t, uint32_t num_bytes);
        const std::vector<std::tuple<int64_t, int64_t, int64_t>>& GetIntervalsNumPackets();
        const std::vector<std::tuple<int64_t, int64_t, int64_t>>& GetIntervalsNumBytes();

    private:

        // Parameters
        Ptr<QueueDisc> m_qdisc;

        // State
        LogUpdateHelper<int64_t> m_log_update_helper_qdisc_pkt;
        LogUpdateHelper<int64_t> m_log_update_helper_qdisc_byte;

    };

}

#endif // QDISC_QUEUE_TRACKER_H
