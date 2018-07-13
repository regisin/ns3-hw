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

#include "ns3/uinteger.h"

#include "ns3/split-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/aodv-module.h"
#include "ns3/etx-module.h"

// #include "ns3/netanim-module.h"

#include "ns3/network-module.h"
#include "ns3/internet-module.h"

#include "ns3/bridge-module.h"
#include "ns3/simple-wireless-module.h"
#include "ns3/wifi-module.h"
#include "ns3/energy-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"

#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MobileSplit");

int          m_numInterfaces    = 1;
int          m_xSize            = 500;
int          m_ySize            = 500;
int          m_numNodes         = 25;
double       m_totalTime        = 200.0;
double       m_totalAppTxTime   = 0.0;
uint32_t     m_nIfaces          = 1;
std::string  m_routing          = "split";
bool         m_pcap             = false;
double       m_initEnergy       = 300.0;
uint32_t     m_histSize         = 10000;
double       m_speed            = 15;

/// Application params
int          m_numAppNodes      = 1;
int          m_numSinkNodes     = 1;
uint32_t     m_appStartTime     = 30;

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
NetDeviceContainer          devices50;
NetDeviceContainer          devices100;
NetDeviceContainer          devices200;
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
void LogResults();

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
    CommandLine cmd;
    cmd.AddValue ("id",          "Experiment ID, to customize output file [1]", m_run);
    cmd.AddValue ("x-size",      "Size of simulation x-limit in meters. [1000]", m_xSize);
    cmd.AddValue ("y-size",      "Size of simulation y-limit in meters. [1000]", m_ySize);
    cmd.AddValue ("nodes",       "Total number of nodes in the scenario. [10]", m_numNodes);
    cmd.AddValue ("hist",        "Size of the history stack. [10000]", m_histSize);
    cmd.AddValue ("speed",       "Speed in m/s to be used in the mobility model (walk2d). [15]", m_speed);
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
}
void
CreateNodes ()
{
    nodes.Create(m_numNodes);

    // Create error model and set as default
    Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
    em->SetAttribute ("ErrorRate", DoubleValue (0.0));
    em->SetAttribute ("ErrorUnit", StringValue ("ERROR_UNIT_PACKET"));
    Config::SetDefault ("ns3::SimpleWirelessNetDevice::ReceiveErrorModel", PointerValue(em));

    /*
     * Add radio single device per node
     */
    // 100m range and DataRate(1000000.0)
    Ptr<SimpleWirelessChannel> phy100 = CreateObject<SimpleWirelessChannel> ();
    phy100->SetAttribute("MaxRange", DoubleValue(100.0));
    phy100->SetErrorModelType(CONSTANT);

    NodeContainer::Iterator it;
    for (it = nodes.Begin (); it != nodes.End (); ++it)
    {
        Ptr<Node> node = *it;
        Ptr<SimpleWirelessNetDevice> simpleWireless100 = CreateObject<SimpleWirelessNetDevice> ();

        simpleWireless100->SetChannel(phy100);
        simpleWireless100->SetNode(node);
        simpleWireless100->SetAddress(Mac48Address::Allocate ());
        simpleWireless100->SetDataRate(DataRate (1000000.0));

       /*
        * Enable pcap (or not)
        */
        if (m_pcap)
        {
            simpleWireless100->EnablePcapAll (
                std::string (m_routing+"-500x500-25-1-"+std::to_string(m_numAppNodes)+"-1-100-"+std::to_string(m_totalTime)+"-"+std::to_string(m_run))+".pcap");
        }

        
        node->AddDevice (simpleWireless100);
        devices.Add (simpleWireless100);
    }
}
void
InstallInternetStack ()
{
    /*
     * Add the routing protocol
     */
 

    InternetStackHelper internet;
    if (m_routing == "aodv")
    {
        AodvHelper routing;
        internet.SetRoutingHelper(routing);
    }else if (m_routing == "dsdv"){
        DsdvHelper routing;
        internet.SetRoutingHelper(routing);
    }else if (m_routing == "olsr"){
        OlsrHelper routing;
        internet.SetRoutingHelper(routing);
    }else if (m_routing == "etx"){
        EtxHelper routing;
        internet.SetRoutingHelper(routing);
    }else if (m_routing == "split"){
        SplitHelper routing;
        routing.Set("HistorySize", UintegerValue(m_histSize));
        internet.SetRoutingHelper(routing);
    }

    internet.SetIpv4StackInstall(true);
    internet.Install (nodes);

    Ipv4AddressHelper ipv4;
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    interfaces = ipv4.Assign (devices);
}
void
InstallEnergy()
{
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

        Ptr<EnergySource> es = energySources.Get (index/m_numInterfaces);
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
    // mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

    // For mobile nodes
    // MobilityHelper mobility;
    // mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
    //                                "X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max="+ std::to_string(m_xSize) +"]"),
    //                                "Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max="+ std::to_string(m_ySize) +"]"));
    std::string randomVar = std::string("ns3::ConstantRandomVariable[Constant=") + std::to_string(m_speed) + "]";
    mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Mode", StringValue ("Time"),
                             "Time", StringValue ("10s"),
                             "Speed", StringValue (randomVar),
                             "Bounds", RectangleValue (Rectangle (0.0, m_xSize, 0.0, m_ySize)));

    mobility.Install (nodes);
}

void
InstallApplication()
{
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
        PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (interfaces.GetAddress (j), port));
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
    apps.Start (Seconds (m_appStartTime));
    apps.Stop (Seconds (m_totalTime));

    OnOffHelper onOffHelper ("ns3::UdpSocketFactory", Address ());
    onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
    onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0.0]"));

    for (uint32_t j = 0; j < appNodes.GetN(); ++j)
    {
        onOffHelper.SetAttribute ("DataRate", DataRateValue (DataRate ("1Mb/s")));
        onOffHelper.SetAttribute ("PacketSize", UintegerValue (1024));

        AddressValue remoteAddress (InetSocketAddress (interfaces.GetAddress (h), port));
        onOffHelper.SetAttribute ("Remote", remoteAddress);

        apps.Add (onOffHelper.Install (appNodes.Get(j)));

        h++;
        if (h >= sinkNodes.GetN ())
        {
            h = 0;
        }
    }
}

int
Run ()
{
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
        Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource> (energySources.Get (e/m_numInterfaces));
        basicSourcePtr->TraceConnect ("RemainingEnergy",std::to_string(e/m_numInterfaces), MakeCallback (&RemainingEnergy));
    }

    FlowMonitorHelper flowmon;
    Ptr<FlowMonitor> monitor = flowmon.Install(nodes);
    monitor->SetAttribute("DelayBinWidth", DoubleValue(0.001));
    monitor->SetAttribute("JitterBinWidth", DoubleValue(0.001));
    monitor->SetAttribute("PacketSizeBinWidth", DoubleValue(1000));

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

    LogResults();

    return 0;
}
void
LogResults()
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

    // std::string pdffilename = "power_" + csvfilename;
    // std::ofstream ofile;
    // ofile.open (pdffilename.c_str());
    // ofile << "node_id,initial_energy,remaining_energy,lifetime" << std::endl;

    for (uint32_t j = 0; j < nodes.GetN(); ++j)
    {
        energy += m_remainingEnergy[j];
        life += m_lifetime[j];
        // ofile << j << "," << m_initEnergy << "," << m_remainingEnergy[j] << "," << m_lifetime[j] << std::endl;
    }
    life = life / nodes.GetN();
    // ofile.close();

    NS_LOG_INFO(
        m_run << "," <<
        bytesTotal << "," <<
        (bytesTotal*8)/m_totalTime << "," <<
        m_appPktRx << "," <<
        ((double)m_appPktRx / (double)m_appPktTx) << "," <<
        hopCount / counter << "," <<
        delay / counter << "," <<
        totalDrop << "," <<
        deviceModels.GetN() * m_initEnergy << "," <<
        energy << "," <<
        life
    );
}


int
main (int argc, char *argv[])
{
    
    LogComponentEnable("MobileSplit", LOG_LEVEL_INFO);

    // UNCOMMENT TO DEBUG
    // LogLevel logInfo = static_cast<LogLevel>(LOG_PREFIX_TIME | LOG_PREFIX_FUNC | LOG_PREFIX_NODE | LOG_LEVEL_INFO);
    // LogComponentEnableAll(logInfo);
    Configure (argc, argv);
    return Run ();
}