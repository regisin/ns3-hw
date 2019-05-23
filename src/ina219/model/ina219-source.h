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
 * Authors: Sidharth Nabar <snabar@uw.edu>, He Wu <mdzz@u.washington.edu>
 *
 * Copyright (c) 2014 Wireless Communications and Networking Group (WCNG),
 * University of Rochester, Rochester, NY, USA.
 *
 * Modifications made by: Cristiano Tapparello <cristiano.tapparello@rochester.edu>
 */


#ifndef INA219_SOURCE_H
#define INA219_SOURCE_H

#include "ina219.h"

#include "ns3/object.h"
#include "ns3/ptr.h"
#include "ns3/type-id.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/traced-value.h"
#include "ns3/energy-source.h"



namespace ns3 {

/**
 * \defgroup energy Energy Models
 *
 */

/**
 * \ingroup energy
 *
 * \brief Energy source base class.
 *
 * This is the base class for energy sources. Energy sources keep track of
 * remaining energy. Device energy models will be updating the remaining energy
 * in the energy source. The energy source itself does not update the remaining
 * energy. Energy source also keeps a list of device energy models installed on
 * the same node. When the remaining energy level reaches 0, the energy source
 * will notify all device energy models stored in the list.
 *
 * EnergySource provides 2 types of interfaces for DeviceEnergyModels to update
 * the remaining energy stored in EnergySource:
 *  -Direct energy update interface (Joules):
 *    DecreaseRemainingEnergy
 *    IncreaseRemainingEnergy
 *  -Indirect energy update interface (Current):
 *    UpdateEnergySource
 * Direct energy update interface will decrease/increase energy from the source
 * directly (in Joules). Direct energy update interface is typically used by
 * simple DeviceEnergyModel which knows only average power consumption for each
 * of its state.
 * Indirect energy update interface uses the total current cumulated from all
 * DeviceEnergyModel to calculate energy to decrease from the source. Indirect
 * energy update interface is typically used by DeviceEnergyModel who knows its
 * current draw for each of its states. Nonlinear EnergySource also uses this
 * interface.
 *
 * Unit of energy is chosen as Joules since energy models typically calculate
 * energy as (time in seconds * power in Watts). If the energy source stores
 * energy in different units (eg. kWh), a simple converter function should
 * suffice.
 */

class Ina219Source : public EnergySource
{
public:
  static TypeId GetTypeId (void);
  Ina219Source ();
  virtual ~Ina219Source ();

  /**
   * \returns Supply voltage of the energy source.
   *
   * Set method is to be defined in child class only if necessary. For sources
   * with a fixed supply voltage, set method is not needed.
   */
  virtual double GetSupplyVoltage (void) const;

  /**
   * \returns Initial energy (capacity) of the energy source.
   *
   * Set method is to be defined in child class only if necessary. For sources
   * with a fixed initial energy (energy capacity), set method is not needed.
   */
  virtual double GetInitialEnergy (void) const;

  /**
   * \returns Remaining energy at the energy source.
   */
  virtual double GetRemainingEnergy (void);

  /**
   * \return Energy fraction = remaining energy / initial energy [0, 1]
   *
   * This function returns the percentage of energy left in the energy source.
   */
  virtual double GetEnergyFraction (void);

  /**
   * This function goes through the list of DeviceEnergyModels to obtain total
   * current draw at the energy source and updates remaining energy. Called by
   * DeviceEnergyModels to inform EnergySource of a state change.
   */
  virtual void UpdateEnergySource (void);

  /**
   * \return Total energy drained by the node.
   *
   * This function returns the total amount of energy (J) drained by the entire node.
   */
  double GetTotalEnergyConsumption (void);



  // ina stuff
  double GetShuntResistance(void) const;
  void SetShuntResistance(double);

  double GetInitialCharge(void) const;
  void SetInitialCharge(double);

  double GetMaxExpectedAmps(void) const;
  void SetMaxExpectedAmps(double);

  Time GetUpdateInterval (void) const;
  void SetUpdateInterval (Time interval);



private:
  /**
   * All child's implementation must call BreakDeviceEnergyModelRefCycle to
   * ensure reference cycles to DeviceEnergyModel objects are broken.
   *
   * Defined in ns3::Object
   */
  virtual void DoDispose (void);

private:
  INA219* ina;
  double m_shuntOhms;
  double m_maxExpectedAmps;
  double m_initialChargeCoulomb;

  TracedValue<double> m_currentChargeCoulomb;
  TracedValue<double> m_tempCurrent;
  TracedValue<double> m_tempPower;
  TracedValue<double> m_tempVoltage;
  TracedValue<double> m_tempShuntVoltage;
  TracedValue<double> m_tempSupplyVoltage;

  TracedValue<double> m_totalEnergyConsumption;

  Time m_lastUpdateTime;                  // last update time
  Time m_updateInterval;
  EventId m_updateStateEvent;            // update event
public:
  void Sleep();
  void Wake();
  void Reset();
  double GetVoltage();
  double GetShuntVoltage();
//  double GetSupplyVoltage();
  double GetCurrent();
  double GetPower();

private:
  /// Defined in ns3::Object
  void DoInitialize (void);

protected:
  /**
   * \returns Total current draw from all DeviceEnergyModels.
   */
  double CalculateTotalCurrent (void); ///////////////////////////////////////////////////// ina219.current();
};

} // namespace ns3

#endif /* ENERGY_SOURCE_H */
