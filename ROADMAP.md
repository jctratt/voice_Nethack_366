# ROADMAP — project housekeeping & priorities
This file holds long-running context, process notes, and the prioritized TODO list. Keep `CLAUDE.md` short; use this file for the details.

## Current priorities (ordered)


## How to use / update this file
- Edit this file and include a short summary + PR/commit link when updating priorities or consolidating context.
- For long running investigations, add a short status entry and reference any reproducer saves or logs (save files live under `~/vnh366/install/games/lib/nethackdir/save`).

## Workspace path policy (AI + tooling)
- Keep all generated files inside the workspace.
- Use `./tmp` for transient temp/build artifacts (and `TMPDIR=$PWD/tmp` when needed).
- Treat `./tmp` as disposable scratch; deleting it is always allowed and build scripts should recreate it.
- Do not write debug logs or test artifacts to `/tmp`.
- Use `~/vnh366/install/games/lib/nethackdir/` (workspace path) for persistent game logs/saves.

## Quick references
- Build: `./compile-all.sh` or `make clean && make all`
- Build with local temp dir: `mkdir -p tmp && TMPDIR=$PWD/tmp make -j6`
- Tests: `tools/test_notes_fkeys.exp`, `tools/test_save_restore_integrity.sh`, `tools/run_valgrind.sh`
- Developer notes: `file_list.txt`, `nethack_save_file_guide.md`, `README`

---
Last updated: 2026-02-14
