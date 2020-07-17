bash rebuild.sh
experiment="distributed_baseline"
cd ns-3.30.1 || exit 1
rm -rf ../example_runs/${experiment}/logs_ns3
mkdir ../example_runs/${experiment}/logs_ns3
./waf --run="main_flows --run_dir='../example_runs/${experiment}'" 2>&1 | tee ../example_runs/${experiment}/logs_ns3/console.txt