/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <iostream>
#include <fstream>

#include "ns3/basic-simulation.h"
#include "ns3/tcp-flow-scheduler.h"
#include "ns3/topology-ptop.h"
#include "ns3/tcp-config-helper.h"
#include "ns3/arbiter-ecmp-helper.h"
#include "ns3/ipv4-arbiter-routing-helper.h"
#include "ns3/ptop-link-net-device-utilization-tracking.h"
#include "ns3/ptop-link-net-device-queue-tracking.h"
#include "ns3/ptop-link-interface-tc-qdisc-queue-tracking.h"

using namespace ns3;

int main(int argc, char *argv[]) {

    // Prepare run directory
    mkdir_if_not_exists("example_programmable");
    const std::string run_dir = "example_programmable/basic-sim-example-single-default";
    mkdir_if_not_exists(run_dir);
    remove_file_if_exists(run_dir + "/config_ns3.properties");
    remove_file_if_exists(run_dir + "/topology.properties");
    remove_file_if_exists(run_dir + "/tcp_flow_schedule.csv");

    // Write config file
    // 5 seconds duration
    std::ofstream config_file;
    config_file.open (run_dir + "/config_ns3.properties");
    config_file << "simulation_end_time_ns=5000000000" << std::endl;
    config_file << "simulation_seed=747385728" << std::endl;
    config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
    config_file << "tcp_config=basic" << std::endl;
    config_file << "enable_link_net_device_queue_tracking=true" << std::endl;
    config_file << "enable_link_net_device_utilization_tracking=true" << std::endl;
    config_file << "link_net_device_utilization_tracking_interval_ns=100000000" << std::endl;
    config_file << "enable_link_interface_tc_qdisc_queue_tracking=true" << std::endl;
    config_file << "enable_tcp_flow_scheduler=true" << std::endl;
    config_file << "tcp_flow_schedule_filename=\"tcp_flow_schedule.csv\"" << std::endl;
    config_file << "tcp_flow_enable_logging_for_tcp_flow_ids=all" << std::endl;
    config_file.close();

    // Write topology file (0 - 1)
    // Channel delay: 10 microsecond delay
    // Net-device rate: 10 Mbit/s
    // Net-device queue: drop-tail of 20 packets
    // Receiving net-device error model: none (as in, no random drops)
    // Interface traffic-control queuing discipline: FIFO (= same as drop-tail) of 20 packets
    std::ofstream topology_file;
    topology_file.open (run_dir + "/topology.properties");
    topology_file << "num_nodes=2" << std::endl;
    topology_file << "num_undirected_edges=1" << std::endl;
    topology_file << "switches=set(0,1)" << std::endl;
    topology_file << "switches_which_are_tors=set(0,1)" << std::endl;
    topology_file << "servers=set()" << std::endl;
    topology_file << "undirected_edges=set(0-1)" << std::endl;
    topology_file << "link_channel_delay_ns=10000" << std::endl;
    topology_file << "link_net_device_data_rate_megabit_per_s=10" << std::endl;
    topology_file << "link_net_device_queue=drop_tail(20p)" << std::endl;
    topology_file << "link_net_device_receive_error_model=none" << std::endl;
    topology_file << "link_interface_traffic_control_qdisc=fifo(20p)" << std::endl;
    topology_file.close();

     // Write TCP flow schedule file
    std::ofstream schedule_file;
    schedule_file.open (run_dir + "/tcp_flow_schedule.csv");
    schedule_file << "0,0,1,5000000,0,," << std::endl; // Flow 0 from node 0 to node 1 of size 5 MB starting at t=0
    schedule_file.close();

    // Load basic simulation environment
    Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(run_dir);

    // Read point-to-point topology, and install routing arbiters
    Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
    ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);

    // Install link net-device utilization trackers
    PtopLinkNetDeviceUtilizationTracking netDeviceUtilizationTracking = PtopLinkNetDeviceUtilizationTracking(basicSimulation, topology); // Requires enable_link_net_device_utilization_tracking=true

    // Install link net-device queue trackers
    PtopLinkNetDeviceQueueTracking netDeviceQueueTracking = PtopLinkNetDeviceQueueTracking(basicSimulation, topology); // Requires enable_link_net_device_queue_tracking=true

    // Install link interface traffic-control qdisc queue trackers
    PtopLinkInterfaceTcQdiscQueueTracking tcQdiscQueueTracking = PtopLinkInterfaceTcQdiscQueueTracking(basicSimulation, topology); // Requires enable_link_interface_tc_qdisc_queue_tracking=true

    // Configure TCP
    TcpConfigHelper::Configure(basicSimulation);

    // Schedule TCP flows
    TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology); // Requires enable_tcp_flow_scheduler=true

    // Run simulation
    basicSimulation->Run();

    // Write TCP flow results
    tcpFlowScheduler.WriteResults();

    // Write link net-device utilization results
    netDeviceUtilizationTracking.WriteResults();

    // Write link net-device queue results
    netDeviceQueueTracking.WriteResults();

    // Write link interface traffic-control qdisc queue results
    tcQdiscQueueTracking.WriteResults();

    // Finalize the simulation
    basicSimulation->Finalize();

    return 0;
}
