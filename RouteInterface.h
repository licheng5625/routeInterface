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
#include "Radio80211aControlInfo_m.h"
#include "ILifecycle.h"
#include "IMobility.h"
#include "INetfilter.h"
#include "IRoutingTable.h"
#include "NodeStatus.h"
#include "NotificationBoard.h"
#include "INETDefs.h"
#include "routeInterface/GlobalPositionTable.h"
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
    double recPow;
    std::string roadid;
    cMessage * checkRoadIDTimer;

protected:
    unsigned int UDPPort;
    IRoutingTable *routingTable;
    IInterfaceTable *interfaceTable;
    NodeStatus * nodeStatus;
    IMobility * mobility;
    cModule * host;
    INetfilter * networkProtocol;
    NotificationBoard *nb;
    static std::string protocalname;

    static GlobalPositionTable globalPositionTable;
    static ofstream inFile;
    std::vector<std::string>roadslist;
    TraCIMobility* traci;
    TraCIScenarioManager* tracimanager;

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

    virtual void receiveChangeNotification(int category, const cObject *details);

    void filterSelfMessage(cMessage * message);
    void  updateGlobalpositionTable();

    virtual void processSelfMessage(cMessage * message){};
    virtual void processMessage(cPacket * ctrlPacket,IPv4ControlInfo *udpProtocolCtrlInfo){};
    std::string  getHostName() const;
    std::string  getRoadID() ;
    bool  isRoadVertical(std::string road1,std::string road2);

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

    std::string adjustRoadID(std::string roadid);
//    std::string adjustRoadIDnew(std::string roadid);

    void EV_LOG(std::string className,std::string context);
    void EV_LOG(std::string context);

   // void SetGloblePositionTable(GloblePositionTable gl);
    void  configureInterfaces(const char * interfaces);
    Coord getSelfPosition();
    Coord getSelfSpeed();
    bool  isRoadOfJunction(std::string road,std::string Junction);
    double getAngel();
    std::vector<std::string> getJunctionsOfRoad(std::string road);
    std::vector<std::string> getRoadsOfJunction(std::string junction);
    Coord getPositionOfJunction(std::string conn);
    void scheduleCheckRoadIDTimer(simtime_t Interval);

    void initRoadsTable();
    std::string getConnectingRoadBetweenTwoRoads(std::string road1,std::string road2);
    std::string getConnectingJunctionBetweenTwoRoads(std::string road1,std::string road2);
    bool hasJunction(std::string road,std::string junction);
    string getConnectingJunction(std::string conn1,std::string conn2);
    bool  isLocalateInIntersection();
    bool  isLocalateInIntersection(string roodID);

    bool isCoordinator(const IPvXAddress & address);

bool hasConnectingJunctionBetweenTwoRoads(std::string road1,std::string road2);
//std::vector<std::string> getRoadsOfJunction(std::string junction);
//std::string getConnectingJunction(std::string conn1,std::string conn2);
bool hasConnectingBetweenJunction(std::string conn1, std::string conn2);
std::string roadBetweenJunctionAndRoad(std::string road, std::string intersection);
std::string roadBetweenFarJunctionAndRoad(std::string road, std::string intersection);
bool hasRoad(std::string road1, std::string road2, std::string road3);
double junctionFrequence();
};

#endif /* RouteInterface_H_ */
