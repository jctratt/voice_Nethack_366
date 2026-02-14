#!/usr/bin/env bash
# Dynamic check: save then restore while tracing SAVE-FIELD / RESTORE-FIELD entries
# Produces a diff if sequences diverge.
set -euo pipefail

NETHACK_BIN="/home/neon/vnh366/install/games/nethack"  # wrapper
DEBUGLOG="/home/neon/vnh366/install/games/lib/nethackdir/nethack_debug.log"
SAVEDIR="/home/neon/vnh366/install/games/lib/nethackdir/save"
PLAYER="dyncheck"

rm -f "$DEBUGLOG" /tmp/save_fields.log /tmp/restore_fields.log || true
rm -f "$SAVEDIR"/*"$PLAYER"* || true

# create a quick character and save
timeout 30 script -q -c "$NETHACK_BIN -u $PLAYER" /dev/null <<'EOF' || true
n
n
archeologistq

M
a
DYNAMIC_CHECK
.
S
y
EOF

# restore the same character (non-interactive)
# Use -D to point to the database dir (wrapper already uses HACKDIR)
timeout 60 script -q -c "$NETHACK_BIN -u $PLAYER -D /home/neon/vnh366/install/games/lib/nethackdir" /dev/null <<'EOF' || true
r
$PLAYER
S
y
EOF

# extract SAVE-FIELD and RESTORE-FIELD lines (if present)
# redirect grep output into temp files (put || true after the redirection)
grep -F "SAVE-FIELD" "$DEBUGLOG" > /tmp/save_fields.log 2>/dev/null || true
grep -F "RESTORE-FIELD" "$DEBUGLOG" > /tmp/restore_fields.log 2>/dev/null || true

if [ ! -s /tmp/save_fields.log ] || [ ! -s /tmp/restore_fields.log ]; then
    echo "WARN: no dynamic SAVE-FIELD/RESTORE-FIELD entries found in $DEBUGLOG"
    echo "This usually means instrumentation isn't enabled in the running binary."
    echo "Please re-build and install the development binary from this tree before retrying."
    exit 2
fi

# normalize by stripping timestamps and prefixes
sed -E 's/^.*SAVE-FIELD: ?//' /tmp/save_fields.log > /tmp/save_fields.norm
sed -E 's/^.*RESTORE-FIELD: ?//' /tmp/restore_fields.log > /tmp/restore_fields.norm

if diff -u /tmp/save_fields.norm /tmp/restore_fields.norm >/tmp/save_restore.diff 2>&1; then
    echo "OK: dynamic save/restore sequences matched"
    rm -f /tmp/save_restore.diff
    exit 0
else
    echo "ERROR: save/restore field sequence mismatch -> /tmp/save_restore.diff"
    sed -n '1,200p' /tmp/save_restore.diff
    exit 3
fi
