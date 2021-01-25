/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowSimpleDoubleServerBindTestCase : public TestCaseWithLogValidators
{
public:
    TcpFlowSimpleDoubleServerBindTestCase () : TestCaseWithLogValidators ("tcp-flow-simple double-server-bind") {};

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
        TcpFlowServerHelper tcpFlowServerHelper(
                InetSocketAddress(nodes.Get(0)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 1029)
        );
        ApplicationContainer tcpFlowServerApp = tcpFlowServerHelper.Install(nodes.Get(0));
        tcpFlowServerApp.Start(NanoSeconds(0));
        tcpFlowServerApp = tcpFlowServerHelper.Install(nodes.Get(0));
        tcpFlowServerApp.Start(NanoSeconds(0));

        ASSERT_EXCEPTION_MATCH_WHAT(Simulator::Run(), "Failed to bind socket");
        Simulator::Destroy ();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class TcpFlowSimpleDoubleClientBindTestCase : public TestCaseWithLogValidators
{
public:
    TcpFlowSimpleDoubleClientBindTestCase () : TestCaseWithLogValidators ("tcp-flow-simple double-client-bind") {};

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
        TcpFlowClientHelper tcpFlowClientHelper(
                InetSocketAddress(nodes.Get(0)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 77),
                InetSocketAddress(nodes.Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 1029),
                6483,
                1000000000,
                "aaa",
                false,
                "log_directory_does_not_matter"
        );
        ApplicationContainer tcpFlowClientApp = tcpFlowClientHelper.Install(nodes.Get(0));
        ASSERT_EQUAL(tcpFlowClientApp.Get(0)->GetObject<TcpFlowClient>()->GetTcpFlowId(), 6483);
        ASSERT_EQUAL(tcpFlowClientApp.Get(0)->GetObject<TcpFlowClient>()->GetAdditionalParameters(), "aaa");
        tcpFlowClientApp.Start(NanoSeconds(0));
        tcpFlowClientApp = tcpFlowClientHelper.Install(nodes.Get(0));
        tcpFlowClientApp.Start(NanoSeconds(0));

        ASSERT_EXCEPTION_MATCH_WHAT(Simulator::Run(), "Failed to bind socket");
        Simulator::Destroy ();

    }
};

////////////////////////////////////////////////////////////////////////////////////////
