NS3_VERSION="ns-3.31"

# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash test.sh [--help, --no_coverage]"
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

# Perform coverage test
echo "Performing tests for coverage"
python test.py -v -s "basic-sim-core" -t ../test_results/test_results_core || exit 1
python test.py -v -s "basic-sim-apps" -t ../test_results/test_results_apps || exit 1

# Check cores
num_cores=$(nproc --all)

# 1 core tests
for experiment in "test_distributed_1_core_default" \
                  "test_distributed_1_core_nullmsg"
do
  rm -rf ../test_runs/${experiment}/logs_ns3
  mkdir ../test_runs/${experiment}/logs_ns3
  mpirun -np 1 ./waf --run="main_flows_and_pingmesh --run_dir='../test_runs/${experiment}'" 2>&1 | tee ../test_runs/${experiment}/logs_ns3/console.txt
  for s in "0"
  do
    if [[ $(< "../test_runs/${experiment}/logs_ns3/system_${s}_finished.txt") != "Yes" ]] ; then
      exit 1
    fi
  done
done

# 2 core tests
if [ "${num_cores}" -ge "2" ]; then
  for experiment in "test_distributed_2_core_default" \
                    "test_distributed_2_core_nullmsg"
  do
    rm -rf ../test_runs/${experiment}/logs_ns3
    mkdir ../test_runs/${experiment}/logs_ns3
    mpirun -np 2 ./waf --run="main_flows_and_pingmesh --run_dir='../test_runs/${experiment}'" 2>&1 | tee ../test_runs/${experiment}/logs_ns3/console.txt
    for s in "0" "1"
    do
      if [[ $(< "../test_runs/${experiment}/logs_ns3/system_${s}_finished.txt") != "Yes" ]] ; then
        exit 1
      fi
    done
  done
fi

# Stop if we don't want a coverage report
if [ "$1" == "--no_coverage" ]; then
  exit 0
fi

# Back to build/ directory
cd .. || exit 1

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
cat test_results/test_results_core.txt
cat test_results/test_results_apps.txt
