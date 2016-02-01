/*
 * RouteInterface.cpp
 *
 *  Created on: Jul 3, 2015
 *      Author: chengli
 */

#include <routeInterface/RouteInterface.h>
#include <routeInterface/RouteData.h>
#include "NodeOperations.h"

GlobalPositionTable RouteInterface::globalPositionTable=GlobalPositionTable();
RouteInterface::RouteInterface() {
    // TODO Auto-generated constructor stub

}

RouteInterface::~RouteInterface() {
    // TODO Auto-generated destructor stub
}
void RouteInterface::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == 0)
        {
    host = getContainingNode(this);
    nodeStatus = dynamic_cast<NodeStatus *>(host->getSubmodule("status"));
    //interfaceTable = InterfaceTableAccess().get();
    mobility = check_and_cast<IMobility *>(host->getSubmodule("mobility"));
    networkProtocol = check_and_cast<INetfilter *>(getModuleByPath(par("networkProtocolModule")));

    //routingTable = RoutingTableAccess().get();
    interfaceTable = InterfaceTableAccess().get();
    traci= TraCIMobilityAccess().get();
    tracimanager=TraCIScenarioManagerAccess().get();
        }

    if (stage == 4)
            {

            nb = NotificationBoardAccess().get();
            networkProtocol->registerHook(0, this);
            nb->subscribe(this, NF_LINK_FULL_PROMISCUOUS);
            }
 }
Coord RouteInterface::getConnectPosition(std::string conn)
{
    return tracimanager->commandGetJunctionPosition(conn);
}
void RouteInterface::receiveChangeNotification(int category, const cObject *details)
{
    EV_LOG("RouteInterface ","receiveChangeNotification");
    Enter_Method("receiveChangeNotification");
    if (category == NF_LINK_FULL_PROMISCUOUS)
      {
          cPacket * pk = dynamic_cast<cPacket*>(  const_cast<cObject *>(details) );
          if (pk)
          {
             Radio80211aControlInfo * cinfo = dynamic_cast<Radio80211aControlInfo*>(pk->getControlInfo());
             if (cinfo)
             {
                recPow = cinfo->getRecPow();
             }
          }
      }
}

void RouteInterface::sendUDPPacket(UDPPacket * packet, double delay)
{

    if (delay == 0)
        send(packet, "ipOut");
    else
        sendDelayed(packet, delay, "ipOut");
}

IPv4Address RouteInterface::getSelfIPAddress() const
{
    return routingTable->getRouterId();
}
void RouteInterface::EV_LOG(std::string className,std::string context)
{
  EV <<  className<<"  "<<context<<std::endl;
  std::cout <<  className<<"  "<<context<<std::endl;
}
void RouteInterface::sendRIPacket(cPacket * packet, const IPvXAddress& destAddr, unsigned int timeToLive, double delay)
{
    ASSERT(timeToLive != 0);
   // Enter_Method_Silent();

    IPv4ControlInfo *networkProtocolControlInfo = new IPv4ControlInfo();

    networkProtocolControlInfo->setTimeToLive(timeToLive);
    networkProtocolControlInfo->setProtocol(IP_PROT_MANET);
    networkProtocolControlInfo->setDestAddr(destAddr.get4());
    networkProtocolControlInfo->setSrcAddr(getSelfIPAddress());

    // TODO: Implement: support for multiple interfaces
   // InterfaceEntry *ifEntry = interfaceTable->getInterfaceByName("wlan0");
 //   networkProtocolControlInfo->setInterfaceId(ifEntry->getInterfaceId());

    UDPPacket *udpPacket = new UDPPacket( packet->getName());
    udpPacket->encapsulate(packet);
    udpPacket->setSourcePort(UDPPort);
    udpPacket->setDestinationPort(UDPPort);
    udpPacket->setControlInfo(dynamic_cast<cObject *>(networkProtocolControlInfo));
    sendUDPPacket(udpPacket,delay);
}

void RouteInterface::MulticastRIPacket(cPacket * packet,  unsigned int timeToLive, double delay)
{
    sendRIPacket(packet,IPv4Address::LL_MANET_ROUTERS,timeToLive,delay);
}

void RouteInterface::MulticastRIPacket(cPacket * packet)
{
    sendRIPacket(packet,IPv4Address::LL_MANET_ROUTERS,255,0);
}


void RouteInterface::handleMessage(cMessage *message)
{
    EV_LOG("RouteInterface","handleMessage");
    if (message->isSelfMessage()) {
        EV<<"RouteInterface  handle self message  "<<message<<endl;
        filterSelfMessage(message);
    }else
    {
        EV<<"RouteInterface  handle outer message  "<<message<<endl;
        UDPPacket *udpPacket = dynamic_cast<UDPPacket *>(message);
        cPacket *ctrlPacket = check_and_cast<cPacket *>(udpPacket->decapsulate());
        IPv4ControlInfo *udpProtocolCtrlInfo = dynamic_cast<IPv4ControlInfo *>(udpPacket->getControlInfo());
        processMessage(  ctrlPacket, udpProtocolCtrlInfo);
     }
}
void RouteInterface:: filterSelfMessage(cMessage * message)
{
    if (message == expungeTimer)
    {
      expungeRoutes();
    }
    else
    {
        processSelfMessage(message);
    }
}

std::string RouteInterface::getHostName() const
{
    return host->getFullName();
    //cout<<host;
}
std::string RouteInterface::getRoadID()
{
    //return traci->getRoadId();
   return adjustRoadID(traci->getRoadId());
    //cout<<host;
}
std::string  RouteInterface::getRoadIntersection(std::string road1,std::string road2)
{
    std::string firstroad="";
    std::string secondroad="";
    firstroad=road1.substr(0,3);
    secondroad=road1.substr(5);

    std::string firstroad2="";
    std::string secondroad2="";
    firstroad2=road2.substr(0,3);
    secondroad2=road2.substr(5);
    if(firstroad==firstroad2)
        return firstroad;
    if(firstroad==secondroad2)
           return firstroad;
    if(secondroad==secondroad2)
           return secondroad;
    if(secondroad==firstroad2)
           return secondroad;
}

bool RouteInterface::hasIntersection(std::string road,std::string intersection)
{
    std::string firstroad="";
     std::string secondroad="";
     firstroad=road.substr(0,3);
     secondroad=road.substr(5);
    if(firstroad==intersection)
            return true;
    if(secondroad==intersection)
              return true;
    return false;
}
std::string RouteInterface::adjustRoadID(std::string roadid)
{
    std::string firstroad="";
    std::string secondroad="";

    if (roadid.length()!=8){
    roadid.erase(roadid.find(":"),roadid.find(":")+1);
    }
    if (roadid[0]>roadid[5])
    {
        firstroad=roadid.substr(5);
        secondroad=roadid.substr(0,3);
        return firstroad+"to"+secondroad;
    }
    else{
        if (roadid[0]==roadid[5]&&roadid[2]>roadid[7])
        {
            firstroad=roadid.substr(5);
            secondroad=roadid.substr(0,3);
            return firstroad+"to"+secondroad;
        }
    }
    return roadid;
    //cout<<host;
}

IPvXAddress RouteInterface::getSelfAddress() const
{
    return routingTable->getRouterId();
}
Coord RouteInterface::getSelfPosition()
{
    return mobility->getCurrentPosition();
}
Coord RouteInterface::getSelfSpeed()
{
    return mobility->getCurrentSpeed();
}
void RouteInterface::expungeRoutes()
{

}

bool RouteInterface::handleOperationStage(LifecycleOperation * operation, int stage, IDoneCallback * doneCallback)
{
    Enter_Method_Silent();
    if (dynamic_cast<NodeStartOperation *>(operation))  {
        if (stage == NodeStartOperation::STAGE_APPLICATION_LAYER)
            int a = 1;//To do clear stage
           //configureInterfaces();
    }
    else if (dynamic_cast<NodeShutdownOperation *>(operation)) {
        if (stage == NodeShutdownOperation::STAGE_APPLICATION_LAYER)
            int a = 1;//To do clear stage
             //neighborPositionTable.clear();
    }
    else if (dynamic_cast<NodeCrashOperation *>(operation)) {
        if (stage == NodeCrashOperation::STAGE_CRASH)
            int a = 1;//To do clear stage
    }
    else throw cRuntimeError("Unsupported lifecycle operation '%s'", operation->getClassName());
    return true;
}
void RouteInterface::configureInterfaces( const char * interfaces)
{
    // join multicast groups
    cPatternMatcher interfaceMatcher(interfaces, false, true, false);
    for (int i = 0; i < interfaceTable->getNumInterfaces(); i++) {
        InterfaceEntry * interfaceEntry = interfaceTable->getInterface(i);
        if (interfaceEntry->isMulticast() && interfaceMatcher.matches(interfaceEntry->getName()))
        {
            EV_LOG("RBVTR","LL_MANET_ROUTERS");
            interfaceEntry->joinMulticastGroup(IPv4Address::LL_MANET_ROUTERS);
        }
    }
}
void RouteInterface::CarPostion()
{
    mobility->getCurrentPosition();
}
/*void RouteInterface::SetGloblePositionTable(GloblePositionTable gl){
    this->globlePositionTable=gl;
}*/

bool  RouteInterface::isRoadOfConn(std::string road,std::string conn)
{
    std::vector<std::string>  conns=getConnOfRoad(road);
    if(std::find(conns.begin(),conns.end(),conn)!=conns.end())
    {
        return true;
    }
    else
    {
        return false;
    }
}
std::vector<std::string>  RouteInterface::getConnOfRoad(std::string road)
{
    std::vector<std::string>  conn;
    conn.push_back(road.substr(0,3));
    conn.push_back(road.substr(5));
    return conn;
}
