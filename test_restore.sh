#!/bin/bash
export NETHACK=/home/neon/vnh366/install/games/nethack
SAVE_DIR=/home/neon/vnh366/install/games/lib/nethackdir/save
DEBUG_LOG=/home/neon/vnh366/install/games/lib/nethackdir/nh_debug.log

# Clean up first
rm -f "$SAVE_DIR"/* "$DEBUG_LOG"
# Create a simple test
expect << 'TESTEOF'
spawn $env(NETHACK) -D
expect "shall I call you?"
send "testplayer\r"
expect "pick the role of"
send "a\r"
expect "Are you"
send "y\r"
expect "@"
send "#invoke\r"
expect "intrinsic"
send "shock resistance\r"
expect "@"
send "#notes\r"
expect "*"
send "a\r"
expect "text:"
send "Test note 1\r\r"
expect "@"
send "S\r"
expect "game"
send "y\r"
expect eof
TESTEOF

# Now try to restore
expect << 'TESTEOF'
spawn $env(NETHACK)
expect "Restoring save file"
sleep 1
send "q\r"
sleep 1
send "y\r"
expect eof
TESTEOF

echo "Test completed"
