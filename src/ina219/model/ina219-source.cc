/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Network Security Lab, University of Washington, Seattle.
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
 * Author: Sidharth Nabar <snabar@uw.edu>, He Wu <mdzz@u.washington.edu>
 *
 * Copyright (c) 2014 Wireless Communications and Networking Group (WCNG),
 * University of Rochester, Rochester, NY, USA.
 *
 * Modifications made by: Cristiano Tapparello <cristiano.tapparello@rochester.edu>
 */

#include <iostream>
#include "ina219-source.h"
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/double.h"
#include "ns3/simulator.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Ina219Source");

NS_OBJECT_ENSURE_REGISTERED (Ina219Source);

TypeId
Ina219Source::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::Ina219Source")
    .SetParent<EnergySource> ()
    .SetGroupName ("Energy")
    .AddConstructor<Ina219Source> ()
    .AddAttribute ("InitialCharge",
                "Initial charge stored in basic energy source.",
                DoubleValue (15000),  // in Coulombs
                MakeDoubleAccessor (&Ina219Source::SetInitialCharge,
                                    &Ina219Source::GetInitialCharge),
                MakeDoubleChecker<double> ())
    .AddAttribute ("ShuntResistance",
                "Shunt resistance in Ohms.",
                DoubleValue (0.1),  // in Ohms
                MakeDoubleAccessor (&Ina219Source::SetShuntResistance,
                                    &Ina219Source::GetShuntResistance),
                MakeDoubleChecker<double> ())
    .AddAttribute ("MaxExpectedAmps",
                "Maximum expected current in Amps.",
                DoubleValue (3.2),  // in Amperes
                MakeDoubleAccessor (&Ina219Source::SetMaxExpectedAmps,
                                    &Ina219Source::GetMaxExpectedAmps),
                MakeDoubleChecker<double> ())
    .AddAttribute ("UpdateInterval",
                   "Time between two consecutive periodic current coulomb counters.",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&Ina219Source::SetUpdateInterval,
                                     &Ina219Source::GetUpdateInterval),
                   MakeTimeChecker ())
    .AddTraceSource ("RemainingCharge",
                    "Remaining charge at Ina219Source.",
                    MakeTraceSourceAccessor (&Ina219Source::m_currentChargeCoulomb),
                    "ns3::TracedValueCallback::Double")
    .AddTraceSource ("Current",
                    "Last current draw reading from the sensor (mA).",
                    MakeTraceSourceAccessor (&Ina219Source::m_tempCurrent),
                    "ns3::TracedValueCallback::Double")
    .AddTraceSource ("Power",
                    "Last power reading from the sensor (mW).",
                    MakeTraceSourceAccessor (&Ina219Source::m_tempPower),
                    "ns3::TracedValueCallback::Double")
    .AddTraceSource ("Voltage",
                    "Last voltage reading from the sensor (V).",
                    MakeTraceSourceAccessor (&Ina219Source::m_tempVoltage),
                    "ns3::TracedValueCallback::Double")
    .AddTraceSource ("ShuntVoltage",
                    "Last shunt voltage drop reading from the sensor (mV).",
                    MakeTraceSourceAccessor (&Ina219Source::m_tempShuntVoltage),
                    "ns3::TracedValueCallback::Double")
    .AddTraceSource ("SupplyVoltage",
                    "Last supply voltage calculated from last voltage reading of the sensor (V).",
                    MakeTraceSourceAccessor (&Ina219Source::m_tempSupplyVoltage),
                    "ns3::TracedValueCallback::Double")
  ;
  return tid;
}

Ina219Source::Ina219Source ()
{

  NS_LOG_FUNCTION (this);
  m_lastUpdateTime = Seconds (0.0);
  ina = new INA219(0.1, 3.2);
  ina->configure(RANGE_16V, GAIN_8_320MV, ADC_12BIT, ADC_12BIT);
}

Ina219Source::~Ina219Source ()
{
  NS_LOG_FUNCTION (this);
}
void
Ina219Source::SetInitialCharge(double initialChargeCoulomb)
{
  NS_LOG_FUNCTION (this << initialChargeCoulomb);
  NS_ASSERT (initialChargeCoulomb >= 0);
  m_initialChargeCoulomb = initialChargeCoulomb;
  m_currentChargeCoulomb = m_initialChargeCoulomb;
}
double
Ina219Source::GetInitialCharge(void) const
{
  NS_LOG_FUNCTION (this);
  return m_initialChargeCoulomb;
}
void
Ina219Source::SetShuntResistance(double shuntOhms)
{
  NS_LOG_FUNCTION (this << shuntOhms);
  NS_ASSERT (shuntOhms >= 0);
  m_shuntOhms = shuntOhms;
}
double
Ina219Source::GetShuntResistance(void) const
{
  NS_LOG_FUNCTION (this);
  return m_shuntOhms;
}

void
Ina219Source::SetMaxExpectedAmps(double maxExpectedAmps)
{
  NS_LOG_FUNCTION (this << maxExpectedAmps);
  NS_ASSERT (maxExpectedAmps >= 0);
  m_maxExpectedAmps = maxExpectedAmps;
}
double
Ina219Source::GetMaxExpectedAmps(void) const
{
  NS_LOG_FUNCTION (this);
  return m_maxExpectedAmps;
}

void
Ina219Source::SetUpdateInterval (Time interval)
{
  NS_LOG_FUNCTION (this << interval);
  m_updateInterval = interval;
}

Time
Ina219Source::GetUpdateInterval (void) const
{
  NS_LOG_FUNCTION (this);
  return m_updateInterval;
}






//////////////////////////////////////////////// mandatory
double
Ina219Source::GetSupplyVoltage (void) const
{
  NS_LOG_FUNCTION (this);
  return m_tempSupplyVoltage;
}

double
Ina219Source::GetInitialEnergy (void) const
{
  NS_LOG_FUNCTION (this);
  // Initial Charge (C) x Voltage (V) = Energy (J)
  float e = m_initialChargeCoulomb * ina->voltage();
  return (double)e;
}

double
Ina219Source::GetRemainingEnergy (void)
{
  NS_LOG_FUNCTION (this);
  // Remaining Charge (C) x Voltage (V) = Energy (J)
  float e = m_currentChargeCoulomb * ina->voltage();
  return (double)e;
}

double
Ina219Source::GetEnergyFraction (void)
{
  NS_LOG_FUNCTION (this);
  return (double)(m_currentChargeCoulomb / m_initialChargeCoulomb);
}

void
Ina219Source::UpdateEnergySource (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_DEBUG ("Ina219Source:Updating remaining energy(charge).");

  // do not update if simulation has finished
  if (Simulator::IsFinished ())
    {
      return;
    }

  m_updateStateEvent.Cancel ();

  Time duration = Simulator::Now () - m_lastUpdateTime;
  NS_ASSERT (duration.GetSeconds () >= 0);

  float current = ina->current();
  float power = ina->power();
  float voltage = ina->voltage();
  float shunt_voltage = ina->shunt_voltage();
  float supply_voltage = ina->supply_voltage();
  m_tempCurrent = (double) current;
  m_tempPower = (double) power;
  m_tempVoltage = (double) voltage;
  m_tempShuntVoltage = (double) shunt_voltage;
  m_tempSupplyVoltage = (double) supply_voltage;

  double coulombToDecrease = (duration.GetSeconds () * m_tempCurrent);

  if (m_currentChargeCoulomb < coulombToDecrease) 
    {
      m_currentChargeCoulomb = 0; // energy never goes below 0
    }
  else
    {
      m_currentChargeCoulomb -= (coulombToDecrease/1000.0);
    }

  NotifyEnergyDrained();
  m_lastUpdateTime = Simulator::Now ();
  m_updateStateEvent = Simulator::Schedule (m_updateInterval,
                                             &Ina219Source::UpdateEnergySource,
                                             this);
  NS_LOG_DEBUG ("Ina219Source:Remaining charge = " << m_currentChargeCoulomb);
}
void
Ina219Source::DoInitialize (void)
{
  NS_LOG_FUNCTION (this);
  UpdateEnergySource ();  // start periodic update
}
void
Ina219Source::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
}


/////////////////////////////////////// ina219 interface functions
void
Ina219Source::Sleep()
{
  ina->sleep();
}
void
Ina219Source::Wake()
{
  ina->wake();
}
void
Ina219Source::Reset()
{
  ina->reset();
}
double
Ina219Source::GetVoltage()
{
  return m_tempVoltage;
}
double
Ina219Source::GetShuntVoltage()
{
  return m_tempShuntVoltage;
}
//double
//Ina219Source::GetSupplyVoltage()
//{
//  return m_tempSupplyVoltage;
//}
double
Ina219Source::GetCurrent()
{
  return m_tempCurrent;
}
double
Ina219Source::GetPower()
{
  return m_tempPower;
}


} // namespace ns3
