#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "DCNET-helper.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/aodv-helper.h"
#include "ns3/nstime.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <iomanip>
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/names.h"


using namespace ns3;
std::string phyMode ("ErpOfdmRate54Mbps");
Ipv4InterfaceContainer ipInterfaceContainer;
NS_LOG_COMPONENT_DEFINE ("DCNET_simulation");

int 
main (int argc, char *argv[])
{
    NS_LOG_UNCOND("Inside Main");
    CommandLine cmd;
    NS_LOG_LOGIC("argc : "<<argc);
	cmd.Parse (argc, argv);
   	int numNodes=21;
    // disable fragmentation for frames below 2200 bytes
    Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
    Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",StringValue (phyMode));
    NodeContainer c;
	c.Create (numNodes);
    WifiHelper wifi;
    YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
    wifiPhy.Set("EnergyDetectionThreshold",DoubleValue(-74 )); 
	wifiPhy.Set("CcaMode1Threshold",DoubleValue(-74 )); 
	wifiPhy.Set("TxGain", DoubleValue (5));
    wifiPhy.Set("RxGain", DoubleValue (4));
    wifiPhy.Set("TxPowerLevels", UintegerValue (1));
    wifiPhy.Set("TxPowerEnd", DoubleValue (35.0)); 
    wifiPhy.Set("TxPowerStart", DoubleValue (35.0)); 
    wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
    YansWifiChannelHelper wifiChannel;
    wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
    wifiPhy.SetChannel (wifiChannel.Create ());

    // Add a non-QoS upper mac, and disable rate control
    NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
    wifi.SetStandard (WIFI_PHY_STANDARD_80211g);
    wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                  "DataMode",StringValue (phyMode),
                                  "ControlMode",StringValue (phyMode));
   // Set it to adhoc mode
    wifiMac.SetType ("ns3::AdhocWifiMac");
    NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);
    MobilityHelper mobility;
	// setup the grid itself: objects are layed out
    // started from (-100,-100) with 20 objects per row, 
    // the x interval between each object is 5 meters
    // and the y interval between each object is 20 meters
    mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                    "MinX", DoubleValue (-100.0),
                                     "MinY", DoubleValue (-100.0),
                                     "DeltaX", DoubleValue (4.0),
                                     "DeltaY", DoubleValue (20.0),
                                     "GridWidth", UintegerValue (20),
                                     "LayoutType", StringValue ("RowFirst"));
    mobility.SetMobilityModel ("ns3::RandomDirection2dMobilityModel");
	//mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (c);

    Ipv4StaticRoutingHelper staticRouting;
    Ipv4ListRoutingHelper list;
    list.Add (staticRouting, 0);
    InternetStackHelper internet;
    internet.SetRoutingHelper (list); // has effect on the next Install ()
    internet.Install (c);
    Ipv4AddressHelper ipv4;
    NS_LOG_INFO ("Assign IP Addresses.");
    ipv4.SetBase ("10.1.1.0", "255.255.255.0");
    ipInterfaceContainer = ipv4.Assign (devices);
	NS_LOG_INFO ("Create Applications.");
    uint16_t port = 9999;  // well-known echo port number
    DCNETHelper app (0,port);
    for (int i=0;i<numNodes-1;i++) {
    	ApplicationContainer apps = app.Install (c.Get (i));
    	apps.Start (Seconds (1.0));
	}
    
	DCNETHelper app1 (1, port);
	ApplicationContainer apps = app1.Install (c.Get (numNodes-1));
    apps.Start (Seconds (2.0));


  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

}