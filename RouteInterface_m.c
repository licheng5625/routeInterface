/*
 * RouteInterface.h
 *
 *  Created on: Jul 3, 2015
 *      Author: chengli
 */

#ifndef RouteInterface_H_
#define RouteInterface_H_

#include <csimplemodule.h>
#include <ILifecycle.h>
#include <INetfilter.h>
#include <INotifiable.h>
#include "UDPPacket.h"
#include "IPv4Address.h"
#include "IRoutingTable.h"
#include "IPv4ControlInfo.h"
#include "RoutingTableAccess.h"
#include "InterfaceTableAccess.h"
#include "ILifecycle.h"
#include "IMobility.h"
#include "INetfilter.h"
#include "IRoutingTable.h"
#include "NodeStatus.h"
#include "NotificationBoard.h"
#include "INETDefs.h"
USING_NAMESPACE


#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "GPSR_m.h"

USING_NAMESPACE


// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}




// Template rule for outputting std::vector<T> types
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');

    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

Register_Class(GPSRBeacon);

GPSRBeacon::GPSRBeacon(const char *name, int kind) : ::cPacket(name,kind)
{
}

GPSRBeacon::GPSRBeacon(const GPSRBeacon& other) : ::cPacket(other)
{
    copy(other);
}

GPSRBeacon::~GPSRBeacon()
{
}
