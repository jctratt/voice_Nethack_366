#!/bin/bash
# Compile all data and build the project. Fixed to run lev_comp from the dat
# directory so generated .lev files are written into 'dat/' rather than the repo root.
set -euo pipefail

make clean

# Generate .des files
cd dat
./GENFILES

# Run level compiler from dat so output files are created inside dat/
../util/lev_comp *.des

# Back to repo root and configure/build
cd ../sys/unix
sh setup.sh hints/linux
cd ../..
make -j$(nproc)

# Optionally: install (uncomment if you want to install locally)
# make install
