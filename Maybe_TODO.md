# Maybe_TODO — Combined list

Maintainer: user:Jeff
Created: 2026-02-10

This file is a consolidated, actionable list combining the repository-wide bug/feature TODOs and the three gameplay items reported by the user. Each entry lists type, files to inspect, reproduction or next steps, and priority.

---

## 🔴 P0 — Critical / High priority

1. **Add automated tests for notes save/restore** 🔧
- Files: `test_save_restore.c`, `src/notes.c`
- Description: Convert the ad-hoc test into an automated test that verifies non-zero `note_count`, note contents, empty/long/multibyte notes, and exit status.
- Next steps: Create tests in repo's test harness, run under valgrind, add regression check.

2. **Fix potential memory leaks / double-free in notes handling** 🧯
- Files: `src/notes.c`, tests
- Description: Audit allocations/frees for `u.note_list` and note strings; add valgrind checks; ensure correct cleanup on save/restore errors and program exit.

3. **#intrinsics tracking must NOT reveal item sources without enlightenment** ⚠️
- Type: Bug / Gameplay
- Files: `src/intrinsics.c` (or similar), `src/invent.c`
- Description: `#intrinsics` should list known intrinsics but must not reveal which item grants them unless the player has been legitimately enlightened (level-up, identification, etc.). Revealing item sources without enlightenment is cheating.
- Next steps: Add regression tests asserting no item source info is shown without the enlightenment flag; implement fix.

4. **Investigate and handle `addinv` panics (merge/quiver edge cases)** ⚠️
- Files: `src/invent.c`
- Description: Reproduce and add defensive handling for panic scenarios (e.g., `panic("addinv: obj not free")` and null merges). Add unit tests that reproduce merge/quiver panic cases.

5. **Add CI workflow to run tests + `tools/run_valgrind.sh`** 🔁
- Files: `.github/workflows/ci.yml` (new)
- Description: Automate building, running tests, and valgrind leak checks. Fail CI if valgrind finds leaks or tests fail.

---

## 🟠 P1 — Important but not immediate

6. **Harden save/restore I/O error handling and compatibility** 💾
- Files: `src/save.c`, `src/restore.c`, `nethack_save_file_guide.md`
- Description: Add robust error checks for read/write failures, corrupted/partial files, and tests for truncated files.

7. **Add test that reproduces reported panics in `sticky` patches** 🐞
- Files: `sticky.diff`, `src/invent.c`
- Description: Add regression tests for `panic("addinv: obj not free")` reported historically.

8. **Inventory: show per-line weight when `F1` is pressed** ✨
- Type: Feature (from user)
- Files: `src/invent.c`, `src/cmd.c`, UI `src/win_*` or curses window code
- Description: Toggle a weight display in the inventory listing when the user presses `F1`. For stacks, show the total stack weight (e.g., `i) 3 swirly potions [60]`). Do not change default display.
- Next steps: Prototype `F1` handler and a manual/automated test for UI.

9. **Add tests / fixes for inventory merge edge cases and artifacts** 🧪
- Files: `src/invent.c`, `src/artifact.c`
- Description: Cover edge cases (wield/quiver merges, sticky invlets, artifact interactions) with unit tests.

---

## 🟡 P2 — Enhancements and lower-priority changes

10. **Centralize/debug logging & cleanup repeated debug scaffolding** 🗂️
- Files: `src/*.c`, debug helpers (e.g., `curses_debug_log`)
- Description: Standardize log levels, helper functions, and cleanup inconsistent debug prints; add a verbose toggle for tests.

11. **Verify compressor (`COMPRESS`) handling in build/test scripts** 🛠️
- Files: `Makefile`, `compile-all.sh`, `grok_conversation.txt`
- Description: Add checks and fallback behavior if compressor tools are missing; add CI matrix with/without compressor tools.

12. **Remove / revisit legacy savefile compatibility TODO** 📦
- Files: `src/restore.c`
- Description: Evaluate whether legacy code blocks (kept for 3.6.x compatibility) can be removed, or add tests documenting expected behavior if kept.
