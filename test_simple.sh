#!/bin/bash
set -x

# Clean up
rm -f /home/neon/vnh366/install/games/lib/nethackdir/save/*
rm -f /home/neon/vnh366/install/games/lib/nethackdir/nethack_debug.log

# Create commands file
cat > /tmp/nh_commands.txt << 'EOF'
testplayer
v
y
#invoke
shock resistance
#notes
a
This is my test note
.
S
y
EOF

# Run nethack with commands piped in
cd /home/neon/vnh366/install/games
cat /tmp/nh_commands.txt | timeout 10 ./nethack -D 2>&1 | tail -20

echo "===== DEBUG LOG ====="
tail -50 /home/neon/vnh366/install/games/lib/nethackdir/nethack_debug.log || echo "No log file"
