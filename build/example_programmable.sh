#!/bin/bash

NS3_VERSION="ns-3.33"

# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash example_programmable.sh [--help]"
  exit 0
fi

# Rebuild
bash rebuild.sh || exit 1

# Enter the ns-3 directory
cd ${NS3_VERSION} || exit 1

# Notify what is going to be run
echo "Performing programmable example runs"

# Function to run multiple test suites but get a single file with all outcomes
run_ns3_example () {
  echo "Running ns-3 example $1"

  # Make the logs directory already such that we can have console.txt in there
  rm -rf "example_programmable/$1/logs_ns3" || exit 1
  mkdir -p "example_programmable/$1/logs_ns3" || exit 1

  # Run
  set -eu -o pipefail
  ./waf --run="$1" 2>&1 | tee "example_programmable/$1/logs_ns3/console.txt" || exit 1

  # Make sure it finished successfully
  if [[ $(< "example_programmable/$1/logs_ns3/finished.txt") != "Yes" ]] ; then
    exit 1
  fi

  # Some quick plotting
  cd ../../tools/plotting || exit 1

  # Plot TCP flow with ID 0
  cd plot_tcp_flow || exit 1
  python3 plot_tcp_flow.py ../../../build/${NS3_VERSION}/example_programmable/$1/logs_ns3 ../../../build/${NS3_VERSION}/example_programmable/$1/logs_ns3/data ../../../build/${NS3_VERSION}/example_programmable/$1/logs_ns3/pdf 0 100000000 || exit 1
  cd .. || exit 1

  # Link net-device queue plot of 0 -> 1
  cd plot_link_net_device_queue || exit 1
  python3 plot_link_net_device_queue.py ../../../build/${NS3_VERSION}/example_programmable/$1/logs_ns3 ../../../build/${NS3_VERSION}/example_programmable/$1/logs_ns3/data ../../../build/${NS3_VERSION}/example_programmable/$1/logs_ns3/pdf 0 1 || exit 1
  cd .. || exit 1

  # Link interface traffic-control queueing discipline queue plot of 0 -> 1
  cd plot_link_interface_tc_qdisc_queue || exit 1
  python3 plot_link_interface_tc_qdisc_queue.py ../../../build/${NS3_VERSION}/example_programmable/$1/logs_ns3 ../../../build/${NS3_VERSION}/example_programmable/$1/logs_ns3/data ../../../build/${NS3_VERSION}/example_programmable/$1/logs_ns3/pdf 0 1 || exit 1
  cd .. || exit 1

  cd ../../build/${NS3_VERSION} || exit 1

}

# Run them all
run_ns3_example "basic-sim-example-single-default" || exit 1
run_ns3_example "basic-sim-example-single-cubic" || exit 1
run_ns3_example "basic-sim-example-single-dctcp" || exit 1
run_ns3_example "basic-sim-example-absolute-priority" || exit 1

# Finished
echo "Finished running programmable examples"
