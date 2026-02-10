#!/usr/bin/env bash  
# Test save/restore under valgrind
set -euo pipefail

NETHACK="$(pwd)/src/nethack"
SAVEDIR=~/vnh366/install/games/lib/nethackdir/save
USER="vtest"

echo "=== Cleaning old saves ==="
rm -f "$SAVEDIR"/*"$USER"* || true

echo ""
echo "=== Test 1: Save game with valgrind ==="
rm -f valgrind_save.log
timeout 30 valgrind --leak-check=full --track-origins=yes --log-file=valgrind_save.log \
    "$NETHACK" -u "$USER" <<'SAVEINPUT' || true
n
n
archeologistq


M
a
Test note from save


S
y

SAVEINPUT

echo ""
echo "Save file created?"
ls -lh "$SAVEDIR"/*"$USER"* 2>&1 || echo "NO SAVE FILE FOUND!"

echo ""  
echo "Valgrind save errors?"
grep "ERROR SUMMARY" valgrind_save.log || echo "No summary"
grep -E "Invalid|uninitialised" valgrind_save.log | head -5 || echo "No critical errors"

echo ""
echo "=== Test 2: Restore game with valgrind ==="
rm -f valgrind_restore.log  
timeout 30 valgrind --leak-check=full --track-origins=yes --log-file=valgrind_restore.log \
    "$NETHACK" -u "$USER" <<'RESTOREINPUT' || true



M


q
y

RESTOREINPUT

echo ""
echo "Valgrind restore errors?"
grep "ERROR SUMMARY" valgrind_restore.log || echo "No summary"
grep -E "Invalid|uninitialised" valgrind_restore.log | head -5 || echo "No critical errors"

echo ""
echo "=== Test complete ==="
echo "Full logs: valgrind_save.log, valgrind_restore.log"
