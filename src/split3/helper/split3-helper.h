/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#ifndef SPLIT3_HELPER_H
#define SPLIT3_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/node.h"
#include "ns3/node-container.h"
#include "ns3/ipv4-routing-helper.h"
#include <map>
#include <set>

namespace ns3 {

/**
 * \ingroup split3
 *
 * \brief Helper class that adds SPLIT3 routing to nodes.
 *
 * This class is expected to be used in conjunction with
 * ns3::InternetStackHelper::SetRoutingHelper
 */
class Split3Helper : public Ipv4RoutingHelper
{
public:
  /**
   * Create an Split3Helper that makes life easier for people who want to install
   * SPLIT3 routing to nodes.
   */
  Split3Helper ();

  /**
   * \brief Construct an Split3Helper from another previously initialized instance
   * (Copy Constructor).
   */
  Split3Helper (const Split3Helper &);

  /**
   * \returns pointer to clone of this Split3Helper
   *
   * This method is mainly for internal use by the other helpers;
   * clients are expected to free the dynamic memory allocated by this method
   */
  Split3Helper* Copy (void) const;

  /**
    * \param node the node for which an exception is to be defined
    * \param interface an interface of node on which SPLIT3 is not to be installed
    *
    * This method allows the user to specify an interface on which SPLIT3 is not to be installed on
    */
  void ExcludeInterface (Ptr<Node> node, uint32_t interface);

  /**
   * \param node the node on which the routing protocol will run
   * \returns a newly-created routing protocol
   *
   * This method will be called by ns3::InternetStackHelper::Install
   */
  virtual Ptr<Ipv4RoutingProtocol> Create (Ptr<Node> node) const;

  /**
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set.
   *
   * This method controls the attributes of ns3::split3::RoutingProtocol
   */
  void Set (std::string name, const AttributeValue &value);

  /**
   * Assign a fixed random variable stream number to the random variables
   * used by this model.  Return the number of streams (possibly zero) that
   * have been assigned.  The Install() method of the InternetStackHelper
   * should have previously been called by the user.
   *
   * \param stream first stream index to use
   * \param c NodeContainer of the set of nodes for which the Split3RoutingProtocol
   *          should be modified to use a fixed stream
   * \return the number of stream indices assigned by this helper
   */
  int64_t AssignStreams (NodeContainer c, int64_t stream);

private:
  /**
   * \brief Assignment operator declared private and not implemented to disallow
   * assignment and prevent the compiler from happily inserting its own.
   */
  Split3Helper &operator = (const Split3Helper &);
  ObjectFactory m_agentFactory; //!< Object factory

  std::map< Ptr<Node>, std::set<uint32_t> > m_interfaceExclusions; //!< container of interfaces excluded from SPLIT3 operations
};

} // namespace ns3

#endif /* SPLIT3_HELPER_H */
