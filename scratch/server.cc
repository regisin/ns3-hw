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

//ipv4l3routing cb
void
SendOutgoingCb (const Ipv4Header &header, Ptr<const Packet> packet, uint32_t interface)
{
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": SendOutgoing: " << header.GetSource() << "->" << header.GetDestination() << " - " << interface);
}
void
UnicastForwardCb (const Ipv4Header &header, Ptr<const Packet> packet, uint32_t interface)
{
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": UnicastForward: " << header.GetSource() << "->" << header.GetDestination() << " - " << interface);
}
void
LocalDeliverCb (const Ipv4Header &header, Ptr<const Packet> packet, uint32_t interface)
{
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": LocalDeliver: " << header.GetSource() << "->" << header.GetDestination() << " - " << interface);
}

//routing cbs
void
OlsrRxCb (const ns3::olsr::PacketHeader & header, const ns3::olsr::MessageList & messages)
{
    std::string msgs ("::");
    // MessageType-HopCount-OriginatorAddress>MessageType-HopCount-OriginatorAddress>....
    for (std::vector<ns3::olsr::MessageHeader>::const_iterator it = messages.begin() ; it != messages.end(); ++it) {
        msgs = msgs + std::to_string(it->GetMessageType());
        msgs = msgs + "-" + std::to_string(it->GetHopCount());
        // msgs = msgs + "-" + it->GetOriginatorAddress() + ">";
    }
    msgs = msgs + "::";
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": Rx: " << msgs);
}
void
OlsrTxCb (const ns3::olsr::PacketHeader & header, const ns3::olsr::MessageList & messages)
{
    std::string msgs ("::");
    //format
    // ::MessageType-HopCount-OriginatorAddress>MessageType-HopCount-OriginatorAddress>....>::
    for (std::vector<ns3::olsr::MessageHeader>::const_iterator it = messages.begin() ; it != messages.end(); ++it) {
        msgs = msgs + std::to_string(it->GetMessageType());
        msgs = msgs + "-" + std::to_string(it->GetHopCount());
        // msgs = msgs + "-" + it->GetOriginatorAddress() + ">";
    }
    msgs = msgs + "::";
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": Tx: " << msgs);
}
void
OlsrRoutingTableChangedCb (uint32_t size)
{
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": RoutingTableChanged: " << std::to_string(size));
}
void
EtxRxCb (const ns3::etx::PacketHeader & header, const ns3::etx::MessageList & messages)
{
    std::string msgs ("::");
    // MessageType-HopCount-OriginatorAddress>MessageType-HopCount-OriginatorAddress>....
    for (std::vector<ns3::etx::MessageHeader>::const_iterator it = messages.begin() ; it != messages.end(); ++it) {
        msgs = msgs + std::to_string(it->GetMessageType());
        msgs = msgs + "-" + std::to_string(it->GetHopCount());
        // msgs = msgs + "-" + it->GetOriginatorAddress() + ">";
    }
    msgs = msgs + "::";
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": Rx: " << msgs);
}
void
EtxTxCb (const ns3::etx::PacketHeader & header, const ns3::etx::MessageList & messages)
{
    std::string msgs ("::");
    //format
    // ::MessageType-HopCount-OriginatorAddress>MessageType-HopCount-OriginatorAddress>....>::
    for (std::vector<ns3::etx::MessageHeader>::const_iterator it = messages.begin() ; it != messages.end(); ++it) {
        msgs = msgs + std::to_string(it->GetMessageType());
        msgs = msgs + "-" + std::to_string(it->GetHopCount());
        // msgs = msgs + "-" + it->GetOriginatorAddress() + ">";
    }
    msgs = msgs + "::";
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": Tx: " << msgs);
}
void
EtxRoutingTableChangedCb (uint32_t size)
{
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": RoutingTableChanged: " << std::to_string(size));
}
void
SplitRxCb (const ns3::split::PacketHeader & header, const ns3::split::MessageList & messages)
{
    std::string msgs ("::");
    // MessageType-HopCount-OriginatorAddress>MessageType-HopCount-OriginatorAddress>....
    for (std::vector<ns3::split::MessageHeader>::const_iterator it = messages.begin() ; it != messages.end(); ++it) {
        msgs = msgs + std::to_string(it->GetMessageType());
        msgs = msgs + "-" + std::to_string(it->GetHopCount());
        // msgs = msgs + "-" + it->GetOriginatorAddress() + ">";
    }
    msgs = msgs + "::";
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": Rx: " << msgs);
}
void
SplitTxCb (const ns3::split::PacketHeader & header, const ns3::split::MessageList & messages)
{
    std::string msgs ("::");
    //format
    // ::MessageType-HopCount-OriginatorAddress>MessageType-HopCount-OriginatorAddress>....>::
    for (std::vector<ns3::split::MessageHeader>::const_iterator it = messages.begin() ; it != messages.end(); ++it) {
        msgs = msgs + std::to_string(it->GetMessageType());
        msgs = msgs + "-" + std::to_string(it->GetHopCount());
        // msgs = msgs + "-" + it->GetOriginatorAddress() + ">";
    }
    msgs = msgs + "::";
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": Tx: " << msgs);
}
void
SplitRoutingTableChangedCb (uint32_t size)
{
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": RoutingTableChanged: " << std::to_string(size));
}

// throughput calc
std::map<Mac48Address, double> link_tp;
void
MacRx (Ptr <const Packet> p)
{
    EthernetHeader header (false);
    Mac48Address source;
    p->PeekHeader (header);
    source = header.GetSource ();
    link_tp[source] += p->GetSize();
}
void
CalculateThroughput ()
{
    for (std::map<Mac48Address, double>::iterator it = link_tp.begin() ; it != link_tp.end() ; ++it)
    {
        double throughput = it->second * (8.0 / 1e6);
        it->second = 0.0;
        NS_LOG_INFO(Simulator::Now().GetSeconds() << ": Throughput (Mbps): " << it->first << ">" << throughput);
    }
    Simulator::Schedule (MilliSeconds (2000), &CalculateThroughput);
}

//app sink rx
void
PacketSinkRxCb(Ptr<const Packet> packet, const Address &address)
{
    NS_LOG_INFO(Simulator::Now().GetSeconds() << ": PacketSinkRx: " << address << "-" << packet->GetSize());
}


////////////////
int
main (int argc, char *argv[])
{
  LogComponentEnable("TestbedServer", LOG_LEVEL_INFO);
  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

  int          m_run              = 0;
  double       m_totalTime        = 3000.0;
  std::string  m_routing          = "split";
  bool         m_pcap             = false;
  double       m_charge           = 6300.0;

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
  emu.EnablePcap (ipAddr + "_" + m_routing + "_server_" + std::to_string(m_run), device);

  Config::ConnectWithoutContext("/NodeList/0/$ns3::Ipv4L3Protocol/SendOutgoing", MakeCallback(&SendOutgoingCb));
  Config::ConnectWithoutContext("/NodeList/0/$ns3::Ipv4L3Protocol/UnicastForward", MakeCallback(&UnicastForwardCb));
  Config::ConnectWithoutContext("/NodeList/0/$ns3::Ipv4L3Protocol/LocalDeliver", MakeCallback(&LocalDeliverCb));

  Config::ConnectWithoutContext("/NodeList/0/DeviceList/0/$ns3::FdNetDevice/MacRx", MakeCallback(&MacRx));
  Simulator::Schedule (Seconds (1.0), &CalculateThroughput);

  Config::ConnectWithoutContext("/NodeList/0/ApplicationList/0/$ns3::PacketSink/Rx", MakeCallback(&PacketSinkRxCb));

  if (m_routing == "aodv")
  {
    ; // nothing to do, no traces available
  }else if (m_routing == "dsdv"){
    ; // nothing to do, no traces available
  }else if (m_routing == "olsr"){
    Config::ConnectWithoutContext("/NodeList/0/$ns3::olsr::RoutingProtocol/Rx", MakeCallback(&OlsrRxCb));
    Config::ConnectWithoutContext("/NodeList/0/$ns3::olsr::RoutingProtocol/Tx", MakeCallback(&OlsrTxCb));
    Config::ConnectWithoutContext("/NodeList/0/$ns3::olsr::RoutingProtocol/RoutingTableChanged", MakeCallback(&OlsrRoutingTableChangedCb));
  }else if (m_routing == "etx"){
    Config::ConnectWithoutContext("/NodeList/0/$ns3::etx::RoutingProtocol/Rx", MakeCallback(&EtxRxCb));
    Config::ConnectWithoutContext("/NodeList/0/$ns3::etx::RoutingProtocol/Tx", MakeCallback(&EtxTxCb));
    Config::ConnectWithoutContext("/NodeList/0/$ns3::etx::RoutingProtocol/RoutingTableChanged", MakeCallback(&EtxRoutingTableChangedCb));
  }else if (m_routing == "split"){
    Config::ConnectWithoutContext("/NodeList/0/$ns3::split::RoutingProtocol/Rx", MakeCallback(&SplitRxCb));
    Config::ConnectWithoutContext("/NodeList/0/$ns3::split::RoutingProtocol/Tx", MakeCallback(&SplitTxCb));
    Config::ConnectWithoutContext("/NodeList/0/$ns3::split::RoutingProtocol/RoutingTableChanged", MakeCallback(&SplitRoutingTableChangedCb));
  }

  ina->TraceConnectWithoutContext ("RemainingCharge", MakeCallback (&RemainingChargeCb));

  Simulator::Stop (Seconds (m_totalTime));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
