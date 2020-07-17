# Flows application

The flows application is the most simple type of application. It schedules flows to start from A to B at time T to transfer X amount of bytes. It saves the results of the flow completion into useful file formats.

It encompasses the following files:
* `model/apps/flow-scheduler.cc/h` - Reads in a schedule of flows and inserts events for them to start over time. Installs flow sinks on all nodes. Once the run is over, it can write the results to file.
* `model/apps/flow-send-application.cc/h` - Application which opens a TCP connection and uni-directionally sends data over it
* `model/apps/flow-sink.cc/h` - Accepts incoming flows and acknowledges incoming data, does not send data back
* `model/apps/schedule-reader.cc/h` - Schedule reader from file
* `helper/apps/flow-send-helper.cc/h` - Helper to install flow send applications
* `helper/apps/flow-sink-helper.cc/h` - Helper to install flow sink applications

You can use the application(s) separately, or make use of the flow scheduler (which is recommended).


## Getting started

1. Create a directory anywhere called `example_run`, and create three files in there:

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
   
   filename_schedule="schedule.csv"
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
   
   **schedule.csv**
   
   (encoded below are two flows:
   1 -> 3 starting at t=0ns of size 10000000 byte,
   2 -> 3 starting at t=10000ns of size 7000000 byte)
   
   ```
   0,1,3,10000000,0,,
   1,2,3,7000000,10000,,
   ```

2. Into your ns-3 `scratch/` folder create a file named `main_flows.cc`

3. The following is an example code for `scratch/main_flows.cc`:

    **main_flows.cc**
    
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
    #include "ns3/flow-scheduler.h"
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
    
        // Schedule flows
        FlowScheduler flowScheduler(basicSimulation, topology); // Requires filename_schedule to be present in the configuration
        flowScheduler.Schedule();
    
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
   ./waf --run="main_flows --run_dir='/your/path/to/example_run'"
   ```
   
   ... or if you also want to save the console output:
   
   ```
   mkdir -p /your/path/to/example_run/logs_ns3
   ./waf --run="main_flows --run_dir='/your/path/to/example_run'" 2>&1 | tee /your/path/to/example_run/logs_ns3/console.txt
   ```
   
5. You can see the result by viewing `/your/path/to/example_run/logs_ns3/flows.txt`, which should have in it:

   ```
   Flow ID     Source    Target    Size            Start time (ns)   End time (ns)     Duration        Sent            Progress     Avg. rate       Finished?     Metadata
   0           1         3         80.00 Mbit      0                 1310034712        1310.03 ms      80.00 Mbit      100.0%       61.1 Mbit/s     YES           
   1           2         3         56.00 Mbit      10000             1420569910        1420.56 ms      56.00 Mbit      100.0%       39.4 Mbit/s     YES           
   ```

## Flow scheduler

You MUST set the following key in `config_ns3.properties`:

* `filename_schedule` : Schedule filename (relative to run folder) (path/to/schedule.csv)

The following are OPTIONAL in `config_ns3.properties`:

* `enable_flow_logging_to_file_for_flow_ids` : Set of flow identifiers for which you want logging to file for progress, cwnd and RTT (located at `logs_dir/flow-[id]-{progress, cwnd, rtt}.txt`). Example value: `set(0, 1`) to log for flows 0 and 1. The file format is: `flow_id,now_in_ns,[progress_byte/cwnd_byte/rtt_ns])`.

**schedule.csv**

Flow arrival schedule. 

Each line defines a flow as follows:

```
flow_id,from_node_id,to_node_id,size_byte,start_time_ns,additional_parameters,metadata
```

Notes: flow_id must increment each line. All values except additional_parameters and metadata are mandatory. `additional_parameters` should be set if you want to configure something special for each flow in main.cc (e.g., different transport protocol). `metadata` you can use for identification later on in the flows.csv/txt logs (e.g., to indicate the workload or coflow it was part of).

**The flow log files**

There are two log files generated by the run in the `logs_ns3` folder within the run folder:

* `flows.txt` : Flow results in a visually appealing table.
* `flows.csv` : Flow results in CSV format for processing with each line:

   ```
   flow_id,from_node_id,to_node_id,size_byte,start_time_ns,end_time_ns,duration_ns,amount_sent_byte,[finished: YES/CONN_FAIL/NO_BAD_CLOSE/NO_ERR_CLOSE/NO_ONGOING],metadata
   ```

   (with `YES` = all data was sent and acknowledged fully and there was a normal socket close, `NO_CONN_FAIL` = connection failed (happens only in a very rare set of nearly impossible-to-reach state, typically `NO_BAD_CLOSE` is the outcome when something funky went down with the 3-way handshake), `NO_BAD_CLOSE` = socket closed normally but not all data was transferred (e.g., due to connection timeout), `NO_ERR_CLOSE` = socket error closed, `NO_ONGOING` = socket is still sending/receiving and is not yet closed)
