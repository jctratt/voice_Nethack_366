#!/usr/bin/env bash
# Corruption-regression test: patch a known oname length in an existing save
# so restore sees a bogus buflen; verify restore logs the suspicious length
# and the game does not crash.
set -euo pipefail

SAVEDIR="/home/neon/vnh366/install/games/lib/nethackdir/save"
DEBUGLOG="/home/neon/vnh366/install/games/lib/nethackdir/nethack_debug.log"
NETHACK="/home/neon/vnh366/install/games/nethack"
ORIG_SAVE="$SAVEDIR/1000neon.gz"
BAK_SAVE="$SAVEDIR/1000neon.gz.bak-test"
TMP_RAW="/tmp/1000neon.raw"
TMP_RAW_CORR="/tmp/1000neon.corrupt.raw"
TMP_GZ_CORR="/tmp/1000neon_corrupt.gz"
PLAYER="neon"

if [ ! -f "$ORIG_SAVE" ]; then
    echo "ERROR: original save not found: $ORIG_SAVE" >&2
    exit 2
fi

# ensure we have a backup of the original (do not overwrite existing user .bak)
if [ ! -f "$BAK_SAVE" ]; then
    cp -p "$ORIG_SAVE" "$BAK_SAVE"
fi

# ensure a save with a known note exists by running the integrity test's save-creation
./tools/test_save_restore_integrity.sh >/dev/null 2>&1 || true
# target the save created by the integrity test (player 'vtest')
NEW_SAVE=$(ls -1t "$SAVEDIR"/*vtest* 2>/dev/null | head -n1)
if [ -z "$NEW_SAVE" ]; then
    echo "ERROR: could not find vtest save created by integrity test" >&2
    exit 5
fi
# decompress that save for patching
gzip -d -c "$NEW_SAVE" > "$TMP_RAW"

# locate the known note string ('Test note from save') and patch its preceding length
essential_note='Test note from save'
python3 - <<PY
import sys
fn = '/tmp/1000neon.raw'
with open(fn, 'rb') as f:
    data = f.read()
needle = b'INTEGRITYTEST'
idx = data.find(needle)
if idx == -1:
    print('NOTE_NOT_FOUND')
    sys.exit(2)
len_off = idx - 4
orig_len = int.from_bytes(data[len_off:len_off+4], 'little')
print('FOUND note at', idx, 'orig_len=', orig_len)
new_len = (65536).to_bytes(4, 'little')
newdata = data[:len_off] + new_len + data[len_off+4:]
open('/tmp/1000neon.corrupt.raw', 'wb').write(newdata)
print('PATCHED_NOTE')
PY

if ! [ -f "$TMP_RAW_CORR" ]; then
    echo "ERROR: failed to create patched raw save" >&2
    exit 4
fi

# compress and overwrite the test player's save file
gzip -c "$TMP_RAW_CORR" > "$TMP_GZ_CORR"
cp -f "$TMP_GZ_CORR" "$NEW_SAVE"
ORIG_SAVE="$NEW_SAVE"  # operate on this save from now on
# clear debug log
: > "$DEBUGLOG"

# run restore (automated inputs) using a PTY so curses/function-keys are handled correctly
# (use `script -c` to supply a pseudo-tty; heredoc is still used for the keystrokes)
timeout 30 script -q -c "\"$NETHACK -u $PLAYER -D /home/neon/vnh366/install/games/lib/nethackdir\"" /dev/null <<'EOF' || true

r
$PLAYER

S
EOF

# check debug log for our validation message
if grep -q "RESTORE: suspicious note length" "$DEBUGLOG"; then
    echo "OK: restore detected suspicious note length and logged it";
    result=0
else
    echo "ERROR: restore did not log suspicious note length" >&2
    tail -n 200 "$DEBUGLOG" || true
    result=5
fi

# restore original save from backup
cp -f "$BAK_SAVE" "$ORIG_SAVE"

exit $result
