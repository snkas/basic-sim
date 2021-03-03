#!/bin/bash

# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash example_main.sh [--help]"
  exit 0
fi

# Rebuild
bash rebuild.sh || exit 1

# Notify what is going to be run
echo "Performing example runs"

########################################################################################

# Single process (without MPI)
bash run_assist.sh "example_run_folders/single" 0 || exit 1
bash run_assist.sh "example_run_folders/ring" 0 || exit 1
bash run_assist.sh "example_run_folders/leaf_spine" 0 || exit 1
bash run_assist.sh "example_run_folders/leaf_spine_servers" 0 || exit 1
bash run_assist.sh "example_run_folders/grid" 0 || exit 1
bash run_assist.sh "example_run_folders/fat_tree_k4_servers" 0 || exit 1
bash run_assist.sh "example_run_folders/single_everything" 0 || exit 1
bash run_assist.sh "example_run_folders/fat_tree_k4_servers_poisson" 0 || exit 1

# Most simple examples
bash one_tcp_flow.sh || exit 1
bash one_udp_burst.sh || exit 1
bash one_udp_ping.sh || exit 1

# Distributed
bash run_assist.sh "example_run_folders/leaf_spine_2_core" 2 || exit 1

########################################################################################

# Go to plot helpers
cd ../tools/plotting || exit 1

## Plotting: fat_tree_k4_servers_poisson
cd plot_tcp_flows_ecdfs || exit 1
python3 plot_tcp_flows_ecdfs.py ../../../build/example_run_folders/fat_tree_k4_servers_poisson/logs_ns3 \
                               ../../../build/example_run_folders/fat_tree_k4_servers_poisson/logs_ns3/data \
                               ../../../build/example_run_folders/fat_tree_k4_servers_poisson/logs_ns3/pdf || exit 1
cd .. || exit 1

## Plotting: single_everything plotting

# Link net-device utilization plots
cd plot_link_net_device_utilization || exit 1
python3 plot_link_net_device_utilization.py ../../../build/example_run_folders/single_everything/logs_ns3 ../../../build/example_run_folders/single_everything/logs_ns3/data ../../../build/example_run_folders/single_everything/logs_ns3/pdf 0 1 || exit 1
python3 plot_link_net_device_utilization.py ../../../build/example_run_folders/single_everything/logs_ns3 ../../../build/example_run_folders/single_everything/logs_ns3/data ../../../build/example_run_folders/single_everything/logs_ns3/pdf 1 0 || exit 1
cd .. || exit 1

# Link net-device queue plots
cd plot_link_net_device_queue || exit 1
python3 plot_link_net_device_queue.py ../../../build/example_run_folders/single_everything/logs_ns3 ../../../build/example_run_folders/single_everything/logs_ns3/data ../../../build/example_run_folders/single_everything/logs_ns3/pdf 0 1 || exit 1
python3 plot_link_net_device_queue.py ../../../build/example_run_folders/single_everything/logs_ns3 ../../../build/example_run_folders/single_everything/logs_ns3/data ../../../build/example_run_folders/single_everything/logs_ns3/pdf 1 0 || exit 1
cd .. || exit 1

# Link interface traffic-control queueing discipline queue plots
cd plot_link_interface_tc_qdisc_queue || exit 1
python3 plot_link_interface_tc_qdisc_queue.py ../../../build/example_run_folders/single_everything/logs_ns3 ../../../build/example_run_folders/single_everything/logs_ns3/data ../../../build/example_run_folders/single_everything/logs_ns3/pdf 0 1 || exit 1
cd .. || exit 1

# TCP flow plots
cd plot_tcp_flow || exit 1
python3 plot_tcp_flow.py ../../../build/example_run_folders/single_everything/logs_ns3 ../../../build/example_run_folders/single_everything/logs_ns3/data ../../../build/example_run_folders/single_everything/logs_ns3/pdf 0 100000000 || exit 1
cd .. || exit 1

# UDP burst plots
cd plot_udp_burst || exit 1
python3 plot_udp_burst.py ../../../build/example_run_folders/single_everything/logs_ns3 ../../../build/example_run_folders/single_everything/logs_ns3/data ../../../build/example_run_folders/single_everything/logs_ns3/pdf 0 100000000 || exit 1
python3 plot_udp_burst.py ../../../build/example_run_folders/single_everything/logs_ns3 ../../../build/example_run_folders/single_everything/logs_ns3/data ../../../build/example_run_folders/single_everything/logs_ns3/pdf 1 100000000 || exit 1
cd .. || exit 1

# UDP ping plots
cd plot_udp_ping || exit 1
python3 plot_udp_ping.py ../../../build/example_run_folders/single_everything/logs_ns3 ../../../build/example_run_folders/single_everything/logs_ns3/data ../../../build/example_run_folders/single_everything/logs_ns3/pdf 0 100000000 || exit 1
python3 plot_udp_ping.py ../../../build/example_run_folders/single_everything/logs_ns3 ../../../build/example_run_folders/single_everything/logs_ns3/data ../../../build/example_run_folders/single_everything/logs_ns3/pdf 1 100000000 || exit 1
cd .. || exit 1
