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
 */

#include <iostream>
#include "ns3/core-module.h"
#include "ns3/ina219-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("INA219test");

Ptr<Ina219Source> ina;

// Trace function for remaining charge at node.
void
RemainingCharge (double oldValue, double remainingCharge)
{
  NS_LOG_INFO(Simulator::Now ().GetSeconds () << "," <<
    ina->GetVoltage() << "," <<
    ina->GetSupplyVoltage() << "," <<
    ina->GetShuntVoltage() << "," <<
    ina->GetCurrent() << "," <<
    ina->GetPower() << "," <<
    remainingCharge << "," <<
    ((100.0*remainingCharge) / ina->GetInitialCharge())
  );
}

int
main (int argc, char *argv[])
{
    LogComponentEnable("INA219test", LOG_LEVEL_INFO);
    int estimatedRuntime = 20000;
    double initialCcharge = 7200.0;
  
    CommandLine cmd;
    cmd.AddValue ("time",        "Estimated experiment run time, seconds [20000 s]", estimatedRuntime);
    cmd.AddValue ("charge",      "Initial charge in Coulombs [7200.0 C]", initialCcharge);
    cmd.Parse (argc, argv);
  
    GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));

    Ptr<Node> node = CreateObject<Node> ();
    Ptr<EnergySourceContainer> esCont = CreateObject<EnergySourceContainer> ();
    ina = CreateObject<Ina219Source> ();
    ina->SetInitialCharge(initialCcharge);
    esCont->Add (ina);
    ina->SetNode (node);
    node->AggregateObject (esCont);

    NS_LOG_INFO("time_s,bus_voltage_V,supply_voltage_V,shunt_voltage_mV,current_mA,power_mW,remaining_C,remaining_percentage");

    ina->TraceConnectWithoutContext ("RemainingCharge", MakeCallback (&RemainingCharge));

    Simulator::Stop (Seconds (estimatedRuntime));
    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}
