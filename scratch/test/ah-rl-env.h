#ifndef AH_RL_ENV_H
#define AH_RL_ENV_H


#include "ns3/opengym-module.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/node-list.h"
#include "ns3/network-module.h"
#include "ns3/log.h"
#include "ns3/olsr-routing-protocol.h"
// #include "ns3/simple-wireless-tdma-module.h"

#include "Statistics.h"
#include "NodeEntry.h"
#include <vector>

extern uint64_t recvBytes;
extern uint64_t totalDelay;
extern uint64_t recvPacket;

namespace ns3 {

class Node;
class Packet;

class AhGymEnv : public OpenGymEnv
{
public:
  AhGymEnv ();
  AhGymEnv (uint32_t, uint32_t, uint32_t, Statistics*, vector<NodeEntry*>);
  virtual ~AhGymEnv ();
  static TypeId GetTypeId (void);
  virtual void DoDispose ();


  // OpenGym interface
  Ptr<OpenGymSpace> GetActionSpace();
  Ptr<OpenGymSpace> GetObservationSpace();
  bool GetGameOver();
  Ptr<OpenGymDataContainer> GetObservation();
  float GetReward();
  std::string GetExtraInfo();
  bool ExecuteActions(Ptr<OpenGymDataContainer> action);



private:
  void ScheduleNextStateRead ();


  uint32_t m_slotNum;
  uint32_t m_repeatChoose;
  uint32_t m_maxGroupNumber;
  uint32_t m_maxStationNumber;
  uint32_t m_simulationTime;
  
  Statistics *m_stats;
  vector<NodeEntry*> m_nodes;
  

  Time m_stepInterval1; // skip to next ctrl slot (ctrl slot size)
  Time m_stepInterval2; // skip all data slot ( data slot num * data slot size)
};




} // namespace ns3

#endif /* Ah_RL_ENV_H */