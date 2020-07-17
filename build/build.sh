NS3_VERSION="ns-3.30.1"

# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash build.sh [--help, --debug_all, --debug_minimal, --optimized, --optimized_with_tests]"
  exit 0
fi

# Extract copy of ns-3
echo "Unzipping clean ns-3 (no overwrites)"
unzip -n ${NS3_VERSION}.zip

# Create the basic-sim module
mkdir -p ${NS3_VERSION}/contrib/basic-sim

# Copy over this module (excluding the build directory)
rsync -ravh ../ ${NS3_VERSION}/contrib/basic-sim --exclude "build/" --exclude ".git/" --exclude ".idea/" --delete

# Create the necessary mains
mkdir -p ${NS3_VERSION}/scratch/main_example/
rsync -ravh example_mains/main_example/ ${NS3_VERSION}/scratch/main_example/ --delete
mkdir -p ${NS3_VERSION}/scratch/main_flows/
rsync -ravh example_mains/main_flows/ ${NS3_VERSION}/scratch/main_flows/ --delete
mkdir -p ${NS3_VERSION}/scratch/main_pingmesh/
rsync -ravh example_mains/main_pingmesh/ ${NS3_VERSION}/scratch/main_pingmesh/ --delete
mkdir -p ${NS3_VERSION}/scratch/main_flows_and_pingmesh/
rsync -ravh example_mains/main_flows_and_pingmesh/ ${NS3_VERSION}/scratch/main_flows_and_pingmesh/ --delete

# Go into ns-3 directory
cd ${NS3_VERSION} || exit 1

# Configure the build
if [ "$1" == "--debug_all" ]; then
  ./waf configure --enable-mpi --enable-tests --enable-examples --enable-gcov --out=build/debug_all

elif [ "$1" == "--debug_minimal" ]; then
  ./waf configure --enable-mpi --out=build/debug_minimal

elif [ "$1" == "--optimized" ]; then
  ./waf configure --build-profile=optimized --enable-mpi --out=build/optimized

elif [ "$1" == "--optimized_with_tests" ]; then
  ./waf configure --build-profile=optimized --enable-mpi --enable-tests --out=build/optimized_with_tests

elif [ "$1" == "" ]; then
  # Default is debug_all
  ./waf configure --enable-mpi --enable-tests --enable-examples --enable-gcov --out=build/debug_all

else
  echo "Invalid build option: $1"
  echo "Usage: bash build.sh [--debug_all, --debug_minimal, --optimized, --optimized_with_tests]"
  exit 1
fi

# Perform the build
./waf -j4
