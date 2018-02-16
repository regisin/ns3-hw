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

#include "ns3/split-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/aodv-module.h"
#include "ns3/etx-module.h"

#include "ns3/netanim-module.h"

#include "ns3/network-module.h"
#include "ns3/internet-module.h"

#include "ns3/simple-wireless-module.h"
#include "ns3/wifi-module.h"
#include "ns3/energy-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"

#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ScratchSimulator");

int          m_xSize            = 500;
int          m_ySize            = 500;
int          m_numNodes         = 25;
double       m_totalTime        = 200.0;
double       m_totalAppTxTime   = 0.0;
uint32_t     m_nIfaces          = 1;
std::string  m_routing          = "split";
bool         m_pcap             = false;
double       m_initEnergy       = 100.0;

/// Application params
int          m_numAppNodes      = 1;
int          m_numSinkNodes     = 1;
uint32_t     m_appStartTime     = 0;

/// Flow monitor stats
uint32_t     totalRx            = 0;
uint32_t     totalTx            = 0;
uint32_t     totalDrop          = 0;
double       delay              = 0;
double       counter            = 0;
double       hopCount           = 0;



/// Custom stats
bool         m_verbose          = false;
int          m_run              = 1;
uint32_t     bytesTotal         = 0;
int          m_appPktRx         = 0;
int          m_appPktTx         = 0;
std::map<uint32_t, double> m_remainingEnergy;
std::map<uint32_t, double> m_lifetime;
std::map<uint64_t, std::vector<uint32_t> > packetPath;

/// List of network nodes
NodeContainer               nodes;
NodeContainer               appNodes;
NodeContainer               sinkNodes;
/// List of all mesh point devices
NetDeviceContainer          devices;
///Addresses of interfaces:
Ipv4InterfaceContainer      interfaces;
/// Device models container
DeviceEnergyModelContainer  deviceModels;
// Energy source container
EnergySourceContainer       energySources;

/* Function declarations */
/// Create nodes and setup their mobility
void CreateNodes ();
/// Install internet m_stack on nodes
void InstallInternetStack ();
/// Install energy model on nodes
void InstallEnergy ();
/// Install mobility
void InstallMobility();
/// Install applications
void InstallApplication ();
/// Print mesh devices diagnostics
void Report ();

/// Callback functions
void RxL3protocol (std::string, Ptr<const Packet>, Ptr<Ipv4>, uint32_t);
void LocalL3protocol (std::string, const Ipv4Header&, Ptr<const Packet>, uint32_t);
void ReceivePacket (std::string, Ptr<const Packet>, const Address&);
void RemainingEnergy (std::string, double, double);
void PrintResults(std::string);

void
RxL3protocol (std::string str, Ptr<const Packet> packet, Ptr<Ipv4> ipv4, uint32_t interface)
{
    Ptr<Node> n = ipv4->GetObject<Node> ();
    packetPath[packet->GetUid()].push_back(n->GetId ());
}
void
LocalL3protocol (std::string str, const Ipv4Header &header, Ptr<const Packet> packet, uint32_t interface)
{
    Ptr<Node> n;
    for(uint32_t i=0; i<devices.GetN(); i++)
    {
        n = devices.Get(i)->GetNode();
		Ptr<Ipv4> ipv4 = n->GetObject<Ipv4>();
        Ipv4InterfaceAddress iaddr = ipv4->GetAddress(1, 0);

        if (header.GetDestination().Get() == iaddr.GetLocal().Get()) {
            packetPath[packet->GetUid()].push_back(packet->GetSize());
        }
        if (header.GetSource().Get() == iaddr.GetLocal().Get()) {
            std::vector<uint32_t>::iterator iterator =packetPath[packet->GetUid()].begin();
            packetPath[packet->GetUid()].insert(iterator,i);
        }
    }
}
void
ReceivePacket (std::string str, Ptr<const Packet> packet, const Address &addr)
{
    bytesTotal += packet->GetSize ();

    m_appPktRx += 1;

    m_totalAppTxTime = Simulator::Now().GetSeconds();
}

void
RemainingEnergy (std::string str, double oldValue, double remainingEnergy)
{
    m_remainingEnergy[atoi(str.c_str())] = remainingEnergy;
    if (remainingEnergy == 0) m_lifetime[atoi(str.c_str())] = Simulator::Now().GetSeconds();
}

void
AppTx (std::string str, Ptr<const Packet> packet)
{
    m_appPktTx += 1;
}

void
Configure (int argc, char *argv[])
{
    if (m_verbose)
        std::cout <<"Configure"<< std::endl;

    CommandLine cmd;
    cmd.AddValue ("id",          "Experiment ID, to customize output file [1]", m_run);
    cmd.AddValue ("x-size",      "Size of simulation x-limit in meters. [1000]", m_xSize);
    cmd.AddValue ("y-size",      "Size of simulation y-limit in meters. [1000]", m_ySize);
    cmd.AddValue ("nodes",       "Total number of nodes in the scenario. [10]", m_numNodes);
    cmd.AddValue ("app",         "Total number of application nodes in the scenario. [1]", m_numAppNodes);
    cmd.AddValue ("sink",        "Total number of sink nodes in the scenario. [1]", m_numSinkNodes);
    cmd.AddValue ("time",        "Simulation time, seconds [100 s]", m_totalTime);
    cmd.AddValue ("interfaces",  "Number of radio interfaces used by each node. [1]", m_nIfaces);
    cmd.AddValue ("routing",     "Routing protocol to use, olsr default. [aodv/dsdv/olsr/olsr-etx/split]", m_routing);
    cmd.AddValue ("v",           "Verbose output. [0]", m_verbose);
    cmd.AddValue ("pcap",        "Enable PCAP traces on interfaces. [0]", m_pcap);
    cmd.Parse (argc, argv);


    SeedManager::SetRun (m_run);

    // Initialize all nodes lifetime to the maximum
    for (uint32_t j = 0; j < (uint32_t)m_numNodes; ++j)
    {
        m_lifetime[j] = m_totalTime;
    }

    NS_LOG_DEBUG ("Limits:" << m_xSize << "*" << m_ySize);
    NS_LOG_DEBUG ("Simulation time: " << m_totalTime << " s");
}
void
CreateNodes ()
{
    if (m_verbose)
        std::cout <<"Create Nodes"<< std::endl;

    nodes.Create(m_numNodes);

    // Create error model and set as default
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
    em->SetAttribute ("ErrorRate", DoubleValue (0.0));
    em->SetAttribute ("ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));
    Config::SetDefault ("ns3::SimpleWirelessNetDevice::ReceiveErrorModel", PointerValue(em));
    Config::SetDefault ("ns3::SimpleWirelessChannel::MaxRange", DoubleValue (100.1));

    /*
     * Add radio single device per node (for now!)
     */
    Ptr<SimpleWirelessChannel> phy = CreateObject<SimpleWirelessChannel> ();
    phy->SetErrorRate(0.0);
    phy->SetErrorModelType(CONSTANT);

    NodeContainer::Iterator it;
    for (it = nodes.Begin (); it != nodes.End (); ++it)
    {
        Ptr<Node> node = *it;

        // create device
        Ptr<SimpleWirelessNetDevice> simpleWireless = CreateObject<SimpleWirelessNetDevice> ();
        simpleWireless->SetChannel(phy);
        simpleWireless->SetNode(node);
        simpleWireless->SetAddress(Mac48Address::Allocate ());
        simpleWireless->SetDataRate(DataRate (1000000.0));

       /*
        * Enable pcap (or not)
        */
        if (m_pcap)
            simpleWireless->EnablePcapAll (std::string ("sm-"));

        node->AddDevice (simpleWireless);
        devices.Add (simpleWireless);
    }
}
void
InstallInternetStack ()
{
    if (m_verbose)
        std::cout <<"Install Internet Stack"<< std::endl;
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

    InternetStackHelper internet;
    internet.SetRoutingHelper (list);
    internet.Install (nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    interfaces = ipv4.Assign (devices);
}
void
InstallEnergy()
{
    if (m_verbose)
        std::cout <<"Install Energy"<< std::endl;
    /*
     * Install energy model
     */
    BasicEnergySourceHelper basicEnergySrc;
    basicEnergySrc.Set("BasicEnergySourceInitialEnergyJ", DoubleValue (m_initEnergy));
    basicEnergySrc.Set("PeriodicEnergyUpdateInterval", TimeValue (MilliSeconds (100)));

    // install source
    energySources = basicEnergySrc.Install (nodes);

    for (uint32_t index = 0; index < devices.GetN (); index++)
    {
        Ptr<NetDevice> device = devices.Get (index);
        Ptr<EnergySource> es = energySources.Get (index);
        Ptr<Node> node = device->GetNode();
        NS_ASSERT (node == es->GetNode ());

        Ptr<SimpleWirelessEnergyModel> model = CreateObject<SimpleWirelessEnergyModel> ();
        model->SetNode (node);
        model->SetEnergySource (es);


        es->SetNode (node);
        es->AppendDeviceEnergyModel (model);

        Ptr<SimpleWirelessNetDevice> sd = device->GetObject<SimpleWirelessNetDevice> ();
        sd->SetEnergyDevice(model);

        deviceModels.Add (model);
    }
}
void
InstallMobility()
{
    if (m_verbose)
        std::cout <<"Install Mobility"<< std::endl;
    /*
     * Install nodes mobility
     */

     MobilityHelper mobility;
     mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                    "MinX", DoubleValue (0.0),
                                    "MinY", DoubleValue (0.0),
                                    "DeltaX", DoubleValue (100.0),
                                    "DeltaY", DoubleValue (100.0),
                                    "GridWidth", UintegerValue (5),
                                    "LayoutType", StringValue ("RowFirst"));
     mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

     // For mobile nodes
//   MobilityHelper mobility;
//   mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
//                                  "X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max="+ std::to_string(m_xSize) +"]"),
//                                  "Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max="+ std::to_string(m_ySize) +"]"));
//   mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
//                              "Mode", StringValue ("Time"),
//                              "Time", StringValue ("10s"),
//                              "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=15.0]"),
//                              "Bounds", RectangleValue (Rectangle (0.0, m_xSize, 0.0, m_ySize)));

    mobility.Install (nodes);
}

void
InstallApplication()
{
    if (m_verbose)
        std::cout <<"Install Applications"<< std::endl;

    uint16_t port = 2014; // App port number

    /* Destination */
    // Set up the application in the receiving node
    // Simple sink application simulating data consumption
    for (int i = 0; i < m_numSinkNodes; ++i)
    {
        sinkNodes.Add(nodes.Get(i));
    }

    for (uint32_t j = 0; j < sinkNodes.GetN(); ++j)
    {
        Ptr<Node> node = sinkNodes.Get(j);
        PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (interfaces.GetAddress (j), port));
        sink.Install(node);
    }

    /* Source */
    //
    // Create OnOff applications to send TCP to a [random] destination.
    //
    Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
    x->SetAttribute ("Min", DoubleValue (1.0));
    x->SetAttribute ("Max", DoubleValue ((double)nodes.GetN()));

    for (int iii = 0; iii < m_numAppNodes; iii++)
    {
        uint32_t ii = (uint32_t)x->GetValue();

        bool iiIsAppAlready = true;
        while (iiIsAppAlready == true)
        {
            Ptr<Node> app2 = nodes.Get(ii);

            bool temp = false;
            for (uint32_t n = 0; n < appNodes.GetN (); n++)
            {
                Ptr<Node> app = appNodes.Get(n);
                if (app == app2)
                {
                    temp = true;
                }
            }
            if (temp == true)
            {
                ii = (uint32_t)x->GetValue();
            }
            else
            {
                iiIsAppAlready = false;
            }
        }
        appNodes.Add(nodes.Get(ii));
    }

    uint32_t h = 0; // remote address index
    ApplicationContainer apps;
    OnOffHelper onOffHelper ("ns3::TcpSocketFactory", Address ());
    onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
    onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));

    for (uint32_t j = 0; j < appNodes.GetN(); ++j)
    {
        onOffHelper.SetAttribute ("DataRate", DataRateValue (DataRate ("50kb/s")));

        AddressValue remoteAddress (InetSocketAddress (interfaces.GetAddress (h), port));
        onOffHelper.SetAttribute ("Remote", remoteAddress);

        h++;
        if (h >= sinkNodes.GetN ())
        {
            h = 0;
        }
        apps.Add (onOffHelper.Install (appNodes.Get(j)));
    }

    // For random start and stop time of flows

    // x->SetAttribute ("Min", DoubleValue (0.01));
    // x->SetAttribute ("Max", DoubleValue (m_appStartTime));
    // for (uint32_t j = 0; j < apps.GetN(); ++j)
    // {
    //   uint32_t start = x->GetValue();
    //   uint32_t end = x->GetValue();
    //   if (start == end) start -= .01;
    //   Ptr<Application> a = apps.Get(j);
    //   a->SetStartTime(Seconds (std::min(start,end)));
    //   a->SetStopTime(Seconds (std::max(start,end)));
    // }


    apps.Start (Seconds (m_appStartTime));
    apps.Stop (Seconds (m_totalTime));
}

int
Run ()
{
    if (m_verbose)
        std::cout <<"Run"<< std::endl;

    CreateNodes ();
    InstallInternetStack ();
    InstallEnergy ();
    InstallMobility();
    InstallApplication ();

    /*
     * Link callback functions for measurements
     */
    Config::Connect("/NodeList/*/ApplicationList/*/$ns3::OnOffApplication/Tx",MakeCallback(&AppTx));
    Config::Connect("/NodeList/*/ApplicationList/*/$ns3::PacketSink/Rx",MakeCallback(&ReceivePacket));
    Config::Connect("/NodeList/*/$ns3::Ipv4L3Protocol/Rx",MakeCallback(&RxL3protocol));
    Config::Connect("/NodeList/*/$ns3::Ipv4L3Protocol/LocalDeliver",MakeCallback(&LocalL3protocol));
    for (uint32_t e = 0; e < deviceModels.GetN (); e++)
    {
        Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource> (energySources.Get (e));
        basicSourcePtr->TraceConnect ("RemainingEnergy",std::to_string(e), MakeCallback (&RemainingEnergy));
    }

    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.Install(nodes);
    monitor->SetAttribute("DelayBinWidth", DoubleValue(0.001));
    monitor->SetAttribute("JitterBinWidth", DoubleValue(0.001));
    monitor->SetAttribute("PacketSizeBinWidth", DoubleValue(1000));

    std::string csvoutput = ""
            + m_routing + "_"
            + std::to_string(m_xSize) + "x"
            + std::to_string(m_ySize) + "_"
            + std::to_string(m_numNodes) + "_"
            + std::to_string(m_nIfaces) + "_"
            + std::to_string(m_numAppNodes) + "_"
            + std::to_string(m_numSinkNodes) + "_"
            + std::to_string(m_totalTime) + "_"
            + std::to_string(m_run) + ".csv";

    Simulator::Stop (Seconds (m_totalTime));
    Simulator::Run ();

    monitor->CheckForLostPackets ();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
    std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
        Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);

        if (t.sourceAddress != t.destinationAddress)
        {
            if (i->second.rxPackets != 0)
            {
                totalRx +=  i->second.rxPackets;
                totalTx +=  i->second.txPackets;
                hopCount += (i->second.timesForwarded / i->second.rxPackets + 1);
                delay += (i->second.delaySum.GetSeconds() / i->second.rxPackets);
                counter++;
                for (uint32_t j=0; j < i->second.packetsDropped.size() ; j++)
                {
                       totalDrop += i->second.packetsDropped[j];
                }
            }
        }
    }

    Simulator::Destroy ();

    PrintResults(csvoutput);

    return 0;
}
void
PrintResults(std::string csvfilename)
{
    if (m_verbose) {
        int n = 0;
        std::cout << "Verbose:" << std::endl;
        std::map<uint64_t, std::vector<uint32_t> >::iterator iter;
        for (iter = packetPath.begin(); iter != packetPath.end(); ++iter)
        {
            if (iter->second.back() > 255)
            {
                n++;
                std::cout << "Pkt " + std::to_string(n) + " (" << iter->first << "," << iter->second.back() << ") path: " << "";
                for(uint32_t x=0; x<iter->second.size()-1; x++)
                {
                    std::cout << iter->second[x] << " ";
                }
                std::cout<<std::endl;
            }
        }
    }

    double energy = 0.0;
    double life = 0.0;

    std::string pdffilename = "power_" + csvfilename;
    std::ofstream ofile;
    ofile.open (pdffilename.c_str());
    ofile << "node_id,initial_energy,remaining_energy,lifetime" << std::endl;

    for (uint32_t j = 0; j < nodes.GetN(); ++j)
    {
        energy += m_remainingEnergy[j];
        life += m_lifetime[j];
        ofile << j << "," << m_initEnergy << "," << m_remainingEnergy[j] << "," << m_lifetime[j] << std::endl;
    }
    life = life / nodes.GetN();
    ofile.close();

    ofile.open (csvfilename.c_str());
    ofile << "total_bytes_received,throughput_(bps),no_of_received_packets,packet_delivery_ratio,mean_hop_count,mean_delay_(s),l3_drop_packets,total_initial_energy,total_remaining_energy,average_lifetime" << std::endl;
    ofile << bytesTotal << ",";
    ofile << (bytesTotal*8)/m_totalTime << ",";
    ofile << m_appPktRx << ",";
    ofile << ((double)m_appPktRx / (double)m_appPktTx) << ",";
    ofile << hopCount / counter << ",";
    ofile << delay / counter << ",";
    ofile << totalDrop << ",";
    ofile << deviceModels.GetN() * m_initEnergy << ",";
    ofile << energy << ",";
    ofile << life << std::endl;
    ofile.close();
}


int
main (int argc, char *argv[])
{
    Configure (argc, argv);
    return Run ();
}
