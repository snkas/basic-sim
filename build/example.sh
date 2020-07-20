# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash build.sh [--help, --all, --utilization_only, --flows_only, --pingmesh_only, --flows_and_pingmesh_only]"
  exit 0
fi

# Rebuild
bash rebuild.sh || exit 1

# Check cores
num_cores=$(nproc --all)

############################################

if [ "$1" == "" ] || [ "$1" == "--all" ] || [ "$1" == "--flows_only" ]; then

  # Flows
  bash run_assist.sh "main_full" "example_runs/tutorial" 0 || exit 1
  bash run_assist.sh "main_full" "example_runs/flows_ring" 0 || exit 1
  bash run_assist.sh "main_full" "example_runs/flows_leaf_spine" 0 || exit 1
  bash run_assist.sh "main_full" "example_runs/flows_leaf_spine_servers" 0 || exit 1
  bash run_assist.sh "main_full" "example_runs/flows_fat_tree_k4_servers" 0 || exit 1

  # Flows distributed
  if [ "${num_cores}" -ge "4" ]; then
    bash run_assist.sh "main_full" "example_runs/flows_fat_tree_k4_servers_distributed" 4 || exit 1
  fi

fi

############################################

if [ "$1" == "" ] || [ "$1" == "--all" ] || [ "$1" == "--pingmesh_only" ]; then

  # Pingmesh
  bash run_assist.sh "main_full" "example_runs/pingmesh_single" 0 || exit 1
  bash run_assist.sh "main_full" "example_runs/pingmesh_grid" 0 || exit 1
  bash run_assist.sh "main_full" "example_runs/pingmesh_grid_select_pairs" 0 || exit 1
  bash run_assist.sh "main_full" "example_runs/pingmesh_fat_tree_k4_servers" 0 || exit 1

  # Pingmesh distributed
  if [ "${num_cores}" -ge "4" ]; then
    bash run_assist.sh "main_full" "example_runs/pingmesh_fat_tree_k4_servers_distributed" 4 || exit 1
  fi

fi

############################################

if [ "$1" == "" ] || [ "$1" == "--all" ] || [ "$1" == "--flows_and_pingmesh_only" ]; then

  # Flows AND pingmesh
  bash run_assist.sh "main_full" "example_runs/pingmesh_and_flows_fat_tree_k4_servers" 0 || exit 1

  # Flows AND pingmesh distributed

  # 1 core
  bash run_assist.sh "main_full" "example_runs/pingmesh_and_flows_fat_tree_k4_servers_distributed_one_core" 1 || exit 1

  # 4 cores
  if [ "${num_cores}" -ge "4" ]; then
    bash run_assist.sh "main_full" "example_runs/pingmesh_and_flows_fat_tree_k4_servers_distributed" 4 || exit 1
  fi

fi

############################################
