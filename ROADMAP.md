# ROADMAP — project housekeeping & priorities
This file holds long-running context, process notes, and the prioritized TODO list. Keep `CLAUDE.md` short; use this file for the details.

## Housekeeping guidance
- When conversation/context grows to >50% of a feature or bug, update this roadmap and consolidate relevant context here.
- Do not let housekeeping consume more time than necessary; focus on code and prioritized tasks.
- Keep local WIP commits for historical context, but separate them from changes intended for `main` or release branches.
- Prefer persistent workspace paths for saves/logs (example: `~/vnh366/install/games/lib/nethackdir/save`) rather than `/tmp` for CI and reproducibility; however this will be lost on make install.
- Use indexing/consolidation when archiving large amounts of conversational/context data.

## Current priorities (ordered)
1. Stabilize `main`: verify latest `main` builds, installs, and passes smoke tests; prioritize fixing regressions that affect the deployed code (high).
2. Reproduce and triage any save/restore failures on `main` only — collect user-run reproducer traces and `nethack_debug.log` for each failure (high).
3. Harden CI to run deterministic smoke tests against `main` (`tools/test_save_restore_integrity.sh`, `tools/test_notes_fkeys.exp`, `tools/run_valgrind.sh`) and fail fast on regressions (medium).
4. Archive `feature/borked` (forensic/archive only); attach any repro artifacts and mark it explicitly as abandoned (low).
5. Add `CONTRIBUTING.md` + onboarding checklist for new contributors (low).

## How to use / update this file
- Edit this file and include a short summary + PR/commit link when updating priorities or consolidating context.
- For long running investigations, add a short status entry and reference any reproducer saves or logs (save files live under `~/vnh366/install/games/lib/nethackdir/save`).

## Quick references
- Build: `./compile-all.sh` or `make clean && make all`
- Tests: `tools/test_notes_fkeys.exp`, `tools/test_save_restore_integrity.sh`, `tools/run_valgrind.sh`
- Developer notes: `file_list.txt`, `nethack_save_file_guide.md`, `README`

---
Last updated: 2026-02-14
