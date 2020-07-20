# Point-to-point topology

A point-to-point topology is aimed to provide a quick way to generate a network topology with point-to-point links connecting nodes.

It encompasses the following files:

* `model/core/topology.h` - Base topology class
* `model/core/ptop-topology.cc/h` -- Point-to-point topology class


## Getting started

1. Add the following to the `config_ns3.properties` in your run folder:

   ```
   topology_filename="topology.properties"
   topology_link_data_rate_megabit_per_s=100.0
   topology_link_delay_ns=10000
   topology_max_queue_size_pkt=100
   topology_disable_traffic_control_endpoint_tors_xor_servers=true
   topology_disable_traffic_control_non_endpoint_switches=true
   ```

2. Add the following topology file `topology.properties` to your run folder for a 3-ToR:

   ```
   num_nodes=4
   num_undirected_edges=3
   switches=set(0)
   switches_which_are_tors=set(0)
   servers=set(1,2,3)
   undirected_edges=set(0-1,0-2,0-3)
   ```

3. In your code, import the point-to-point topology:

   ```
   #include "ns3/ptop-topology.h"
   // Possibly also: 
   // #include "ns3/arbiter-ecmp-helper.h"
   // #include "ns3/ipv4-arbiter-routing-helper.h"
   ```

4. Before the start of the simulation run, in your code add to create the topology:

    ```c++
    // Read point-to-point topology, and install routing arbiters
    Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());
   
   // Install some type of routing, for example:
    ArbiterEcmpHelper::InstallArbiters(basicSimulation, topology);
   
   // You can also use the traditional one, but it is very inefficient,
   // as it does linear look-up, which works poor with many sub-networks
   // which cannot be aggregated, as is the case for point-to-point topology. 
   
   // Ipv4GlobalRoutingHelper::PopulateRoutingTables();
   // You should have also given the Ipv4GlobalRoutingHelper() before.
    ```
   
5. From then on you can use it. For example to get the nodes:

    ```c++
    NodeContainer nodes = topology->GetNodes(); // Note: passed by reference
    ```


## Configuration properties

If one uses the default point-to-point topology, the following properties MUST also be defined in `config_ns3.properties`:
* `topology_filename` : Topology filename (relative to run folder)
* `topology_link_data_rate_megabit_per_s` : Data rate set for all links (Mbit/s)
* `topology_link_delay_ns` : Propagation delay set for all links (ns)
* `topology_max_queue_size_pkt` : Maximum queue size for all links (no. of packets)
* `topology_disable_traffic_control_endpoint_tors_xor_servers` : Whether to disable the traffic control queueing discipline at the endpoint nodes (if there are servers, servers, else those are the ToRs) (boolean: true/false)
* `topology_disable_traffic_control_non_endpoint_switches` : Whether to disable the traffic control queueing discipline at non-endpoint nodes (if there are servers, all switches incl. ToRs, else all switches excl. ToRs) (boolean: true/false)


## Topology file: topology.properties

The topological layout of the network. The format is best described by example:

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

All six properties must be present. Empty sets are permitted. Empty lines and lines starting with a comment sign (#) are permitted. Besides it just defining a graph, the following rules apply:

* If there are servers defined, they can only have edges to a ToR.
* There is only a semantic difference between switches, switches which are ToRs and servers. If there are servers, only servers should be valid endpoints for applications. If there are no servers, ToRs should be valid endpoints instead.
