#!/usr/bin/env bash
# Helper: run Nethack under valgrind and capture full logs to workspace
# Usage: ./tools/run_valgrind.sh [nethack-args]
set -euo pipefail
LOG="$(pwd)/valgrind.log"
CORE_GLOB=core

echo "Valgrind log: $LOG"
rm -f "$LOG" $CORE_GLOB || true

valgrind --tool=memcheck \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --num-callers=50 \
    --log-file="$LOG" \
    ./src/nethack "$@" || true

# If a core file was generated, capture a gdb backtrace into the same log
if ls $CORE_GLOB 1>/dev/null 2>&1; then
    echo "\n=== GDB backtraces (captured core files) ===\n" >> "$LOG"
    for c in $CORE_GLOB; do
        echo "--- backtrace for $c ---" >> "$LOG"
        gdb -batch -ex "set pagination off" -ex "thread apply all bt full" -ex "quit" ./src/nethack "$c" >> "$LOG" 2>&1 || true
    done
fi

echo "Done. Full output written to: $LOG"
