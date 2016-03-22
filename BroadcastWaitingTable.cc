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

#include "routeInterface/BroadcastWaitingTable.h"





void PacketWaitingTable::addPacket(cMessage * timer ,cPacket * packet,IPv4Datagram * ipv4)
{
    waitinglist[timer] = cPacketTOIPv4Datagram(packet, ipv4);

    // waitinglist[timer] = packet;
}

cPacket * PacketWaitingTable::getcPacket(cMessage * timer )
{
    TimetoRDPacket::const_iterator it = waitinglist.find(timer);
        if (it == waitinglist.end())
            return NULL;
        else
        {
            return it->second.first;
        }
}

IPv4Datagram * PacketWaitingTable::getDataPacket(cMessage * timer )
{
    TimetoRDPacket::const_iterator it = waitinglist.find(timer);
        if (it == waitinglist.end())
            return NULL;
        else
        {
            return it->second.second;
        }
}

void PacketWaitingTable::removePacket(cMessage*  timer) {
    TimetoRDPacket::iterator it = waitinglist.find(timer);
    printf("delete timer  %p\n",timer);
    waitinglist.erase(it);
 }

cMessage* PacketWaitingTable::findPacket(cPacket*  packet) {
    for (TimetoRDPacket::const_iterator it = waitinglist.begin(); it != waitinglist.end(); it++)
    {
        if(it->second.first==packet)
        {
            return it->first;
        }
    }
    return NULL;
}
cMessage* PacketWaitingTable::findPacket(std::string packetname) {
    for (TimetoRDPacket::const_iterator it = waitinglist.begin(); it != waitinglist.end(); it++)
    {
        if(it->second.first->getName()==packetname)
        {
            return it->first;
        }
    }
    return NULL;
}

std::vector<cMessage*> PacketWaitingTable::getAllmessages() {
    std::vector<cMessage*> ret;
    for (TimetoRDPacket::const_iterator it = waitinglist.begin(); it != waitinglist.end(); it++)
    {
        ret.push_back( it->first);
    }
    return ret;
}
std::vector<cPacket*> PacketWaitingTable::getAllcPackets() {
    std::vector<cPacket *> ret;
    for (TimetoRDPacket::const_iterator it = waitinglist.begin(); it != waitinglist.end(); it++)
    {
        ret.push_back( it->second.first);
    }
    return ret;
}
void PacketWaitingTable::clear() {
    waitinglist.clear();
}


