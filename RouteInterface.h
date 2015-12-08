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
#include "routeInterface/GloblePositionTable.h"
#include "TraCIMobility.h"
#include "routeInterface/DelayPacketTable.h"
#include <string>
#include <iostream>
#include <algorithm>

class RouteInterface: public cSimpleModule,
        public ILifecycle,
        public INotifiable,
        public INetfilter::IHook {
public:
    RouteInterface();
    virtual ~RouteInterface();
    DelayPacketTable delayPacketlist;
protected:
    unsigned int UDPPort;
    IRoutingTable *routingTable;
    IInterfaceTable *interfaceTable;
    NodeStatus * nodeStatus;
    IMobility * mobility;
    TraCIMobility* traci;
    TraCIScenarioManager* tracimanager;
    cModule * host;
    INetfilter * networkProtocol;
    NotificationBoard *nb;


    static GloblePositionTable globlePositionTable;

    //self message
    cMessage *expungeTimer;    // timer to clean the routing table out


    IPv4Address getSelfIPAddress() const;
    void sendUDPPacket(UDPPacket * packet, double delay);
    void sendRIPacket(cPacket * packet, const IPvXAddress& destAddr, unsigned int timeToLive, double delay);
    void initialize(int stage);
    void MulticastRIPacket(cPacket * packet,  unsigned int timeToLive, double delay);
    void MulticastRIPacket(cPacket * packet);
    void handleMessage(cMessage *message);

    void processUDPPacket(UDPPacket * packet);
    void expungeRoutes();


    void filterSelfMessage(cMessage * message);

    virtual void processSelfMessage(cMessage * message){};
    virtual void processMessage(cPacket * ctrlPacket,IPv4ControlInfo *udpProtocolCtrlInfo){};
    std::string  getHostName() const;
    std::string  getRoadID() ;
    std::string getRoadIntersection(std::string road1,std::string road2);
    bool hasIntersection(std::string road,std::string intersce);

    IPvXAddress  getSelfAddress() const;
    virtual int numInitStages() const { return 5; }

    // netfilter
    virtual Result datagramPreRoutingHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop){EV   <<"RBVTR" <<"inter :PreRouting"<<std::endl; return ACCEPT; };
    virtual Result datagramForwardHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop){ EV   <<"RBVTR" <<"inter :ForwardHook"<<std::endl; return ACCEPT; };
    virtual Result datagramPostRoutingHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop) { EV   <<"RBVTR" <<"inter:Postrouting"<<std::endl; return ACCEPT; };
    virtual Result datagramLocalInHook(IPv4Datagram * datagram, const InterfaceEntry * inputInterfaceEntry) {EV   <<"RBVTR" <<"inter:IN"<<std::endl; return ACCEPT; };
    virtual Result datagramLocalOutHook(IPv4Datagram * datagram, const InterfaceEntry *& outputInterfaceEntry, IPv4Address & nextHop){ EV   <<"RBVTR" <<"inter:Out"<<std::endl;return ACCEPT; };

    // lifecycle
    virtual bool handleOperationStage(LifecycleOperation * operation, int stage, IDoneCallback * doneCallback);

    // notification
    virtual void receiveChangeNotification(int signalID, const cObject *obj){};

    void CarPostion();
    std::string adjustRoadID(std::string roadid);

    void EV_LOG(std::string className,std::string context);
   // void SetGloblePositionTable(GloblePositionTable gl);
    void  configureInterfaces(const char * interfaces);
    Coord getSelfPosition();
    bool  isRoadOfConn(std::string road,std::string conn);
    std::vector<std::string> getConnOfRoad(std::string road);

};

#endif /* RouteInterface_H_ */
