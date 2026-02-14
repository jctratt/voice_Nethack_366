#!/usr/bin/expect -f
# Regression test: verify development binary can restore the preserved 1000neon.gz.jeff save
# - does not modify or delete the original .jeff (read-only verification)
# - exits 0 on success, non-zero on failure

set timeout 20
set nethack_bin "/home/neon/source/voice_Nethack_366/src/nethack"
set save_dir "/home/neon/vnh366/install/games/lib/nethackdir"
set player "neon"

# Spawn the dev binary with the save directory overridden
spawn env NETHACKDIR=$save_dir $nethack_bin -u $player

expect {
    -re "Restoring save file" {
        # first menu: select saved game -> send the default/first choice
        send "\r"
        exp_continue
    }
    -re "Read .* instead of .* bytes\.|Error restoring old game\." {
        puts "RESTORE-ERROR: detected restore failure"
        exit 1
    }
    -re "Shall I call you|pick the role of|@|\$" {
        # restore succeeded and we've reached an in-game prompt
        puts "RESTORE-OK: save restored successfully"
        send "#quit\r"
        expect eof
        exit 0
    }
    timeout {
        puts "RESTORE-ERROR: timed out waiting for restore result"
        exit 2
    }
}
