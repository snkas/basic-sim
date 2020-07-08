NS3_VERSION="ns-3.30.1"

# Test results output folder
rm -rf test_results
mkdir -p test_results
cd ${NS3_VERSION} || exit 1

# Empty coverage counters
lcov --directory build/debug_all --zerocounters

# Perform coverage test
python test.py -v -s "basic-sim" -t ../test_results/test_results_basic_sim

# Show results
cat ../test_results/test_results_basic_sim.txt
