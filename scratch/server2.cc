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
#include "ns3/applications-module.h"
#include "ns3/config-store-module.h"
#include "ns3/fd-net-device-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TestbedServer");

// perf indicators
double consumption      = 0.0;
double sinkRx           = 0.0;

void
PrintResults()
{
    std::cout << "sink_rx_B,consumption_mC" << std::endl;
    std::cout << sinkRx << "," << consumption << std::endl;
}

//ina cb
Ptr<Ina219Source> ina;
void
RemainingChargeCb (double oldValue, double remainingCharge)
{
    consumption += ina->GetCurrent();
}

//app sink rx
void
PacketSinkRxCb(Ptr<const Packet> packet, const Address &address)
{
    sinkRx += packet->GetSize();
}


////////////////
int
main (int argc, char *argv[])
{
  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

  int          m_run              = 0;
  double       m_totalTime        = 3000.0;
  uint32_t     m_histSize         = 10000;
  std::string  m_routing          = "split";
  bool         m_pcap             = false;
  double       m_charge           = 150.0;//6300.0; for 4x AA

  uint16_t     sinkPort           = 8000;
  
  std::string deviceName ("wlan0");
  std::string ipAddr ("10.1.1.1");
  std::string netmask ("255.255.255.0");
  std::string macAddr ("00:00:00:00:00:01");

  Ipv4Address localIp = Ipv4Address (ipAddr.c_str ());
  Mac48AddressValue localMac = Mac48AddressValue (macAddr.c_str ());
  Ipv4Mask localMask (netmask.c_str ());

  CommandLine cmd;
  cmd.AddValue ("id",          "Experiment ID, to customize output file [1]", m_run);
  cmd.AddValue ("time",        "Simulation time, seconds [1000 s]", m_totalTime);
  cmd.AddValue ("hist",        "Size of the history stack. [10000]", m_histSize);
  cmd.AddValue ("charge",      "Initial State of Charge [150 Coulombs]", m_charge);
  cmd.AddValue ("routing",     "Routing protocol to use, olsr default. [aodv/dsdv/olsr/olsr-etx/split]", m_routing);
  cmd.AddValue ("pcap",        "Enable PCAP traces on interfaces. [0]", m_pcap);
  cmd.Parse (argc, argv);

  Ptr<Node> node = CreateObject<Node> ();
  Ptr<EnergySourceContainer> esCont = CreateObject<EnergySourceContainer> ();
  ina = CreateObject<Ina219Source> ();
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
  InternetStackHelper internetStackHelper;
  if (m_routing == "aodv")
  {
      AodvHelper routing;
      internetStackHelper.SetRoutingHelper (routing);
  }else if (m_routing == "dsdv"){
      DsdvHelper routing;
      internetStackHelper.SetRoutingHelper (routing);
  }else if (m_routing == "olsr"){
      OlsrHelper routing;
      internetStackHelper.SetRoutingHelper (routing);
  }else if (m_routing == "etx"){
      EtxHelper routing;
      internetStackHelper.SetRoutingHelper (routing);
  }else if (m_routing == "split"){
      SplitHelper routing;
      routing.Set("HistorySize", UintegerValue(m_histSize));
      internetStackHelper.SetRoutingHelper (routing);
  }

  internetStackHelper.SetIpv4StackInstall(true);
  internetStackHelper.Install (node);

  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  uint32_t interface = ipv4->AddInterface (device);
  Ipv4InterfaceAddress address = Ipv4InterfaceAddress (localIp, localMask);
  ipv4->AddAddress (interface, address);
  ipv4->SetUp (interface);

  Address sinkLocalAddress (InetSocketAddress (localIp, sinkPort));
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
  ApplicationContainer sinkApp = sinkHelper.Install (node);
  sinkApp.Start (Seconds (1.0));
  sinkApp.Stop (Seconds (m_totalTime - 1.0));
//   emu.EnablePcap (ipAddr + "_" + m_routing + "_server_" + std::to_string(m_run), device);

  Config::ConnectWithoutContext("/NodeList/0/ApplicationList/0/$ns3::PacketSink/Rx", MakeCallback(&PacketSinkRxCb));
  ina->TraceConnectWithoutContext ("RemainingCharge", MakeCallback (&RemainingChargeCb));

  Simulator::Stop (Seconds (m_totalTime));
  Simulator::Run ();
  Simulator::Destroy ();

  PrintResults();

  return 0;
}
