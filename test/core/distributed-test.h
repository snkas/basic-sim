/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/arbiter-ecmp.h"
#include "ns3/arbiter-ecmp-helper.h"
#include "ns3/test.h"
#include "../test-helpers.h"

using namespace ns3;

const std::string arbiter_test_dir = ".tmp-arbiter-test";

void prepare_arbiter_test_config(std::string k, std::string lp_assignment) {
    mkdir_if_not_exists(arbiter_test_dir);

    std::ofstream config_file(arbiter_test_dir + "/config_ns3.properties");
    config_file << "filename_topology=\"topology.properties.temp\"" << std::endl;
    config_file << "simulation_end_time_ns=10000000000" << std::endl;
    config_file << "simulation_seed=123456789" << std::endl;
    config_file << "link_data_rate_megabit_per_s=100.0" << std::endl;
    config_file << "link_delay_ns=10000" << std::endl;
    config_file << "link_max_queue_size_pkts=100" << std::endl;
    config_file << "disable_qdisc_endpoint_tors_xor_servers=true" << std::endl;
    config_file << "disable_qdisc_non_endpoint_switches=true" << std::endl;
    config_file << "enable_distributed=true" << std::endl;
    config_file << "distributed_logical_processes_k=" << k << std::endl;
    config_file << "distributed_node_logical_process_assignment=" << lp_assignment << std::endl;
    config_file.close();
}

void prepare_distributed_plus_grid_5x5_topology() {
    std::ofstream topology_file;
    topology_file.open (arbiter_test_dir + "/topology.properties.temp");
    topology_file << "num_nodes=25" << std::endl;
    topology_file << "num_undirected_edges=50" << std::endl;
    topology_file << "switches=set(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24)" << std::endl;
    topology_file << "switches_which_are_tors=set(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24)" << std::endl;
    topology_file << "servers=set()" << std::endl;
    topology_file << "undirected_edges=set(0-1,0-4,0-5,0-20,1-2,1-6,1-21,2-3,2-7,2-22,3-4,3-8,3-23,4-9,4-24,5-6,5-9,5-10,6-7,6-11,7-8,7-12,8-9,8-13,9-14,10-11,10-14,10-15,11-12,11-16,12-13,12-17,13-14,13-18,14-19,15-16,15-19,15-20,16-17,16-21,17-18,17-22,18-19,18-23,19-24,20-21,20-24,21-22,22-23,23-24)" << std::endl;
    topology_file.close();
}

void prepare_arbiter_test(std::string k, std::string lp_assignment) {
    prepare_arbiter_test_config(k, lp_assignment);
    prepare_distributed_plus_grid_5x5_topology();
}

void cleanup_arbiter_test() {
    remove_file_if_exists(arbiter_test_dir + "/config_ns3.properties");
    remove_file_if_exists(arbiter_test_dir + "/topology.properties.temp");
    remove_file_if_exists(arbiter_test_dir + "/logs_ns3/finished.txt");
    remove_file_if_exists(arbiter_test_dir + "/logs_ns3/timing_results.txt");
    remove_dir_if_exists(arbiter_test_dir + "/logs_ns3");
    remove_dir_if_exists(arbiter_test_dir);
}

////////////////////////////////////////////////////////////////////////////////////////

class ArbiterEcmpStringReprTestCase : public TestCase
{
public:
    ArbiterEcmpStringReprTestCase () : TestCase ("routing-arbiter-ecmp string-repr") {};
    void DoRun () {
        prepare_arbiter_test();

        // Create topology
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(arbiter_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Create nodes, setup links and create arbiter
        NodeContainer nodes = topology->GetNodes();
        std::vector<std::pair<uint32_t, uint32_t>> interface_idxs_for_edges = topology->GetInterfaceIdxsForEdges();
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

class ArbiterBadImplTestCase : public TestCase
{
public:
    ArbiterBadImplTestCase () : TestCase ("routing-arbiter bad-impl") {};
    void DoRun () {
        prepare_arbiter_test();

        // Create topology
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(arbiter_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // Create nodes, setup links
        NodeContainer nodes = topology->GetNodes();
        std::vector<std::pair<uint32_t, uint32_t>> interface_idxs_for_edges = topology->GetInterfaceIdxsForEdges();
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
        ASSERT_EXCEPTION(arbiterBad.BaseDecide(p1, p1header));

        // Out of range <
        arbiterBad.SetDecision(-2);
        p1 = Create<Packet>(555);
        create_headered_packet(p1, {1, Ipv4Address("10.0.2.1").Get(), Ipv4Address("10.0.3.2").Get(), true, false, 4663, 8888});
        p1->RemoveHeader(p1header);
        ASSERT_EXCEPTION(arbiterBad.BaseDecide(p1, p1header));

        // Out of range >
        arbiterBad.SetDecision(4);
        p1 = Create<Packet>(555);
        create_headered_packet(p1, {1, Ipv4Address("10.0.2.1").Get(), Ipv4Address("10.0.3.2").Get(), true, false, 4663, 8888});
        p1->RemoveHeader(p1header);
        ASSERT_EXCEPTION(arbiterBad.BaseDecide(p1, p1header));

        // Clean-up
        basicSimulation->Finalize();
        cleanup_arbiter_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

// Currently disabled because it takes too long for a quick test
class ArbiterEcmpTooBigFailTestCase : public TestCase
{
public:
    ArbiterEcmpTooBigFailTestCase () : TestCase ("routing-arbiter-ecmp too-big-fail") {};
    void DoRun () {
        prepare_arbiter_test_config();

        std::ofstream topology_file;
        topology_file.open (arbiter_test_dir + "topology.properties.temp");
        topology_file << "num_nodes=40001" << std::endl;
        topology_file << "num_undirected_edges=0" << std::endl;
        topology_file << "switches=set(";
        for (int i = 0; i < 40001; i++) {
            if (i == 0) {
                topology_file << i;
            } else {
                topology_file << "," << i;
            }
        }
        topology_file << ")" << std::endl;
        topology_file << "switches_which_are_tors=set()" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set()" << std::endl;
        topology_file.close();

        // Create topology
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(arbiter_test_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ASSERT_EXCEPTION(ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology)); // > 40000 nodes is not allowed

        // Clean-up
        basicSimulation->Finalize();
        cleanup_arbiter_test();

    }
};
