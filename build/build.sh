NS3_VERSION="ns-3.30.1"

# Extract copy of ns-3
unzip ${NS3_VERSION}.zip

# Create the basic-sim module
mkdir ${NS3_VERSION}/src/basic-sim

# Copy over this module (excluding the build directory)
scp -r ../wscript ${NS3_VERSION}/src/basic-sim
scp -r ../examples ${NS3_VERSION}/src/basic-sim/examples
scp -r ../helper ${NS3_VERSION}/src/basic-sim/helper
scp -r ../model ${NS3_VERSION}/src/basic-sim/model
scp -r ../test ${NS3_VERSION}/src/basic-sim/test
scp -r ../doc ${NS3_VERSION}/src/basic-sim/doc

# Go into ns-3 directory
cd ${NS3_VERSION} || exit 1

# Perform build
./waf configure --enable-gcov --enable-examples --enable-tests --out=build/debug_all
./waf -j4
