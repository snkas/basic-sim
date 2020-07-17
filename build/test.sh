NS3_VERSION="ns-3.30.1"

# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash test.sh [--help, --no_coverage]"
  exit 0
fi

# Test results output folder
rm -rf test_results
mkdir -p test_results
cd ${NS3_VERSION} || exit 1

# Rebuild to have the most up-to-date
bash rebuild.sh

# Empty coverage counters
echo "Zeroing coverage counters"
lcov --directory build/debug_all --zerocounters

# Perform coverage test
echo "Performing tests for coverage"
python test.py -v -s "basic-sim-core" -t ../test_results/test_results_core
python test.py -v -s "basic-sim-apps" -t ../test_results/test_results_apps

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
