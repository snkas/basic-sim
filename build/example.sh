NS3_VERSION="ns-3.30.1"

# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash build.sh [--help, --all, --example_only, --flows_only, --pingmesh_only, --flows_and_pingmesh_only]"
  exit 0
fi

# Run all examples with their respective mains
cd ${NS3_VERSION} || exit 1

############################################

if [ "$1" == "" ] || [ "$1" == "--all" ] || [ "$1" == "--example_only" ]; then

  # Example
  experiment="example_getting_started"
  rm -rf ../example_runs/${experiment}/logs_ns3
  mkdir ../example_runs/${experiment}/logs_ns3
  ./waf --run="main_example --run_dir='../example_runs/${experiment}'" 2>&1 | tee ../example_runs/${experiment}/logs_ns3/console.txt
  if [[ $(< "../example_runs/${experiment}/logs_ns3/finished.txt") != "Yes" ]]; then
    exit 1
  fi

fi

############################################

if [ "$1" == "" ] || [ "$1" == "--all" ] || [ "$1" == "--flows_only" ]; then

  # Flows
  for experiment in "flows_getting_started" \
                    "flows_ring" \
                    "flows_tor_servers" \
                    "flows_leaf_spine" \
                    "flows_leaf_spine_servers" \
                    "flows_fat_tree_k4_servers"
  do
    rm -rf ../example_runs/${experiment}/logs_ns3
    mkdir ../example_runs/${experiment}/logs_ns3
    ./waf --run="main_flows --run_dir='../example_runs/${experiment}'" 2>&1 | tee ../example_runs/${experiment}/logs_ns3/console.txt
    if [[ $(< "../example_runs/${experiment}/logs_ns3/finished.txt") != "Yes" ]]; then
      exit 1
    fi
  done

  # Flows distributed
  experiment="flows_fat_tree_k4_servers_distributed"
  rm -rf ../example_runs/${experiment}/logs_ns3
  mkdir ../example_runs/${experiment}/logs_ns3
  mpirun -np 4 ./waf --run="main_flows --run_dir='../example_runs/${experiment}'" 2>&1 | tee ../example_runs/${experiment}/logs_ns3/console.txt
  for s in "0" "1" "2" "3"
  do
    if [[ $(< "../example_runs/${experiment}/logs_ns3/system_${s}_finished.txt") != "Yes" ]] ; then
      exit 1
    fi
  done

fi

############################################

if [ "$1" == "" ] || [ "$1" == "--all" ] || [ "$1" == "--pingmesh_only" ]; then

  # Pingmesh
  for experiment in "pingmesh_getting_started" \
                    "pingmesh_single" \
                    "pingmesh_grid" \
                    "pingmesh_grid_select_pairs" \
                    "pingmesh_fat_tree_k4_servers"
  do
    rm -rf ../example_runs/${experiment}/logs_ns3
    mkdir ../example_runs/${experiment}/logs_ns3
    ./waf --run="main_pingmesh --run_dir='../example_runs/${experiment}'" 2>&1 | tee ../example_runs/${experiment}/logs_ns3/console.txt
    if [[ $(< "../example_runs/${experiment}/logs_ns3/finished.txt") != "Yes" ]]; then
      exit 1
    fi
  done

  # Pingmesh distributed
  experiment="pingmesh_fat_tree_k4_servers_distributed"
  rm -rf ../example_runs/${experiment}/logs_ns3
  mkdir ../example_runs/${experiment}/logs_ns3
  mpirun -np 4 ./waf --run="main_pingmesh --run_dir='../example_runs/${experiment}'" 2>&1 | tee ../example_runs/${experiment}/logs_ns3/console.txt
  for s in "0" "1" "2" "3"
  do
    if [[ $(< "../example_runs/${experiment}/logs_ns3/system_${s}_finished.txt") != "Yes" ]] ; then
      exit 1
    fi
  done

fi

############################################

if [ "$1" == "" ] || [ "$1" == "--all" ] || [ "$1" == "--flows_and_pingmesh_only" ]; then

  # Flows AND pingmesh
  experiment="pingmesh_and_flows_fat_tree_k4_servers"
  rm -rf ../example_runs/${experiment}/logs_ns3
  mkdir ../example_runs/${experiment}/logs_ns3
  ./waf --run="main_flows_and_pingmesh --run_dir='../example_runs/${experiment}'" 2>&1 | tee ../example_runs/${experiment}/logs_ns3/console.txt
  if [[ $(< "../example_runs/${experiment}/logs_ns3/finished.txt") != "Yes" ]]; then
    exit 1
  fi

  # Flows AND pingmesh distributed
  experiment="pingmesh_and_flows_fat_tree_k4_servers_distributed"
  rm -rf ../example_runs/${experiment}/logs_ns3
  mkdir ../example_runs/${experiment}/logs_ns3
  mpirun -np 4 ./waf --run="main_flows_and_pingmesh --run_dir='../example_runs/${experiment}'" 2>&1 | tee ../example_runs/${experiment}/logs_ns3/console.txt
  for s in "0" "1" "2" "3"
  do
    if [[ $(< "../example_runs/${experiment}/logs_ns3/system_${s}_finished.txt") != "Yes" ]] ; then
      exit 1
    fi
  done

fi

############################################
