#!/bin/bash
set -x

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CMD_FILE="$SCRIPT_DIR/tmp/nh_commands.txt"
mkdir -p "$SCRIPT_DIR/tmp"

# Clean up
rm -f /home/neon/vnh366/install/games/lib/nethackdir/save/*
rm -f /home/neon/vnh366/install/games/lib/nethackdir/nethack_debug.log

# Create commands file
cat > "$CMD_FILE" << 'EOF'
testplayer
v
y
#invoke
shock resistance

# exercise #name submenu: intrinsics, notes, and price id
# open name menu, select I then quit
# (the game will log the intrinsics menu header)
# open again, select N then quit
# (the game will log the notes menu header)
# open again, select P then quit
# (the game will log the price-id chooser header)
# an actual note follows to exercise #notes itself
#
#name
I
q
#name
N
q
#name
P
q

# now exercise notes command directly
#notes
a
This is my test note
.
S
y
EOF

# Run nethack with commands piped in
cd /home/neon/vnh366/install/games
cat "$CMD_FILE" | timeout 10 ./nethack -D 2>&1 | tail -20

echo "===== DEBUG LOG ====="
grep -e "Toggle which intrinsics to track" -e "Notes" -e "Price ID - choose" /home/neon/vnh366/install/games/lib/nethackdir/nethack_debug.log || echo "missing expected log entries"
tail -50 /home/neon/vnh366/install/games/lib/nethackdir/nethack_debug.log || echo "No log file"
