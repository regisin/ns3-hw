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

NS_LOG_COMPONENT_DEFINE ("TestbedClient");

//ina cb
Ptr<Ina219Source> ina;
void
RemainingChargeCb (double oldValue, double remainingCharge)
{
  NS_LOG_INFO(Simulator::Now ().GetSeconds () << ": RemainingCharge: " <<
    ina->GetVoltage() << "," <<
    ina->GetSupplyVoltage() << "," <<
    ina->GetShuntVoltage() << "," <<
    ina->GetCurrent() << "," <<
    ina->GetPower() << "," <<
    remainingCharge << "," <<
    ((100.0*remainingCharge) / ina->GetInitialCharge())
  );
}

//routing cb
void
SendOutgoingCb (const Ipv4Header &header, Ptr<const Packet> packet, uint32_t interface)
{
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": SendOutgoing: " << header->GetSource() << "->" <<header->GetDestination() << " - " << interface);
}
void
UnicastForwardCb (const Ipv4Header &header, Ptr<const Packet> packet, uint32_t interface)
{
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": UnicastForward: " << header->GetSource() << "->" <<header->GetDestination() << " - " << interface);
}
void
LocalDeliverCb (const Ipv4Header &header, Ptr<const Packet> packet, uint32_t interface)
{
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": LocalDeliver: " << header->GetSource() << "->" <<header->GetDestination() << " - " << interface);
}

//routing cbs
void
RxCb (const PacketHeader & header, const MessageList & messages)
{
    std::string msgs = "::";
    // MessageType-HopCount-OriginatorAddress>MessageType-HopCount-OriginatorAddress>....
    for (std::vector<MessageHeader>::iterator it = messages->begin() ; it != messages->end(); ++it) {
        msgs = msgs + std::to_string(*it->GetMessageType());
        msgs = msgs + "-" + std::to_string(*it->GetHopCount());
        msgs = msgs + "-" + std::to_string(*it->GetOriginatorAddress()) + ">";
    }
    msgs = msgs + "::"
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": Rx: " << msgs);
}
void
TxCb (const PacketHeader & header, const MessageList & messages)
{
    std::string msgs = "::";
    //format
    //::MessageType-HopCount-OriginatorAddress>MessageType-HopCount-OriginatorAddress>....>::
    for (std::vector<MessageHeader>::iterator it = messages->begin() ; it != messages->end(); ++it) {
        msgs = msgs + std::to_string(*it->GetMessageType());
        msgs = msgs + "-" + std::to_string(*it->GetHopCount());
        msgs = msgs + "-" + std::to_string(*it->GetOriginatorAddress()) + ">";
    }
    msgs = msgs + "::"
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": Tx: " << msgs);
}
void
RoutingTableChangedCb (uint32_t size)
{
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": RoutingTableChanged: " << std::to_string(size));
}

// throughput calc
uint32_t totalBytesReceived;
void
MacRx (Ptr <const Packet> p)
{
	totalBytesReceived += p->GetSize();
}
void
CalculateThroughput ()
{
    double throughput;
    throughput = totalBytesReceived * 8.0;
    totalBytesReceived = 0;
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": Throughput: " << throughput);
    Simulator::Schedule (Seconds (1.0), &CalculateThroughput);
}

//app onoff tx
void
OnOffApplicationTxCb(Ptr<const Packet> packet)
{
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": OnOffApplicationTx: " << packet->GetSize());
}

////////////////
int
main (int argc, char *argv[])
{
  LogComponentEnable("TestbedClient", LOG_LEVEL_INFO);
  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

  int          m_run              = 0;
  double       m_totalTime        = 1000.0;
  std::string  m_routing          = "split";
  bool         m_pcap             = false;
  double       m_charge           = 6300.0;

  uint16_t     sinkPort           = 8000;
  uint32_t     packetSize         = 1024; // bytes
  std::string dataRate("11Mb/s");
  
  std::string deviceName ("wlan0");
  std::string ipAddr ("10.1.1.XX");
  std::string dstAddr ("10.1.1.1");
  std::string netmask ("255.255.255.0");
  std::string macAddr ("00:00:00:00:00:XX");

  Ipv4Address localIp = Ipv4Address (ipAddr.c_str ());
  Ipv4Address remoteIp = Ipv4Address (dstAddr.c_str ());
  Mac48AddressValue localMac = Mac48AddressValue (macAddr.c_str ());
  Ipv4Mask localMask (netmask.c_str ());

  CommandLine cmd;
  cmd.AddValue ("id",          "Experiment ID, to customize output file [1]", m_run);
  cmd.AddValue ("time",        "Simulation time, seconds [1000 s]", m_totalTime);
  cmd.AddValue ("routing",     "Routing protocol to use, olsr default. [aodv/dsdv/olsr/etx/split]", m_routing);
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
//   Ipv4ListRoutingHelper list;
//   Ipv4StaticRoutingHelper staticRouting;
//   list.Add (staticRouting, 0);

  InternetStackHelper internetStackHelper;
  if (m_routing == "aodv")
  {
      AodvHelper routing;
      internetStackHelper.SetRoutingHelper (routing);
    //   list.Add (routing, 10);
  }else if (m_routing == "dsdv"){
      DsdvHelper routing;
      internetStackHelper.SetRoutingHelper (routing);
    //   list.Add (routing, 10);
  }else if (m_routing == "olsr"){
      OlsrHelper routing;
      internetStackHelper.SetRoutingHelper (routing);
    //   list.Add (routing, 10);
  }else if (m_routing == "etx"){
      EtxHelper routing;
      internetStackHelper.SetRoutingHelper (routing);
    //   list.Add (routing, 10);
  }else if (m_routing == "split"){
      SplitHelper routing;
      internetStackHelper.SetRoutingHelper (routing);
    //   list.Add (routing, 10);
  }

  
//   internetStackHelper.SetRoutingHelper (list);
  internetStackHelper.SetRoutingHelper (list);
  internetStackHelper.SetIpv4StackInstall(true);
  internetStackHelper.Install (node);

  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
  uint32_t interface = ipv4->AddInterface (device);
  Ipv4InterfaceAddress address = Ipv4InterfaceAddress (localIp, localMask);
  ipv4->AddAddress (interface, address);
  ipv4->SetUp (interface);

  AddressValue remoteAddress (InetSocketAddress (remoteIp, sinkPort));
  OnOffHelper onoff ("ns3::TcpSocketFactory", Address ());
  onoff.SetAttribute ("Remote", remoteAddress);
  onoff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onoff.SetAttribute ("DataRate", DataRateValue (dataRate));
  onoff.SetAttribute ("PacketSize", UintegerValue (packetSize));
  ApplicationContainer clientApps = onoff.Install (node);
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (m_totalTime - 2.0));
  emu.EnablePcap (ipAddr + "_" + m_routing + "_client_" + std::to_string(m_run), device);

  Config::Connect("/NodeList/0/$ns3::Ipv4L3Protocol/SendOutgoing", MakeCallback(&SendOutgoingCb));
  Config::Connect("/NodeList/0/$ns3::Ipv4L3Protocol/UnicastForward", MakeCallback(&UnicastForwardCb));
  Config::Connect("/NodeList/0/$ns3::Ipv4L3Protocol/LocalDeliver", MakeCallback(&LocalDeliverCb));

  Config::Connect("/NodeList/0/DeviceList/0/$ns3::FdNetDevice/Mac/MacRx", MakeCallback(&MacRx));
  Simulator::Schedule (Seconds (1.0), &CalculateThroughput);

  Config::Connect("/NodeList/0/ApplicationList/0/$ns3::OnOffApplication/Tx";, MakeCallback(&OnOffApplicationTxCb));

  if (m_routing == "aodv")
  {
    ; // nothing to do, no traces available
  }else if (m_routing == "dsdv"){
    ; // nothing to do, no traces available
  }else if (m_routing == "olsr"){
    Config::Connect("/NodeList/0/$ns3::olsr::RoutingProtocol/Rx", MakeCallback(&RxCb));
    Config::Connect("/NodeList/0/$ns3::olsr::RoutingProtocol/Tx", MakeCallback(&TxCb));
    Config::Connect("/NodeList/0/$ns3::olsr::RoutingProtocol/RoutingTableChanged", MakeCallback(&RoutingTableChangedCb));
  }else if (m_routing == "etx"){
    Config::Connect("/NodeList/0/$ns3::etx::RoutingProtocol/Rx", MakeCallback(&RxCb));
    Config::Connect("/NodeList/0/$ns3::etx::RoutingProtocol/Tx", MakeCallback(&TxCb));
    Config::Connect("/NodeList/0/$ns3::etx::RoutingProtocol/RoutingTableChanged", MakeCallback(&RoutingTableChangedCb));
  }else if (m_routing == "split"){
    Config::Connect("/NodeList/0/$ns3::split::RoutingProtocol/Rx", MakeCallback(&RxCb));
    Config::Connect("/NodeList/0/$ns3::split::RoutingProtocol/Tx", MakeCallback(&TxCb));
    Config::Connect("/NodeList/0/$ns3::split::RoutingProtocol/RoutingTableChanged", MakeCallback(&RoutingTableChangedCb));
  }

  ina->TraceConnectWithoutContext ("RemainingCharge", MakeCallback (&RemainingChargeCb));

  Simulator::Stop (Seconds (m_totalTime));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
