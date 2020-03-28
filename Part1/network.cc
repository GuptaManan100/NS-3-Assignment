//2020 - Quarantine
//Group 8 - Manan Gupta, Ashish Agarwal, Mriganka Basu Roy Chaudhary
//Networks Lab Assignment 4 - Part 1

//Header Files
#include <iostream>
#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/stats-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/enum.h" 

#define pb push_back

using namespace ns3;

class ClientApp : public Application
{
public:
  ClientApp ();
  virtual ~ClientApp ();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

ClientApp::ClientApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

ClientApp::~ClientApp ()
{
  m_socket = 0;
}

void
ClientApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
ClientApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  if (InetSocketAddress::IsMatchingType (m_peer))
    {
      m_socket->Bind ();
    }
  else
    {
      m_socket->Bind6 ();
    }
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
ClientApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
ClientApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void
ClientApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &ClientApp::SendPacket, this);
    }
}


int main(int argc, char *argv[])
{
	std::cout << std::fixed;
	std::string tcpAgent = "TCPveno";

	CommandLine cmd;
	cmd.AddValue ("tcpAgent", "TCP Agent to use", tcpAgent);
	cmd.Parse (argc, argv);

	if(tcpAgent == "TCPvegas")
	{
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVegas"));
	}
	else if(tcpAgent == "TCPveno")
	{
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVeno"));
	}
	else if(tcpAgent == "TCPwestwood")
	{
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpWestwood"));
	}
	else
	{
		std::cout<<"Incorrect Parameters"<<std::endl;
		exit(1);
	}

	std::vector<int> sizPack;
	sizPack.pb(40);
	sizPack.pb(44);
	sizPack.pb(48);
	sizPack.pb(52);
	sizPack.pb(60);
	sizPack.pb(250);
	sizPack.pb(300);
	sizPack.pb(552);
	sizPack.pb(576);
	sizPack.pb(628);
	sizPack.pb(1420);
	sizPack.pb(1500);

	for(auto siz : sizPack)
	{
		NodeContainer nodes;
		nodes.Create (4);

		PointToPointHelper p2pOuter;
		p2pOuter.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
		p2pOuter.SetChannelAttribute ("Delay", StringValue ("20ms"));
		int x = 100*20*1000/(8*siz);
		std::string sp;
		sp = std::to_string(x)+"p";
		//std::cout<<sp<<std::endl;
		p2pOuter.SetQueue ("ns3::DropTailQueue","MaxSize", StringValue(sp));
		
		NetDeviceContainer devicesL,devicesM,devicesR;
		devicesL = p2pOuter.Install (nodes.Get(0),nodes.Get(1));
		devicesR = p2pOuter.Install (nodes.Get(2),nodes.Get(3));

		PointToPointHelper p2pInner;
		p2pInner.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
		p2pInner.SetChannelAttribute ("Delay", StringValue ("50ms"));
		x = 10*50*1000/(8*siz);
		sp = std::to_string(x)+"p";
		//std::cout<<sp<<std::endl;
		p2pInner.SetQueue ("ns3::DropTailQueue","MaxSize", StringValue(sp));
		devicesM = p2pInner.Install(nodes.Get(1),nodes.Get(2));

		InternetStackHelper stack;
		stack.Install (nodes);

		uint16_t serverPort = 4290;
		Address serverAddress;
		Address anyAddress;

		Ipv4AddressHelper addressL;
		addressL.SetBase("10.10.10.0","255.255.255.0");
		Ipv4InterfaceContainer interfacesL = addressL.Assign (devicesL);

		Ipv4AddressHelper addressM;
		addressM.SetBase("10.10.11.0","255.255.255.0");
		Ipv4InterfaceContainer interfacesM = addressM.Assign (devicesM);

		Ipv4AddressHelper addressR;
		addressR.SetBase("10.10.12.0","255.255.255.0");
		Ipv4InterfaceContainer interfacesR = addressR.Assign (devicesR);

		serverAddress = InetSocketAddress (interfacesR.GetAddress(1),serverPort);
		anyAddress = InetSocketAddress(Ipv4Address::GetAny(),serverPort);

		Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

		PacketSinkHelper packetServerHelper ("ns3::TcpSocketFactory", anyAddress);
		ApplicationContainer serverApp = packetServerHelper.Install (nodes.Get (3));
		serverApp.Start (Seconds (0.));
		serverApp.Stop (Seconds (20.));


		Ptr<Socket> ns3TcpSocket = Socket::CreateSocket (nodes.Get (0), TcpSocketFactory::GetTypeId ());
		Ptr<ClientApp> clientApp = CreateObject<ClientApp> ();
		clientApp->Setup (ns3TcpSocket, serverAddress, siz, 10000, DataRate ("20Mbps"));
		nodes.Get (0)->AddApplication (clientApp);
		clientApp->SetStartTime (Seconds (1.));
		clientApp->SetStopTime (Seconds (20.));

		FlowMonitorHelper flowmon;
		Ptr<FlowMonitor> monitor = flowmon.InstallAll();
		
		Simulator::Stop (Seconds (20));
		Simulator::Run ();

		std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

		double totTime, totBits, throughput, sumThr, sumsqThr;
		sumThr = 0;
		sumsqThr = 0;
		int coun = 0;

		auto iter = stats.begin();
		totBits = 8.0 * iter->second.rxBytes;
		totTime = iter->second.timeLastRxPacket.GetSeconds();
		totTime -= iter->second.timeFirstTxPacket.GetSeconds();

		//std::cout<<"totBits: "<<totBits<<" time: "<<totTime<<std::endl;
		throughput = totBits/totTime;
		sumThr+= throughput;
		sumsqThr += throughput*throughput;
		coun++;

		double fairness = (sumThr*sumThr)/(sumsqThr*(coun+0.0));
		std::cout<<"Packet Size: "<<siz<<" ";
		std::cout<<"Total Bits: "<<totBits<<" Time:"<<totTime<<" ";
		std::cout<<"Throughput: "<<throughput<<" Fairness: "<<fairness<<std::endl;

		monitor->SerializeToXmlFile(tcpAgent+"_"+std::to_string(siz)+".xml", true, true);
		Simulator::Destroy ();
	}

	return 0;
}
