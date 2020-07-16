NS3_VERSION="ns-3.30.1"

# Create the basic-sim module
mkdir -p ${NS3_VERSION}/contrib/basic-sim

# Copy over this module (excluding the build directory)
rsync -ravh ../ ${NS3_VERSION}/contrib/basic-sim --exclude "build/" --exclude ".git/" --exclude ".idea/" --delete

# Go into ns-3 directory
cd ${NS3_VERSION} || exit 1

# Perform build
./waf -j4
