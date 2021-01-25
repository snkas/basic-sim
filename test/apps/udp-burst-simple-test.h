/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstSimpleHeaderTestCase : public TestCaseWithLogValidators
{
public:
    UdpBurstSimpleHeaderTestCase () : TestCaseWithLogValidators ("udp-burst-simple header") {};

    void DoRun () {
        UdpBurstHeader header;
        header.SetId(55);
        header.SetSeq(66);
        ASSERT_EQUAL(header.GetId(), 55);
        ASSERT_EQUAL(header.GetSeq(), 66);
        ASSERT_EQUAL(header.GetSerializedSize(), 16);
        std::ostringstream stream;
        header.Print(stream);
        ASSERT_EQUAL(stream.str(), "(id=55, seq=66)");
    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstSimpleDoubleServerBindTestCase : public TestCaseWithLogValidators
{
public:
    UdpBurstSimpleDoubleServerBindTestCase () : TestCaseWithLogValidators ("udp-burst-simple double-server-bind") {};

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
        UdpBurstServerHelper burstServerHelper(
                InetSocketAddress(nodes.Get(0)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 1029),
                "log_directory_does_not_matter"
        );
        ApplicationContainer udpBurstServerApp = burstServerHelper.Install(nodes.Get(0));
        udpBurstServerApp.Start(NanoSeconds(0));
        udpBurstServerApp = burstServerHelper.Install(nodes.Get(0));
        udpBurstServerApp.Start(NanoSeconds(0));

        ASSERT_EXCEPTION_MATCH_WHAT(Simulator::Run(), "Failed to bind socket");
        Simulator::Destroy ();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class UdpBurstSimpleDoubleClientBindTestCase : public TestCaseWithLogValidators
{
public:
    UdpBurstSimpleDoubleClientBindTestCase () : TestCaseWithLogValidators ("udp-burst-simple double-client-bind") {};

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
        UdpBurstClientHelper udpBurstClientHelper(
                InetSocketAddress(nodes.Get(0)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 77),
                InetSocketAddress(nodes.Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 1029),
                33,
                15.0,
                NanoSeconds(700000000),
                "abc",
                false,
                "log_directory_does_not_matter"
        );
        ApplicationContainer udpBurstClientApp = udpBurstClientHelper.Install(nodes.Get(0));
        ASSERT_EQUAL(udpBurstClientApp.Get(0)->GetObject<UdpBurstClient>()->GetUdpBurstId(), 33);
        ASSERT_EQUAL(udpBurstClientApp.Get(0)->GetObject<UdpBurstClient>()->GetAdditionalParameters(), "abc");
        udpBurstClientApp.Start(NanoSeconds(0));
        udpBurstClientApp = udpBurstClientHelper.Install(nodes.Get(0));
        udpBurstClientApp.Start(NanoSeconds(0));

        ASSERT_EXCEPTION_MATCH_WHAT(Simulator::Run(), "Failed to bind socket");
        Simulator::Destroy ();

    }
};

////////////////////////////////////////////////////////////////////////////////////////
