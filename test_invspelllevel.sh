#!/bin/bash
# Smoke test / manual instructions for inventory spellbook level display
# Usage: run this script then follow instructions printed below.

LOG_FILE="/home/neon/vnh366/install/games/lib/nethackdir/nethack_debug.log"
rm -f "$LOG_FILE"
echo "Debug log cleared ($LOG_FILE). Now:"
echo "1. Start nethack: /home/neon/vnh366/install/games/nethack"  

echo "2. Acquire a spellbook and identify it (read it normally with 'r')."
echo "   The level is shown once the book is known; you do **not** need wizard"
echo "   privileges, though wizard mode can simplify setup if desired."
echo "3. Press 'i' to inspect your inventory; a known spellbook entry should"
echo "   include its level immediately before the spell name (e.g.":

echo "      spellbook of L3 cure sickness [50]"
echo "   Unidentified books and novels do not show a level."
echo "4. (optional) Run '#inventory' (wizard or normal) and check $LOG_FILE for a"
echo "   plain-text listing containing the level suffix."
echo "5. Quit nethack and verify the log if you used #inventory:"
echo "   grep -n '\\[L[1-7]\\]' $LOG_FILE"
echo ""  
echo "This script is a smoke test; manual verification is required."