NS3_VERSION="ns-3.30.1"

# Create the necessary mains
mkdir -p ${NS3_VERSION}/scratch/main_flows/
rsync -ravh extra_examples/main/main_flows/ ${NS3_VERSION}/scratch/main_flows/ --delete
mkdir -p ${NS3_VERSION}/scratch/main_pingmesh/
rsync -ravh extra_examples/main/main_pingmesh/ ${NS3_VERSION}/scratch/main_pingmesh/ --delete
mkdir -p ${NS3_VERSION}/scratch/main_flows_and_pingmesh/
rsync -ravh extra_examples/main/main_flows_and_pingmesh/ ${NS3_VERSION}/scratch/main_flows_and_pingmesh/ --delete

# Rebuild
bash rebuild.sh

# Run them all
cd ${NS3_VERSION} || exit 1
for experiment in "flows_example_single" "flows_example_single_many_small_flows" "flows_example_single_one_large_flow" \
                  "flows_example_ring" "flows_example_leaf_spine" "flows_example_leaf_spine_servers" "flows_example_fat_tree_k4_servers" \
                  "flows_example_big_grid" "flows_example_big_grid_one_flow"  #  Too long in debug: "flows_example_single_one_10g"
do
  rm -rf ../extra_examples/${experiment}/logs_ns3
  mkdir ../extra_examples/${experiment}/logs_ns3
  ./waf --run="main_flows --run_dir='../extra_examples/${experiment}'" 2>&1 | tee ../extra_examples/${experiment}/logs_ns3/console.txt
done

for experiment in "pingmesh_example_grid" "pingmesh_example_single" "pingmesh_example_grid_select_pairs"
do
  rm -rf ../extra_examples/${experiment}/logs_ns3
  mkdir ../extra_examples/${experiment}/logs_ns3
  ./waf --run="main_pingmesh --run_dir='../extra_examples/${experiment}'" 2>&1 | tee ../extra_examples/${experiment}/logs_ns3/console.txt
done
