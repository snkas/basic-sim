# Point-to-point topology

A point-to-point topology is aimed to provide a quick way to generate a network topology with point-to-point links connecting nodes.

It encompasses the following files:

* **Topology:** `model/core/topology.h` 

  Base topology class.
  
* **TopologyPtop:** `model/core/topology-ptop.cc/h`

  Point-to-point topology class.
  
* **TopologyPtopQueueSelectorDefault:** `model/core/topology-ptop-queue-selector-default.cc/h`

  Selector to parse a value into its corresponding queue for a sending
  net-device.

* **TopologyPtopReceiveErrorModelSelectorDefault:** `model/core/topology-ptop-receive-error-model-selector-default.cc/h`

  Selector to parse a value into its corresponding receive error model
  for a receiving net-device.
  
* **TopologyPtopTcQdiscSelectorDefault:** `model/core/topology-ptop-tc-qdisc-selector-default.cc/h`

  Selector to parse a value into its corresponding traffic-control queue
  discipline for a sending net-device.


## Getting started

1. Add the following to the `config_ns3.properties` in your run folder:

   ```
   topology_ptop_filename="topology.properties"
   ```

2. Add the following topology file `topology.properties` to your run folder for a 3-ToR:

   ```
   num_nodes=4
   num_undirected_edges=3
   switches=set(0)
   switches_which_are_tors=set(0)
   servers=set(1,2,3)
   undirected_edges=set(0-1,0-2,0-3)
   
   link_channel_delay_ns=10000
   link_net_device_data_rate_megabit_per_s=100.0
   link_net_device_queue=drop_tail(100p)
   link_net_device_receive_error_model=none
   link_interface_traffic_control_qdisc=disabled
   ```

3. In your code, import the point-to-point topology:

   ```
   #include "ns3/ptop-topology.h"
   // Possibly also: 
   // #include "ns3/ipv4-arbiter-routing-helper.h"
   // #include "ns3/arbiter-ecmp-helper.h"
   ```

4. Before the start of the simulation run, in your code add to create the topology:

    ```c++
    // Read point-to-point topology
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
    NodeContainer nodes = topology->GetNodes();
    ```


## Configuration properties

If one uses the default point-to-point topology, the following property MUST 
also be defined in `config_ns3.properties`:

* `topology_ptop_filename`
  - **Description:** Topology filename (relative to run folder)
  - **Value type:** Path (string)

## Topology file: ptop_topology.properties

Because the topology file can get quite big, and is independent to some extent, 
it is in a separate configuration file. This configuration file contains both the 
topological layout, and the link properties (channel delay, net-device rate, queue
and error model, and interface traffic-control queueing discipline queue).

**Example:**

```
num_nodes=4
num_undirected_edges=3
switches=set(0)
switches_which_are_tors=set(0)
servers=set(1,2,3)
undirected_edges=set(0-1,0-2,0-3)

link_channel_delay_ns=map(0-1: 10000, 0-2: 10000, 0-3: 10000)
link_net_device_data_rate_megabit_per_s=100.0
link_net_device_queue=map(0->1: drop_tail(50p), 1->0: drop_tail(100p), 0->2: drop_tail(100000B), 2->0: drop_tail(50p), 0->3: drop_tail(100p), 3->0: drop_tail(100p))
link_net_device_receive_error_model=uniform_random_pkt(0.0)
link_interface_traffic_control_qdisc=disabled
```

### Point-to-point topology properties constraints

All mandatory properties must be present. Empty sets are permitted. 
Empty lines and lines starting with a comment sign (#) are permitted. 
Besides it just defining a graph, the following rules apply:

* If there are servers defined, they can only have edges to a ToR.
* There is only a semantic difference between switches, switches which 
  are ToRs and servers. If there are servers, only servers should be 
  valid endpoints for applications. If there are no servers, ToRs should be valid endpoints instead.
  
### Point-to-point topology properties

#### `num_nodes`

* **Description:** Number of nodes

* **Value type:** positive integer
  
#### `num_undirected_edges`

* **Description:** number of undirected edges (each undirected 
  edge will be expressed into two links). This must be equal to the 
  amount of entries in the `undirected_edges` property.
  
* **Value type:** positive integer

#### `switches`

* **Description:** All node identifiers of switches

* **Value type:** Set of node identifiers, expressed as `set(a, b, ...)` 

* **Examples:**
  - `set(1, 2, 5)` means node 1, 2 and 5 are switches
  - `set(2)` means only node 2 is a switch
  
#### `switches_which_are_tors`

* **Description:** All node identifiers of switches which are also ToRs

* **Value type:** Set of node identifiers, expressed as `set(a, b, ...)` 

* **Examples:**
  - `set(1, 2)` means node 1 and2 are switches which are also ToRs
  - `set(2)` means only node 2 is a switch which is also a ToR
  
#### `servers`

* **Description:** All node identifiers which are servers

* **Value type:** Set of node identifiers, expressed as `set(a, b, ...)`
  
* **Examples:**
  - `set(0, 3, 4)` means node 0, 3, and 4 are servers
  
#### `undirected_edges`

* **Description:** All undirected edges

* **Value type:** Set of undirected edges, expressed as `set(a-b, b-c, ...)` 
    
* **Examples:**
  - `set(0-2, 2-3)` means two undirected edges, between 0 and 2, and between 2 and 3.

#### `all_nodes_are_endpoints`

* **Description:**  
  Whether to allow all nodes to be endpoints for applications or not. 
  Normally, the topology only considers servers (or in absence thereof: ToRs) 
  as valid endpoints. Setting this to `true` will have the topology return 
  all nodes as valid endpoints. This is a soft enforce, meaning that only 
  code that checks with topology explicitly will adhere to the endpoints the 
  topology claims are permissible for applications.
  
* **Value type:** boolean: `true` or `false`

#### `link_channel_delay_ns` 

* **Description:** Propagation delay set for undirected edges (= two links) (ns).

* **Value type:** 
  - Single global value as a positive integer (ns) applied to all links
  - Map of all undirected edges (= two links) to their respective delays as positive integers (ns),
    i.e. `map(a-b: [delay in ns], b-c: [delay in ns], ...)`

* **Examples:**
  - `link_channel_delay_ns=10000` for 10 microseconds propagation delay
  - `link_channel_delay_ns=1000000000` for 1 second propagation delay
  - `link_channel_delay_ns=map(0-1: 10000, 1-2: 30000)`
   
#### `link_net_device_data_rate_megabit_per_s` 
  
* **Description:** Data rate set for the sending network device 
  of this directed edge (link) (Mbit/s).
   
* **Value type:** 
  - Single global value as a positive double (Mbit/s) applied to all links
  - Map of all directed edges (= links) map to their respective rate as positive double (Mbit/s)
    i.e. `map(a->b: [rate in Mbit/s], b->a: [rate in Mbit/s], ...)`
   
* **Examples:**
  - `link_net_device_data_rate_megabit_per_s=0.6` for 0.6 Mbit/s
  - `link_net_device_data_rate_megabit_per_s=100.0` for 100 Mbit/s
  - `link_net_device_data_rate_megabit_per_s=map(0->1: 0.8, 1->0: 12.0, 1->2: 10, 2->1: 100.8)`
   
#### `link_net_device_queue`

* **Description:** queue implementation for the sending network device
  of this directed edge (link).

* **Value type:** 
  - Single global queue implementation value applied to all links
  - Map of all directed edges (= links) map to their respective queue implementation
    i.e. `map(a->b: [queue], b->a: [queue], ...)`
    
* **Possible queue types:**
  - `drop_tail(<integer>p)` for a drop-tail queue with a size in packets
  - `drop_tail(<integer>B)` for a drop-tail queue with a size in byte

* **Examples:**
  - `link_net_device_queue=drop_tail(100p)`
  - `link_net_device_queue=drop_tail(100000B)`
  - `link_net_device_queue=map(0->1: drop_tail(100p), 1->0: drop_tail(90p), 
    1->2: drop_tail(100000B), 2->1: drop_tail(100p))`
  
#### `link_net_device_receive_error_model` 

* **Description:** error model of the receiving network device
  of the directed edge (link).

* **Value type:** 
  - Single global receive error model implementation value applied to all links
  - Map of all directed edges (= links) map to their respective receive error model implementation
    i.e. `map(a->b: [error model], b->a: [error model], ...)`
    
* **Possible error model types:**
  - `none` for no random drops (if it is sent, it will arrive perfectly always)
  - `iid_uniform_random_pkt([double in [0.0, 1.0]-range])` for a i.i.d. uniform random 
    drop probability for each packet traversing the directed edge (link) 

* **Examples:**
  - `link_net_device_receive_error_model=none` for no reception errors (perfect)
  - `link_net_device_receive_error_model=iid_uniform_random_pkt(0.0001)` for a 0.01% 
    probability of a packet getting dropped when traversing every link
  - `link_net_device_receive_error_model=map(0->1: none, 1->0: iid_uniform_random_pkt(0.0001), 
    1->2: iid_uniform_random_pkt(1.0), 2->1: iid_uniform_random_pkt(0.3))`

#### `link_interface_traffic_control_qdisc` 

* **Description:** traffic control queueing discipline for the interface
  in front of the sending network device of the directed edge (link).
    
* **Value type:**
  - Single global traffic-control qdisc implementation value applied to all links
  - Map of all directed edges (= links) map to their respective traffic-control qdisc implementation
    i.e. `map(a->b: [tc qdisc], b->a: [tc qdisc], ...)`
    
* **Possible traffic control qdisc types:**
  - `default` for the ns-3 default (fq_codel with a pretty high RTT estimate)
  - `disabled` for no queueing discipline
  - `fifo(<integer>p)` or `fifo(<integer>B)` for first-in-first-out (= tail-drop)
  - `fq_codel_better_rtt` for fq_codel but with an RTT estimate based on the topology
  - `simple_red(ecn/drop; min_th; max_th; max_size)` for a simple RED queueing discipline.
    It does a simple linear probability between the minimum and maximum threshold.
    The instantaneous queue size is used as the "average queue size" (which means, no exponential
    weighted averaging is done). The action taken if a packet is probabilistically
    determined to be marked by RED can be set to either mark ECN (ecn) or drop the packet (drop).
     
    The parameters:
    - `ecn/drop`: the action; set to `ecn` or to `drop`
    - `min_th`: RED minimum threshold in packets
    - `max_th`: RED maximum threshold in packets
    - `max_size`: Maximum queue size in packets (if the action is `drop`, 
      `max_th` will effectively be a lower `max_size`)

* **Examples:**
  - `link_interface_traffic_control_qdisc=none` for no traffic-control at any link
  - `link_interface_traffic_control_qdisc=fifo(100p)` for a 100 packets tail-drop for all links
  - `link_interface_traffic_control_qdisc=simple_red(ecn; 50; 80; 100)` for a queue which has linear probability to
    mark ECN from 50 to 80, and maximum queue size of 100 packets before tail-drop
  - `link_interface_traffic_control_qdisc=map(0->1: none, 1->0: fifo(100000B), 
    1->2: fifo(80p), 2->1: fq_codel_better_rtt)`
    