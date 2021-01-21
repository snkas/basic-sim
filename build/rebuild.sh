#!/bin/bash

NS3_VERSION="ns-3.31"

# Extract copy of ns-3
echo "Unzipping clean ns-3 (no overwrites)"
unzip -nq ${NS3_VERSION}.zip || exit 1  # -nq will not override, -oq will override

# Removing non-dependent modules for quicker compilation
echo "Removing modules in ${NS3_VERSION}/src/ on which basic-sim is not dependent"
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
mkdir -p ${NS3_VERSION}/contrib/basic-sim

# Copy over this module (excluding the build directory)
rsync -ravh ../ ${NS3_VERSION}/contrib/basic-sim --exclude "build/" --exclude ".git/" --exclude ".idea/" --delete

# Go into ns-3 directory
cd ${NS3_VERSION} || exit 1

# Rebuild whichever build is configured right now
./waf -j4 || exit 1
