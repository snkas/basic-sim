# Basic simulation ns-3 module

[![Build Status](https://travis-ci.org/snkas/basic-sim.svg?branch=master)](https://travis-ci.org/snkas/basic-sim) [![codecov](https://codecov.io/gh/snkas/basic-sim/branch/master/graph/badge.svg)](https://codecov.io/gh/snkas/basic-sim)

This module is used to make the simulation of ns-3 (data center) networks a bit easier.

**This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details (in ./LICENSE).**

This module in a nutshell:

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


## Run folder

The run folder must contain the input of a simulation. It is the basis of every basic simulation.

**config_ns3.properties**

General properties of the simulation. The following MUST always be defined:

* `filename_topology` : Topology filename (relative to run folder)
* `simulation_end_time_ns` : How long to run the simulation in simulation time (ns)
* `simulation_seed` : If there is randomness present in the simulation, this guarantees reproducibility (exactly the same outcome) if the seed is the same
* `link_data_rate_megabit_per_s` : Data rate set for all links (Mbit/s)
* `link_delay_ns` : Propagation delay set for all links (ns)
* `link_max_queue_size_pkts` : Maximum queue size for all links (no. of packets)
* `disable_qdisc_endpoint_tors_xor_servers` : Whether to disable the traffic control queueing discipline at the endpoint nodes (if there are servers, servers, else those are the ToRs) (boolean: true/false)
* `disable_qdisc_non_endpoint_switches` : Whether to disable the traffic control queueing discipline at non-endpoint nodes (if there are servers, all switches incl. ToRs, else all switches excl. ToRs) (boolean: true/false)

**topology.properties**

The topological layout of the network. Please see the examples to understand each property. Besides it just defining a graph, the following rules apply:

* If there are servers defined, they can only have edges to a ToR.
* There is only a semantic difference between switches, switches which are ToRs and servers. If there are servers, only servers should be valid endpoints for applications. If there are no servers, ToRs should be valid endpoints instead.

**The log files**

There are is always one log file guaranteed generated by the run in the `logs_ns3` folder within the run folder:

* `finished.txt` : Contains "Yes" if the run has finished, "No" if not.


## Testing

Requirements:

* Python 3.6+
* lcov: `sudo apt-get -y install lcov`

To perform the full range of testing of this module:

```
sudo apt-get update
sudo apt-get -y install lcov
cd build
bash build.sh
bash test.sh
bash example.sh
```


## Acknowledgements

Based on code written by Hussain, who did his master thesis in the NDAL group.
Refactored, extended and maintained by Simon. The ECMP routing hashing function is inspired by https://github.com/mkheirkhah/ecmp (accessed: February 20th, 2020).
