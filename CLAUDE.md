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

## Code Style
- Follow existing NetHack coding conventions
- Use K&R style with 4-space indentation
- Keep the VOICE_ENABLED sections clearly marked and isolated; there is a compile time flag

## Testing
- Use Expect scripts (e.g. `tools/test_notes_fkeys.exp`) or `tools/test_save_restore_integrity.sh` for automated UI tests.
- Run Valgrind with `tools/run_valgrind.sh` to capture logs.
- Prefer workspace save/log paths (`~/vnh366/install/games/lib/nethackdir/save`) over `/tmp` for CI.

## ROADMAP.md
- See `ROADMAP.md` for long-running context, priorities and housekeeping — keep `CLAUDE.md` intentionally short.
