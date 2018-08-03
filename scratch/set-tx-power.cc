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


#include "ns3/core-module.h"
#include "ns3/fd-net-device-module.h"
#include "ns3/log.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("PiNetDevTxPowerChangeTest");

void
PrintTxPower(Ptr<FdNetDevice> device)
{
    std::cout << device->GetTxPower() << std::endl;
}

void
SetTxPower(Ptr<FdNetDevice> device, int newPower)
{
    device->SetTxPower(newPower);
}

int
main (int argc, char *argv[])
{
  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
  
  std::string  deviceName ("wlan0");
  double       m_totalTime        = 10.0;
  int          ipower             = 20;
  int          fpower             = 10;

  CommandLine cmd;
  cmd.AddValue ("time",        "Simulation time, seconds [10 s]", m_totalTime);
  cmd.AddValue ("device",      "Device name to be used (use ifconfig to find out) [wlan0]", deviceName);
  cmd.AddValue ("ipower",      "Initial tx-power [20 dBm]", ipower);
  cmd.AddValue ("fpower",      "Final power, will change in the middle of the simulation [10 dBm]", fpower);

  cmd.Parse (argc, argv);

  Ptr<Node> node = CreateObject<Node> ();

  EmuFdNetDeviceHelper pi;
  pi.SetDeviceName (deviceName);
  NetDeviceContainer devices = pi.Install (node);
  Ptr<FdNetDevice> device = devices.Get (0)->GetObject<FdNetDevice>();

  Simulator::Schedule(Seconds (0.01), &SetTxPower, device, ipower);
  Simulator::Schedule(Seconds (1.0), &PrintTxPower, device);

  Simulator::Schedule(Seconds (5), &SetTxPower, device, fpower);
  Simulator::Schedule(Seconds (9), &PrintTxPower, device);
  
  
  
  Simulator::Stop (Seconds (m_totalTime));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}