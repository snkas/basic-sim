/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/ipv4-routing-protocol.h"

////////////////////////////////////////////////////////////////////////////////////////

class ArbiterTestCase : public TestCaseWithLogValidators
{
public:
    ArbiterTestCase (std::string s) : TestCaseWithLogValidators (s) {};
    std::string test_run_dir = ".tmp-test-arbiter";

    void prepare_arbiter_test_config() {
        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=10000000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file.close();
    }

    void prepare_arbiter_test_default_topology() {
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=4" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1,2,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3,0-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
    }

    void cleanup_arbiter_test() {
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);
    }

};

////////////////////////////////////////////////////////////////////////////////////////

class ArbiterIpResolutionTestCase : public ArbiterTestCase
{
public:
    ArbiterIpResolutionTestCase () : ArbiterTestCase ("routing-arbiter basic") {};
    void DoRun () {
        test_run_dir = ".tmp-test-arbiter-basic";
        prepare_clean_run_dir(test_run_dir);
        prepare_arbiter_test_config();
        prepare_arbiter_test_default_topology();

        // Create topology
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Create nodes, setup links and create arbiter
        NodeContainer nodes = topology->GetNodes();
        std::vector<std::pair<uint32_t, uint32_t>> interface_idxs_for_edges = topology->GetInterfaceIdxsForUndirectedEdges();
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // Test valid IPs
        Ptr<Arbiter> arbiter = nodes.Get(0)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->GetArbiter();
        ASSERT_EQUAL(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.0.1").Get()), 0);
        ASSERT_EQUAL(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.0.2").Get()), 1);
        ASSERT_EQUAL(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.1.1").Get()), 0);
        ASSERT_EQUAL(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.1.2").Get()), 3);
        ASSERT_EQUAL(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.2.1").Get()), 1);
        ASSERT_EQUAL(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.2.2").Get()), 2);
        ASSERT_EQUAL(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.3.1").Get()), 2);
        ASSERT_EQUAL(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.3.2").Get()), 3);

        // All other should be invalid, a few examples
        ASSERT_EXCEPTION(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.0.0").Get()));
        ASSERT_EXCEPTION(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.1.3").Get()));
        ASSERT_EXCEPTION(arbiter->ResolveNodeIdFromIp(Ipv4Address("10.0.4.1").Get()));

        basicSimulation->Finalize();
        cleanup_arbiter_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class ArbiterResultTestCase : public TestCase
{
public:
    ArbiterResultTestCase () : TestCase ("arbiter result") {};
    void DoRun () {

        // Correct success outcome
        ArbiterResult result_success(false, 4, 53);
        ASSERT_FALSE(result_success.Failed());
        ASSERT_EQUAL(result_success.GetOutIfIdx(), 4);
        ASSERT_EQUAL(result_success.GetGatewayIpAddress(), 53);

        // Correct failure outcome
        ArbiterResult result_fail(true, 0, 0);
        ASSERT_TRUE(result_fail.Failed());
        ASSERT_EXCEPTION_MATCH_WHAT(result_fail.GetOutIfIdx(), "Cannot retrieve out interface index if the arbiter did not succeed in finding a next hop");
        ASSERT_EXCEPTION_MATCH_WHAT(result_fail.GetGatewayIpAddress(), "Cannot retrieve gateway IP address if the arbiter did not succeed in finding a next hop");

        // Invalid constructions
        ASSERT_EXCEPTION_MATCH_WHAT(ArbiterResult(true, 1, 0), "If the arbiter result is a failure, the out interface index must be zero.");
        ASSERT_EXCEPTION_MATCH_WHAT(ArbiterResult(true, 0, 1), "If the arbiter result is a failure, the gateway IP address must be zero.");
        ASSERT_EXCEPTION_MATCH_WHAT(ArbiterResult(false, 0, 0), "If the arbiter result is not a failure, the out interface index cannot be zero (= loop-back interface).");

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class ArbiterPtopOneTestCase : public ArbiterTestCase
{
public:
    ArbiterPtopOneTestCase () : ArbiterTestCase ("arbiter-ptop one") {};
    void DoRun () {

        // Prepare
        test_run_dir = ".tmp-test-arbiter-ptop-one";
        prepare_clean_run_dir(test_run_dir);
        prepare_arbiter_test_config();
        prepare_arbiter_test_default_topology();

        // Create topology
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Create nodes, setup links and create arbiter
        NodeContainer nodes = topology->GetNodes();
        std::vector<std::pair<uint32_t, uint32_t>> interface_idxs_for_edges = topology->GetInterfaceIdxsForUndirectedEdges();
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // IP header which was stripped before
        Ipv4Header ipHeader;
        ipHeader.SetSource(Ipv4Address("10.0.0.1")); // Interface of node 0
        ipHeader.SetDestination(Ipv4Address("10.0.1.2")); // Interface of node 3
        ipHeader.SetProtocol(6);

        // TCP packet
        Ptr<Packet> p = Create<Packet>(100);
        TcpHeader tcpHeader;
        tcpHeader.SetSourcePort(24245);
        tcpHeader.SetDestinationPort(222);
        p->AddHeader(tcpHeader);

        // Test one forwarding decision extensively
        std::set<int64_t> neighbors_of_0;
        neighbors_of_0.insert(1);
        neighbors_of_0.insert(3);
        Ptr<ArbiterEcmp> arbiter = nodes.Get(0)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->GetArbiter()->GetObject<ArbiterEcmp>();
        ASSERT_EQUAL(3, arbiter->TopologyPtopDecide(0, 3, neighbors_of_0, p, ipHeader, false));

        // Test one forwarding decision extensively
        Ptr<Arbiter> arbiterParent = nodes.Get(0)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->GetArbiter();
        ArbiterResult result = arbiterParent->BaseDecide(p, ipHeader);
        ASSERT_FALSE(result.Failed());
        ASSERT_EQUAL(result.GetOutIfIdx(), 2);
        ASSERT_EQUAL(result.GetGatewayIpAddress(), 0);

        // Check the string representation
        ASSERT_EQUAL(
                arbiterParent->StringReprOfForwardingState(),
                "ECMP state of node 0\n  -> 0: {}\n  -> 1: {1}\n  -> 2: {1,3}\n  -> 3: {3}\n"
        );

        // Clean-up
        basicSimulation->Finalize();
        cleanup_arbiter_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class Ipv4ArbiterRoutingExceptionsTestCase : public ArbiterTestCase
{
public:
    Ipv4ArbiterRoutingExceptionsTestCase () : ArbiterTestCase ("ipv4-arbiter-routing exceptions") {};
    void DoRun () {

        // Prepare
        test_run_dir = ".tmp-test-ipv4-arbiter-routing-exceptions";
        prepare_clean_run_dir(test_run_dir);
        prepare_arbiter_test_config();
        prepare_arbiter_test_default_topology();

        // Create topology
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Fetch nodes
        NodeContainer nodes = topology->GetNodes();

        // IP header which was stripped before
        Ipv4Header ipHeader;
        ipHeader.SetSource(Ipv4Address("10.0.0.1"));
        ipHeader.SetDestination(Ipv4Address("10.0.3.1"));
        ipHeader.SetProtocol(6);

        // TCP packet
        Ptr<Packet> p = Create<Packet>(100);
        TcpHeader tcpHeader;
        tcpHeader.SetSourcePort(24245);
        tcpHeader.SetDestinationPort(222);
        p->AddHeader(tcpHeader);

        // Now check that indeed it throws exception the arbiter is not set
        Ipv4RoutingProtocol::UnicastForwardCallback ucb;
        Ipv4RoutingProtocol::MulticastForwardCallback mcb;
        Ipv4RoutingProtocol::LocalDeliverCallback lcb;
        Ipv4RoutingProtocol::ErrorCallback ecb;
        ASSERT_EXCEPTION_MATCH_WHAT(
                (nodes.Get(0)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->RouteInput(
                        p,
                        ipHeader,
                        nodes.Get(0)->GetDevice(1),
                        ucb,
                        mcb,
                        lcb,
                        ecb
                )),
                "Arbiter has not been set"
        );

        // Check that interfaces are not permitted to go down
        ASSERT_EXCEPTION_MATCH_WHAT(
                (nodes.Get(0)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->NotifyInterfaceDown(1)),
                "Interfaces are not permitted to go down."
        );

        // Check that addresses are not allowed to be removed
        ASSERT_EXCEPTION_MATCH_WHAT(
                (nodes.Get(0)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->NotifyRemoveAddress(1, Ipv4InterfaceAddress())),
                "Not permitted to remove IP addresses."
        );

        // Clean-up
        basicSimulation->Finalize();
        cleanup_arbiter_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

struct ecmp_fields_t {
    int32_t node_id;
    uint32_t src_ip;
    uint32_t dst_ip;
    bool is_tcp;
    bool is_udp;
    uint16_t src_port;
    uint16_t dst_port;
};

void create_headered_packet(Ptr<Packet> p, ecmp_fields_t e) {

    // Set IP header
    Ipv4Header ipHeader;
    ipHeader.SetSource(Ipv4Address(e.src_ip));
    ipHeader.SetDestination(Ipv4Address(e.dst_ip));

    if (e.is_tcp) { // Set TCP+IP header
        TcpHeader tcpHeader;
        tcpHeader.SetSourcePort(e.src_port);
        tcpHeader.SetDestinationPort(e.dst_port);
        ipHeader.SetProtocol(6);
        p->AddHeader(tcpHeader);
        p->AddHeader(ipHeader);

    } else if (e.is_udp) { // Setup UDP+IP header
        UdpHeader udpHeader;
        udpHeader.SetSourcePort(e.src_port);
        udpHeader.SetDestinationPort(e.dst_port);
        ipHeader.SetProtocol(17);
        p->AddHeader(udpHeader);
        p->AddHeader(ipHeader);

    } else { // Setup only IP header
        p->AddHeader(ipHeader);
    }

}

class ArbiterEcmpHashTestCase : public ArbiterTestCase
{
public:
    ArbiterEcmpHashTestCase () : ArbiterTestCase ("routing-arbiter-ecmp hash") {};
    void DoRun () {
        test_run_dir = ".tmp-test-routing-arbiter-ecmp-hash";
        prepare_clean_run_dir(test_run_dir);
        prepare_arbiter_test_config();
        prepare_arbiter_test_default_topology();

        // Create topology
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Create nodes, setup links and create arbiter
        NodeContainer nodes = topology->GetNodes();
        std::vector<std::pair<uint32_t, uint32_t>> interface_idxs_for_edges = topology->GetInterfaceIdxsForUndirectedEdges();
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // One example ECMP arbiter
        Ptr<ArbiterEcmp> routingArbiterEcmp = nodes.Get(0)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->GetArbiter()->GetObject<ArbiterEcmp>();

        ///////
        // All hashes should be different if any of the 5 values are different

        ecmp_fields_t cases[] = {
                {1, 2, 3, true, false, 4, 5}, // TCP
                {6, 2, 3, true, false, 4, 5}, // TCP different node id
                {1, 6, 3, true, false, 4, 5}, // TCP different source IP
                {1, 2, 6, true, false, 4, 5}, // TCP different destination IP
                {1, 2, 3, true, false, 6, 5}, // TCP different source port
                {1, 2, 3, true, false, 4, 6}, // TCP different destination port
                {1, 2, 3, false, true, 4, 5}, // UDP
                {6, 2, 3, false, true, 4, 5}, // UDP different node id
                {1, 6, 3, false, true, 4, 5}, // UDP different source IP
                {1, 2, 6, false, true, 4, 5}, // UDP different destination IP
                {1, 2, 3, false, true, 6, 5}, // UDP different source port
                {1, 2, 3, false, true, 4, 6}, // UDP different destination port
                {1, 2, 3, false, false, 1, 1}, // Not TCP/UDP
                {6, 2, 3, false, false, 1, 1}, // Not TCP/UDP different node id
                {1, 6, 3, false, false, 1, 1}, // Not TCP/UDP different source IP
                {1, 2, 6, false, false, 1, 1}, // Not TCP/UDP different destination IP
        };
        int num_cases = sizeof(cases) / sizeof(ecmp_fields_t);
        std::vector<uint32_t> hash_results;
        std::set<uint32_t> hash_results_set;
        for (ecmp_fields_t e : cases) {
            Ptr<Packet> p = Create<Packet>(5);
            create_headered_packet(p, e);
            Ipv4Header ipHeader;
            p->RemoveHeader(ipHeader);
            hash_results.push_back(routingArbiterEcmp->ComputeFiveTupleHash(ipHeader, p, e.node_id, false));
        }
        for (int i = 0; i < num_cases; i++) {
            for (int j = i + 1; j < num_cases; j++) {
                ASSERT_NOT_EQUAL(hash_results[i], hash_results[j]);
            }
            // std::cout << "Hash[" << i << "] = " << hash_results[i] << std::endl;
        }

        ///////
        // Test same hash

        // Same TCP for same 5-tuple
        Ptr<Packet> p1 = Create<Packet>(555);
        Ptr<Packet> p2 = Create<Packet>(20);
        create_headered_packet(p1, {6666, 4363227, 215326, true, false, 4663, 8888});
        Ipv4Header p1header;
        p1->RemoveHeader(p1header);
        create_headered_packet(p2, {6666, 4363227, 215326, true, false, 4663, 8888});
        Ipv4Header p2header;
        p2->RemoveHeader(p2header);
        ASSERT_EQUAL(
                routingArbiterEcmp->ComputeFiveTupleHash(p1header, p1, 6666, false),
                routingArbiterEcmp->ComputeFiveTupleHash(p2header, p2, 6666, false)
        );

        // Same UDP for same 5-tuple
        p1 = Create<Packet>(555);
        p2 = Create<Packet>(8888);
        create_headered_packet(p1, {123456, 32543526, 937383, false, true, 1234, 6737});
        p1->RemoveHeader(p1header);
        create_headered_packet(p2, {123456, 32543526, 937383, false, true, 1234, 6737});
        p2->RemoveHeader(p2header);
        ASSERT_EQUAL(
                routingArbiterEcmp->ComputeFiveTupleHash(p1header, p1, 123456, false),
                routingArbiterEcmp->ComputeFiveTupleHash(p2header, p2, 123456, false)
        );

        // Same not TCP/UDP for same 3-tuple
        p1 = Create<Packet>(77);
        p2 = Create<Packet>(77);
        create_headered_packet(p1, {7, 3626, 22, false, false, 55, 123});
        p1->RemoveHeader(p1header);
        create_headered_packet(p2, {7, 3626, 22, false, false, 44, 7777});
        p2->RemoveHeader(p2header);
        ASSERT_EQUAL(
                routingArbiterEcmp->ComputeFiveTupleHash(p1header, p1, 7, false),
                routingArbiterEcmp->ComputeFiveTupleHash(p2header, p2, 7, false)
        );

        // TCP: Different source and destination port, so hash should also be different
        p1 = Create<Packet>(77);
        p2 = Create<Packet>(77);
        create_headered_packet(p1, {7, 3626, 22, true, false, 55, 123});
        p1->RemoveHeader(p1header);
        create_headered_packet(p2, {7, 3626, 22, true, false, 44, 7777});
        p2->RemoveHeader(p2header);
        ASSERT_NOT_EQUAL(
                routingArbiterEcmp->ComputeFiveTupleHash(p1header, p1, 7, false),
                routingArbiterEcmp->ComputeFiveTupleHash(p2header, p2, 7, false)
        );

        // TCP: If the protocol field is to be ignored because the other headers are said to be not there explicitly
        p1 = Create<Packet>(77);
        p2 = Create<Packet>(77);
        create_headered_packet(p1, {7, 3626, 22, true, false, 55, 123});
        p1->RemoveHeader(p1header);
        create_headered_packet(p2, {7, 3626, 22, true, false, 44, 7777});
        p2->RemoveHeader(p2header);
        ASSERT_EQUAL(
                routingArbiterEcmp->ComputeFiveTupleHash(p1header, p1, 7, true),
                routingArbiterEcmp->ComputeFiveTupleHash(p2header, p2, 7, true)
        );

        // UDP: Different source and destination port, so hash should also be different
        p1 = Create<Packet>(77);
        p2 = Create<Packet>(77);
        create_headered_packet(p1, {7, 3626, 22, false, true, 55, 123});
        p1->RemoveHeader(p1header);
        create_headered_packet(p2, {7, 3626, 22, false, true, 44, 7777});
        p2->RemoveHeader(p2header);
        ASSERT_NOT_EQUAL(
                routingArbiterEcmp->ComputeFiveTupleHash(p1header, p1, 7, false),
                routingArbiterEcmp->ComputeFiveTupleHash(p2header, p2, 7, false)
        );

        // UDP: If the protocol field is to be ignored because the other headers are said to be not there explicitly
        p1 = Create<Packet>(77);
        p2 = Create<Packet>(77);
        create_headered_packet(p1, {7, 3626, 22, false, true, 55, 123});
        p1->RemoveHeader(p1header);
        create_headered_packet(p2, {7, 3626, 22, false, true, 44, 7777});
        p2->RemoveHeader(p2header);
        ASSERT_EQUAL(
                routingArbiterEcmp->ComputeFiveTupleHash(p1header, p1, 7, true),
                routingArbiterEcmp->ComputeFiveTupleHash(p2header, p2, 7, true)
        );

        // Expected hash input buffer
        uint8_t buf[21];

        // Source IP address = 3301134135 = 11000100 11000011 01001111 00110111
        buf[0] = (uint8_t) 196;
        buf[1] = (uint8_t) 195;
        buf[2] = (uint8_t) 79;
        buf[3] = (uint8_t) 55;

        // Destination IP address = 1232533515 = 01001001 01110110 11111000 00001011
        buf[4] = (uint8_t) 73;
        buf[5] = (uint8_t) 118;
        buf[6] = (uint8_t) 248;
        buf[7] = (uint8_t) 11;

        // Protocol specific
        // buf[8] = (uint8_t) ;

        // Source port = 12325 = 00110000 00100101
        buf[9] = (uint8_t) 48;
        buf[10] = (uint8_t) 37;

        // Destination port = 33321 = 10000010 00101001
        buf[11] = (uint8_t) 130;
        buf[12] = (uint8_t) 41;

        // Per-node perturbation = 1233533413 = 01001001 10000110 00111001 11100101
        buf[13] = (uint8_t) 73;
        buf[14] = (uint8_t) 134;
        buf[15] = (uint8_t) 57;
        buf[16] = (uint8_t) 229;

        // ECMP constant perturbation = 1333527522 = 01001111 01111100 00000011 11100010
        buf[17] = (uint8_t) 79;
        buf[18] = (uint8_t) 124;
        buf[19] = (uint8_t) 3;
        buf[20] = (uint8_t) 226;

        // TCP: exact match expectation with murmur3
        p1 = Create<Packet>(77);
        create_headered_packet(p1, {1233533413, 3301134135, 1232533515, true, false, 12325, 33321});
        p1->RemoveHeader(p1header);
        // Protocol = 6 = 00000110
        buf[8] = (uint8_t) 6;
        ASSERT_EQUAL(
                routingArbiterEcmp->ComputeFiveTupleHash(p1header, p1, 1233533413, false),
                Hash32 ((char*) buf, 21)
        );

        // UDP: exact match expectation with murmur3
        p1 = Create<Packet>(434);
        create_headered_packet(p1, {1233533413, 3301134135, 1232533515, false, true, 12325, 33321});
        p1->RemoveHeader(p1header);
        // Protocol = 17 = 00010001
        buf[8] = (uint8_t) 17;
        ASSERT_EQUAL(
                routingArbiterEcmp->ComputeFiveTupleHash(p1header, p1, 1233533413, false),
                Hash32 ((char*) buf, 21)
        );

        // Clean-up
        basicSimulation->Finalize();
        cleanup_arbiter_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class ArbiterEcmpStringReprTestCase : public ArbiterTestCase
{
public:
    ArbiterEcmpStringReprTestCase () : ArbiterTestCase ("routing-arbiter-ecmp string-repr") {};
    void DoRun () {
        test_run_dir = ".tmp-test-routing-arbiter-ecmp-string-repr";
        prepare_clean_run_dir(test_run_dir);
        prepare_arbiter_test_config();
        prepare_arbiter_test_default_topology();

        // Create topology
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Create nodes, setup links and create arbiter
        NodeContainer nodes = topology->GetNodes();
        std::vector<std::pair<uint32_t, uint32_t>> interface_idxs_for_edges = topology->GetInterfaceIdxsForUndirectedEdges();
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        for (int i = 0; i < topology->GetNumNodes(); i++) {
            std::ostringstream res;
            OutputStreamWrapper out_stream = OutputStreamWrapper(&res);
            nodes.Get(i)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->PrintRoutingTable(&out_stream);
            std::ostringstream expected;
            if (i == 0) {
                expected << "ECMP state of node 0" << std::endl;
                expected << "  -> 0: {}" << std::endl;
                expected << "  -> 1: {1}" << std::endl;
                expected << "  -> 2: {1,3}" << std::endl;
                expected << "  -> 3: {3}" << std::endl;
                ASSERT_EQUAL(res.str(), expected.str());
            } else if (i == 1) {
                expected << "ECMP state of node 1" << std::endl;
                expected << "  -> 0: {0}" << std::endl;
                expected << "  -> 1: {}" << std::endl;
                expected << "  -> 2: {2}" << std::endl;
                expected << "  -> 3: {0,2}" << std::endl;
                ASSERT_EQUAL(res.str(), expected.str());
            } else if (i == 2) {
                expected << "ECMP state of node 2" << std::endl;
                expected << "  -> 0: {1,3}" << std::endl;
                expected << "  -> 1: {1}" << std::endl;
                expected << "  -> 2: {}" << std::endl;
                expected << "  -> 3: {3}" << std::endl;
                ASSERT_EQUAL(res.str(), expected.str());
            } else if (i == 3) {
                expected << "ECMP state of node 3" << std::endl;
                expected << "  -> 0: {0}" << std::endl;
                expected << "  -> 1: {0,2}" << std::endl;
                expected << "  -> 2: {2}" << std::endl;
                expected << "  -> 3: {}" << std::endl;
                ASSERT_EQUAL(res.str(), expected.str());
            } else {
                ASSERT_TRUE(false);
            }
        }

        // Clean-up
        basicSimulation->Finalize();
        cleanup_arbiter_test();

    }
};

//////////////////////////////////////////////////////////////////////////////////////////

class ArbiterBad: public ArbiterPtop
{
public:

    ArbiterBad(Ptr<Node> this_node, NodeContainer nodes, Ptr<TopologyPtop> topology) : ArbiterPtop(this_node, nodes, topology) {
        // Left empty intentionally
    }

    int32_t TopologyPtopDecide(
        int32_t source_node_id,
        int32_t target_node_id,
        const std::set<int64_t>& neighbor_node_ids,
        ns3::Ptr<const ns3::Packet> pkt,
        ns3::Ipv4Header const &ipHeader,
        bool is_socket_request_for_source_ip
    ) {
        return m_next_decision;
    }

    void SetDecision(int32_t val) {
        m_next_decision = val;
    }

    std::string StringReprOfForwardingState() {
        return "";
    }

private:
    int32_t m_next_decision = -1;

};

class ArbiterBadImplTestCase : public ArbiterTestCase
{
public:
    ArbiterBadImplTestCase () : ArbiterTestCase ("routing-arbiter bad-impl") {};
    void DoRun () {
        test_run_dir = ".tmp-test-routing-arbiter-bad-impl";
        prepare_clean_run_dir(test_run_dir);
        prepare_arbiter_test_config();
        prepare_arbiter_test_default_topology();

        // Create topology
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Create nodes, setup links
        NodeContainer nodes = topology->GetNodes();
        std::vector<std::pair<uint32_t, uint32_t>> interface_idxs_for_edges = topology->GetInterfaceIdxsForUndirectedEdges();
        ArbiterBad arbiterBad = ArbiterBad(nodes.Get(1), nodes, topology);

        // This should be fine
        arbiterBad.SetDecision(2);
        Ptr<Packet> p1 = Create<Packet>(555);
        create_headered_packet(p1, {1, Ipv4Address("10.0.2.1").Get(), Ipv4Address("10.0.3.2").Get(), true, false, 4663, 8888});
        Ipv4Header p1header;
        p1->RemoveHeader(p1header);
        ASSERT_EQUAL(2, arbiterBad.BaseDecide(p1, p1header).GetOutIfIdx()); // Should also be interface 2

        // This should also be fine = drop
        arbiterBad.SetDecision(-1);
        p1 = Create<Packet>(555);
        create_headered_packet(p1, {1, Ipv4Address("10.0.2.1").Get(), Ipv4Address("10.0.3.2").Get(), true, false, 4663, 8888});
        p1->RemoveHeader(p1header);
        ASSERT_TRUE(arbiterBad.BaseDecide(p1, p1header).Failed());

        // Not a neighbor
        arbiterBad.SetDecision(3);
        p1 = Create<Packet>(555);
        create_headered_packet(p1, {1, Ipv4Address("10.0.2.1").Get(), Ipv4Address("10.0.3.2").Get(), true, false, 4663, 8888});
        p1->RemoveHeader(p1header);
        ASSERT_EXCEPTION_MATCH_WHAT(arbiterBad.BaseDecide(p1, p1header), "The selected next node 3 is not a neighbor of node 1.");

        // Out of range <
        arbiterBad.SetDecision(-2);
        p1 = Create<Packet>(555);
        create_headered_packet(p1, {1, Ipv4Address("10.0.2.1").Get(), Ipv4Address("10.0.3.2").Get(), true, false, 4663, 8888});
        p1->RemoveHeader(p1header);
        ASSERT_EXCEPTION_MATCH_WHAT(arbiterBad.BaseDecide(p1, p1header), "The selected next node -2 is out of node id range of [0, 4).");

        // Out of range >
        arbiterBad.SetDecision(4);
        p1 = Create<Packet>(555);
        create_headered_packet(p1, {1, Ipv4Address("10.0.2.1").Get(), Ipv4Address("10.0.3.2").Get(), true, false, 4663, 8888});
        p1->RemoveHeader(p1header);
        ASSERT_EXCEPTION_MATCH_WHAT(arbiterBad.BaseDecide(p1, p1header), "The selected next node 4 is out of node id range of [0, 4).");

        // Clean-up
        basicSimulation->Finalize();
        cleanup_arbiter_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class ArbiterEcmpTooManyNodesTestCase : public ArbiterTestCase
{
public:
    ArbiterEcmpTooManyNodesTestCase () : ArbiterTestCase ("routing-arbiter-ecmp too-many-nodes") {};
    void DoRun () {
        test_run_dir = ".tmp-test-routing-arbiter-ecmp-too-many-nodes";
        prepare_clean_run_dir(test_run_dir);
        prepare_arbiter_test_config();

        // String for the nodes
        std::string nodes = "set(";
        for (int i = 0; i < 40001; i++) {
            if (i != 0) {
                nodes += "," + std::to_string(i);
            } else {
                nodes += std::to_string(i);
            }
        }
        nodes += ")";

        // Topology with > 40000 nodes
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=40001" << std::endl;
        topology_file << "num_undirected_edges=0" << std::endl;
        topology_file << "switches=" << nodes << std::endl;
        topology_file << "switches_which_are_tors=" << nodes << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set()" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // Create topology
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ASSERT_EXCEPTION(ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology));

        // Clean-up
        basicSimulation->Finalize();
        cleanup_arbiter_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class ArbiterEcmpSeparatedTestCase : public ArbiterTestCase
{
public:
    ArbiterEcmpSeparatedTestCase () : ArbiterTestCase ("routing-arbiter-ecmp separated") {};
    void DoRun () {
        test_run_dir = ".tmp-test-routing-arbiter-ecmp-separated";
        prepare_clean_run_dir(test_run_dir);
        prepare_arbiter_test_config();

        // Topology file
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=2" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1,2,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        // Create topology
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // Create some packet (not relevant what's in it)
        Ptr<Packet> pkt = Create<Packet>(555);
        Ipv4Header ipHeader;
        ipHeader.SetSource(Ipv4Address(1234));
        ipHeader.SetDestination(Ipv4Address(5678));
        TcpHeader tcpHeader;
        tcpHeader.SetSourcePort(435);
        tcpHeader.SetDestinationPort(777);
        ipHeader.SetProtocol(6);
        pkt->AddHeader(tcpHeader);
        pkt->AddHeader(ipHeader);

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if ((i < 2 && j >= 2) || (j < 2 && i >= 2) || i == j) {
                    ASSERT_EXCEPTION_MATCH_WHAT(topology->GetNodes().Get(i)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->GetArbiter()->GetObject<ArbiterEcmp>()->TopologyPtopDecide(
                            i,
                            j,
                            std::set<int64_t>(),
                            pkt,
                            ipHeader,
                            false
                    ), "There are no candidate ECMP next hops available at current node " + std::to_string(i) + " for a packet from source " + std::to_string(i) + " to destination " + std::to_string(j));
                } else {
                    ASSERT_EQUAL(topology->GetNodes().Get(i)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->GetArbiter()->GetObject<ArbiterEcmp>()->TopologyPtopDecide(
                            i,
                            j,
                            std::set<int64_t>(),
                            pkt,
                            ipHeader,
                            false
                    ), j);
                }
            }
        }

        // Clean-up
        basicSimulation->Finalize();
        cleanup_arbiter_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class Ipv4ArbiterRoutingNoRouteTestCase : public TestCaseWithLogValidators
{
public:
    Ipv4ArbiterRoutingNoRouteTestCase () : TestCaseWithLogValidators ("ipv4-arbiter-routing no-route") {};
    std::string run_test_dir = ".tmp-test-ipv4-arbiter-routing-no-route";

    void DoRun () {
        prepare_clean_run_dir(run_test_dir);

        std::ofstream config_file(run_test_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=100000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file << "enable_tcp_flow_scheduler=true" << std::endl;
        config_file << "tcp_flow_schedule_filename=\"tcp_flow_schedule.csv\"" << std::endl;
        config_file << "tcp_flow_enable_logging_for_tcp_flow_ids=set(0)" << std::endl;
        config_file.close();

        std::ofstream topology_file;
        topology_file.open (run_test_dir + "/topology.properties");
        topology_file << "num_nodes=2" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1)" << std::endl;
        topology_file << "all_nodes_are_endpoints=false" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=10.0" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(10p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=none" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();

        std::ofstream schedule_file;
        schedule_file.open (run_test_dir + "/tcp_flow_schedule.csv");
        schedule_file << "0,0,1,100000,0,," << std::endl;
        schedule_file.close();

        // Prepare simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(run_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

        // Install an arbiter on 0 which always return there is no route
        NodeContainer nodes = topology->GetNodes();
        Ptr<ArbiterBad> arbiterBad = CreateObject<ArbiterBad>(
                nodes.Get(0),
                nodes,
                topology
        );
        nodes.Get(0)->GetObject<Ipv4>()->GetRoutingProtocol()->GetObject<Ipv4ArbiterRouting>()->SetArbiter(arbiterBad);

        // Flow schedule
        TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology);

        // Finalize
        basicSimulation->Run();
        tcpFlowScheduler.WriteResults();
        basicSimulation->Finalize();

        // Validate TCP flow logs
        std::vector <TcpFlowScheduleEntry> tcp_flow_schedule;
        tcp_flow_schedule.push_back(TcpFlowScheduleEntry(0, 0, 1, 100000, 0, "", ""));
        std::vector <int64_t> end_time_ns_list;
        std::vector <int64_t> sent_byte_list;
        std::vector <std::string> finished_list;
        std::set<int64_t> tcp_flow_ids_with_logging;
        tcp_flow_ids_with_logging.insert(0);
        validate_tcp_flow_logs(
                100000000,
                run_test_dir,
                tcp_flow_schedule,
                tcp_flow_ids_with_logging,
                end_time_ns_list,
                sent_byte_list,
                finished_list
        );

        // Check that the connection failed
        ASSERT_EQUAL(end_time_ns_list.at(0), 100000000);
        ASSERT_EQUAL(sent_byte_list.at(0), 0);
        ASSERT_EQUAL(finished_list.at(0), "NO_CONN_FAIL");

        // Clean-up
        remove_file_if_exists(run_test_dir + "/config_ns3.properties");
        remove_file_if_exists(run_test_dir + "/topology.properties");
        remove_file_if_exists(run_test_dir + "/tcp_flow_schedule.csv");
        remove_file_if_exists(run_test_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(run_test_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(run_test_dir + "/logs_ns3/timing_results.csv");
        remove_file_if_exists(run_test_dir + "/logs_ns3/tcp_flows.csv");
        remove_file_if_exists(run_test_dir + "/logs_ns3/tcp_flows.txt");
        remove_file_if_exists(run_test_dir + "/logs_ns3/tcp_flow_0_progress.csv");
        remove_file_if_exists(run_test_dir + "/logs_ns3/tcp_flow_0_rtt.csv");
        remove_file_if_exists(run_test_dir + "/logs_ns3/tcp_flow_0_rto.csv");
        remove_file_if_exists(run_test_dir + "/logs_ns3/tcp_flow_0_cwnd.csv");
        remove_file_if_exists(run_test_dir + "/logs_ns3/tcp_flow_0_cwnd_inflated.csv");
        remove_file_if_exists(run_test_dir + "/logs_ns3/tcp_flow_0_ssthresh.csv");
        remove_file_if_exists(run_test_dir + "/logs_ns3/tcp_flow_0_inflight.csv");
        remove_file_if_exists(run_test_dir + "/logs_ns3/tcp_flow_0_state.csv");
        remove_file_if_exists(run_test_dir + "/logs_ns3/tcp_flow_0_cong_state.csv");
        remove_dir_if_exists(run_test_dir + "/logs_ns3");
        remove_dir_if_exists(run_test_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////
