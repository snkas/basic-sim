#!/bin/bash

# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash one_udp_burst.sh [--help]"
  exit 0
fi

# Rebuild
bash rebuild.sh || exit 1

# one_udp_burst
bash run_assist.sh "example_run_folders/one_udp_burst" 0 || exit 1

# Go plot helpers
cd ../tools/plotting || exit 1

# UDP burst plots
cd plot_udp_burst || exit 1
python3 plot_udp_burst.py ../../../build/example_run_folders/one_udp_burst/logs_ns3 ../../../build/example_run_folders/one_udp_burst/logs_ns3/data ../../../build/example_run_folders/one_udp_burst/logs_ns3/pdf 0 100000000 || exit 1
python3 plot_udp_burst.py ../../../build/example_run_folders/one_udp_burst/logs_ns3 ../../../build/example_run_folders/one_udp_burst/logs_ns3/data ../../../build/example_run_folders/one_udp_burst/logs_ns3/pdf 1 100000000 || exit 1
cd .. || exit 1

# Link net-device utilization plots
cd plot_link_net_device_utilization || exit 1
python3 plot_link_net_device_utilization.py ../../../build/example_run_folders/one_udp_burst/logs_ns3 ../../../build/example_run_folders/one_udp_burst/logs_ns3/data ../../../build/example_run_folders/one_udp_burst/logs_ns3/pdf 0 1 || exit 1
python3 plot_link_net_device_utilization.py ../../../build/example_run_folders/one_udp_burst/logs_ns3 ../../../build/example_run_folders/one_udp_burst/logs_ns3/data ../../../build/example_run_folders/one_udp_burst/logs_ns3/pdf 1 0 || exit 1
cd .. || exit 1

# Link net-device queue plots
cd plot_link_net_device_queue || exit 1
python3 plot_link_net_device_queue.py ../../../build/example_run_folders/one_udp_burst/logs_ns3 ../../../build/example_run_folders/one_udp_burst/logs_ns3/data ../../../build/example_run_folders/one_udp_burst/logs_ns3/pdf 0 1 || exit 1
python3 plot_link_net_device_queue.py ../../../build/example_run_folders/one_udp_burst/logs_ns3 ../../../build/example_run_folders/one_udp_burst/logs_ns3/data ../../../build/example_run_folders/one_udp_burst/logs_ns3/pdf 1 0 || exit 1
cd .. || exit 1
