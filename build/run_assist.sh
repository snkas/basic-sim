NS3_VERSION="ns-3.31"

# Usage help
if [ "$1" == "--help" ] || [ "$#" -ne 3 ]; then
  echo "Usage: bash run_assist.sh [main] [run_folder] [mpi number of processes, 0 if not to use mpi]"
  exit 1
fi

# Retrieve arguments
main=$1
run_folder=$2
mpi_np=$3

# Rebuild
bash rebuild.sh || exit 1

# Run all examples with their respective mains
cd ${NS3_VERSION} || exit 1

# Make the logs directory already such that we can have console.txt in there
rm -rf "../${run_folder}/logs_ns3"
mkdir "../${run_folder}/logs_ns3"

# Perform the run
if [ "${mpi_np}" == "0" ]; then
  ./waf --run="${main} --run_dir='../${run_folder}'" 2>&1 | tee "../${run_folder}/logs_ns3/console.txt"
  if [[ $(< "../${run_folder}/logs_ns3/finished.txt") != "Yes" ]] ; then
    exit 1
  fi
else
  mpirun -np "${mpi_np}" ./waf --run="${main} --run_dir='../${run_folder}'" 2>&1 | tee "../${run_folder}/logs_ns3/console.txt"
  for (( s=1; s<$((mpi_np)); s++ ))
  do
    if [[ $(< "../${run_folder}/logs_ns3/system_${s}_finished.txt") != "Yes" ]] ; then
      exit 1
    fi
  done
fi
