NS3_VERSION="ns-3.31"

# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash build.sh [--help, --debug_all, --debug_minimal, --optimized, --optimized_with_tests]"
  exit 0
fi

# Extract copy of ns-3
echo "Unzipping clean ns-3 (no overwrites)"
unzip -n ${NS3_VERSION}.zip || exit 1

# Create the basic-sim module
mkdir -p ${NS3_VERSION}/contrib/basic-sim || exit 1

# Copy over this module (excluding the build directory)
rsync -ravh ../ ${NS3_VERSION}/contrib/basic-sim --exclude "build/" --exclude ".git/" --exclude ".idea/" --delete || exit 1

# Create the necessary mains
mkdir -p ${NS3_VERSION}/scratch/main_example/ || exit 1
rsync -ravh example_mains/main_example/ ${NS3_VERSION}/scratch/main_example/ --delete || exit 1
mkdir -p ${NS3_VERSION}/scratch/main_flows/ || exit 1
rsync -ravh example_mains/main_flows/ ${NS3_VERSION}/scratch/main_flows/ --delete || exit 1
mkdir -p ${NS3_VERSION}/scratch/main_pingmesh/ || exit 1
rsync -ravh example_mains/main_pingmesh/ ${NS3_VERSION}/scratch/main_pingmesh/ --delete || exit 1
mkdir -p ${NS3_VERSION}/scratch/main_flows_and_pingmesh/ || exit 1
rsync -ravh example_mains/main_flows_and_pingmesh/ ${NS3_VERSION}/scratch/main_flows_and_pingmesh/ --delete || exit 1

# Go into ns-3 directory
cd ${NS3_VERSION} || exit 1

# Configure the build
if [ "$1" == "--debug_all" ]; then
  ./waf configure --enable-mpi --enable-tests --enable-examples --enable-gcov --out=build/debug_all || exit 1

elif [ "$1" == "--debug_minimal" ]; then
  ./waf configure --enable-mpi --out=build/debug_minimal || exit 1

elif [ "$1" == "--optimized" ]; then
  ./waf configure --build-profile=optimized --enable-mpi --out=build/optimized || exit 1

elif [ "$1" == "--optimized_with_tests" ]; then
  ./waf configure --build-profile=optimized --enable-mpi --enable-tests --out=build/optimized_with_tests || exit 1

elif [ "$1" == "" ]; then
  # Default is debug_all
  ./waf configure --enable-mpi --enable-tests --enable-examples --enable-gcov --out=build/debug_all || exit 1

else
  echo "Invalid build option: $1"
  echo "Usage: bash build.sh [--debug_all, --debug_minimal, --optimized, --optimized_with_tests]"
  exit 1
fi

# Perform the build
./waf -j4 || exit 1
