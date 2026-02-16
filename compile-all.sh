#!/bin/bash
# Compile all data and build the project. Fixed to run lev_comp from the dat
# directory so generated .lev files are written into 'dat/' rather than the repo root.
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Avoid build failures when /tmp is full by defaulting to a workspace-local
# temporary directory. Respect caller-provided TMPDIR if already set.
if [ -z "${TMPDIR:-}" ]; then
    export TMPDIR="$ROOT_DIR/tmp"
fi
mkdir -p "$TMPDIR"

make clean

# Generate .des files
cd dat
./GENFILES

# Ensure `dat/dungeon` is regenerated if the source PDF changed.
# This is only necessary when you've edited `dat/dungeon.pdf` or changed
# structure/compatibility constants (makedefs). It avoids stale data
# being left in the installed DLB. It does NOT install `nhdat` for you.
if [ ! -f dungeon ] || [ "dungeon.pdf" -nt "dungeon" ]; then
    echo "Regenerating dat/dungeon from dat/dungeon.pdf..."
    ../util/dgn_comp dungeon.pdf
    echo "Note: If you want the runtime to use the new data, run 'make dlb' and then 'make install' or copy dat/nhdat to your HACKDIR with a backup."
fi

# Run level compiler from dat so output files are created inside dat/
../util/lev_comp *.des

# Back to repo root and configure/build
cd ../sys/unix
sh setup.sh hints/linux
cd ../..
make -j$(nproc)

# Optionally: install (uncomment if you want to install locally)
# make install
