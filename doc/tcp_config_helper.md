# TCP config(uration) helper

The TCP config helper can be used to set a few key TCP attributes.

It encompasses the following files:
  
* **Extra initial attribute value helpers:** `helper/core/initial-helpers.cc/h`

  A few extra useful functions to retrieve typed initial attribute values.
  
* **TcpConfigHelper:** `helper/core/tcp-config-helper.cc/h`

  TCP config helper.


## Getting started

1. In your `config_ns3.properties` add the following line:

   ```
   tcp_config=basic
   ```

2. In your code, import the TCP config helper:

   ```c++
   #include "ns3/tcp-config-helper.h"
   ```

3. Before the start of the simulation run, in your code add the following:

    ```c++
    // Configure TCP
    TcpConfigHelper::Configure(basicSimulation);
   ```
   
  
## Configuration properties

If one uses the TCP config helper, the following property MUST 
also be defined in `config_ns3.properties`:

* `tcp_config`
  - **Description:** the type of configuration to help with
  - **Value types:**
    - `default` to not change the TCP attributes at all and keep the ns-3 defaults
    - `basic` to have a TCP attributes setting close to Linux (though not exactly to include ns-3 peculiarities)
    - `custom` to set all TCP attributes manually in the `config_ns3.properties`

Below are properties that MUST be defined if the `tcp_config` property is set to `custom`:

* `tcp_clock_granularity`
  - **Description:** clock granularity
  - **Value type:** positive integer (ns)

* `tcp_init_cwnd_pkt`
  - **Description:** initial congestion window (cwnd)
  - **Value type:** positive integer (number of packets)

* `tcp_snd_buf_size_byte`
  - **Description:** send buffer size
  - **Value type:** positive integer (byte)

* `tcp_rcv_buf_size_byte`
  - **Description:** receive buffer size
  - **Value type:** positive integer (byte)

* `tcp_segment_size_byte`
  - **Description:** segment size
  - **Value type:** positive integer (byte)

* `tcp_opt_timestamp_enabled`
  - **Description:** whether to enable the Timestamp option
  - **Value type:** boolean (true/false)

* `tcp_opt_sack_enabled`
  - **Description:** whether to enable the Selective ACK (SACK) option
  - **Value type:** boolean (true/false)

* `tcp_opt_win_scaling_enabled`
  - **Description:** whether to enable window scaling
  - **Value type:** boolean (true/false)

* `tcp_opt_pacing_enabled`
  - **Description:** whether to enable pacing
  - **Value type:** boolean (true/false)

* `tcp_delayed_ack_packet_count`
  - **Description:** number of packets before sending delayed ACK
  - **Value type:** positive integer (ns)

* `tcp_no_delay`
  - **Description:** whether to enable TCP NoDelay (= disable Nagle's algorithm)
  - **Value type:** boolean (true/false)

* `tcp_max_seg_lifetime_ns`
  - **Description:** maximum segment lifetime
  - **Value type:** positive integer (ns)

* `tcp_min_rto_ns`
  - **Description:** minimum retransmission timeout (RTO)
  - **Value type:** positive integer (ns)

* `tcp_initial_rtt_estimate_ns`
  - **Description:** initial RTT estimate
  - **Value type:** positive integer (ns)

* `tcp_connection_timeout_ns`
  - **Description:** connection timeout
  - **Value type:** positive integer (ns)

* `tcp_delayed_ack_timeout_ns`
  - **Description:** delayed ACK timeout
  - **Value type:** positive integer (ns)

* `tcp_persist_timeout_ns`
  - **Description:** persist timeout
  - **Value type:** positive integer (ns)
