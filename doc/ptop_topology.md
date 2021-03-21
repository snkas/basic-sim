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
  - **Description:** topology filename (relative to run folder)
  - **Value type:** path (string)

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
link_net_device_receive_error_model=iid_uniform_random_pkt(0.0)
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

* **Description:** number of nodes

* **Value type:** positive integer
  
#### `num_undirected_edges`

* **Description:** number of undirected edges (each undirected 
  edge will be expressed into two links). This must be equal to the 
  amount of entries in the `undirected_edges` property.
  
* **Value type:** positive integer

#### `switches`

* **Description:** all node identifiers of switches

* **Value type:** set of node identifiers, expressed as `set(a, b, ...)` 

* **Examples:**
  - `set(1, 2, 5)` means node 1, 2 and 5 are switches
  - `set(2)` means only node 2 is a switch
  
#### `switches_which_are_tors`

* **Description:** all node identifiers of switches which are also ToRs

* **Value type:** set of node identifiers, expressed as `set(a, b, ...)` 

* **Examples:**
  - `set(1, 2)` means node 1 and2 are switches which are also ToRs
  - `set(2)` means only node 2 is a switch which is also a ToR
  
#### `servers`

* **Description:** all node identifiers which are servers

* **Value type:** set of node identifiers, expressed as `set(a, b, ...)`
  
* **Examples:**
  - `set(0, 3, 4)` means node 0, 3, and 4 are servers
  
#### `undirected_edges`

* **Description:** all undirected edges

* **Value type:** set of undirected edges, expressed as `set(a-b, b-c, ...)` 
    
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

* **Description:** propagation delay set for undirected edges (= two links) (ns).

* **Value type:** 
  - Single global value as a positive integer (ns) applied to all links
  - Map of all undirected edges (= two links) to their respective delays as positive integers (ns),
    i.e. `map(a-b: [delay in ns], b-c: [delay in ns], ...)`

* **Examples:**
  - `link_channel_delay_ns=10000` for 10 microseconds propagation delay
  - `link_channel_delay_ns=1000000000` for 1 second propagation delay
  - `link_channel_delay_ns=map(0-1: 10000, 1-2: 30000)`
   
#### `link_net_device_data_rate_megabit_per_s` 
  
* **Description:** data rate set for the sending network device 
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
  
  - `fifo(<integer>p)` or `fifo(<integer>B)` for first-in-first-out (= drop-tail)
  
  - `pfifo_fast(<integer>p)` or `pfifo_fast(<integer>B)` for priority first-in-first-out.
    It uses makes use of three bands. It uses the IP TOS field to map to a band.
    The band it is assigned to is based on bits 3-6 (with 0 the most significant) of the IP TOS field.
    
    ```
    Bits 3-6    Priority                pfifo_fast band
    0 to 3	    0 (Best Effort)         1
    4 to 7	    2 (Bulk)                2
    8 to 11	    6 (Interactive)         0
    12 to 15    4 (Interactive Bulk)    1
    ```
    (sources: https://www.nsnam.org/docs/models/html/sockets-api.html 
    and https://www.nsnam.org/docs/models/html/pfifo-fast.html)
    
    As a concrete example, let's say we set TOS to 55:
    ```
    55 = 0b00110111
    
    ... the least significant 2 bits are set to zero for ECN (for a stream socket):
    0b00110100
    
    ... of this, take bits 3-6, which is: 
    
    0b00110100
         ^^^^
    
    ... resulting in:
    0b00010100
    
    ... and then shift one to the right (to get in range 0-15):
    0b00001010
    
    ... which equals: 10
    ... which (from the table) corresponds to band: 0 (highest priority)
    ```
  
  - `fq_codel(interval_ns; target_ns; max_queue_size)`
     for an fq_codel with a particular interval, target and maximum queue size (only in packets, e.g., "30p").
  
  - `simple_red(ecn/drop; mean_pkt_size_byte; queue_weight; min_th_pkt; max_th_pkt; max_size; max_p; wait/no_wait; gentle/not_gentle)` 
    for a simple RED queueing discipline. It does an increasing linear probability
    between the minimum (having p_b = 0.0) and maximum threshold (p_b = MaxP).
    The action taken if a packet is probabilistically determined to be marked by
    RED can be set to either mark ECN (ecn) or drop the packet (drop).
    
    For an explanation of the behavior of RED, see the source code in ns3:
    `src/traffic-control/model/red-queue-disc.h/cc` and the original
    RED paper "Random Early Detection Gateways for Congestion Avoidance" by 
    Sally Floyd and Van Jacobson. Additionally helpful webpage:
    https://sites.google.com/a/ncsu.edu/tail-drop-vs-red/plan-of-work/red-algorithm
    
    The parameters:
    - `ecn/drop`: the action; set to `ecn` or to `drop`
    - `mean_pkt_size_byte`: mean packet size in byte
    - `queue_weight`: the queue weight for EWMA estimate of average queue size (in range (0, 1]).
      Instantaneous queue size only is when the queue weight is 1.
    - `min_th`: RED minimum threshold in packets
    - `max_th`: RED maximum threshold in packets
    - `max_size`: maximum queue size in either packets (`<number>p`) or byte (`<number>B`)
    - `max_p`: p_b probability at the maximum threshold (in range (0, 1])
    - `wait/no_wait`: whether to wait between dropping packets
    - `gentle/not_gentle`:  if gentle, then the p_b probability
      increases linearly from `max_p` to 1.0 between `max_th` and `2 * max_th`.
      If not_gentle, the p_b probability at `max_th` will be 1.0 immediately 
      (effectively, `max_th` will is a lower `max_size`) -- this is called not
      gentle because it goes from `max_p` to 1.0 immediately from `max_th` to `max_th + 1`
      queue size.

* **Examples:**

  - `link_interface_traffic_control_qdisc=none` for no traffic-control at any link
  
  - `link_interface_traffic_control_qdisc=fifo(100p)` for a 100 packets drop-tail for all links
  
  - `link_interface_traffic_control_qdisc=simple_red(ecn; 1500; 1.0; 50; 80; 300p; 0.2; wait; gentle)` 
    for a RED queue which marks ECN with min. threshold of 50 and a max. threshold of 80.
    It waits between ECN marking. Between 50 and 80 probability goes from 0.0 to 0.3.
    Between 80 and 160 packets because it is gentle the probability goes from 0.2 to 1.0.
    
  - `link_interface_traffic_control_qdisc=simple_red(drop; 1500; 1.0; 20; 60; 200p; 0.3; wait; gentle)` 
    for a RED queue which marks ECN with min. threshold of 20 and a max. threshold of 60.
    The maximum queue size of 200p will not be reached (as 60p * 2 = 120p with gentle on).
    It waits between dropping. Between 20 and 60 probability goes from 0.0 to 0.3.
    Between 60 and 120 packets because it is gentle the probability goes from 0.3 to 1.0.
    
  - `link_interface_traffic_control_qdisc=map(0->1: none, 1->0: fifo(100000B), 
    1->2: fifo(80p), 2->1: fq_codel(30000; 2000; 10240p))`
