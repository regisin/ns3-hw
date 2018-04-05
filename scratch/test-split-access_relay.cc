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
// #include <iostream>

#include "ns3/core-module.h"
#include "ns3/ina219-module.h"

#include "ns3/dsdv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/etx-module.h"
#include "ns3/split-module.h"
#include "ns3/aodv-module.h"

#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/config-store-module.h"
#include "ns3/fd-net-device-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TestbedClient");

int
main (int argc, char *argv[])
{
  LogComponentEnable("TestbedClient", LOG_LEVEL_INFO);
  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

  int          m_run              = 0;
  double       m_totalTime        = 30000.0;
  std::string  m_routing          = "split";
  bool         m_pcap             = false;
  double       m_charge           = 6300.0;

  uint16_t     sinkPort           = 8000;
  uint32_t     packetSize         = 10000; // bytes
  std::string dataRate("1000Mb/s");
  
  std::string deviceName ("wlan0");
  std::string ipAddr ("10.1.1.2");
  std::string dstAddr ("10.1.1.1");
  std::string netmask ("255.255.255.0");
  std::string macAddr ("00:00:00:00:00:01");

  Ipv4Address localIp = Ipv4Address (ipAddr.c_str ());
  Ipv4Address remoteIp = Ipv4Address (dstAddr.c_str ());
  Mac48AddressValue localMac = Mac48AddressValue (macAddr.c_str ());
  Ipv4Mask localMask (netmask.c_str ());

  CommandLine cmd;
  cmd.AddValue ("id",          "Experiment ID, to customize output file [1]", m_run);
  cmd.AddValue ("time",        "Simulation time, seconds [100 s]", m_totalTime);
  cmd.AddValue ("routing",     "Routing protocol to use, olsr default. [aodv/dsdv/olsr/olsr-etx/split]", m_routing);
  cmd.AddValue ("pcap",        "Enable PCAP traces on interfaces. [0]", m_pcap);
  cmd.Parse (argc, argv);

  Ptr<Node> node = CreateObject<Node> ();
  Ptr<EnergySourceContainer> esCont = CreateObject<EnergySourceContainer> ();
  Ptr<Ina219Source> ina = CreateObject<Ina219Source> ();
  ina->SetInitialCharge(m_charge);
  esCont->Add (ina);
  ina->SetNode (node);
  node->AggregateObject (esCont);

  EmuFdNetDeviceHelper emu;
  emu.SetDeviceName (deviceName);
  NetDeviceContainer devices = emu.Install (node);
  Ptr<NetDevice> device = devices.Get (0);
  device->SetAttribute ("Address", localMac);

  /*
    * Add the routing protocol
    */
  Ipv4ListRoutingHelper list;
  Ipv4StaticRoutingHelper staticRouting;
  list.Add (staticRouting, 0);

  if (m_routing == "aodv")
  {
      AodvHelper routing;
      list.Add (routing, 10);
  }else if (m_routing == "dsdv"){
      DsdvHelper routing;
      list.Add (routing, 10);
  }else if (m_routing == "olsr"){
      OlsrHelper routing;
      list.Add (routing, 10);
  }else if (m_routing == "split"){
      SplitHelper routing;
      list.Add (routing, 10);
  }

  InternetStackHelper internetStackHelper;
  internetStackHelper.SetRoutingHelper (list);
  internetStackHelper.SetIpv4StackInstall(true);
  internetStackHelper.Install (node);

  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  uint32_t interface = ipv4->AddInterface (device);
  Ipv4InterfaceAddress address = Ipv4InterfaceAddress (localIp, localMask);
  ipv4->AddAddress (interface, address);
  ipv4->SetUp (interface);

  emu.EnablePcap (std::to_string(m_run), device);

  Simulator::Stop (Seconds (m_totalTime));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
