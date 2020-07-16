NS3_VERSION="ns-3.30.1"

# Extract copy of ns-3
if [ "$1" != "--rebuild" ]; then
  echo "Unzipping clean ns-3"
  unzip ${NS3_VERSION}.zip
else
  echo "Skipping unzipping"
fi

# Create the basic-sim module
mkdir -p ${NS3_VERSION}/src/basic-sim

# Copy over this module (excluding the build directory)
rsync -ravh ../ ${NS3_VERSION}/src/basic-sim --exclude "build/" --exclude ".git/" --exclude ".idea/" --delete

# Go into ns-3 directory
cd ${NS3_VERSION} || exit 1

# Perform build
if [ "$1" != "--rebuild" ]; then
  ./waf configure --enable-gcov --enable-examples --enable-tests --out=build/debug_all
fi
./waf -j4
