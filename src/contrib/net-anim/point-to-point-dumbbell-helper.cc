/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: George F. Riley<riley@ece.gatech.edu>
 */

// Implement an object to create a dumbbell topology.

#include <iostream>
#include <sstream>

// ns3 includes
#include "ns3/animation-interface.h"
#include "ns3/point-to-point-dumbbell-helper.h"
#include "ns3/node-location.h"

#include "ns3/node-list.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/vector.h"

using namespace std;

NS_LOG_COMPONENT_DEFINE("PointToPointDumbbellHelper");

namespace ns3 {
  
PointToPointDumbbellHelper::PointToPointDumbbellHelper (uint32_t nLeftLeaf,
                   PointToPointHelper& leftHelper,
                   uint32_t nRightLeaf,
                   PointToPointHelper& rightHelper,
                   PointToPointHelper& bottleneckHelper)
{
  // Create the bottleneck routers
  m_routers.Create (2);
  // Create the leaf nodes
  m_leftLeaf.Create (nLeftLeaf);
  m_rightLeaf.Create (nRightLeaf);

  // Add the link connecting routers
  m_routerDevices = bottleneckHelper.Install (m_routers);
  // Add the left side links
  for (uint32_t i = 0; i < nLeftLeaf; ++i)
    {
      NetDeviceContainer c = leftHelper.Install (m_routers.Get (0),
                                                m_leftLeaf.Get (i));
      m_leftRouterDevices.Add (c.Get (0));
      m_leftLeafDevices.Add (c.Get(1));
    }
  // Add the right side links
  for (uint32_t i = 0; i < nRightLeaf; ++i)
    {
      NetDeviceContainer c = rightHelper.Install (m_routers.Get (1),
                                                m_rightLeaf.Get (i));
      m_rightRouterDevices.Add (c.Get (0));
      m_rightLeafDevices.Add (c.Get (1));
    }
}

Ptr<Node> PointToPointDumbbellHelper::GetLeft () const
{ // Get the left side bottleneck router
  return m_routers.Get (0);
}

Ptr<Node> PointToPointDumbbellHelper::GetLeft (uint32_t i) const
{ // Get the i'th left side leaf
  return m_leftLeaf.Get (i);
}

Ptr<Node> PointToPointDumbbellHelper::GetRight () const
{ // Get the right side bottleneck router
  return m_routers.Get (1);
}

Ptr<Node> PointToPointDumbbellHelper::GetRight (uint32_t i) const
{ // Get the i'th right side leaf
  return m_rightLeaf.Get (i);
}

Ipv4Address PointToPointDumbbellHelper::GetLeftAddress (uint32_t i) const
{
  return m_leftLeafInterfaces.GetAddress (i);
}

Ipv4Address PointToPointDumbbellHelper::GetRightAddress (uint32_t i) const
{
  return m_rightLeafInterfaces.GetAddress (i);
}

uint32_t  PointToPointDumbbellHelper::LeftCount () const
{ // Number of left side nodes
  return m_leftLeaf.GetN ();
}

uint32_t  PointToPointDumbbellHelper::RightCount () const
{ // Number of right side nodes
  return m_rightLeaf.GetN ();
}

void PointToPointDumbbellHelper::InstallStack (InternetStackHelper stack)
{
  stack.Install (m_routers);
  stack.Install (m_leftLeaf);
  stack.Install (m_rightLeaf);
}

void PointToPointDumbbellHelper::AssignAddresses (Ipv4AddressHelper leftIp,
                               Ipv4AddressHelper rightIp,
                               Ipv4AddressHelper routerIp)
{
  // Assign the router network
  m_routerInterfaces = routerIp.Assign (m_routerDevices);
  // Assign to left side 
  for (uint32_t i = 0; i < LeftCount (); ++i)
    {
      NetDeviceContainer ndc;
      ndc.Add (m_leftLeafDevices.Get (i));
      ndc.Add (m_leftRouterDevices.Get (i));
      Ipv4InterfaceContainer ifc = leftIp.Assign(ndc);
      m_leftLeafInterfaces.Add (ifc.Get (0));
      m_leftRouterInterfaces.Add (ifc.Get (1));
      leftIp.NewNetwork ();
    }
  // Assign to right size
  for (uint32_t i = 0; i < RightCount (); ++i)
    {
      NetDeviceContainer ndc;
      ndc.Add (m_rightLeafDevices.Get (i));
      ndc.Add (m_rightRouterDevices.Get (i));
      Ipv4InterfaceContainer ifc = rightIp.Assign (ndc);
      m_rightLeafInterfaces.Add (ifc.Get (0));
      m_rightRouterInterfaces.Add (ifc.Get (1));
      rightIp.NewNetwork ();
    }
}


void PointToPointDumbbellHelper::BoundingBox (double ulx, double uly, // Upper left x/y
                           double lrx, double lry) // Lower right y
{
  double xDist = lrx - ulx;
  double yDist = lry - uly;
  double xAdder = xDist / 3.0;
  double  thetaL = M_PI / (LeftCount () + 1.0);
  double  thetaR = M_PI / (RightCount () + 1.0);

  // Place the left router
  Ptr<Node> lr = GetLeft ();
  Ptr<NodeLocation> loc = lr->GetObject<NodeLocation> ();
  if (loc == 0)
    {
      loc = CreateObject<NodeLocation> ();
      lr->AggregateObject (loc);
    }
  Vector lrl (ulx + xAdder, uly + yDist/2.0, 0);
  loc->SetLocation (lrl);
  
  // Place the right router
  Ptr<Node> rr = GetRight ();
  loc = rr->GetObject<NodeLocation> ();
  if (loc == 0)
    {
      loc = CreateObject<NodeLocation> ();
      rr->AggregateObject (loc);
    }
  Vector rrl (ulx + xAdder * 2, uly + yDist/2.0, 0); // Right router location
  loc->SetLocation (rrl);

  // Place the left leaf nodes
  double theta = -M_PI_2 + thetaL;
  for (uint32_t l = 0; l < LeftCount (); ++l)
    {
      // Make them in a circular pattern to make all line lengths the same
      // Special case when theta = 0, to be sure we get a straight line
      if ((LeftCount () % 2) == 1)
        { // Count is odd, see if we are in middle
          if (l == (LeftCount () / 2))
            {
              theta = 0.0;
            }
        }
      Ptr<Node> ln = GetLeft (l);
      loc = ln->GetObject<NodeLocation> ();
      if (loc == 0)
        {
          loc = CreateObject<NodeLocation> ();
          ln->AggregateObject (loc);
        }
      Vector lnl (lrl.x - cos (theta) * xAdder,
                   lrl.y + sin (theta) * xAdder, 0);  // Left Node Location
      // Insure did not exceed bounding box
      if (lnl.y < uly) 
        {
          lnl.y = uly; // Set to upper right y
        }
      if (lnl.y > lry) 
        {
          lnl.y = lry; // Set to lower right y
        }
      loc->SetLocation (lnl);
      theta += thetaL;
    }
  // Place the right nodes
  theta = -M_PI_2 + thetaR;
  for (uint32_t r = 0; r < RightCount (); ++r)
    {
      // Special case when theta = 0, to be sure we get a straight line
      if ((RightCount () % 2) == 1)
        { // Count is odd, see if we are in middle
          if (r == (RightCount () / 2))
            {
              theta = 0.0;
            }
        }
      Ptr<Node> rn = GetRight (r);
      loc = rn->GetObject<NodeLocation> ();
      if (loc == 0)
        {
          loc = CreateObject<NodeLocation> ();
          rn->AggregateObject (loc);
        }
      Vector rnl (rrl.x + cos (theta) * xAdder, // Right node location
                   rrl.y + sin (theta) * xAdder, 0);
      // Insure did not exceed bounding box
      if (rnl.y < uly) 
        {
          rnl.y = uly; // Set to upper right y
        }
      if (rnl.y > lry) 
        {
          rnl.y = lry; // Set to lower right y
        }
      loc->SetLocation (rnl);
      theta += thetaR;
    }
}
  
} // namespace ns3