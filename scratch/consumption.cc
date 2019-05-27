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
#include <fstream>
#include <vector>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/config-store-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/ina219-module.h"
#include "ns3/fd-net-device-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("EnergyProfilerINA");

double initialEnergy = 500.0;
double timeInterval = 1.0;

/// Trace function for energy at node.
void
TraceConsumption (double oldValue, double newValue)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << "," << oldValue << "," << newValue);
}

// void EnergyStateLogger(Ptr<Ina219Source> s)
// {
//   double ec = s->GetTotalEnergyConsumption();
//   NS_LOG_UNCOND(Simulator::Now().GetSeconds() << "," << ec);
//   Simulator::Schedule(Seconds(timeInterval), &EnergyStateLogger, s);
// }

////////////////
int main(int argc, char *argv[])
{
  LogComponentEnable("EnergyProfilerINA",
                     LogLevel(LOG_PREFIX_TIME | LOG_PREFIX_NODE | LOG_LEVEL_INFO));
  GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));
  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

  Ptr<Ina219Source> ina;
  double ttime = 100.0; // seconds
  uint32_t PpacketSize = 1000; // bytes
  std::string dataRate ("1Mb/s");
  
  std::string deviceName ("wlan0");
  std::string ipAddr ("10.1.1.1");
  std::string dstAddr ("10.1.1.255");
  std::string netmask ("255.255.255.0");
  std::string macAddr ("00:00:00:00:00:01");

  // Ipv4Address localIp = Ipv4Address (ipAddr.c_str ());
  // Ipv4Address remoteIp = Ipv4Address (dstAddr.c_str ());
  Mac48AddressValue localMac = Mac48AddressValue (macAddr.c_str ());
  Ipv4Mask localMask (netmask.c_str ());

  // simulation parameters
  /*
   * This is a magic number used to set the transmit power, based on other
   * configuration.
   */
  int m_run = 1;

  CommandLine cmd;
  cmd.AddValue("PpacketSize", "size of application packet sent", PpacketSize);
  cmd.AddValue("id", "Experiment ID, to customize output file [1]", m_run);
  cmd.Parse (argc, argv);
  RngSeedManager::SetRun (m_run);

  NodeContainer c;
  Ptr<Node> node = CreateObject<Node>();
  c.Add(node);

  Ptr<EnergySourceContainer> esCont = CreateObject<EnergySourceContainer>();
  ina = CreateObject<Ina219Source>();

  ina->SetInitialCharge(initialEnergy);
  ina->SetUpdateInterval(MilliSeconds(100.0));
  esCont->Add(ina);
  ina->SetNode(node);
  node->AggregateObject(esCont);

  EmuFdNetDeviceHelper emu;
  emu.SetDeviceName(deviceName);
  NetDeviceContainer devices = emu.Install(node);
  Ptr<NetDevice> device = devices.Get(0);
  device->SetAttribute("Address", localMac);

  /** Internet stack **/
  InternetStackHelper internet;
  internet.Install (c);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> source = Socket::CreateSocket (c.Get (0), tid); // node 0, sender
  InetSocketAddress remote = InetSocketAddress (Ipv4Address::GetBroadcast (), 80);
  source->SetAllowBroadcast (true);
  source->Connect (remote);

  AddressValue remoteAddress (remote);

  //////////////////////////////////////////////////////////////////////////////////////////////
  OnOffHelper onoff("ns3::UdpSocketFactory", Address());
  onoff.SetAttribute("Remote", remoteAddress);
  onoff.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
  onoff.SetAttribute("DataRate", DataRateValue(dataRate));
  onoff.SetAttribute("PacketSize", UintegerValue(PpacketSize));
  ApplicationContainer clientApps = onoff.Install(node);
  //////////////////////////////////////////////////////////////////////////////////////////////

  clientApps.Start(Seconds(0.01));
  clientApps.Stop(Seconds(ttime - 0.01));

  // Simulator::Schedule (Seconds (timeInterval), &EnergyStateLogger, ina);
  ina->TraceConnectWithoutContext("TotalEnergyConsumption", MakeCallback(&TraceConsumption));

  /** simulation setup **/
  NS_LOG_UNCOND ("time,energy_fraction,total_energy_consumption");
  NS_LOG_UNCOND ("0,1.0,0.0");

  Simulator::Stop(Seconds(ttime));
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}
