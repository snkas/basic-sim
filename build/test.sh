#!/bin/bash

NS3_VERSION="ns-3.31"

# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash test.sh [--help] [--core, --apps, --distributed, --outside, --coverage]*"
  exit 0
fi

# Test results output folder
rm -rf test_results
mkdir -p test_results

# Rebuild to have the most up-to-date
bash rebuild.sh || exit 1

# Go into the ns-3 folder
cd ${NS3_VERSION} || exit 1

# Empty coverage counters
echo "Zeroing coverage counters"
lcov --directory build/debug_all --zerocounters

# Function to run multiple test suites but get a single file with all outcomes
run_test_suite_and_append () {
  echo "Running test suite $1"
  python test.py -v -s "$1" -t ../test_results/temp || exit 1
  cat ../test_results/temp.txt >> "$2" || exit 1
  rm ../test_results/temp.txt || exit 1
}

# Core tests
if [ "$1" == "" ] || [ "$1" == "--core" ] || [ "$2" == "--core" ] || [ "$3" == "--core" ] || [ "$4" == "--core" ] || [ "$5" == "--core" ]; then
  echo "Performing core tests"
  run_test_suite_and_append "basic-sim-core-basic-simulation" "../test_results/test_results_core.txt"
  run_test_suite_and_append "basic-sim-core-exp-util" "../test_results/test_results_core.txt"
  run_test_suite_and_append "basic-sim-core-log-update-helper" "../test_results/test_results_core.txt"
  run_test_suite_and_append "basic-sim-core-ptop" "../test_results/test_results_core.txt"
  run_test_suite_and_append "basic-sim-core-arbiter" "../test_results/test_results_core.txt"
  run_test_suite_and_append "basic-sim-core-ptop-tracking"  "../test_results/test_results_core.txt"
  run_test_suite_and_append "basic-sim-core-tcp-optimizer"  "../test_results/test_results_core.txt"
  cat ../test_results/test_results_core.txt
fi

# Apps tests
if [ "$1" == "" ] || [ "$1" == "--apps" ] || [ "$2" == "--apps" ] || [ "$3" == "--apps" ] || [ "$4" == "--apps" ] || [ "$5" == "--apps" ]; then
  echo "Performing apps tests"
  run_test_suite_and_append "basic-sim-apps-initial-helpers" "../test_results/test_results_apps.txt"
  run_test_suite_and_append "basic-sim-apps-manual" "../test_results/test_results_apps.txt"
  run_test_suite_and_append "basic-sim-apps-tcp-flow" "../test_results/test_results_apps.txt"
  run_test_suite_and_append "basic-sim-apps-udp-burst" "../test_results/test_results_apps.txt"
  run_test_suite_and_append "basic-sim-apps-udp-ping" "../test_results/test_results_apps.txt"
  cat ../test_results/test_results_apps.txt
fi

# Back to build/ directory
cd .. || exit 1

# Distributed tests for equivalent outcome
if [ "$1" == "" ] || [ "$1" == "--distributed" ] || [ "$2" == "--distributed" ] || [ "$3" == "--distributed" ] || [ "$4" == "--distributed" ] || [ "$5" == "--distributed" ]; then
  echo "Performing distributed tests"

  # Baseline
  bash run_assist.sh "test_run_folders/leaf_spine" 0 || exit 1

  # 1 core tests
  bash run_assist.sh "test_run_folders/leaf_spine_distributed_1_core_default" 1 || exit 1
  python test_distributed_exactly_equal.py "test_run_folders/leaf_spine" "test_run_folders/leaf_spine_distributed_1_core_default" 1 || exit 1

  bash run_assist.sh "test_run_folders/leaf_spine_distributed_1_core_nullmsg" 1 || exit 1
  python test_distributed_exactly_equal.py "test_run_folders/leaf_spine" "test_run_folders/leaf_spine_distributed_1_core_nullmsg" 1 || exit 1

  # 2 core tests
  bash run_assist.sh "test_run_folders/leaf_spine_distributed_2_core_default" 2 || exit 1
  python test_distributed_exactly_equal.py "test_run_folders/leaf_spine" "test_run_folders/leaf_spine_distributed_2_core_default" 2 || exit 1

  bash run_assist.sh "test_run_folders/leaf_spine_distributed_2_core_nullmsg" 2 || exit 1
  python test_distributed_exactly_equal.py "test_run_folders/leaf_spine" "test_run_folders/leaf_spine_distributed_2_core_nullmsg" 2 || exit 1

fi

# Tests to see if outside calling checks and checks for distributed validity are being hit
if [ "$1" == "" ] || [ "$1" == "--outside" ] || [ "$2" == "--outside" ] || [ "$3" == "--outside" ] || [ "$4" == "--outside" ] || [ "$5" == "--outside" ]; then
  echo "Performing outside tests"

  # main-full fail-without-argument
  cd ${NS3_VERSION} || exit 1
  ./waf --run="basic-sim-main-full" > /dev/null 2>&1
  if [ $? -eq 0 ]; then
    echo "Failed test: main-full fail-without-argument"
    exit 1
  fi
  cd .. || exit 1

  # basic-simulation distributed
  bash run_assist.sh "test_run_folders/basic_simulation_distributed" 2 > /dev/null 2>&1
  if [ $? -eq 1 ]; then
    echo "Failed test: basic-simulation distributed"
    exit 1
  fi

  # basic-simulation distributed-wrong-systems-number
  bash run_assist.sh "test_run_folders/basic_simulation_distributed" 3 > /dev/null 2>&1
  if [ $? -eq 0 ]; then
    echo "Failed test: basic-simulation distributed-wrong-systems-number"
    exit 1
  fi

  # basic-simulation distributed-invalid-system-id
  bash run_assist.sh "test_run_folders/basic_simulation_distributed_invalid_system_id" 2 > /dev/null 2>&1
  if [ $? -eq 0 ]; then
    echo "Failed test: basic-simulation distributed-invalid-system-id"
    exit 1
  fi

  # basic-simulation distributed-incorrect-type
  bash run_assist.sh "test_run_folders/basic_simulation_distributed_incorrect_type" 2 > /dev/null 2>&1
  if [ $? -eq 0 ]; then
    echo "Failed test: basic-simulation incorrect-distributed-type"
    exit 1
  fi

  # ptop-topology incorrect-node-assignment
  bash run_assist.sh "test_run_folders/ptop_topology_incorrect_node_assignment" 2 > /dev/null 2>&1
  if [ $? -eq 0 ]; then
    echo "Failed test: ptop-topology incorrect-node-assignment"
    exit 1
  fi

fi

# Coverage report
if [ "$1" == "" ] || [ "$1" == "--coverage" ] || [ "$2" == "--coverage" ] || [ "$3" == "--coverage" ] || [ "$4" == "--coverage" ] || [ "$5" == "--coverage" ]; then
  echo "Generating coverage report"

  # Make coverage report
  rm -rf coverage_report
  mkdir -p coverage_report
  cd ${NS3_VERSION}/build/debug_all/ || exit 1
  lcov --capture --directory contrib/basic-sim --output-file ../../../coverage_report/coverage.info

  # Remove directories from coverage report which we don't want
  lcov -r ../../../coverage_report/coverage.info "/usr/*" "*/build/debug_all/ns3/*" "*/test/*" "test/*" --output-file ../../../coverage_report/coverage.info

  # Generate html
  cd ../../../ || exit 1
  genhtml --output-directory coverage_report coverage_report/coverage.info
  echo "Coverage report is located at: coverage_report/index.html"

  # Show results
  echo "Display test results"
  if [ "$1" == "" ] || [ "$1" == "--core" ] || [ "$2" == "--core" ] || [ "$3" == "--core" ] || [ "$4" == "--core" ] || [ "$5" == "--core" ]; then
    cat test_results/test_results_core.txt
  fi
  if [ "$1" == "" ] || [ "$1" == "--apps" ] || [ "$2" == "--apps" ] || [ "$3" == "--apps" ] || [ "$4" == "--apps" ] || [ "$5" == "--apps" ]; then
    cat test_results/test_results_apps.txt
  fi

fi
