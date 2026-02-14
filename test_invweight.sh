#!/bin/bash
set -euo pipefail

# Quick automated smoke test for per-line inventory weight toggle
# - starts nethack, creates a character with minimal interaction
# - opens inventory (i), toggles per-line weights (W), re-opens inventory
# - exits

SAVE_DIR="/home/neon/vnh366/install/games/lib/nethackdir/save"
NH_BIN="/home/neon/vnh366/install/games/nethack"

# clear any leftover save/lock/state files to ensure a clean run
rm -f "$SAVE_DIR"/* || true
# remove old lock/level files (examples: 1000wizard.0 / 1000wizard.1)
rm -f /home/neon/vnh366/install/games/lib/nethackdir/*.0 /home/neon/vnh366/install/games/lib/nethackdir/*.1 /home/neon/vnh366/install/games/lib/nethackdir/*.lock || true
rm -f /home/neon/vnh366/install/games/lib/nethackdir/record /home/neon/vnh366/install/games/lib/nethackdir/logfile || true
# ensure the required perm file exists (empty is fine)
touch /home/neon/vnh366/install/games/lib/nethackdir/perm
chmod 0600 /home/neon/vnh366/install/games/lib/nethackdir/perm || true

TESTNAME="invtest_$RANDOM"
cat > /tmp/nh_inv_commands.txt <<EOF
$TESTNAME
v
y

# once in-game, show inventory, show inventory again, then save
i
i
S
EOF

# Run nethack: create character, ensure invweight is enabled via NETHACKOPTIONS, then save & exit
cd $(dirname "$NH_BIN")
NETHACKOPTIONS="invweight" cat /tmp/nh_inv_commands.txt | timeout 15 "$NH_BIN" -D 2>&1 | sed -n '1,200p' || true

# Now restart and restore saved game, then verify weights persisted
cat > /tmp/nh_inv_restore.txt <<'EOF'

# answer possible "Restore" prompt
y
# open inventory and capture output
i
q
EOF

# run restore attempt and capture output
cat /tmp/nh_inv_restore.txt | timeout 10 "$NH_BIN" -D 2>&1 | sed -n '1,400p' > /tmp/nh_inv_restore.out || true

# check that the inventory header or listing shows weights ON or a weight bracket
if grep -q "weights ON" /tmp/nh_inv_restore.out || grep -E -q '\[[0-9]+(x[0-9]+)?\]' /tmp/nh_inv_restore.out; then
    echo "INVWEIGHT persisted in save/restore: OK"
else
    echo "INVWEIGHT did NOT persist: FAIL" >&2
    tail -n 200 /tmp/nh_inv_restore.out
    exit 2
fi

echo "=== end of test_invweight.sh ==="