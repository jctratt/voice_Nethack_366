#!/bin/bash
# Clear debug log in workspace-contained install dir
LOG_FILE="/home/neon/vnh366/install/games/lib/nethackdir/nethack_debug.log"
rm -f "$LOG_FILE"
echo "Debug log cleared ($LOG_FILE). Now:"
echo "1. Start nethack: /home/neon/vnh366/install/games/nethack"
echo "2. Press #notes"
echo "3. Add a note and type something"
echo "4. Try Ctrl+Right and Ctrl+Left"
echo "5. Press F2 to save"
echo "6. Exit nethack"
echo ""
echo "Then run: tail -n 100 $LOG_FILE"
echo "to see notes editor key handling events"
