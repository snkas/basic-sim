# Application: UDP ping

The UDP ping application is a simple type of application.
It schedules pings, meaning "send from A to B pings at
an interval I starting at time T for duration D and
wait afterwards W time to get the last replies". 
It saves the results of the pings into useful file formats.

It encompasses the following files:

* **UdpPingClient:** `model/apps/udp-ping-client.cc/h`

  Sends out UDP pings and receives replies.
  
* **UdpPingServer:** `model/apps/udp-ping-server.cc/h`

  Receives UDP pings, adds a received timestamp, and pings back.
  
* **UdpPingHelper:** `helper/apps/udp-ping-helper.cc/h`

  Helpers to install UDP ping servers and clients.
  
* **UdpPingScheduler:** `helper/apps/udp-ping-scheduler.cc/h`

  Schedules the pings to start. Once the run is over, it can write the results to file.
  
* **UdpPingScheduleReader:** `helper/apps/udp-ping-schedule-reader.cc/h`

  Reader for the UDP ping schedule.

You can use the application(s) separately, or make use of the UDP ping scheduler
(which is recommended).


## Getting started: UDP ping scheduler

1. Add the following to the `config_ns3.properties` in your run folder:

   ```
   enable_udp_ping_scheduler=true
   udp_ping_schedule_filename="udp_ping_schedule.csv"
   ```
   
2. Add the following schedule file `udp_ping_schedule.csv` to your run folder
   (in this example, two pings, one 1 -> 2 and the other 2 -> 1, each sending
   in a 100ms interval, starting at t=0s for duration 5s without waiting afterwards):

   ```
   0,1,2,100000000,0,5000000000,0,,
   1,2,1,100000000,0,5000000000,0,,
   ```

3. In your code, import the UDP ping scheduler:

   ```c++
   #include "ns3/udp-ping-scheduler.h"
   ```

4. Before the start of the simulation run, in your code add:

    ```c++
    // Schedule UDP pings
    UdpPingScheduler udpPingScheduler(basicSimulation, topology); // Requires enable_udp_ping_scheduler=true
    ```
   
5. After the run, in your code add:

    ```c++
    // Write UDP pings results
    udpPingScheduler.WriteResults();
    ```

6. After the run, you should have the UDP ping log files in the `logs_ns3`
   of your run folder.


## Getting started: directly installing applications

1. In your code, import the UDP ping helper:

   ```
   #include "ns3/udp-ping-helper.h"
   ```
   
2. Before the start of the simulation run, in your code add:

   ```c++
   // Install the reply server on node B: Ptr<Node> node_b
   UdpPingServerHelper udpPingServerHelper(1025);
   ApplicationContainer app = udpPingServerHelper.Install(node_b);
   app.Start(Seconds(0.0));
   
   // Install the client on node A: Ptr<Node> node_a
   UdpPingClientHelper source(
        InetSocketAddress(m_nodes.Get(node_b)->GetObject<Ipv4>()->GetAddress(1, 0).GetLocal(), 1025),
        0, // UDP ping ID
        NanoSeconds(100000000),  // Interval
        NanoSeconds(1000000000),  // Duration
        NanoSeconds(0),  // Wait afterwards (before closing socket; to await final replies)
        ""  // Additional parameters
   );
   ApplicationContainer app_a_to_b = source.Install(node_a);
   app_a_to_b.Start(NanoSeconds(0));
   ```

3. After the run, in your code add:

   ```c++
   // Retrieve client
   Ptr<UdpPingClient> client = app_a_to_b->GetObject<UdpPingClient>();

   // Data about this pair
   int64_t udp_ping_id = client->GetUdpPingId();
   uint32_t sent = client->GetSent();
   std::vector<int64_t> sendRequestTimestamps = client->GetSendRequestTimestamps();
   std::vector<int64_t> replyTimestamps = client->GetReplyTimestamps();
   std::vector<int64_t> receiveReplyTimestamps = client->GetReceiveReplyTimestamps();
   
   // Now do whatever you want; a timestamp is -1 if it did not arrive (yet)
   ```


## UDP ping scheduler configuration

You MUST set the following keys in `config_ns3.properties`:

* `enable_udp_ping_scheduler`
  - **Description:** true iff enable the UDP ping scheduler
  - **Value type:** boolean: `true` or `false`
* `udp_ping_schedule_filename`
  - **Description:** schedule filename (relative to run folder)
  - **Value type:** path (string)


## UDP ping schedule format (input)

UDP ping start schedule. 

Each line defines an UDP ping as follows:

```
[udp ping id],[from node id],[to node id],[interval (ns)],[start time (ns since epoch)],[duration (ns)],[wait afterwards (ns)],[additional parameters],[metadata]
```

This translates to: send from `from node id` to `to node id` a UDP ping every
`interval (ns)` starting from `start time (ns since epoch)` for a duration
of `duration (ns)`. Afterwards, wait `wait afterwards (ns)` amount of time
before closing the client socket (after which any arriving reply is not handled).

Notes: `udp burst id` must increment each line. All values except `additional parameters`
and `metadata` are mandatory. `additional parameters` should be set if you want to configure
something special for each burst (e.g., different priority). `metadata` you can use for
identification later on in the `udp_bursts.csv/txt` logs (e.g., to indicate the workload
it was part of).


## UDP ping scheduler logs (output)

There are two log files generated by the run in the `logs_ns3` folder within the run folder:

#### `udp_pings.txt`

- **Description:** UDP ping results in a human readable table.
- **Distributed filename:** `system_[X]_udp_pings.txt`
- **Format:** none. Use the CSV equivalent for processing logs automatically.

#### `udp_pings.csv`

- **Description:** UDP ping results in CSV format for processing.
- **Distributed filename:** `system_[X]_udp_pings.csv`
- **Format:** 
  ```
  [udp ping id],[i],[send request timestamp (ns since epoch)],[reply timestamp (ns since epoch)],[receive reply timestamp (ns since epoch)],[latency to there (ns)],[latency from there (ns)],[rtt (ns)],[YES/LOST]
  ```
  (with `YES` = ping completed successfully, `LOST` = ping reply did not arrive (either it got 
  lost, or the simulation ended before it could arrive). Some values are -1 if the ping got lost.)
