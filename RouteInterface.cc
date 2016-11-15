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
std::string RouteInterface::protocalname;
//ofstream RouteInterface::inFile("/home/xixi/Workspace/inetmanet-2.0/src/networklayer/routing/log.txt",ios::trunc);
ofstream RouteInterface::inFile("/home/xixiang/omnetpp-4.5/samples/inetmanet-2.0/src/networklayer/routing/log.txt",ios::trunc);
//ofstream RouteInterface::inFile("/home/mehdi/workspace/inetmanet-2.0/src/networklayer/routing/log.txt",ios::trunc);
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
        checkRoadIDTimer = new cMessage("checkRoadIDTimer");
        nb = NotificationBoardAccess().get();
        networkProtocol->registerHook(0, this);
        nb->subscribe(this, NF_LINK_FULL_PROMISCUOUS);
        initRoadsTable();
        globalPositionTable.setHostName(getSelfIPAddress(),getHostName());
        globalPositionTable.setPosition(getSelfIPAddress(),getSelfPosition());
        globalPositionTable.setRoadID(getSelfIPAddress(),getRoadID());
        scheduleCheckRoadIDTimer(1);
    }
}
void RouteInterface::scheduleCheckRoadIDTimer(simtime_t Interval)
{
//    EV_LOG("Scheduling beacon timer");
    //todo :  The fewer neighbors there are, the more frequent is a node��s HELLO beaconing.
    scheduleAt(simTime() + Interval, checkRoadIDTimer);
}

Coord RouteInterface::getPositionOfJunction(std::string conn)
{
    return tracimanager->commandGetJunctionPosition(conn);
}
void RouteInterface::receiveChangeNotification(int category, const cObject *details)
{
    //EV_LOG("RouteInterface ","receiveChangeNotification");
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
        delete udpPacket;
    }
}
void RouteInterface:: updateGlobalpositionTable()
{
    globalPositionTable.setPosition(getSelfIPAddress(),getSelfPosition());
    globalPositionTable.setRoadID(getSelfIPAddress(),getRoadID());
    scheduleCheckRoadIDTimer(1);
}

void RouteInterface:: filterSelfMessage(cMessage * message)
{
    if (message == checkRoadIDTimer)
    {
        updateGlobalpositionTable();
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
    std::string inter=getConnectingJunctionBetweenTwoRoads(road1,road2);
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

std::string  RouteInterface::getConnectingJunctionBetweenTwoRoads(std::string road1,std::string road2)
{
    EV_LOG("getConnectingJunctionBetweenTwoRoads in "+road1+"  "+road2);

    std::string firstroad="";
    std::string secondroad="";
    firstroad=road1.substr(0,3);
    secondroad=road1.substr(5);
    //    std::cout << firstroad << endl;
    //    std::cout << secondroad << endl;

    //        std::cout << road1 << endl;
    //        std::cout << road2 << endl;

    std::string firstroad2="";
    std::string secondroad2="";
    firstroad2=road2.substr(0,3);
    secondroad2=road2.substr(5);
    //    std::cout << firstroad2 << endl;
    //    std::cout << secondroad2 << endl;

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

bool RouteInterface::hasJunction(std::string road,std::string junction)
{
    std::string firstroad="";
    std::string secondroad="";
    if(road.length()!=8||junction.length()!=3)
        return false;
    firstroad=road.substr(0,3);
    secondroad=road.substr(5);
    if(firstroad==junction)
        return true;
    if(secondroad==junction)
        return true;
    return false;
}

std::string RouteInterface::adjustRoadID(std::string roadid)
{
//    std::cout << "roadid = " << roadid << endl;
    std::string firstroad="";
    std::string secondroad="";
    if (roadid.length()==9)
    {
//        std::cout << simTime() << " = 6.020545636002 ?!" << endl;
        roadid.erase(roadid.find(":"),roadid.find(":")+1);
//        std::cout << "roadid = " << roadid << endl;
//        std::cout << " " << endl;
    }
    else
    {
        if(roadid.length()==6)
        {
            roadid.erase(roadid.find(":"),roadid.find(":")+1);
//            std::cout << roadid.substr(0,3) << endl;
//            std::cout << " " << endl;
            return roadid.substr(0,3);
        }
        else
        {
            if(roadid.length()==7)
            {
                roadid.erase(roadid.find(":"),roadid.find(":")+1);
//                std::cout << roadid.substr(0,3) << endl;
//                std::cout << " " << endl;
                return roadid.substr(0,3);
            }
            if(roadid.length()!=8)
            {
//                std::cout << "roadid = " << roadid << endl;
//                std::cout << " " << endl;
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

bool  RouteInterface::isRoadOfJunction(std::string road,std::string junction)
{
    std::vector<std::string>  junctions=getJunctionsOfRoad(road);
    if(std::find(junctions.begin(),junctions.end(),junction)!=junctions.end())
    {
        return true;
    }
    else
    {
        return false;
    }
}

std::vector<std::string>  RouteInterface::getJunctionsOfRoad(std::string road)
{
    std::vector<std::string>  conn;
    conn.push_back(road.substr(0,3));
    conn.push_back(road.substr(5));
    return conn;
}

void RouteInterface::initRoadsTable()
{
    std::list<std::string> roadsname=tracimanager->commandGetLaneIds();
    for(std::list<std::string>::iterator iter=roadsname.begin();iter!=roadsname.end();iter++)
    {
        std::string templane=*iter;
        //        std::cout<<templane<<endl;
        if(templane.length()==10)
        {
            templane=templane.substr(0,8);
            if(std::find(roadslist.begin(),roadslist.end(),templane)==roadslist.end()){
                //            templane=templane.substr(0,8);
                roadslist.push_back(templane);
            }
        }
    }
}



std::string RouteInterface::getConnectingRoadBetweenTwoRoads(std::string road1,std::string road2)
{
    //calculate the road between 2 roads like 1/3to2/3  3/3to4/3  return 2/3to3/3
    std::vector<std::string> conn1road=getJunctionsOfRoad(road1);
    std::vector<std::string> conn2road=getJunctionsOfRoad(road2);
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

bool RouteInterface::hasConnectingJunctionBetweenTwoRoads(std::string road1,std::string road2)
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
        return true;
    if(firstroad==secondroad2)
        return true;
    if(secondroad==secondroad2)
        return true;
    if(secondroad==firstroad2)
        return true;
    return false;
}





bool  RouteInterface::isLocalateInIntersection()
{
    return getRoadID().length()<8;
}

std::vector<std::string>  RouteInterface::getRoadsOfJunction(std::string junction)
{
    std::vector<std::string>  roads;
    for(std::vector<std::string>::iterator it=roadslist.begin();it!=roadslist.end();it++){
        std::string road= *it;
        //std::cout<<road<<endl;
    }
    for(std::vector<std::string>::iterator it=roadslist.begin();it!=roadslist.end();it++)
        //    for (int i=0;i<roadslist.size();i++)
    {
        std::string road= *it;
        if(isRoadOfJunction(road,junction))
        {
            roads.push_back(road);
            //            std::cout<<road<<endl;
            if(roads.size()==8)
            {
                break;
            }
        }
    }
    return roads;
}
std::string RouteInterface::getConnectingJunction(std::string conn1,std::string conn2)
{
    //calculate the road between 2 roads like 1/3to2/3  3/3to4/3  return 2/3to3/3
    std::vector<std::string>roadsofconn1 = getRoadsOfJunction(conn1);
    std::vector<std::string>roadsofconn2 = getRoadsOfJunction(conn2);
    for (int i=0;i<roadsofconn1.size();i++)
    {
        for (int j=0;j<roadsofconn2.size();j++)
        {
            string junction =getConnectingJunctionBetweenTwoRoads(roadsofconn2[j],roadsofconn1[i]);
            if(junction!="none")
                return junction;
        }
    }
    return std::string("none");
    // UDPPacket*o=NULL;
    // cPacket *p=check_and_cast<cPacket*>(o);
}

bool RouteInterface::hasConnectingBetweenJunction(std::string conn1, std::string conn2)
{
    std::vector<std::string>roadsofconn1 = getRoadsOfJunction(conn1);
    std::vector<std::string>roadsofconn2 = getRoadsOfJunction(conn2);
    for(int i=0;i<roadsofconn1.size();i++)
    {
        //      std::cout<<roadsofconn1[i];
        for(int j=0;j<roadsofconn2.size();j++)
        {
            //          std::cout<<roadsofconn2[2];
            if(roadsofconn1[i]==roadsofconn2[j])
                return true;
        }
    }
    return false;
}
std::string RouteInterface::roadBetweenJunctionAndRoad(std::string road, std::string intersection)
{
    std::vector<std::string> connroad=getJunctionsOfRoad(road);
    std::string testconn0=adjustRoadID(connroad[0]+"to"+intersection);
    std::string testconn1=adjustRoadID(connroad[1]+"to"+intersection);
    for(int i=0;i<roadslist.size();i++)
    {
        std::cout <<roadslist[i]<<endl;
        if(roadslist[i]==testconn0)
            return testconn0;
        if(roadslist[i]==testconn1)
            return testconn1;

    }
    return std::string("none");

}

std::string RouteInterface::roadBetweenFarJunctionAndRoad(std::string road, std::string intersection)
{
    std::vector<std::string> connroad=getJunctionsOfRoad(road);
    std::string junction=getConnectingJunction(connroad[0],intersection);
    std::string road1="none";
    if(junction!="none"){
        road1=adjustRoadID(connroad[0]+"to"+junction);

    }
    else{
        junction=getConnectingJunction(connroad[1],intersection);
        if(junction!="none")
        road1=adjustRoadID(connroad[1]+"to"+junction);

    }
    return road1;
}
bool RouteInterface::hasRoad(std::string road1, std::string road2, std::string road3)
{
    std::string testconn0=adjustRoadID(road1+"to"+road3);
    std::string testconn1=adjustRoadID(road2+"to"+road3);
    for(int i=0;i<roadslist.size();i++)
    {
        std::cout <<roadslist[i]<<endl;
        if(roadslist[i]==testconn0)
            return true;
        if(roadslist[i]==testconn1)
            return true;

    }
    return false;
}

double RouteInterface::junctionFrequence()
{
    std::vector<IPvXAddress> vehicleAddresses = globalPositionTable.getAddresses();
    std::vector<std::string>junctionids;
    for (std::vector<IPvXAddress>::iterator it = vehicleAddresses.begin(); it != vehicleAddresses.end(); it++) {
        const IPvXAddress & vehicleAddress = *it;
        std::string roadid=globalPositionTable.getRoadID(vehicleAddress);
        if(roadid.length()==3){
            if(std::find(junctionids.begin(),junctionids.end(),roadid)==junctionids.end())
                junctionids.push_back(roadid);
        }

    }
    double  junctionSize=junctionids.size();
    double  junctionNumber=1-junctionSize/25;
    std::cout<<"junctionid size"<<junctionids.size()<<endl;
    return junctionNumber;
}
