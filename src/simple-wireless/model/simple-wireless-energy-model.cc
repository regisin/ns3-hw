/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Andrea Sacco
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
 * Author: Andrea Sacco <andrea.sacco85@gmail.com>
 */

#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/energy-source.h"
#include "simple-wireless-energy-model.h"
#include "ns3/log.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SimpleWirelessEnergyModel");

NS_OBJECT_ENSURE_REGISTERED (SimpleWirelessEnergyModel);

TypeId
SimpleWirelessEnergyModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SimpleWirelessEnergyModel")
    .SetParent<DeviceEnergyModel> ()
    .SetGroupName ("Energy")
    .AddConstructor<SimpleWirelessEnergyModel> ()
//    .AddAttribute ("CurrentA",
//                   "Current to be drained",
//                   DoubleValue (0.273),  // idle mode = 273mA
//                   MakeDoubleAccessor (&SimpleWirelessEnergyModel::SetCurrentA,
//                                       &SimpleWirelessEnergyModel::GetCurrentA),
//                 MakeDoubleChecker<double> ())
    .AddTraceSource ("TotalEnergyConsumption",
                     "Total energy consumption of the radio device.",
                     MakeTraceSourceAccessor (&SimpleWirelessEnergyModel::m_totalEnergyConsumption),
                     "ns3::TracedValueCallback::Double")
  ;
  return tid;
}

SimpleWirelessEnergyModel::SimpleWirelessEnergyModel ()
{
  NS_LOG_FUNCTION (this);
  m_lastUpdateTime = Seconds (0.0);
  m_actualCurrentA = 0.1;
  m_source = 0;
}

SimpleWirelessEnergyModel::~SimpleWirelessEnergyModel ()
{
  NS_LOG_FUNCTION (this);
}

void
SimpleWirelessEnergyModel::SetEnergySource (Ptr<EnergySource> source)
{
  NS_LOG_FUNCTION (this << source);
  NS_ASSERT (source != NULL);
  m_source = source;
}

void
SimpleWirelessEnergyModel::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  NS_ASSERT (node != NULL);
  m_node = node;
}

Ptr<Node>
SimpleWirelessEnergyModel::GetNode () const
{
  NS_LOG_FUNCTION (this);
  return m_node;
}

double
SimpleWirelessEnergyModel::GetTotalEnergyConsumption (void) const
{
  NS_LOG_FUNCTION (this);
  return m_totalEnergyConsumption;
}

void
SimpleWirelessEnergyModel::SetCurrentA (double current)
{
  NS_LOG_FUNCTION (this << current);
  Time duration = Simulator::Now () - m_lastUpdateTime;

  double energyToDecrease = 0.0;
  double supplyVoltage = m_source->GetSupplyVoltage ();
  energyToDecrease = duration.GetSeconds () * current * supplyVoltage;

  // update total energy consumption
  m_totalEnergyConsumption += energyToDecrease;
  // update last update time stamp
  m_lastUpdateTime = Simulator::Now ();
  // notify energy source
  m_source->UpdateEnergySource ();
  // update the current drain
  m_actualCurrentA = current;
}

double
SimpleWirelessEnergyModel::GetRemainingEnergy()
{
    return m_source->GetRemainingEnergy();
}

void
SimpleWirelessEnergyModel::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_source = 0;
}

double
SimpleWirelessEnergyModel::GetCurrentA (void) const
{
    return DoGetCurrentA();
}

double
SimpleWirelessEnergyModel::GetSupplyVoltage (void)
{
    return m_source->GetSupplyVoltage();
}

double
SimpleWirelessEnergyModel::DoGetCurrentA (void) const
{
  NS_LOG_FUNCTION (this);
  return m_actualCurrentA;
}

} // namespace ns3
