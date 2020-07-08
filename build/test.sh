NS3_VERSION="ns-3.30.1"

# Test results output folder
rm -rf test_results
mkdir -p test_results
cd ${NS3_VERSION} || exit 1

# Empty coverage counters
echo "Zeroing coverage counters"
lcov --directory build/debug_all --zerocounters

# Perform coverage test
echo "Performing tests for coverage"
python test.py -v -s "basic-sim" -t ../test_results/test_results_basic_sim

# Show results
echo "Display test results"
cat ../test_results/test_results_basic_sim.txt

# Back to build/ directory
cd .. || exit 1

# Make coverage report
rm -rf coverage_report
mkdir -p coverage_report
cd ${NS3_VERSION}/build/debug_all/ || exit 1
#  --exclude "/usr/*" --exclude "src/basic-sim/test/*"
lcov  --capture --directory src/basic-sim --exclude "/usr/*" --exclude "*/build/debug_all/ns3/*" --exclude "*/test/*" --exclude "test/*" --output-file ../../../coverage_report/coverage.info
cd ../../../ || exit 1
genhtml --output-directory coverage_report coverage_report/coverage.info
echo "Coverage report is located at: coverage_report/index.html"
