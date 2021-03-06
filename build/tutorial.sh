#!/bin/bash

# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash tutorial.sh [--help]"
  exit 0
fi

# Rebuild
bash rebuild.sh || exit 1

# Notify what is going to be run
echo "Performing tutorial run"

# Tutorial
bash run_assist.sh "example_run_folders/tutorial" 0 || exit 1

# Go plot helpers
cd ../tools/plotting || exit 1

# Link net-device utilization plots
cd plot_link_net_device_utilization || exit 1
python3 plot_link_net_device_utilization.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 0 1 || exit 1
python3 plot_link_net_device_utilization.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 1 0 || exit 1
python3 plot_link_net_device_utilization.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 0 2 || exit 1
python3 plot_link_net_device_utilization.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 2 0 || exit 1
python3 plot_link_net_device_utilization.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 0 3 || exit 1
python3 plot_link_net_device_utilization.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 3 0 || exit 1
cd .. || exit 1

# Link net-device queue plots
cd plot_link_net_device_queue || exit 1
python3 plot_link_net_device_queue.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 0 1 || exit 1
python3 plot_link_net_device_queue.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 1 0 || exit 1
python3 plot_link_net_device_queue.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 0 2 || exit 1
python3 plot_link_net_device_queue.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 2 0 || exit 1
python3 plot_link_net_device_queue.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 0 3 || exit 1
python3 plot_link_net_device_queue.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 3 0 || exit 1
cd .. || exit 1

# TCP flow plots
cd plot_tcp_flow || exit 1
python3 plot_tcp_flow.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 0 100000000 || exit 1
python3 plot_tcp_flow.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 1 100000000 || exit 1
python3 plot_tcp_flow.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 2 100000000 || exit 1
cd .. || exit 1

# UDP burst plots
cd plot_udp_burst || exit 1
python3 plot_udp_burst.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 0 100000000 || exit 1
python3 plot_udp_burst.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 1 100000000 || exit 1
cd .. || exit 1

# UDP ping plots
cd plot_udp_ping || exit 1
python3 plot_udp_ping.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 0 100000000 || exit 1
python3 plot_udp_ping.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 1 100000000 || exit 1
python3 plot_udp_ping.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 2 100000000 || exit 1
python3 plot_udp_ping.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 3 100000000 || exit 1
python3 plot_udp_ping.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 4 100000000 || exit 1
python3 plot_udp_ping.py ../../../build/example_run_folders/tutorial/logs_ns3 ../../../build/example_run_folders/tutorial/logs_ns3/data ../../../build/example_run_folders/tutorial/logs_ns3/pdf 5 100000000 || exit 1
cd .. || exit 1
