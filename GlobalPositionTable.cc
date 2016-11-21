//
// Copyright (C) 2004 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "routeInterface/GlobalPositionTable.h"

static double const NaN = 0.0 / 0.0;

std::vector<IPvXAddress> GlobalPositionTable::getAddresses() const {
    std::vector<IPvXAddress> addresses;
    for (AddressToPositionMap::const_iterator it = addressToPositionMap.begin(); it != addressToPositionMap.end(); it++)
        addresses.push_back(it->first);
    return addresses;
}

bool GlobalPositionTable::hasPosition(const IPvXAddress & address) const {
    AddressToPositionMap::const_iterator it = addressToPositionMap.find(address);
    return it != addressToPositionMap.end();
}

Coord GlobalPositionTable::getPosition(const IPvXAddress & address) const {
    AddressToPositionMap::const_iterator it = addressToPositionMap.find(address);
    if (it == addressToPositionMap.end())
        return Coord(NaN, NaN, NaN);
    else
        return it->second.second;
}
void GlobalPositionTable::setPosition(const IPvXAddress & address, const Coord & coord){
    ASSERT(!address.isUnspecified());
    addressToPositionMap[address] = AddressToPositionMapValue(simTime(), coord);
}

void GlobalPositionTable::setHostName(const IPvXAddress & address, const std::string hostname)
{
    ASSERT(!address.isUnspecified());
    addressToHostnameMap[address] = hostname;
}

std::string GlobalPositionTable::getHostName(const IPvXAddress & address) const {
    AddressToHostnameMap::const_iterator it = addressToHostnameMap.find(address);
        if (it == addressToHostnameMap.end())
            return "NONE";
        else
            return it->second;
}

void GlobalPositionTable::removePosition(const IPvXAddress & address) {
    AddressToPositionMap::iterator it = addressToPositionMap.find(address);
    addressToPositionMap.erase(it);
}

void GlobalPositionTable::removeOldPositions(simtime_t timestamp) {
    for (AddressToPositionMap::iterator it = addressToPositionMap.begin(); it != addressToPositionMap.end();)
        if (it->second.first <= timestamp)
            addressToPositionMap.erase(it++);
        else
            it++;
}

void GlobalPositionTable::clear() {
    addressToPositionMap.clear();
    addressToHostnameMap.clear();
}

void GlobalPositionTable::setRoadID(const IPvXAddress & address, const std::string roadID)
{
    ASSERT(!address.isUnspecified());
    addressToRoadIDMap[address] = roadID;
}

std::string GlobalPositionTable::getRoadID(const IPvXAddress & address) const
{
    AddressToRoadIDMap::const_iterator it = addressToRoadIDMap.find(address);
    if (it == addressToRoadIDMap.end()) {
        return "NONE";
    }
    else {
        return it->second;
    }
}
simtime_t
GlobalPositionTable::getOldestPosition() const {
    simtime_t oldestPosition =
SimTime::getMaxTime();
    for (AddressToPositionMap::const_iterator it
= addressToPositionMap.begin(); it != addressToPositionMap.end();
it++) {
        const simtime_t & time =
it->second.first;
        if (time < oldestPosition)
            oldestPosition = time;
    }
    return oldestPosition;
}
