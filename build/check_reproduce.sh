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

# Compare to expectation
echo "Comparing logs to expectation..."
diff -x "timing_results.*" -x "console.txt" -r "test_run_folders/reproduce/logs_ns3" "test_run_folders/reproduce/expected_logs_ns3"
if [ $? -eq 1 ]; then
  echo "FAIL: reproduce test run yielded differences"
  exit 1
fi

# Done
echo "SUCCESS: logs were reproduced"
