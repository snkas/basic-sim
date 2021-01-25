#!/bin/bash

NS3_VERSION="ns-3.33"

# Usage help
if [ "$1" == "--help" ] || [ "$#" -ne 2 ]; then
  echo "Usage: bash run_assist.sh [run_folder] [mpi number of processes, 0 if not to use mpi]"
  echo ""
  echo "Usage with ns-3 logging:"
  echo ""
  echo "   NS_LOG=\"TcpFlowServer=all:TcpSocketBase=all\" bash run_assist.sh [run_folder] [mpi number of processes, 0 if not to use mpi]"
  echo ""
  exit 1
fi

# Retrieve arguments
run_folder=$1
mpi_np=$2

# Rebuild
bash rebuild.sh || exit 1

# Run all examples with their respective mains
cd ${NS3_VERSION} || exit 1

# Make the logs directory already such that we can have console.txt in there
rm -rf "../${run_folder}/logs_ns3" || exit 1
mkdir "../${run_folder}/logs_ns3" || exit 1

# Perform the run
if [ "${mpi_np}" == "0" ]; then

  # Single logical process
  rm -f "../${run_folder}/logs_ns3/finished.txt"
  set -eu -o pipefail
  ./waf --run="basic-sim-main-full --run_dir='../${run_folder}'" 2>&1 | tee "../${run_folder}/logs_ns3/console.txt" || exit 1
  if [[ $(< "../${run_folder}/logs_ns3/finished.txt") != "Yes" ]] ; then
    exit 1
  fi

else

  # Delete all finished
  for (( s=1; s<$((mpi_np)); s++ ))
  do
    rm -f "../${run_folder}/logs_ns3/system_${s}_finished.txt"
  done

  # Multiple logical processes

  # Debug version (through waf shell): (cannot set -o pipefail because it might be a dash shell)
  echo "set -eu; cd build/debug_all; mpirun -np ${mpi_np} contrib/basic-sim/main/ns3.33-basic-sim-main-full-debug --run_dir='../../../${run_folder}' 2>&1 | tee '../../../${run_folder}/logs_ns3/console.txt' || exit 1" | ./waf shell || exit 1

  # Optimized version (through waf shell): (cannot set -o pipefail because it might be a dash shell)
  # echo "set -eu; cd build/optimized; mpirun -np ${mpi_np} contrib/basic-sim/main/ns3.33-basic-sim-main-full-optimized --run_dir='../../../${run_folder}' 2>&1 | tee '../../../${run_folder}/logs_ns3/console.txt' || exit 1" | ./waf shell || exit 1

  # Not going through the waf shell can cause some concurrent file check errors:
  # set -eu -o pipefail; mpirun -np "${mpi_np}" ./waf --run="basic-sim-main-full --run_dir='../${run_folder}'" 2>&1 | tee "../${run_folder}/logs_ns3/console.txt || exit 1"

  # Verify all logical processes ("systems") finished successfully
  for (( s=1; s<$((mpi_np)); s++ ))
  do
    if [[ $(< "../${run_folder}/logs_ns3/system_${s}_finished.txt") != "Yes" ]] ; then
      exit 1
    fi
  done

fi
