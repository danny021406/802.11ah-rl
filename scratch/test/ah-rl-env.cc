#include "ah-rl-env.h"
#include <sstream>

#include <iostream>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <algorithm>

namespace ns3 {


NS_LOG_COMPONENT_DEFINE ("AhGymEnv");

NS_OBJECT_ENSURE_REGISTERED (AhGymEnv);

//uint64_t recvBytes = 1;

AhGymEnv::AhGymEnv ()
{
  NS_LOG_FUNCTION (this);

  Simulator::Schedule (Seconds(5), &AhGymEnv::ScheduleNextStateRead, this);

}

AhGymEnv::AhGymEnv (uint32_t MaxGroupNumber, uint32_t MaxStationNumber, uint32_t SimulationTime, Statistics *stats, vector<NodeEntry*> nodes):m_stats(stats), m_nodes(nodes)
{
  NS_LOG_FUNCTION (this);
  m_slotNum = 0;
  m_repeatChoose = 0;
  m_maxGroupNumber = MaxGroupNumber;
  m_maxStationNumber = MaxStationNumber;
  m_simulationTime = SimulationTime;

  Simulator::Schedule (Seconds(5), &AhGymEnv::ScheduleNextStateRead, this);

}

void
AhGymEnv::ScheduleNextStateRead ()
{
  NS_LOG_FUNCTION (this);
//   if (m_slotNum <15){
//   	Simulator::Schedule (m_stepInterval1, &AhGymEnv::ScheduleNextStateRead, this);
//   }
//   else {
//   	Simulator::Schedule (m_stepInterval2, &AhGymEnv::ScheduleNextStateRead, this);
//   }
  
  Simulator::Schedule (Seconds(5), &AhGymEnv::ScheduleNextStateRead, this);
  Notify();
  
}

AhGymEnv::~AhGymEnv ()
{
  NS_LOG_FUNCTION (this);
}

TypeId
AhGymEnv::GetTypeId (void)
{
  static TypeId tid = TypeId ("AhGymEnv")
    .SetParent<OpenGymEnv> ()
    .SetGroupName ("OpenGym")
    .AddConstructor<AhGymEnv> ()
  ;
  return tid;
}

void
AhGymEnv::DoDispose ()
{
  NS_LOG_FUNCTION (this);
}

/*
Define action space
*/
Ptr<OpenGymSpace>
AhGymEnv::GetActionSpace()
{
  // output : grouping config propotion
  uint32_t configRange = m_maxGroupNumber * 3; // the range of slot number the node could choose

  float low = 0;
  float high = 1;
  std::vector<uint32_t> shape = {configRange,};
  std::string dtype = TypeNameGet<float> ();

  Ptr<OpenGymBoxSpace> box = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);
  NS_LOG_UNCOND ("AhGetActionSpace: "<<box);
  return box;
}

/*
Define observation space
*/
Ptr<OpenGymSpace>
AhGymEnv::GetObservationSpace()
{
  NS_LOG_FUNCTION (this);
  // input :
    
  // Grouping config box
  uint32_t groupingConfigNum = m_maxGroupNumber * 3;

  float low = 0;
  float high = 1;
  std::vector<uint32_t> shape = {groupingConfigNum, };
  std::string dtype = TypeNameGet<float> ();

  Ptr<OpenGymBoxSpace> groupingConfig_box = CreateObject<OpenGymBoxSpace> (low, high, shape, dtype);

    
  // Station state box
  uint32_t stationNum = m_maxStationNumber * 3;

  float low2 = 0;
  float high2 = 2000;
  std::vector<uint32_t> shape2 = {stationNum, };
  std::string dtype2 = TypeNameGet<float> ();

  Ptr<OpenGymBoxSpace> stationsStat_box = CreateObject<OpenGymBoxSpace> (low2, high2, shape2, dtype2);

  // Statistics box
  uint32_t statisticNum = m_maxGroupNumber + 1;

  float low3 = 0;
  float high3 = 1;
  std::vector<uint32_t> shape3 = {statisticNum, };
  std::string dtype3 = TypeNameGet<float> ();

  Ptr<OpenGymBoxSpace> statistics_box = CreateObject<OpenGymBoxSpace> (low3, high3, shape3, dtype3);
    
    
    
  Ptr<OpenGymDictSpace> space = CreateObject<OpenGymDictSpace> ();
  space->Add("groupng config", groupingConfig_box);
  space->Add("stations state", stationsStat_box);
  space->Add("statistics", statistics_box);
 

  NS_LOG_UNCOND ("AhGetObservationSpace"<<space);

  return space;

}


/*
Define game over condition
*/
bool
AhGymEnv::GetGameOver()
{
  NS_LOG_FUNCTION (this);

  
  bool isGameOver = false;
  
  // Time up
  if (Simulator::Now ().GetSeconds () > Seconds(m_simulationTime))
  {
      isGameOver = true;
  } 
  
/*    
  // Reward < -300
  Ptr<Node> node = NodeList::GetNode (m_slotNum);
  Ptr<NetDevice> dev = node-> GetDevice(0);
  Ptr<TdmaNetDevice> m_tdmaDevice = DynamicCast<TdmaNetDevice>(dev);
  float* reward = m_tdmaDevice->GetTdmaController()->GetRLReward(m_slotNum);
 
  for (uint32_t i=0;i<3;i++)
  {
     if (*(reward+i) <= -100)
     {
       m_repeatChoose++;   
     }
  }
  
  
  
  if (m_repeatChoose >= 3)
  {
     isGameOver = true;
  }
*/    
  return isGameOver;
}


/*
Collect observations
*/
Ptr<OpenGymDataContainer>
AhGymEnv::GetObservation()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_UNCOND("Now: "<<Simulator::Now().GetNanoSeconds ());
    
    int pay = 0;
    int totalSuccessfulPackets = 0;
    int totalSentPackets = 0;
    int totalPacketsEchoed = 0;
    for (int i = 0; i < m_maxStationNumber; i++)
    {
        totalSuccessfulPackets += m_stats->get(i).NumberOfSuccessfulPackets;
        totalSentPackets += m_stats->get(i).NumberOfSentPackets;
        totalPacketsEchoed += m_stats->get(i).NumberOfSuccessfulRoundtripPackets;
        
        pay += m_stats->get(i).TotalPacketPayloadSize;
//         cout << i << m_stats->get(i).getIPCameraSendingRate() << " sent: " << m_stats->get(i).NumberOfSentPackets
//                 << " ; delivered: " << m_stats->get(i).NumberOfSuccessfulPackets
//                 << " ; echoed: " << m_stats->get(i).NumberOfSuccessfulRoundtripPackets
//                 << "; packetloss: "<< endl;
        cout << i << " AID " << m_nodes[i]->aId << " group number: " << m_nodes[i]->rawGroupNumber <<endl;
//                 << stats.get(i).GetPacketLoss(config.trafficType) << endl;
    }
    
    
    // To Do
    uint32_t groupingconfigNum = m_maxGroupNumber * 3;
    std::vector<uint32_t> shape1 = {groupingconfigNum,};
    Ptr<OpenGymBoxContainer<float> > groupConfig_box = CreateObject<OpenGymBoxContainer<float> >(shape1);

    for (uint32_t i=0; i<m_maxGroupNumber; i++) //nodeUsedList.size()
    {
        groupConfig_box->AddValue ( m_stats->get(i).getIPCameraSendingRate());
        groupConfig_box->AddValue ( m_stats->get(i).getIPCameraSendingRate());
        groupConfig_box->AddValue ( m_stats->get(i).getIPCameraSendingRate());
    }
    
    uint32_t stationNum = m_maxStationNumber * 3;
    std::vector<uint32_t> shape2 = {stationNum,};
    Ptr<OpenGymBoxContainer<float> > stationStats_box = CreateObject<OpenGymBoxContainer<float> >(shape2);

    for (uint32_t i=0; i<m_maxStationNumber; i++) //nodeUsedList.size()
    {
        stationStats_box->AddValue ( m_stats->get(i).getIPCameraSendingRate());
        stationStats_box->AddValue ( m_stats->get(i).getIPCameraSendingRate());
        stationStats_box->AddValue ( m_stats->get(i).getIPCameraSendingRate());
    }
    
    
    uint32_t statisticNum = m_maxGroupNumber + 1;
    std::vector<uint32_t> shape3 = {statisticNum, };
    Ptr<OpenGymBoxContainer<float> > statistic_box = CreateObject<OpenGymBoxContainer<float> >(shape3);
    
    for (uint32_t i=0; i<m_maxGroupNumber; i++) //nodeUsedList.size()
    {
        statistic_box->AddValue ( m_stats->get(i).getIPCameraSendingRate());
        statistic_box->AddValue ( m_stats->get(i).getIPCameraSendingRate());
        statistic_box->AddValue ( m_stats->get(i).getIPCameraSendingRate());
    }
    statistic_box->AddValue ( m_stats->get(3).getIPCameraSendingRate());
    
    Ptr<OpenGymTupleContainer> data = CreateObject<OpenGymTupleContainer> ();
    data->Add(groupConfig_box);
    data->Add(stationStats_box);
    data->Add(statistic_box);
    
//   Ptr<Node> node = NodeList::GetNode (m_slotNum);
//   Ptr<NetDevice> dev = node-> GetDevice(0);
//   Ptr<TdmaNetDevice> m_tdmaDevice = DynamicCast<TdmaNetDevice>(dev);
//   // Get slot usage table
//   std::vector<std::pair<uint32_t,uint32_t> > nodeUsedList = m_tdmaDevice->GetTdmaController()->GetNodeUsedList(m_slotNum);
//   // Get routing table
//   std::vector<ns3::olsr::RoutingTableEntry> tdmaRoutingTable = node->GetObject<ns3::olsr::RoutingProtocol> ()->GetRoutingTableEntries() ;
//   // Get queued information
//   std::vector<std::pair<Ipv4Address, uint32_t>> queuePktStatus = m_tdmaDevice->GetTdmaController()->GetQueuePktStatus(m_slotNum);
//   std::vector<std::pair<Ipv4Address, uint32_t>> twoHopsPktStatus;
//   // Get total queued bytes
//   uint32_t queuingBytes = m_tdmaDevice->GetTdmaController()->GetQueuingBytes(m_slotNum);
  
  
//   // Calculate weight vector
//   // Filter one-hop packets
//   for (uint32_t i=0;i<queuePktStatus.size();i++)
//   {
// 	for(uint32_t j=0;j<tdmaRoutingTable.size();j++)
// 	{
// 		if (tdmaRoutingTable[j].destAddr == queuePktStatus[i].first && tdmaRoutingTable[j].distance >= 2)
// 		{
//             twoHopsPktStatus.push_back(std::pair<Ipv4Address,uint32_t> (tdmaRoutingTable[j].nextAddr,queuePktStatus[i].second));
// 			break;
// 		}
// 	}
//   }

//   int32_t nodeUsedList_top3Pkt[32];
//   std::fill_n(nodeUsedList_top3Pkt,32,4);

//   for (uint32_t i=0;i<32;i++)
//   {
// 	if (nodeUsedList[i].first == 0)
// 	{
// 		nodeUsedList_top3Pkt[i] = 0;
// 	}
//   }

//   uint32_t counter = 1;
//   bool isInList = false;
//   std::vector<uint32_t> top3PktSize;
                                       
//   for (uint32_t i=0;i<twoHopsPktStatus.size();i++)
//   {
//     // change ip->nodeId
//     uint32_t topN_nodeId = twoHopsPktStatus[i].first.CombineMask(Ipv4Mask(255)).Get()-1;
//     isInList = false;

// 	for (uint32_t j=0;j<nodeUsedList.size();j++)
// 	{
//         // replace NodeId with priority
// 		if(topN_nodeId == nodeUsedList[j].second && nodeUsedList[j].first != 0) // node is top 3 
// 		{
//             isInList = true;
// 			nodeUsedList_top3Pkt[j] = counter;
            
// 		}
// 	}
//     if (isInList){
//         counter++;
//         top3PktSize.push_back(twoHopsPktStatus[i].second);
//     }
      
//     if (counter >= 4){
//         break;
//     }


//   }

  
//   // Store weight vector in box, which is used to send message to python

  
//   std::vector<uint32_t> shape2 = {3+1,};
//   Ptr<OpenGymBoxContainer<uint32_t>> pktBytes_box = CreateObject<OpenGymBoxContainer<uint32_t> >(shape2);

//   // Store total packet bytes
//   pktBytes_box->AddValue (queuingBytes);
    
//   // Store Top K packetbytes in box
//   for (uint32_t i=0;i<top3PktSize.size();i++)
//   {
// 	pktBytes_box->AddValue (top3PktSize[i]);
//   }


//   Ptr<OpenGymBoxContainer<int32_t> > mslotUsedTablebox = DynamicCast<OpenGymBoxContainer<int32_t> >(data->Get(0));
//   Ptr<OpenGymBoxContainer<uint32_t> > mpktBytesbox = DynamicCast<OpenGymBoxContainer<uint32_t>>(data->Get(1));
//   NS_LOG_UNCOND ("MyGetObservation: " << data);
//   NS_LOG_UNCOND ("---" << mslotUsedTablebox);
//   NS_LOG_UNCOND ("---" << mpktBytesbox);

  NS_LOG_UNCOND("END: "<<Simulator::Now().GetNanoSeconds ());
  
  return data;
}

/*
Define reward function
*/
float
AhGymEnv::GetReward()
{
  NS_LOG_FUNCTION (this);

  float reward = 0;
  
  return reward;

}

/*
Define extra info. Optional
Send r_i, throughput, delay to python
*/
 
std::string
AhGymEnv::GetExtraInfo()
{
  NS_LOG_FUNCTION (this);
  
//   uint32_t previous_slotNum = m_slotNum == 0 ? 15 : m_slotNum - 1;
//   Ptr<Node> node = NodeList::GetNode (previous_slotNum);
//   Ptr<NetDevice> dev = node-> GetDevice(0);
//   Ptr<TdmaNetDevice> m_tdmaDevice = DynamicCast<TdmaNetDevice>(dev);

//   float* reward = m_tdmaDevice->GetTdmaController()->GetRLReward(previous_slotNum);
//   int64_t tdmaDataBytes = 0;
  
//   if (Simulator::Now().GetSeconds () < 6) tdmaDataBytes = 0;
//   else tdmaDataBytes = recvBytes / 16 / (Simulator::Now().GetSeconds () - 5);
//   //NS_LOG_UNCOND("Now: "<<Simulator::Now().GetNanoSeconds ());
  
//   uint64_t avgDelay = 0;
//   if (recvPacket == 0) avgDelay = 0;
//   else avgDelay = totalDelay/recvPacket;
    
    
//   std::stringstream stream;
//   stream << std::fixed << std::setprecision(2) << *(reward) << ",";
//   stream << std::fixed << std::setprecision(2) << *(reward+1) << ",";
//   stream << std::fixed << std::setprecision(2) << *(reward+2) << ",";
//   stream << tdmaDataBytes<<",";
//   stream << avgDelay;
//   std::string Info = stream.str();
  

//   m_tdmaDevice->GetTdmaController()->ResetRLReward(previous_slotNum);
    
  //std::string Info = std::to_string(m_slotNum);
//   NS_LOG_UNCOND("MyGetExtraInfo: " << Info);
  return "ss";
}


/*
Execute received actions
*/
bool
AhGymEnv::ExecuteActions(Ptr<OpenGymDataContainer> action)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_UNCOND ("ExecuteActions: " << action);
  
//   Ptr<Node> node = NodeList::GetNode (m_slotNum);
//   Ptr<NetDevice> dev = node-> GetDevice(0);
//   Ptr<TdmaNetDevice> m_tdmaDevice = DynamicCast<TdmaNetDevice>(dev);  
    
//   uint32_t max_slots = 3;

//   Ptr<OpenGymBoxContainer<int32_t> > box = DynamicCast<OpenGymBoxContainer<int32_t> >(action);

//   for (uint32_t i=0;i<max_slots;i++)
//   {
// 	if (box->GetValue(i) != -1)
// 	{
// 		m_tdmaDevice->GetTdmaController()->SetRLAction(box->GetValue(i));
// 	}

//   }  


  return true;
}


} // ns3 namespace