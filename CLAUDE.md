# NetHack Voice Commands Project
Hi, AI, I'm user Jeff!

## Build Commands
- A newer `compile-all.sh` is available to run setup + compile when more than `make` is required (e.g. changed options or updated documentation). Example: `cd sys/unix && sh setup.sh hints/linux && cd ../.. && make all`.
- Clean build: `make clean && make all`

## Tools
- /tools contains various tools to help with development
- /tools may be specific to a previous commit and should not just be used blindly without knowing its purpose and understanding the current state of the project and if it can produce the desired results.  It is provided as a reference and starting point for understanding how to use and create tools to help with development.

## Code Information
- This is a modified NetHack 3.6.6 with voice command functionality. This feature has been completed.
- Voice commands are wrapped in `#ifdef VOICE_ENABLED` blocks
- To enable voice features at build time, define `VOICE_ENABLED` (for example add `-DVOICE_ENABLED` to `CFLAGS` or the Makefile).
- Key files are listed in `file_list.txt`: flag.h, decl.h, pline.c, files.c, decl.c, options.c

### Recent change — `invweight` (2026-02-14)
- Added runtime option `OPTIONS=invweight` (persistent) to show per-item inventory weight in the inventory display in the format `[TOTALxITEM]` (e.g. `[60x20]`).
- Files changed: `include/flag.h`, `src/options.c`, `src/invent.c`, `dat/opthelp`, `include/patchlevel.h`.
- Savefile impact: `EDITLEVEL` incremented due to `struct flag` layout change; older savefiles/bones will be invalidated.
- Tests: removed an expect-based test and prepared for a compliant unit-style replacement.
- Usage: `#set invweight` or add `OPTIONS=invweight` to your `.nethackrc`.


## Code Style
- Follow existing NetHack coding conventions
- Use K&R style with 4-space indentation
- Keep the VOICE_ENABLED sections clearly marked and isolated; there is a compile time flag

## Testing
- Do not run the game yourself, it uses curses and cannot see piped input, so prompt Jeff for the desired results you want from the game and Jeff will run it for you; he's very helpful. For example, "test save and restore" or "test the #notes by adding a note 'test note' and save and restore"
- valgrind is installed and can be used to test for memory leaks and other issues
- Use the `tools/run_valgrind.sh` script to run the game under valgrind and capture logs to the workspace
- avoid using /tmp as it is outside the workspace; prefer the ~/vnh366/install/games/lib/nethackdir/save directory for saves and logs.

## Housekeeping guidance
- When conversation/context grows to >50% of a feature or bug, update this roadmap and consolidate relevant context here.
- Do not let housekeeping consume more time than necessary; focus on code and prioritized tasks.
- Keep local WIP commits for historical context, but separate them from changes intended for `main` or release branches.
- Prefer persistent workspace paths for saves/logs (example: `~/vnh366/install/games/lib/nethackdir/save`) rather than `/tmp` for CI and reproducibility; however this will be lost on make install.
- Use indexing/consolidation when archiving large amounts of conversational/context data.
- `/compact` is the command to run.

## ROADMAP.md
- See `ROADMAP.md` for long-running context, priorities and housekeeping — keep `CLAUDE.md` intentionally short.
