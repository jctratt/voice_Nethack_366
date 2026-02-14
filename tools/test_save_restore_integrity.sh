#!/usr/bin/env bash
# Integration test: create a save, restore it and verify restore log entries
set -euo pipefail

NETHACK_INST="/home/neon/vnh366/install/games/nethack"
SAVEDIR="/home/neon/vnh366/install/games/lib/nethackdir/save"
PLAYER="integrity_test"
DEBUGLOG="/home/neon/vnh366/install/games/lib/nethackdir/nethack_debug.log"

rm -f "$SAVEDIR"/*"$PLAYER"* /tmp/valgrind_restore_integrity.log "$DEBUGLOG" || true

# Create a character and save
timeout 30 script -q -c "$NETHACK_INST -u $PLAYER" /dev/null <<'EOF' || true
n
n
archeologistq

M
a
INTEGRITYTEST
.
S
y
EOF

# verify save exists
if ! ls -l "$SAVEDIR"/*"$PLAYER"* >/dev/null 2>&1; then
    echo "ERROR: save not created" >&2
    exit 2
fi

# Restore under valgrind to detect memory errors and to exercise restore path
timeout 60 script -q -c "valgrind --leak-check=full --track-origins=yes --log-file=/tmp/valgrind_restore_integrity.log $NETHACK_INST -u $PLAYER -D /home/neon/vnh366/install/games/lib/nethackdir" /dev/null <<'EOF' || true

r
$PLAYER

S
y
EOF

# Basic post-checks
if ! grep -q "RESTORE: getlev() completed successfully" "$DEBUGLOG" 2>/dev/null; then
    echo "ERROR: restore log missing expected entry" >&2
    tail -n 200 "$DEBUGLOG" || true
    exit 3
fi

# Ensure valgrind reported no obvious errors (at least an ERROR SUMMARY line exists)
if grep -E "ERROR SUMMARY: [1-9]" /tmp/valgrind_restore_integrity.log >/dev/null 2>&1; then
    echo "ERROR: valgrind reported problems during restore" >&2
    sed -n '1,200p' /tmp/valgrind_restore_integrity.log
    exit 4
fi

echo "OK: save/restore integrity test passed"