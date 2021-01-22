# Application: UDP burst

The UDP burst application is a simple type of application.
It schedules bursts, meaning "send from A to B at a rate of
X Mbit/s at time T for duration D". It saves the results of
the bursts into useful file formats.

It encompasses the following files:

* **UdpBurstClient:** `model/apps/udp-burst-client.cc/h` 

  Client which sends the burst uni-directional to a UDP burst server.
  
* **UdpBurstServer:** `model/apps/udp-burst-server.cc/h` 

  Server which receives incoming bursts and logs.
  
* **UdpBurstInfo:** `model/apps/udp-burst-info.cc/h`

  Information to start a UDP burst.
  
* **IdSeqHeader:** `model/apps/id-seq-header.cc/h`

  Header put into the UDP burst packets to keep track of which burst
  it belongs to and which packet in the sequence it is.
  
* **UdpBurstHelper:** `helper/apps/udp-burst-helper.cc/h`

  Helper to install UDP burst clients and servers.
  
* **UdpBurstScheduler:** `helper/apps/udp-burst-scheduler.cc/h`
  
  Reads in a schedule of bursts. It installs burst applications on
  all relevant nodes. Once the run is over, it can retrieve the UDP
  burst application results and write them to file. 
  
* **UdpBurstScheduleReader:** `helper/apps/udp-burst-schedule-reader.cc/h`
  
  Reader for the UDP burst schedule.

You can use the application(s) separately, or make use of the UDP burst
scheduler (which is recommended).


## Getting started: UDP burst scheduler

1. Add the following to the `config_ns3.properties` in your run folder (for 2
   UDP bursts that need to be tracked):

   ```
   enable_udp_burst_scheduler=true
   udp_burst_schedule_filename="udp_burst_schedule.csv"
   udp_burst_enable_logging_for_udp_burst_ids=set(0,1)
   ```

2. Add the following schedule file `udp_burst_schedule.csv` to your run folder
   (two bursts, 50 Mbit/s each, from 0s to 5s, one 1 -> 2 and the other 2 -> 1):

   ```
   0,1,2,50,0,5000000000,,
   1,2,1,50,0,5000000000,,
   ```

3. In your code, import the UDP burst scheduler:

   ```c++
   #include "ns3/udp-burst-scheduler.h"
   ```

3. Before the start of the simulation run, in your code add:

    ```c++
    // Schedule UDP bursts
    UdpBurstScheduler udpBurstScheduler(basicSimulation, topology); // Requires enable_udp_burst_scheduler=true
    ```
   
4. After the run, in your code add:

    ```c++
    // Write UDP burst results
    udpBurstScheduler.WriteResults();
    ```

5. After the run, you should have the UDP burst log files in the `logs_ns3` of your run folder.


## Getting started: directly installing applications

1. In your code, import the UDP burst helper:

   ```c++
   #include "ns3/udp-burst-helper.h"
   ```
   
2. Before the start of the simulation run, in your code add:

   ```c++
   // Setup a UDP burst server on node 23
   UdpBurstServerHelper burstServerHelper(
           InetSocketAddress(topology->GetNodes().Get(23)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 1029),
           m_basicSimulation->GetLogsDir()
   );
   ApplicationContainer udpBurstServerApp = burstServerHelper.Install(topology->GetNodes().Get(23));
   udpBurstServerApp.Start(NanoSeconds(0));
   
   // Register a burst with ID 0 expected to be received on the server
   udpBurstServerApp.Get(0)->GetObject<UdpBurstServer>()->RegisterIncomingBurst(0, true);
   
   // Start UDP burst with ID 0 
   // From node 23 to 6 it sends at 15 Mbit/s for 700ms, starting at t=1 microsecond
   UdpBurstClientHelper burstClientHelper(
            InetSocketAddress(topology->GetNodes().Get(6)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 0),
            InetSocketAddress(topology->GetNodes().Get(23)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 1029),
            0,
            15.0,
            NanoSeconds(700000000),
            "",
            true,
            m_basicSimulation->GetLogsDir()
   );
   ApplicationContainer udpBurstClientApp = burstClientHelper.Install(topology->GetNodes().Get(23));
   udpBurstClientApp.Start(NanoSeconds(1000));
   ```

3. After the run, in your code add:

   ```c++
   // Each burst you get the number of sent out packets
   uint64_t sent_packets = udpBurstClientApp.Get(0)->GetSent();
   
   // Each burst you get the number of received packets
   std::vector<std::tuple<int64_t, uint64_t>> incoming_bursts = udpBurstServerApp.Get(0)->GetIncomingBurstsInformation();
   ```


## UDP burst scheduler configuration

You MUST set the following keys in `config_ns3.properties`:

* `enable_udp_burst_scheduler`
  - **Description:** true iff enable the UDP flow scheduler
  - **Value type:** boolean: `true` or `false`
* `udp_burst_schedule_filename`
  - **Description:** schedule filename (relative to run folder)
  - **Value type:** path (string)

The following are OPTIONAL in `config_ns3.properties`:

* `udp_burst_enable_logging_for_udp_burst_ids` : 
  - **Description:** for which bursts to enable detailed logging (sent/receive timestamps)
  - **Value types:** 
    - `all` to enable for all bursts
    - Set of UDP burst identifiers, i.e., `set(a, b, ...)` (default: `set()`)
  - **Example:**
    - `udp_burst_enable_logging_for_udp_burst_ids=all` to log for all UDP bursts
    - `udp_burst_enable_logging_for_udp_burst_ids=set(2, 8)` to log for UDP bursts 2 and 8
    
## UDP burst schedule format (input)

UDP burst start schedule. 

Each line defines a burst as follows:

```
[udp burst id],[from node id],[to node id],[target rate (Mbit/s)],[start time (ns since epoch)],[duration (ns)],[additional parameters],[metadata]
```

Notes: `udp burst id` must increment each line. All values except additional_parameters
and metadata are mandatory. `additional parameters` should be set if you want to configure
something special for each burst (e.g., different priority). `metadata` you can use for
identification later on in the `udp_bursts.csv/txt` logs (e.g., to indicate the workload
it was part of).

## UDP burst scheduler logs (output)

There are four log files generated by the run in the `logs_ns3` folder within the run folder:

#### `udp_bursts_{incoming, outgoing}.txt`

- **Description:** UDP burst results in a human readable table.
- **Distributed filename:** `system_[X]_udp_bursts_{incoming, outgoing}.txt`
- **Format:** None. Use the CSV equivalent for processing logs automatically.

#### `udp_bursts_outgoing.csv`

- **Description:** UDP outgoing burst results in CSV format for processing with each line:
- **Distributed filename:** `system_[X]_udp_bursts_outgoing.csv`
- **Format:** 
   ```
   [udp burst id],[from node id],[to node id],[target rate (Mbit/s)],[start time (ns since epoch)],[duration (ns)],[outgoing rate with header (Mbit/s)],[outgoing rate only payload (Mbit/s)],[packets sent],[data sent including headers (byte)],[data sent only payload (byte)],[metadata]
   ```
  
#### `udp_bursts_incoming.csv`

- **Description:** UDP incoming burst results in CSV format for processing with each line:
- **Distributed filename:** `system_[X]_udp_bursts_incoming.csv`
- **Format:** 
   ```
   [udp burst id],[from node id],[to node id],[target rate (Mbit/s)],[start time (ns since epoch)],[duration (ns)],[incoming rate with header (Mbit/s)],[incoming rate only payload (Mbit/s)],[packets received],[data received including heades (byte)],[data received only payload (byte)],[metadata]
   ```
  
Additionally, if the `udp_burst_enable_logging_for_udp_burst_ids` was set for some UDP bursts,
there will have also been generated for each of those burst:

#### `udp_burst_[id]_outgoing.csv`

- **Description:** outgoing sent packets
- **Format:** 
  ```
  [udp burst id],[sequence number],[sent (ns since epoch)]
  ```

#### `udp_burst_[id]_incoming.csv`

 - **Description:** incoming received packets
- **Format:** 
  ```
  [udp burst id],[sequence number],[received (ns since epoch)]
  ```
