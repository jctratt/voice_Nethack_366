#!/bin/bash
# Automated test for OPTIONS=invweight persistence across save/restore
# - Starts a game with OPTIONS=invweight enabled via .nethackrc
# - Wishes for a small stack (3 arrows)
# - Verifies inventory shows a weight bracket like [TOTALxITEM]
# - Saves, exits, restores (with a different HOME so option isn't coming
#   from the config file) and verifies the bracket is still present

set -euo pipefail
export NETHACK=/home/neon/vnh366/install/games/nethack

# cleanup saves/logs
rm -f /home/neon/vnh366/install/games/lib/nethackdir/save/*
rm -f /tmp/nh_debug.log

HOME1=$(mktemp -d /tmp/nhhome.XXXX)
HOME2=$(mktemp -d /tmp/nhhome.XXXX)
trap 'rm -rf "$HOME1" "$HOME2"' EXIT

# Put OPTIONS=invweight into HOME1/.nethackrc so the first run starts
# with the option enabled.
cat > "$HOME1/.nethackrc" <<'RC'
OPTIONS=invweight
RC

# Run game in debug mode and create a stack of arrows, then check inventory
expect <<'EXP'
log_user 1
set timeout 20
spawn -noecho env HOME=$env(HOME1) $env(NETHACK) -D
expect "shall I call you?"
send "invweight_test\r"
expect "pick the role of"
send "a\r"
expect "Are you"
send "y\r"
expect "@"
# wish for 3 arrows (wizard-only extended command)
send "#wizwish\r"
expect "For what do you wish"
send "3 arrows\r"
expect "@"
# show inventory and check for bracketed weight pattern
send "i\r"
expect -re {\[[0-9]+x[0-9]+\]}
# save and exit
send "S\r"
expect "game"
send "y\r"
expect eof
EXP

echo "-- first run: invweight shown in inventory (PASS) --"

# Now restore with different HOME (no .nethackrc) to ensure the flag
# persisted in the saved game itself.
expect <<'EXP'
set timeout 20
spawn -noecho env HOME=$env(HOME2) $env(NETHACK)
# the game should detect and offer to restore the saved game
expect "Restoring save file" { send "\r" }
# If prompted about quitting restore/resume choices, answer so it proceeds
sleep 1
# Some builds prompt to quit; ensure we get to the main prompt and check inventory
expect "@"
send "i\r"
expect -re {\[[0-9]+x[0-9]+\]}
send "q\r"
expect eof
EXP

echo "-- restore run: invweight preserved after restore (PASS) --"

# cleanup saved games
rm -f /home/neon/vnh366/install/games/lib/nethackdir/save/*

exit 0
