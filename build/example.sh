NS3_VERSION="ns-3.30.1"

# Build optimized
bash build.sh

# Run all examples with their respective mains
cd ${NS3_VERSION} || exit 1

# Example
rm -rf ../example_runs/example_getting_started/logs_ns3
mkdir ../example_runs/example_getting_started/logs_ns3
./waf --run="main_example --run_dir='../example_runs/example_getting_started'" 2>&1 | tee ../example_runs/example_getting_started/logs_ns3/console.txt

# Flows
for experiment in "flows_getting_started" \
                  "flows_ring" \
                  "flows_leaf_spine" \
                  "flows_leaf_spine_servers" \
                  "flows_fat_tree_k4_servers"
do
  rm -rf ../example_runs/${experiment}/logs_ns3
  mkdir ../example_runs/${experiment}/logs_ns3
  ./waf --run="main_flows --run_dir='../example_runs/${experiment}'" 2>&1 | tee ../example_runs/${experiment}/logs_ns3/console.txt
done

# Pingmesh
for experiment in "pingmesh_getting_started" \
                  "pingmesh_single" \
                  "pingmesh_grid" \
                  "pingmesh_grid_select_pairs"
do
  rm -rf ../example_runs/${experiment}/logs_ns3
  mkdir ../example_runs/${experiment}/logs_ns3
  ./waf --run="main_pingmesh --run_dir='../example_runs/${experiment}'" 2>&1 | tee ../example_runs/${experiment}/logs_ns3/console.txt
done

# Flows AND pingmesh
rm -rf ../example_runs/pingmesh_and_flows_fat_tree_k4_servers/logs_ns3
mkdir ../example_runs/pingmesh_and_flows_fat_tree_k4_servers/logs_ns3
./waf --run="main_flows_and_pingmesh --run_dir='../example_runs/pingmesh_and_flows_fat_tree_k4_servers'" 2>&1 | tee ../example_runs/pingmesh_and_flows_fat_tree_k4_servers/logs_ns3/console.txt
