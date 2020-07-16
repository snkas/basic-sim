NS3_VERSION="ns-3.30.1"

# Build
bash build.sh

# Copy over scratch main example
scp example/main_example.cc ${NS3_VERSION}/scratch

# Run the example from the README
cd ${NS3_VERSION} || exit 1
mkdir -p ../example/example_run/logs_ns3
./waf --run="main_example --run_dir='../example/example_run'" 2>&1 | tee ../example/example_run/logs_ns3/console.txt
