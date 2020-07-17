# Pingmesh application

The pingmesh application is when you want to continuously sends UDP pings between endpoints to measure their RTT. 

It encompasses the following files:

* `model/apps/pingmesh-scheduler.cc/h` - Schedules the pings to start. Once the run is over, it can write the results to file.
* `model/apps/udp-rtt-client.cc/h` - Sends out UDP pings and receives replies
* `model/apps/udp-rtt-server.cc/h` - Receives UDP pings, adds a received timestamp, and pings back
* `helper/apps/udp-rtt-helper.cc/h` - Helpers to install UDP RTT servers and clients

You can use the application(s) separately, or make use of the pingmesh scheduler (which is recommended).

## Getting started

1. Create a directory anywhere called `example_run`, and create two files in there:

   **config_ns3.properties**
   
   ```
   simulation_end_time_ns=10000000000
   simulation_seed=123456789
   
   filename_topology="topology.properties"
   link_data_rate_megabit_per_s=100.0
   link_delay_ns=10000
   link_max_queue_size_pkts=100
   disable_qdisc_endpoint_tors_xor_servers=true
   disable_qdisc_non_endpoint_switches=true
   
   pingmesh_interval_ns=100000000
   pingmesh_endpoint_pairs=all
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

2. Into your ns-3 `scratch/` folder create a file named `main_pingmesh.cc`

3. The following is an example code for `scratch/main_pingmesh.cc`:

    **main_pingmesh.cc**
    
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
    #include "ns3/pingmesh-scheduler.h"
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
        cmd.Usage("Usage: ./waf --run=\"main_pingmesh --run_dir='<path/to/run/directory>'\"");
        cmd.AddValue("run_dir",  "Run directory", run_dir);
        cmd.Parse(argc, argv);
        if (run_dir.compare("") == 0) {
            printf("Usage: ./waf --run=\"main_pingmesh --run_dir='<path/to/run/directory>'\"");
            return 0;
        }
    
        // Load basic simulation environment
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(run_dir);
    
        // Read point-to-point topology, and install routing arbiters
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
        ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
    
        // Schedule pings
        PingmeshScheduler pingmeshScheduler(basicSimulation, topology); // Requires pingmesh_interval_ns to be present in the configuration
        pingmeshScheduler.Schedule();
    
        // Run simulation
        basicSimulation->Run();
    
        // Write results
        pingmeshScheduler.WriteResults();
    
        // Finalize the simulation
        basicSimulation->Finalize();
    
        return 0;
    
    }

    ```

4. Run it by executing in your ns-3 folder:

   ```
   ./waf --run="main_pingmesh --run_dir='/your/path/to/example_run'"
   ```
   
   ... or if you also want to save the console output:
   
   ```
   mkdir -p /your/path/to/example_run/logs_ns3
   ./waf --run="main_pingmesh --run_dir='/your/path/to/example_run'" 2>&1 | tee /your/path/to/example_run/logs_ns3/console.txt
   ```
   
5. You can see the result by viewing `/your/path/to/example_run/logs_ns3/pingmesh.txt`, which should have in it:

   ```
    Source    Target    Mean latency there    Mean latency back     Min. RTT        Mean RTT        Max. RTT        Smp.std. RTT    Reply arrival
    1         2         0.03 ms               0.03 ms               0.05 ms         0.05 ms         0.05 ms         0.00 ms         100/100 (100%)
    1         3         0.03 ms               0.03 ms               0.05 ms         0.05 ms         0.05 ms         0.00 ms         100/100 (100%)
    2         1         0.03 ms               0.03 ms               0.05 ms         0.05 ms         0.05 ms         0.00 ms         100/100 (100%)
    2         3         0.03 ms               0.03 ms               0.06 ms         0.06 ms         0.06 ms         0.00 ms         100/100 (100%)
    3         1         0.03 ms               0.03 ms               0.06 ms         0.06 ms         0.06 ms         0.00 ms         100/100 (100%)
    3         2         0.03 ms               0.03 ms               0.05 ms         0.05 ms         0.05 ms         0.00 ms         100/100 (100%)
   ```

## Pingmesh scheduler

You MUST set the following key in `config_ns3.properties`:

* `pingmesh_interval_ns` : Interval to send a ping (ns)

The following are OPTIONAL:

* `pingmesh_endpoint_pairs` : Endpoint directed pingmesh pairs (either `all` (default) or e.g., `set(0-1, 5-6)` to only have pinging from 0 to 1 and from 5 to 6 (directed pairs))

**The pingmesh log files**

There are two log files generated by the run in the `logs_ns3` folder within the run folder:

* `pingmesh.txt` : Pingmesh results in a human readable table.
* `pingmesh.csv` : Pingmesh results in CSV format for processing with each line:

   ```
   from_node_id,to_node_id,i,send_request_timestamp,reply_timestamp,receive_reply_timestamp,latency_to_there_ns,latency_from_there_ns,rtt_ns,[YES/LOST]
   ```
  
  (with `YES` = ping completed successfully, `LOST` = ping reply did not arrive (either it got lost, or the simulation ended before it could arrive))
