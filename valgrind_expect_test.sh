#!/bin/bash
set -e

NETHACK=/home/neon/source/voice_Nethack_366/src/nethack
SAVEDIR=~/vnh366/install/games/lib/nethackdir/save

# Clean saves
rm -f $SAVEDIR/*testplayer*

echo "=== SAVE TEST with valgrind ==="
expect << 'SAVETEST'
set timeout 30
spawn valgrind --leak-check=full --track-origins=yes --log-file=valgrind_save.log /home/neon/source/voice_Nethack_366/src/nethack -u testplayer
expect "shall I call you?"
send "testplayer\r"
expect "pick the role"
send "archeologistq\r"
expect "@"
send "S"
expect "save"
send "y\r"
expect eof
SAVETEST

echo ""
echo "=== Save file? ==="
ls -lh $SAVEDIR/

echo ""  
echo "=== Valgrind SAVE summary ==="
grep "ERROR SUMMARY" valgrind_save.log

echo ""
echo "=== RESTORE TEST with valgrind ==="
expect << 'RESTORETEST'
set timeout 30
spawn valgrind --leak-check=full --track-origins=yes --log-file=valgrind_restore.log /home/neon/source/voice_Nethack_366/src/nethack -u testplayer
expect {
    "Restoring" { send "\r" }
    "@" { }
    timeout { puts "Timeout waiting for restore"; exit 1 }
}
expect "@"
send "q"
expect "quit"
send "y\r"
expect eof
RESTORETEST

echo ""
echo "=== Valgrind RESTORE summary ==="
grep "ERROR SUMMARY" valgrind_restore.log

echo ""
echo "=== TEST COMPLETE ==="
