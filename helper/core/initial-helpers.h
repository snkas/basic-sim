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

#ifndef INITIAL_HELPERS_H
#define INITIAL_HELPERS_H

#include "ns3/core-module.h"
#include "ns3/queue-size.h"

namespace ns3 {

    struct TypeId::AttributeInformation GetAttributeInformation(std::string tidName, std::string attributeName);
    int64_t GetInitialUintValue(std::string tidName, std::string attributeName);
    double GetInitialDoubleValue(std::string tidName, std::string attributeName);
    TimeValue GetInitialTimeValue(std::string tidName, std::string attributeName);
    bool GetInitialBooleanValue(std::string tidName, std::string attributeName);
    QueueSize GetInitialQueueSizeValue(std::string tidName, std::string attributeName);

} // namespace ns3

#endif /* INITIAL_HELPERS_H */
