# Application: TCP flow

The TCP flow application is the most simple type of application.
It schedules flows to start from A to B at time T to transfer X
amount of bytes. It saves the results of the flow completion into
useful file formats.

It encompasses the following files:

* **TcpFlowClient:** `model/apps/tcp-flow-client.cc/h` 

  Application which opens a TCP connection and uni-directionally sends data over it.

* **TcpFlowServer:** `model/apps/tcp-flow-server.cc/h`

  Accepts incoming flows and acknowledges incoming data, does not send data back.

* **TcpFlowClientHelper:** `helper/apps/tcp-flow-helper.cc/h`

  Helper to install flow client ("send") applications.
  
* **TcpFlowServerHelper:** `helper/apps/tcp-flow-helper.cc/h`

  Helper to install flow server ("sink") applications.
  
* **TcpFlowScheduler:** `helper/apps/tcp-flow-scheduler.cc/h`

  Reads in a schedule of flows and inserts events for them to start over time. 
  Installs flow servers on all endpoints. Once the run is over, it can write the results to file.

* **TcpFlowScheduleReader:** `helper/apps/tcp-flow-schedule-reader.cc/h`

  Schedule reader for the TCP flow schedule from file.

You can use the application(s) separately, or make use of the scheduler (which is recommended).


## Getting started: flow scheduler

1. Add the following to the `config_ns3.properties` in your run folder
  (for 2 flows that need to be tracked):

   ```
   enable_tcp_flow_scheduler=true
   tcp_flow_schedule_filename="tcp_flow_schedule.csv"
   tcp_flow_enable_logging_for_tcp_flow_ids=set(0,1)
   ```

2. Add the following schedule file `tcp_flow_schedule.csv` to your run folder
  (three flows from 0 to 1, resp. of size 10/3/34 KB and starting at T=0/10000/30000ns):

   ```
   0,0,1,10000,0,,
   1,0,1,3000,10000,,
   2,0,1,34000,30000,,
   ```

3. In your code, import the TCP flow scheduler:

   ```c++
   #include "ns3/tcp-flow-scheduler.h"
   ```

3. Before the start of the simulation run, in your code add:

    ```c++
    // Schedule flows
    TcpFlowScheduler tcpFlowScheduler(basicSimulation, topology); // Requires enable_tcp_flow_scheduler=true
    ```
   
4. After the run, in your code add:

    ```c++
    // Write result
    tcpFlowScheduler.WriteResults();
    ```

5. After the run, you should have the TCP flow log files in the `logs_ns3` of your run folder.


## Getting started: directly installing applications

1. In your code, import the TCP flow client and server helper:

   ```c++
   #include "ns3/tcp-flow-helper.h"
   ```
   
2. Before the start of the simulation run, in your code add:

   ```c++
   // Install a flow server on node B: Ptr<Node> node_b
   TcpFlowServerHelper tcpFlowServerHelper(
       InetSocketAddress(node_a->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1024)
   );
   ApplicationContainer serverApp = tcpFlowServerHelper.Install(node_b);
   serverApp.Start(Seconds(0.0));
   
   // Install the client on node A: Ptr<Node> node_a
   // TCP flow ID 0 from A to B of size 1000000000 byte with detailed logging enabled
   TcpFlowClientHelper tcpFlowClientHelper(
            InetSocketAddress(node_a->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 0),
            InetSocketAddress(node_b->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1024),
            0,
            1000000000,
            "", 
            true, // Enable detailed tracking of the TCP connection (e.g., progress, rtt, rto, cwnd, ...)
            m_basicSimulation->GetLogsDir() // Log directory where the tcp_flow_0_{progress, rtt, rto, cwnd, ...}.csv are written   
   );
   ApplicationContainer clientApp = tcpFlowClientHelper.Install(node_a);
   clientApp.Start(NanoSeconds(0)); // Flow start time (ns since epoch)
   ```

3. After the run, in your code add:

   ```c++
   // Retrieve client
   Ptr<TcpFlowClient> tcpFlowClient = app_flow_0->GetObject<TcpFlowClient>();

   // Data about this flow
   bool is_completed = tcpFlowClient->IsCompleted();
   bool is_conn_failed = tcpFlowClient->IsConnFailed();
   bool is_closed_err = tcpFlowClient->IsClosedByError();
   bool is_closed_normal = tcpFlowClient->IsClosedNormally();
   int64_t sent_byte = tcpFlowClient->GetAckedBytes();
   int64_t fct_ns;
   if (is_completed) {
       fct_ns = tcpFlowClient->GetCompletionTimeNs() - entry.start_time_ns;
   } else {
       fct_ns = m_simulation_end_time_ns - entry.start_time_ns;
   }
   std::string finished_state;
   if (is_completed) {
       finished_state = "YES";
   } else if (is_conn_failed) {
       finished_state = "NO_CONN_FAIL";
   } else if (is_closed_normal) {
       finished_state = "NO_BAD_CLOSE";
   } else if (is_closed_err) {
       finished_state = "NO_ERR_CLOSE";
   } else {
       finished_state = "NO_ONGOING";
   }
   
   // ... now do whatever you want with this information
   ```


## TCP flow schedule configuration

You MUST set the following keys in `config_ns3.properties`:

* `enable_tcp_flow_scheduler`
  - **Description:** true iff enable the TCP flow scheduler
  - **Value type:** boolean: `true` or `false`
* `tcp_flow_schedule_filename`
  - **Description:** schedule filename (relative to run folder)
  - **Value type:** path (string)

The following are OPTIONAL in `config_ns3.properties`:

* `tcp_flow_enable_logging_for_tcp_flow_ids` : 
  - **Description:** for which flows to enable detailed logging (e.g., progress, rtt, rto, cwnd, ...)
  - **Value types:** 
    - `all` to enable for all flows
    - Set of flow identifiers, i.e., `set(a, b, ...)` (default: `set()`)
  - **Example:**
    - `tcp_flow_enable_logging_for_tcp_flow_id=all` to log for all flows defined.
    - `tcp_flow_enable_logging_for_tcp_flow_id=set(0, 1)` to log for flows 0 and 1.


## TCP flow schedule format (input)

TCP flow arrival schedule. 

Each line defines a flow as follows:

```
[tcp flow id],[from node id],[to node id],[size (byte)],[start time (ns since epoch)],[additional parameters],[metadata]
```

Notes: `tcp flow id` must increment each line. Start time has to be weakly increasing.
All values except additional_parameters and metadata are mandatory.
`additional parameters` should be set if you want to configure something special
for each flow (e.g., different transport protocol, priority). `metadata` you can
use for identification later on in the `tcp_flows.csv/txt` logs (e.g., to indicate
the workload or coflow it was part of).

## TCP flow scheduler logs (output)

There are two log files generated by the run in the `logs_ns3` folder within the run folder:

#### `tcp_flows.txt`

- **Description:** TCP flow results in a human readable table.
- **Distributed filename:** `system_[X]_tcp_flows.txt`
- **Format:** none. Use the CSV equivalent for processing logs automatically.

#### `tcp_flows.csv`

- **Description:** TCP flow results in CSV format for processing with each line:
- **Distributed filename:** `system_[X]_tcp_flows.csv`
- **Format:** 
   ```
   [tcp flow id],[from node id],[to node id],[size (byte)],[start time (ns since epoch)],[end time (ns since epoch)],[duration (ns)],[amount sent (byte)],[finished],[metadata]
   ```
  
  Finished (`[finished]`) can have the following values:
  
  * `YES`
  
    All data was sent and acknowledged fully and there was a normal socket close.
    
  * `NO_CONN_FAIL`
  
    Connection could not be established (e.g., the handshake failed).
    This can be caused by:
    (a) Sending of SYN / SYN+ACK reaching too many timeouts resulting in no more retries left.
    (b) It can also be caused by the routing protocol unable to find a route.
  
  * `NO_BAD_CLOSE`
  
    Socket was closed prematurely meaning it was closed normally but not all data was transferred.
    This should in general not occur.
    This can be caused by:
    (a) Something external calls Close() on the TcpSocket directly when not
        everything has yet been put into the send buffer AND the send buffer occupancy reaches 0.
    (b) In rare cases, it is also possible to be the outcome of an unsuccessful handshake.
  
  * `NO_ERR_CLOSE`
  
    Socket closed because of an error.
    This can be caused:
    (a) A RST was received.
    (b) It reached too many timeouts (no more (data) retries left).
  
  * `NO_ONGOING`
  
    Socket is still sending/receiving and is not yet closed because
    not all data has been transferred yet.

Additionally, if the `tcp_flow_enable_logging_for_tcp_flow_ids` was set for some TCP flows,
there will have also been generated for those flows:

#### `tcp_flow_[id]_progress.csv`

- **Description:** progress (in byte)
- **Format:** 
  ```
  [tcp flow id],[now (ns since epoch)],[progress (byte)]
  ```
  
#### `tcp_flow_[id]_rto.csv`

- **Description:** retransmission timeout (RTO) (in ns)
- **Format:** 
  ```
  [tcp flow id],[now (ns since epoch)],[rto (ns)]
  ```

   
#### `tcp_flow_[id]_cwnd.csv`

- **Description:** congestion window (in byte)
- **Format:** 
   ```
   [tcp flow id],[now (ns since epoch)],[cwnd (byte)]
   ```
  
#### `tcp_flow_[id]_cwnd_inflated.csv`

- **Description:** inflated congestion window (in byte)
- **Format:** 
   ```
   [tcp flow id],[now (ns since epoch)],[cwnd inflated (byte)]
   ```
  
#### `tcp_flow_[id]_ssthresh.csv`

- **Description:** slow-start threshold (in byte)
- **Format:** 
   ```
   [tcp flow id],[now (ns since epoch)],[ssthresh (byte)]
   ```
  
#### `tcp_flow_[id]_inflight.csv`

- **Description:** in-flight (in byte)
- **Format:** 
   ```
   [tcp flow id],[now (ns since epoch)],[in-flight (byte)]
   ```

#### `tcp_flow_[id]_state.csv`

- **Description:** state (CLOSED, LISTEN, SYN_SENT, SYN_RCVD, 
  ESTABLISHED, CLOSE_WAIT, LAST_ACK, FIN_WAIT_1, FIN_WAIT_2, 
  CLOSING, TIME_WAIT, LAST_STATE)
- **Format:** 
   ```
   [tcp flow id],[now (ns since epoch)],[state (string)]
   ```
  
#### `tcp_flow_[id]_cong_state.csv`

- **Description:** congestion state (CA_OPEN, CA_DISORDER, CA_CWR, 
  CA_RECOVERY, CA_LOSS, CA_LAST_STATE)
- **Format:** 
   ```
   [tcp flow id],[now (ns since epoch)],[congestion state (string)]
   ```
