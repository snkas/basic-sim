#!/bin/bash

# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash check_reproduce.sh [--help]"
  exit 0
fi

# Run reproduce run
echo "Performing reproduce run..."
bash run_assist.sh "test_run_folders/reproduce" 0 > /dev/null 2>&1
if [ $? -eq 1 ]; then
  echo "FAIL: reproduce test run failed"
  exit 1
fi

# Go to plot helpers
cd ../tools/plotting || exit 1

# TCP flow plots
cd plot_tcp_flow || exit 1
python plot_tcp_flow.py ../../../build/test_run_folders/reproduce/logs_ns3 ../../../build/test_run_folders/reproduce/logs_ns3/data ../../../build/test_run_folders/reproduce/logs_ns3/pdf 0 100000000 || exit 1
cd .. || exit 1

# UDP burst plots
cd plot_udp_burst || exit 1
python plot_udp_burst.py ../../../build/test_run_folders/reproduce/logs_ns3 ../../../build/test_run_folders/reproduce/logs_ns3/data ../../../build/test_run_folders/reproduce/logs_ns3/pdf 0 100000000 || exit 1
python plot_udp_burst.py ../../../build/test_run_folders/reproduce/logs_ns3 ../../../build/test_run_folders/reproduce/logs_ns3/data ../../../build/test_run_folders/reproduce/logs_ns3/pdf 1 100000000 || exit 1
cd .. || exit 1

# UDP ping plots
cd plot_udp_ping || exit 1
python plot_udp_ping.py ../../../build/test_run_folders/reproduce/logs_ns3 ../../../build/test_run_folders/reproduce/logs_ns3/data ../../../build/test_run_folders/reproduce/logs_ns3/pdf 0 100000000 || exit 1
python plot_udp_ping.py ../../../build/test_run_folders/reproduce/logs_ns3 ../../../build/test_run_folders/reproduce/logs_ns3/data ../../../build/test_run_folders/reproduce/logs_ns3/pdf 1 100000000 || exit 1
cd .. || exit 1

# Return to build directory
cd ../../build

# Compare to expectation
result=0
echo "Comparing logs to expectation..."
diff -x "timing_results.*" -x "console.txt" -x ".gitignore" -x "data" -x "pdf" -r "test_run_folders/reproduce/logs_ns3" "test_run_folders/reproduce/expected_logs_ns3"
if [ $? -eq 1 ]; then
  echo "FAIL: reproduce test run yielded differences"
  exit 1
else
  echo "SUCCESS: logs were reproduced"
  exit 0
fi
