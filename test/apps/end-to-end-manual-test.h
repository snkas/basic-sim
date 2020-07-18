/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/flow-scheduler.h"
#include "ns3/tcp-optimizer.h"
#include "ns3/arbiter-ecmp-helper.h"
#include "ns3/test.h"
#include "../test-helpers.h"
#include <iostream>
#include <fstream>

using namespace ns3;

////////////////////////////////////////////////////////////////////////////////////////

class EndToEndManualTestCase : public TestCase {
public:
    EndToEndManualTestCase () : TestCase ("end-to-end manual") {};
    const std::string temp_dir = ".tmp-end-to-end-manual-test";

    void DoRun () {

        // Create temporary run directory
        mkdir_if_not_exists(temp_dir);
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");

        // Basic configuration
        std::ofstream config_file;
        config_file.open (temp_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=2000000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "filename_topology=\"topology.properties\"" << std::endl;
        config_file << "link_data_rate_megabit_per_s=30" << std::endl;
        config_file << "link_delay_ns=10000" << std::endl;
        config_file << "link_max_queue_size_pkts=100" << std::endl;
        config_file << "disable_qdisc_endpoint_tors_xor_servers=true" << std::endl;
        config_file << "disable_qdisc_non_endpoint_switches=true" << std::endl;
        config_file.close();

        // Topology
        std::ofstream topology_file;
        topology_file.open (temp_dir + "/topology.properties");
        topology_file << "num_nodes=2" << std::endl;
        topology_file << "num_undirected_edges=1" << std::endl;
        topology_file << "switches=set(0,1)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1)" << std::endl;
        topology_file.close();

        // Perform basic simulation
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(temp_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
        TcpOptimizer::OptimizeBasic(basicSimulation);

        // Install flow sink on all
        FlowSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 1024));
        sink.Install(topology->GetNodes()).Start(Seconds(0.0));

        // 0 --> 1
        FlowSendHelper source0(
                "ns3::TcpSocketFactory",
                InetSocketAddress(topology->GetNodes().Get(1)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1024),
                1000000,
                0,
                true,
                basicSimulation->GetLogsDir()
        );
        source0.Install(topology->GetNodes().Get(0)).Start(NanoSeconds(0));

        // 1 --> 0
        FlowSendHelper source1(
                "ns3::TcpSocketFactory",
                InetSocketAddress(topology->GetNodes().Get(0)->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1024),
                89999,
                1,
                true,
                basicSimulation->GetLogsDir()
        );
        source1.Install(topology->GetNodes().Get(1)).Start(NanoSeconds(7000));

        // Install echo server on all nodes
        UdpRttServerHelper echoServerHelper(1025);
        echoServerHelper.Install(topology->GetNodes()).Start(Seconds(0.0));;

        // Pinging 0 --> 1
        UdpRttClientHelper source_ping0(
                topology->GetNodes().Get(1)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(),
                1025,
                0,
                1
        );
        source_ping0.SetAttribute("Interval", TimeValue(NanoSeconds(100000000)));
        source_ping0.Install(topology->GetNodes().Get(0)).Start(NanoSeconds(0));

        // Pinging 1 --> 0
        UdpRttClientHelper source_ping1(
                topology->GetNodes().Get(0)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(),
                1025,
                1,
                0
        );
        source_ping1.SetAttribute("Interval", TimeValue(NanoSeconds(100000000)));
        source_ping1.Install(topology->GetNodes().Get(1)).Start(NanoSeconds(0));

        // Run simulation
        basicSimulation->Run();
        basicSimulation->Finalize();

        // Check finished.txt
        std::vector<std::string> finished_lines = read_file_direct(temp_dir + "/logs_ns3/finished.txt");
        ASSERT_EQUAL(finished_lines.size(), 1);
        ASSERT_EQUAL(finished_lines[0], "Yes");

        // Make sure these are removed
        remove_file_if_exists(temp_dir + "/config_ns3.properties");
        remove_file_if_exists(temp_dir + "/topology.properties");
        remove_file_if_exists(temp_dir + "/schedule.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/flows.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/flows.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/pingmesh.csv");
        remove_file_if_exists(temp_dir + "/logs_ns3/pingmesh.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/flow_0_cwnd.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/flow_0_progress.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/flow_0_rtt.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/flow_1_cwnd.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/flow_1_progress.txt");
        remove_file_if_exists(temp_dir + "/logs_ns3/flow_1_rtt.txt");
        remove_dir_if_exists(temp_dir + "/logs_ns3");
        remove_dir_if_exists(temp_dir);

    }

};

////////////////////////////////////////////////////////////////////////////////////////
