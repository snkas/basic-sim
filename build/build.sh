NS3_VERSION="ns-3.30.1"

# Extract copy of ns-3
echo "Unzipping clean ns-3"
unzip ${NS3_VERSION}.zip

# Create the basic-sim module
mkdir -p ${NS3_VERSION}/contrib/basic-sim

# Copy over this module (excluding the build directory)
rsync -ravh ../ ${NS3_VERSION}/contrib/basic-sim --exclude "build/" --exclude ".git/" --exclude ".idea/" --delete

# Go into ns-3 directory
cd ${NS3_VERSION} || exit 1

# Configure the build
if [ "$1" == "--debug_all" ]; then
  ./waf configure --enable-gcov --enable-examples --enable-tests --out=build/debug_all

elif [ "$1" == "--debug_minimal" ]; then
  ./waf configure --out=build/debug_minimal

elif [ "$1" == "--optimized" ]; then
  ./waf configure --build-profile=optimized --out=build/optimized

elif [ "$1" == "--optimized_with_tests" ]; then
  ./waf configure --enable-tests --build-profile=optimized --out=build/optimized_with_tests

elif [ "$1" == "" ]; then
  # Default is debug_all
  ./waf configure --enable-gcov --enable-examples --enable-tests --out=build/debug_all

else
  echo "Invalid build option: $1"
  echo "Usage: bash build.sh [--debug_all, --debug_minimal, --optimized, --optimized_with_tests]"
  exit 1
fi

# Perform the build
./waf -j4
