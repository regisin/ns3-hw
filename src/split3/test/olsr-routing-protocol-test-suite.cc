/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2004 Francisco J. Ros
 * Copyright (c) 2007 INESC Porto
 *
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
 * Authors: Francisco J. Ros  <fjrm@dif.um.es>
 *          Gustavo J. A. M. Carneiro <gjc@inescporto.pt>
 */

#include "ns3/test.h"
#include "ns3/split3-routing-protocol.h"
#include "ns3/ipv4-header.h"

/********** Willingness **********/

/// Willingness for forwarding packets from other nodes: never.
#define SPLIT3_WILL_NEVER         0
/// Willingness for forwarding packets from other nodes: low.
#define SPLIT3_WILL_LOW           1
/// Willingness for forwarding packets from other nodes: medium.
#define SPLIT3_WILL_DEFAULT       3
/// Willingness for forwarding packets from other nodes: high.
#define SPLIT3_WILL_HIGH          6
/// Willingness for forwarding packets from other nodes: always.
#define SPLIT3_WILL_ALWAYS        7

using namespace ns3;
using namespace split3;

/// Testcase for MPR computation mechanism
class Split3MprTestCase : public TestCase
{
public:
  Split3MprTestCase ();
  ~Split3MprTestCase ();
  /// \brief Run test case
  virtual void DoRun (void);
};


Split3MprTestCase::Split3MprTestCase ()
  : TestCase ("Check SPLIT3 MPR computing mechanism")
{
}
Split3MprTestCase::~Split3MprTestCase ()
{
}
void
Split3MprTestCase::DoRun ()
{
  Ptr<RoutingProtocol> protocol = CreateObject<RoutingProtocol> ();
  protocol->m_mainAddress = Ipv4Address ("10.0.0.1");
  Split3State & state = protocol->m_state;

  /*
   *  1 -- 2
   *  |    |
   *  3 -- 4
   *
   * Node 1 must select only one MPR (2 or 3, doesn't matter)
   */
  NeighborTuple neigbor;
  neigbor.status = NeighborTuple::STATUS_SYM;
  neigbor.willingness = SPLIT3_WILL_DEFAULT;
  neigbor.neighborMainAddr = Ipv4Address ("10.0.0.2");
  protocol->m_state.InsertNeighborTuple (neigbor);
  neigbor.neighborMainAddr = Ipv4Address ("10.0.0.3");
  protocol->m_state.InsertNeighborTuple (neigbor);
  TwoHopNeighborTuple tuple;
  tuple.expirationTime = Seconds (3600);
  tuple.neighborMainAddr = Ipv4Address ("10.0.0.2");
  tuple.twoHopNeighborAddr = Ipv4Address ("10.0.0.4");
  protocol->m_state.InsertTwoHopNeighborTuple (tuple);
  tuple.neighborMainAddr = Ipv4Address ("10.0.0.3");
  tuple.twoHopNeighborAddr = Ipv4Address ("10.0.0.4");
  protocol->m_state.InsertTwoHopNeighborTuple (tuple);

  protocol->MprComputation ();
  NS_TEST_EXPECT_MSG_EQ (state.GetMprSet ().size (), 1, "An only address must be chosen.");
  /*
   *  1 -- 2 -- 5
   *  |    |
   *  3 -- 4
   *
   * Node 1 must select node 2 as MPR.
   */
  tuple.neighborMainAddr = Ipv4Address ("10.0.0.2");
  tuple.twoHopNeighborAddr = Ipv4Address ("10.0.0.5");
  protocol->m_state.InsertTwoHopNeighborTuple (tuple);

  protocol->MprComputation ();
  MprSet mpr = state.GetMprSet ();
  NS_TEST_EXPECT_MSG_EQ (mpr.size (), 1, "An only address must be chosen.");
  NS_TEST_EXPECT_MSG_EQ ((mpr.find ("10.0.0.2") != mpr.end ()), true, "Node 1 must select node 2 as MPR");
  /*
   *  1 -- 2 -- 5
   *  |    |
   *  3 -- 4
   *  |
   *  6
   *
   * Node 1 must select nodes 2 and 3 as MPRs.
   */
  tuple.neighborMainAddr = Ipv4Address ("10.0.0.3");
  tuple.twoHopNeighborAddr = Ipv4Address ("10.0.0.6");
  protocol->m_state.InsertTwoHopNeighborTuple (tuple);

  protocol->MprComputation ();
  mpr = state.GetMprSet ();
  NS_TEST_EXPECT_MSG_EQ (mpr.size (), 2, "An only address must be chosen.");
  NS_TEST_EXPECT_MSG_EQ ((mpr.find ("10.0.0.2") != mpr.end ()), true, "Node 1 must select node 2 as MPR");
  NS_TEST_EXPECT_MSG_EQ ((mpr.find ("10.0.0.3") != mpr.end ()), true, "Node 1 must select node 3 as MPR");
  /*
   *  7 (SPLIT3_WILL_ALWAYS)
   *  |
   *  1 -- 2 -- 5
   *  |    |
   *  3 -- 4
   *  |
   *  6
   *
   * Node 1 must select nodes 2, 3 and 7 (since it is WILL_ALWAYS) as MPRs.
   */
  neigbor.willingness = SPLIT3_WILL_ALWAYS;
  neigbor.neighborMainAddr = Ipv4Address ("10.0.0.7");
  protocol->m_state.InsertNeighborTuple (neigbor);

  protocol->MprComputation ();
  mpr = state.GetMprSet ();
  NS_TEST_EXPECT_MSG_EQ (mpr.size (), 3, "An only address must be chosen.");
  NS_TEST_EXPECT_MSG_EQ ((mpr.find ("10.0.0.7") != mpr.end ()), true, "Node 1 must select node 7 as MPR");
  /*
   *                7 <- WILL_ALWAYS
   *                |
   *      9 -- 8 -- 1 -- 2 -- 5
   *                |    |
   *           ^    3 -- 4
   *           |    |
   *   WILL_NEVER   6
   *
   * Node 1 must select nodes 2, 3 and 7 (since it is WILL_ALWAYS) as MPRs.
   * Node 1 must NOT select node 8 as MPR since it is WILL_NEVER
   */
  neigbor.willingness = SPLIT3_WILL_NEVER;
  neigbor.neighborMainAddr = Ipv4Address ("10.0.0.8");
  protocol->m_state.InsertNeighborTuple (neigbor);
  tuple.neighborMainAddr = Ipv4Address ("10.0.0.8");
  tuple.twoHopNeighborAddr = Ipv4Address ("10.0.0.9");
  protocol->m_state.InsertTwoHopNeighborTuple (tuple);

  protocol->MprComputation ();
  mpr = state.GetMprSet ();
  NS_TEST_EXPECT_MSG_EQ (mpr.size (), 3, "An only address must be chosen.");
  NS_TEST_EXPECT_MSG_EQ ((mpr.find ("10.0.0.9") == mpr.end ()), true, "Node 1 must NOT select node 8 as MPR");
}

static class Split3ProtocolTestSuite : public TestSuite
{
public:
  Split3ProtocolTestSuite ();
} g_split3ProtocolTestSuite;

Split3ProtocolTestSuite::Split3ProtocolTestSuite ()
  : TestSuite ("routing-split3", UNIT)
{
  AddTestCase (new Split3MprTestCase (), TestCase::QUICK);
}
