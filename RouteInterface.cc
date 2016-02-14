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
ofstream RouteInterface::inFile("/Applications/omnetpp-4.5/samples/inetmanet-2.0/src/networklayer/routing/RBVTR/log.txt",ios::trunc);
//RouteInterface::inFile.open("/Applications/omnetpp-4.5/samples/inetmanet-2.0/src/networklayer/routing/RBVTR/log.txt",ios::trunc);
RouteInterface::RouteInterface() {
    // TODO Auto-generated constructor stub
}

RouteInterface::~RouteInterface() {
   // inFile.close();
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
            initRoadsTable();
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
void RouteInterface::EV_LOG(std::string context)
{
    EV_LOG("RouteInterface",context);
}
void RouteInterface::EV_LOG(std::string className,std::string context)
{
  EV <<  className<<"  "<<context<<std::endl;
  std::cout <<getHostName()<<"   "<<  className<<"  "<<context<<std::endl;
}
void RouteInterface::sendRIPacket(cPacket * packet, const IPvXAddress& destAddr, unsigned int timeToLive, double delay)
{
    ASSERT(timeToLive != 0);
   // Enter_Method_Silent();
    EV_LOG("create UDP");

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
    EV_LOG("MulticastRIPacket");
    sendRIPacket(packet,IPv4Address::LL_MANET_ROUTERS,255,0);
}


void RouteInterface::handleMessage(cMessage *message)
{
    EV_LOG("handleMessage");
    if (message->isSelfMessage()) {
        EV_LOG("RouteInterface  handle self message  "+std::string(message->getName()));
        filterSelfMessage(message);
    }else
    {
        EV_LOG("RouteInterface  handle outer message  "+std::string(message->getName()));
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
bool  RouteInterface::isRoadVertical(std::string road1,std::string road2)
{
    std::string inter=getRoadIntersection(road1,road2);
    if(inter==std::string("none"))
        return false;
    std::string firstroad="";
    std::string secondroad="";
    firstroad=road1.substr(0,3);
    secondroad=road1.substr(5);
    if(secondroad!=inter)
    {
        firstroad=secondroad;
    }
    std::string firstroad2="";
    std::string secondroad2="";
    firstroad2=road2.substr(0,3);
    secondroad2=road2.substr(5);
    if(secondroad2!=inter)
        {
            firstroad2=secondroad2;
        }
    if((firstroad.substr(0,1)!=firstroad2.substr(0,1))&&(firstroad.substr(1)!=firstroad2.substr(1)))
    {
        return true;
    }
    return false;
}

std::string  RouteInterface::getRoadIntersection(std::string road1,std::string road2)
{
    EV_LOG("getRoadIntersection in"+road1+"  "+road2);

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
    return std::string("none");
}

bool RouteInterface::hasIntersection(std::string road,std::string intersection)
{
    std::string firstroad="";
     std::string secondroad="";
     if(road.length()!=8||intersection.length()!=3)
         return false;
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
    if (roadid.length()==9){
    roadid.erase(roadid.find(":"),roadid.find(":")+1);
    }else
    {
        if(roadid.length()==6)
        {
            roadid.erase(roadid.find(":"),roadid.find(":")+1);
            return roadid.substr(0,3);
        }else
        {
            if(roadid.length()!=8)
               {
            return roadid;
               }
        }
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
            interfaceEntry->joinMulticastGroup(IPv4Address::LL_MANET_ROUTERS);
        }
    }
}
double RouteInterface::getAngel()
{
    //return traci->getRoadId();
   return  traci->getAngleRad();
    //cout<<host;
}

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
void RouteInterface::initRoadsTable()
{
std::list<std::string> roadsname=tracimanager->commandGetLaneIds();
    for(std::list<std::string>::iterator iter=roadsname.begin();iter!=roadsname.end();++iter)
    {
        std::string templane=*iter;
        if(templane.length()==10)
        {
            templane=templane.substr(0,8);
            roadslist.push_back(templane);
        }
    }
  }
string RouteInterface::caculateConnection(std::string conn1,std::string conn2)
{
    std::vector<std::string> conn1road=getConnOfRoad(conn1);
    std::vector<std::string> conn2road=getConnOfRoad(conn2);
    std::string testconn0=adjustRoadID(conn1road[0]+"to"+conn2road[0]);
    std::cout <<testconn0<<endl;
    std::string testconn1=adjustRoadID(conn1road[0]+"to"+conn2road[1]);
    std::cout <<testconn1<<endl;

    std::string testconn2=adjustRoadID(conn1road[1]+"to"+conn2road[0]);
    std::cout <<testconn2<<endl;

    std::string testconn3=adjustRoadID(conn1road[1]+"to"+conn2road[1]);
    std::cout <<testconn3<<endl;

    for(int i=0;i<roadslist.size();i++)
    {
        std::cout <<roadslist[i]<<endl;
        if(roadslist[i]==testconn0)
            return testconn0;
        if(roadslist[i]==testconn1)
                   return testconn1;
        if(roadslist[i]==testconn2)
                   return testconn2;
        if(roadslist[i]==testconn3)
                   return testconn3;

    }
    return std::string("none");
   // UDPPacket*o=NULL;
   // cPacket *p=check_and_cast<cPacket*>(o);
  }

