NS3_VERSION="ns-3.31"

# Usage help
if [ "$1" == "--help" ]; then
  echo "Usage: bash build.sh [--help, --debug_all, --debug_minimal, --optimized, --optimized_with_tests]"
  exit 0
fi

# Extract copy of ns-3
echo "Unzipping clean ns-3 (no overwrites)"
unzip -nq ${NS3_VERSION}.zip || exit 1

# Removing non-dependent modules for quicker compilation
echo "Removing modules in ${NS3_VERSION}/src/ on which basic-sim is not dependent on"
cd "${NS3_VERSION}/src" || exit 1
for src_module in */ ; do

  # Check if the folder is one of the dependent modules
  is_dependency=0
  for dependency in "core" "internet" "applications" "point-to-point" "traffic-control" "bridge" "network" "config-store" "stats" "mpi" "visualizer"
  do
    if [ "${src_module}" == "${dependency}/" ]; then
      is_dependency=1
    fi
  done

  # If not, the module can be removed from the src/ directory
  if [ "${is_dependency}" != "1" ]; then
    echo "Removing non-dependent module: ${src_module}"
    rm -rf "${src_module}"
  fi

done

# Return to build directory
cd ../../ || exit 1

# Create the basic-sim module
mkdir -p ${NS3_VERSION}/contrib/basic-sim || exit 1

# Copy over this module (excluding the build directory)
rsync -ravh ../ ${NS3_VERSION}/contrib/basic-sim --exclude "build/" --exclude ".git/" --exclude ".idea/" --delete || exit 1

# Go into ns-3 directory
cd ${NS3_VERSION} || exit 1

# Configure the build
if [ "$1" == "--debug_all" ]; then
  ./waf configure --build-profile=debug --enable-mpi --enable-examples --enable-tests --enable-gcov --out=build/debug_all || exit 1

elif [ "$1" == "--debug_minimal" ]; then
  ./waf configure --build-profile=debug --enable-mpi --out=build/debug_minimal || exit 1

elif [ "$1" == "--optimized" ]; then
  ./waf configure --build-profile=optimized --enable-mpi --out=build/optimized || exit 1

elif [ "$1" == "--optimized_with_tests" ]; then
  ./waf configure --build-profile=optimized --enable-mpi --enable-tests --out=build/optimized_with_tests || exit 1

elif [ "$1" == "" ]; then
  # Default is debug_all
  ./waf configure --build-profile=debug --enable-mpi --enable-examples --enable-tests --enable-gcov --out=build/debug_all || exit 1

else
  echo "Invalid build option: $1"
  echo "Usage: bash build.sh [--debug_all, --debug_minimal, --optimized, --optimized_with_tests]"
  exit 1
fi

# Perform the build
./waf -j4 || exit 1
