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

#ifndef __INET_ROADTABLE_H_
#define __INET_ROADTABLE_H_

#include <vector>
#include <map>
#include "INETDefs.h"
#include "IPvXAddress.h"
#include "Coord.h"

using namespace std;

/**
 * This class provides a mapping between node addresses and their positions.
 */
class INET_API RoadTable {
    private:
          typedef std::map<IPvXAddress, Coord  > AddressToPosition;

         typedef std::map<IPvXAddress, std::vector<std::string> > AddressToRoad;
         AddressToRoad roadtable;
         AddressToPosition positontable;

    public:
         RoadTable() { }


        vector<IPvXAddress>  getAllIP();

        bool hasRoadTable(const IPvXAddress & address);
        bool addRoadTable(const IPvXAddress & address,vector<string> roads,Coord position);
        void updataRoadTable(const IPvXAddress & address,vector<string> roads,Coord position);
        vector<string>  getRoadTable(const IPvXAddress & address);
        void  findnewRoadid(const IPvXAddress & address,string roadid) ;

        void removeRoadTable(const IPvXAddress & address);
        Coord  getPositionTable(const IPvXAddress & address) ;


        void clear();

};

#endif
