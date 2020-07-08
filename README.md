# Basic simulation ns-3 module

This module is used to make the simulation of ns-3 (data center) networks a bit easier.

It introduces a wrapper around ns-3 calls commonly made 
It includes the following features:

(a) A wrapper around common ns-3 calls (e.g., `Simulator::...`, `RngSeedManager::...`) which takes care of loading in a run folder configuration key-value store, showing progress during simulation, creating a logs directory, and providing the ability to store timing moments to trace how long certain things took.

(b) A TCP optimizer which uses some heuristics to tune the TCP stack parameters.

(c) A topology file reader to easily create your own (point-to-point) topologies.

(d) A new routing protocol called `Ipv4ArbiterRouting` which enables you to easily implement your own routing protocol, including an example ECMP flow-based implementation.


## Installation

1. You copy-paste (or clone) this module into your ns-3 `src/` folder as `basic-sim`

2. Now you should be able to compile it along with all your other modules. It has been tested for ns-3 version 3.30.1.


## Getting started

1. Create a directory anywhere called `example_run`, and create two files in there:

   **config_ns3.properties**
   
   ```
   filename_topology="topology.properties"
   simulation_end_time_ns=10000000000
   simulation_seed=123456789
   link_data_rate_megabit_per_s=100.0
   link_delay_ns=10000
   link_max_queue_size_pkts=100
   disable_qdisc_endpoint_tors_xor_servers=false
   disable_qdisc_non_endpoint_switches=false
   ```
   
   **topology.properties**
   
   ```
   # One ToR with 3 servers
   #
   #    0
   #  / | \
   # 1  2  3
   
   num_nodes=4
   num_undirected_edges=3
   switches=set(0)
   switches_which_are_tors=set(0)
   servers=set(1,2,3)
   undirected_edges=set(0-1,0-2,0-3)
   ```

2. Into your ns-3 `scratch/` folder create a file named `main_example.cc`

3. The following is an example code for `scratch/main_example.cc`:

    ```c++
    #include <map>
    #include <iostream>
    #include <fstream>
    #include <string>
    #include <ctime>
    #include <iostream>
    #include <fstream>
    #include <sys/stat.h>
    #include <dirent.h>
    #include <unistd.h>
    #include <chrono>
    #include <stdexcept>
    #include "ns3/basic-simulation.h"
    #include "ns3/topology-ptop.h"
    #include "ns3/tcp-optimizer.h"
    #include "ns3/arbiter-ecmp-helper.h"
    #include "ns3/ipv4-arbiter-routing-helper.h"
    
    using namespace ns3;
    
    int main(int argc, char *argv[]) {
    
        // No buffering of printf
        setbuf(stdout, nullptr);
        
        // Retrieve run directory
        CommandLine cmd;
        std::string run_dir = "";
        cmd.Usage("Usage: ./waf --run=\"main_flows --run_dir='<path/to/run/directory>'\"");
        cmd.AddValue("run_dir",  "Run directory", run_dir);
        cmd.Parse(argc, argv);
        if (run_dir.compare("") == 0) {
            printf("Usage: ./waf --run=\"main_flows --run_dir='<path/to/run/directory>'\"");
            return 0;
        }
    
        // Load basic simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(run_dir);
    
        // Read point-to-point topology, and install routing arbiters
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
    
        // Optimize TCP
        TcpOptimizer::OptimizeUsingWorstCaseRtt(basicSimulation, topology->GetWorstCaseRttEstimateNs());
    
        // Install applications
        // TODO: Here you have to start your own applications
    
        // Run simulation
        basicSimulation->Run();
    
        // Write result
        flowScheduler.WriteResults();
    
        // Finalize the simulation
        basicSimulation->Finalize();
    
        return 0;
    
    }
    ```

4. Run it by executing in your ns-3 folder:
   ```
   mkdir -p /your/path/to/example_run/logs_ns3
   ./waf --run="main_example --run_dir='/your/path/to/example_run'" 2>&1 | tee /your/path/to/example_run/logs_ns3/console.txt
   ```
   
## Acknowledgements

Based on code written by Hussain, who did his master thesis in the NDAL group.
Refactored, extended and maintained by Simon. The ECMP routing hashing function is inspired by https://github.com/mkheirkhah/ecmp (accessed: February 20th, 2020).
