# Flows application

The flows application is the most simple type of application. It schedules flows to start from A to B at time T to transfer X amount of bytes. It saves the results of the flow completion into useful file formats.

It encompasses the following files:

* `model/apps/flow-send-application.cc/h` - Application which opens a TCP connection and uni-directionally sends data over it
* `model/apps/flow-sink.cc/h` - Accepts incoming flows and acknowledges incoming data, does not send data back
* `helper/apps/flow-send-helper.cc/h` - Helper to install flow send applications
* `helper/apps/flow-sink-helper.cc/h` - Helper to install flow sink applications
* `helper/apps/flow-scheduler.cc/h` - Reads in a schedule of flows and inserts events for them to start over time. Installs flow sinks on all nodes. Once the run is over, it can write the results to file.
* `helper/apps/schedule-reader.cc/h` - Schedule reader from file

You can use the application(s) separately, or make use of the flow scheduler (which is recommended).


## Getting started: flow scheduler

1. Add the following to the `config_ns3.properties` in your run folder (for 2 flows that need to be tracked):

   ```
   enable_flow_scheduler=true
   flow_schedule_filename="schedule.csv"
   enable_flow_logging_to_file_for_flow_ids=set(0,1)
   ```

2. Add the following schedule file `schedule.csv` to your run folder (three flows from 0 to 1, resp. of size 10/3/34 KB and starting at T=0/10000/30000ns):

   ```
   0,0,1,10000,0,,
   1,0,1,3000,10000,,
   2,0,1,34000,30000,,
   ```

3. In your code, import the pingmesh scheduler:

   ```
   #include "ns3/flow-scheduler.h"
   ```

3. Before the start of the simulation run, in your code add:

    ```c++
    // Schedule flows
    FlowScheduler flowScheduler(basicSimulation, topology); // Requires enable_flow_scheduler=true
    ```
   
4. After the run, in your code add:

    ```c++
    // Write result
    flowScheduler.WriteResults();
    ```

5. After the run, you should have the flows log files in the `logs_ns3` of your run folder.


## Getting started: directly installing applications

1. In your code, import the UDP RTT helper:

   ```
   #include "ns3/udp-rtt-helper.h"
   ```
   
2. Before the start of the simulation run, in your code add:

   ```c++
   // Install a flow sink server on node B: Ptr<Node> node_b
   FlowSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), 1024));
   ApplicationContainer app = sink.Install(node_b);
   app.Start(Seconds(0.0));
   
   // Install the client on node A: Ptr<Node> node_a
    FlowSendHelper source(
            "ns3::TcpSocketFactory",
            InetSocketAddress(node_b->GetObject<Ipv4>()->GetAddress(1,0).GetLocal(), 1024),
            1000000000, // Flow size (byte)
            0, // Flow id (must be unique!)
            true, // Enable tracking cwnd / rtt / progress
            m_basicSimulation->GetLogsDir() // Log directory where the flow_0_{cwnd, rtt, progress}.csv are written
    );
    ApplicationContainer app_flow_0 = source.Install(node_a);
    app.Start(NanoSeconds(0)); // Flow start time (ns)
   ```

3. After the run, in your code add:

   ```c++
   // Retrieve client
   Ptr<FlowSendApplication> flowSendApp = app_flow_0->GetObject<FlowSendApplication>();

   // Data about this flow
   bool is_completed = flowSendApp->IsCompleted();
   bool is_conn_failed = flowSendApp->IsConnFailed();
   bool is_closed_err = flowSendApp->IsClosedByError();
   bool is_closed_normal = flowSendApp->IsClosedNormally();
   int64_t sent_byte = flowSendApp->GetAckedBytes();
   int64_t fct_ns;
   if (is_completed) {
       fct_ns = flowSendApp->GetCompletionTimeNs() - entry.start_time_ns;
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


## Flow scheduler information

You MUST set the following keys in `config_ns3.properties`:

* `enable_flow_scheduler` : Must be set to `true`
* `flow_schedule_filename` : Schedule filename (relative to run folder) (path/to/schedule.csv)

The following are OPTIONAL in `config_ns3.properties`:

* `enable_flow_logging_to_file_for_flow_ids` : Set of flow identifiers for which you want logging to file for progress, cwnd and RTT (located at `logs_dir/flow-[id]-{progress, cwnd, rtt}.csv`). Example value: `set(0, 1`) to log for flows 0 and 1. The file format is: `flow_id,now_in_ns,[progress_byte/cwnd_byte/rtt_ns])`.

**schedule.csv**

Flow arrival schedule. 

Each line defines a flow as follows:

```
flow_id,from_node_id,to_node_id,size_byte,start_time_ns,additional_parameters,metadata
```

Notes: flow_id must increment each line. All values except additional_parameters and metadata are mandatory. `additional_parameters` should be set if you want to configure something special for each flow in main.cc (e.g., different transport protocol). `metadata` you can use for identification later on in the flows.csv/txt logs (e.g., to indicate the workload or coflow it was part of).

**The flow log files**

There are two log files generated by the run in the `logs_ns3` folder within the run folder:

* `flows.txt` : Flow results in a human readable table.
* `flows.csv` : Flow results in CSV format for processing with each line:

   ```
   flow_id,from_node_id,to_node_id,size_byte,start_time_ns,end_time_ns,duration_ns,amount_sent_byte,[finished: YES/CONN_FAIL/NO_BAD_CLOSE/NO_ERR_CLOSE/NO_ONGOING],metadata
   ```

   (with `YES` = all data was sent and acknowledged fully and there was a normal socket close, `NO_CONN_FAIL` = connection failed (happens only in a very rare set of nearly impossible-to-reach state, typically `NO_BAD_CLOSE` is the outcome when something funky went down with the 3-way handshake), `NO_BAD_CLOSE` = socket closed normally but not all data was transferred (e.g., due to connection timeout), `NO_ERR_CLOSE` = socket error closed, `NO_ONGOING` = socket is still sending/receiving and is not yet closed)
