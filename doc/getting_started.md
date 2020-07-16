# Getting started

The "basic simulation" is just a wrapper which makes life easier. Essentially, it is a super complicated way to write 4 lines of C++ code:

```
ns3::RngSeedManager::SetSeed(simulation_seed);
Simulator::Stop(NanoSeconds(simulation_end_time_ns));
Simulator::Run();
Simulator::Destroy();
```

However, it makes life a lot easier. It allows you to read and manage a configuration file. It keeps track of time and shows progress. It does a two-phase writing in a file called 'finished.txt' ("No" at the start, "Yes" after the run) to be able to verify that the run finished successfully.

The basic-sim module is divided into two sub-modules: `core` and `apps`. `core` is the framework, including an improved routing implementation and topology management. `apps` is about applications to run -- either inside or outside of this framework. One can simply take the apps and use them somewhere else without even having to deal with the framework.

## Tutorial

1. Create a directory anywhere called `example_run`, and create two files in there:

   **config_ns3.properties**
   
   ```
   simulation_end_time_ns=10000000000
   simulation_seed=123456789
   
   filename_topology="topology.properties"
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
    
        // TODO: Here you have to start your own applications
        // TODO: Right now there is nothing being scheduled in the network
    
        // Run simulation
        basicSimulation->Run();
    
        // TODO: Here you would be collecting the results from your applications and writing them
        // TODO: to the logs folder within your run folder
    
        // Finalize the simulation
        basicSimulation->Finalize();
    
        return 0;
    
    }
    ```

4. Run it by executing in your ns-3 folder:

   ```
   ./waf --run="main_example --run_dir='/your/path/to/example_run'"
   ```
   
   ... or if you also want to save the console output:
   
   ```
   mkdir -p /your/path/to/example_run/logs_ns3
   ./waf --run="main_example --run_dir='/your/path/to/example_run'" 2>&1 | tee /your/path/to/example_run/logs_ns3/console.txt
   ```
