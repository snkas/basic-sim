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

#include "initial-helpers.h"

namespace ns3 {

struct TypeId::AttributeInformation GetAttributeInformation(std::string tidName, std::string attributeName) {

    // Retrieve the type
    TypeId tid;
    if (!TypeId::LookupByNameFailSafe (tidName, &tid)) {
        throw std::invalid_argument("Type not found: " + tidName);
    }

    // Retrieve the attribute
    struct TypeId::AttributeInformation info;
    if (!tid.LookupAttributeByName (attributeName, &info)) {
        throw std::invalid_argument("Type " + tidName + " does not have an attribute named " + attributeName);
    }

    return info;

}

int64_t GetInitialUintValue(std::string tidName, std::string attributeName) {

    // Retrieve the base value
    struct TypeId::AttributeInformation info = GetAttributeInformation(tidName, attributeName);

    // Convert into unsigned integer
    const UintegerValue* uintAttribute = dynamic_cast<const UintegerValue*>(PeekPointer(info.initialValue));
    if (uintAttribute == 0) {
        throw std::invalid_argument("Value provided for " + tidName + "::" + attributeName + " is not an unsigned integer");
    }
    return uintAttribute->Get();

}

double GetInitialDoubleValue(std::string tidName, std::string attributeName) {

    // Retrieve the base value
    struct TypeId::AttributeInformation info = GetAttributeInformation(tidName, attributeName);

    // Convert into a double
    const DoubleValue* doubleAttribute = dynamic_cast<const DoubleValue*>(PeekPointer(info.initialValue));
    if (doubleAttribute == 0) {
        throw std::invalid_argument("Value provided for " + tidName + "::" + attributeName + " is not a double");
    }
    return doubleAttribute->Get();

}

TimeValue GetInitialTimeValue(std::string tidName, std::string attributeName) {

    // Retrieve the base value
    struct TypeId::AttributeInformation info = GetAttributeInformation(tidName, attributeName);

    // Convert into time value
    const TimeValue* timeAttribute = dynamic_cast<const TimeValue*>(PeekPointer(info.initialValue));
    if (timeAttribute == 0) {
        throw std::invalid_argument("Value provided for " + tidName + "::" + attributeName + " is not a time type");
    }
    return timeAttribute->Get();

}

bool GetInitialBooleanValue(std::string tidName, std::string attributeName) {

    // Retrieve the base value
    struct TypeId::AttributeInformation info = GetAttributeInformation(tidName, attributeName);

    // Convert into boolean
    const BooleanValue* boolAttribute = dynamic_cast<const BooleanValue*>(PeekPointer(info.initialValue));
    if (boolAttribute == 0) {
        throw std::invalid_argument("Value provided for " + tidName + "::" + attributeName + " is not a boolean");
    }
    return boolAttribute->Get();

}

}
