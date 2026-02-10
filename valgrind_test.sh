#!/bin/bash
set -e

NETHACK=/home/neon/source/voice_Nethack_366/src/nethack
SAVEDIR=~/vnh366/install/games/lib/nethackdir/save

# Clean old saves
rm -f $SAVEDIR/*testplayer*

echo "=== Testing SAVE with valgrind ==="
valgrind --leak-check=full --track-origins=yes --log-file=valgrind_save.log \
  $NETHACK -u testplayer <<'EOF' 2>&1 | tail -5
n
n
archeologistq
S
y

EOF

echo ""
echo "=== Save file created? ==="
ls -lh $SAVEDIR/ 2>&1

echo ""
echo "=== Valgrind save errors? ==="
grep -E "ERROR SUMMARY|Invalid|uninitialised|Conditional jump" valgrind_save.log | head -10

echo ""
echo "=== Testing RESTORE with valgrind ==="
valgrind --leak-check=full --track-origins=yes --log-file=valgrind_restore.log \
  $NETHACK -u testplayer <<'EOF' 2>&1 | tail -5
q
y

EOF

echo ""
echo "=== Valgrind restore errors? ==="
grep -E "ERROR SUMMARY|Invalid|uninitialised|Conditional jump" valgrind_restore.log | head -10

echo ""
echo "=== Test complete ==="
