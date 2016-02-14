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

#include "RoadTable.h"

#define RBVTR_EV EV << "RBVTR at "




bool RoadTable::addRoadTable(const IPvXAddress & address,vector<string> roads,Coord position)
{
    AddressToRoad::const_iterator it = roadtable.find(address);
    positontable[address]=position;

           if (it == roadtable.end())
           {
               roadtable[address]=roads;
               std::cout<<"addRoadTable true"<<endl;
               return true;
           }
           else
           {
               if (it->second.size()>roads.size())
               {
                std::cout<<"addRoadTable true size: "<<it->second.size()<<" new size: "<<roads.size()<<endl;
                roadtable[address]=roads;
                return true;
               }
           }
           return false;
}
void RoadTable::updataRoadTable(const IPvXAddress & address,vector<string> roads,Coord position)
{
                positontable[address]=position;
                roadtable[address]=roads;
}
vector<IPvXAddress>  RoadTable::getAllIP()
        {
      AddressToRoad::iterator it ;
      vector<IPvXAddress> IPs;
           for(it=roadtable.begin();it != roadtable.end();it++)
           {
               IPs.push_back(it->first);
           }
           return IPs;
        }

bool RoadTable::hasRoadTable(const IPvXAddress & address)   {

    AddressToRoad::const_iterator it = roadtable.find(address);
              if (it == roadtable.end())
              {
                  return false;
              }
              else
              {
                  return true;
              }
}
void RoadTable::findnewRoadid(const IPvXAddress & address,string roadid)   {

    vector<string> roads =roadtable[address];
    vector<string>:: iterator it =  roads.begin() ;
    EV<< "road size: "<<roads.size()<<endl ;
    std::cout<< "road size: "<<roads.size()<<endl ;
    for(int i=0;i<roads.size();i++)
    {
        EV<< "road "<<roads[i]<<endl ;
        std::cout<< "road   "<<roads[i]<<endl ;
    }
    for ( ; it != roads.end(); ++it )
    {
        if(*it == roadid&&it!=roads.begin())
        {
            EV<< "road shorter"<<endl ;
            std::cout<< "road shorter"<<endl ;
            roads.erase(roads.begin(),it-1);
           /* vector<string> temproad(roads.size(), "hello");
            for(int i =roads.size()-1;i>=0;i--)
            {
                temproad.push_back()
            }*/
            roadtable[address]=roads;
            return;
         }
      }

    if (roadid.substr(0,3)==roads[0].substr(5,7))
    {
        EV<< "turn "<<roads[0]<<"to"<<roadid<<endl ;
        std::cout<< "turn "<<roads[0]<<"to"<<roadid<<endl ;
        roads.erase(roads.begin(),roads.begin()+1);
    }
    roads.insert(roads.begin(),1,roadid);
    roadtable[address]=roads;
      return;

}
vector<string> RoadTable::getRoadTable(const IPvXAddress & address)   {

    return roadtable[address];
}
Coord RoadTable::getPositionTable(const IPvXAddress & address)   {

    return positontable[address];
}
void RoadTable::removeRoadTable(const IPvXAddress & address) {

}



void RoadTable::clear() {
    roadtable.clear();
}


