/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

////////////////////////////////////////////////////////////////////////////////////////

class UdpPingSimpleHeaderTestCase : public TestCaseWithLogValidators
{
public:
    UdpPingSimpleHeaderTestCase () : TestCaseWithLogValidators ("udp-ping-simple header") {};

    void DoRun () {
        UdpPingHeader header;
        header.SetId(5);
        header.SetSeq(89);
        header.SetTs(648449);
        ASSERT_EQUAL(header.GetId(), 5);
        ASSERT_EQUAL(header.GetSeq(), 89);
        ASSERT_EQUAL(header.GetTs(), 648449);
        ASSERT_EQUAL(header.GetSerializedSize(), 24);
        std::ostringstream stream;
        header.Print(stream);
        ASSERT_EQUAL(stream.str(), "(id=5, seq=89, ts=648449)");
    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpPingSimpleDoubleServerBindTestCase : public TestCaseWithLogValidators
{
public:
    UdpPingSimpleDoubleServerBindTestCase () : TestCaseWithLogValidators ("udp-ping-simple double-server-bind") {};

    void DoRun () {

        NodeContainer nodes;
        nodes.Create (2);

        PointToPointHelper pointToPoint;
        pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
        pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

        NetDeviceContainer devices;
        devices = pointToPoint.Install (nodes);

        InternetStackHelper stack;
        stack.Install (nodes);

        Ipv4AddressHelper address;
        address.SetBase ("10.1.1.0", "255.255.255.0");

        Ipv4InterfaceContainer interfaces = address.Assign (devices);

        // Start two servers with the same IP and port, which should cause a Bind() problem
        UdpPingServerHelper pingServerHelper(
                InetSocketAddress(nodes.Get(0)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 1029)
        );
        ApplicationContainer udpPingServerApp = pingServerHelper.Install(nodes.Get(0));
        udpPingServerApp.Start(NanoSeconds(0));
        udpPingServerApp = pingServerHelper.Install(nodes.Get(0));
        udpPingServerApp.Start(NanoSeconds(0));

        ASSERT_EXCEPTION_MATCH_WHAT(Simulator::Run(), "Failed to bind socket");
        Simulator::Destroy ();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpPingSimpleDoubleClientBindTestCase : public TestCaseWithLogValidators
{
public:
    UdpPingSimpleDoubleClientBindTestCase () : TestCaseWithLogValidators ("udp-ping-simple double-client-bind") {};

    void DoRun () {

        NodeContainer nodes;
        nodes.Create (2);

        PointToPointHelper pointToPoint;
        pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
        pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

        NetDeviceContainer devices;
        devices = pointToPoint.Install (nodes);

        InternetStackHelper stack;
        stack.Install (nodes);

        Ipv4AddressHelper address;
        address.SetBase ("10.1.1.0", "255.255.255.0");

        Ipv4InterfaceContainer interfaces = address.Assign (devices);

        // Start two clients with the same IP and port, which should cause a Bind() problem
        UdpPingClientHelper udpPingClientHelper(
                InetSocketAddress(nodes.Get(0)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 77),
                InetSocketAddress(nodes.Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 1029),
                79,
                NanoSeconds(10000000),
                NanoSeconds(700000000),
                NanoSeconds(10000000),
                "Something"
        );
        ApplicationContainer udpPingClientApp = udpPingClientHelper.Install(nodes.Get(0));
        ASSERT_EQUAL(udpPingClientApp.Get(0)->GetObject<UdpPingClient>()->GetUdpPingId(), 79);
        ASSERT_EQUAL(udpPingClientApp.Get(0)->GetObject<UdpPingClient>()->GetAdditionalParameters(), "Something");
        udpPingClientApp.Start(NanoSeconds(0));
        udpPingClientApp = udpPingClientHelper.Install(nodes.Get(0));
        udpPingClientApp.Start(NanoSeconds(0));

        ASSERT_EXCEPTION_MATCH_WHAT(Simulator::Run(), "Failed to bind socket");
        Simulator::Destroy ();

    }
};

////////////////////////////////////////////////////////////////////////////////////////
