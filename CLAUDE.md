# NetHack Voice Commands Project

## Build Commands
- newer copile-all.sh has created to replace the following lines when more than just make is needed
- see compile-all.sh (Setup and Compile: `cd sys/unix/ && sh setup.sh hints/linux && cd ../.. && make all`)
- Clean Build: `make clean && make all`
- Voice Feature Script: `./voice.sh` (Extracts VOICE_ENABLED sections from code)

## Code Information
- This is a modified NetHack 3.6.6 with voice command functionality
- Voice commands are wrapped in `#ifdef VOICE_ENABLED` blocks
- Key files are listed in `file_list.txt`: flag.h, decl.h, pline.c, files.c, decl.c, options.c

## Code Style
- Follow existing NetHack coding conventions
- Use K&R style with 4-space indentation
- Keep the VOICE_ENABLED sections clearly marked and isolated; there is a compile time flag

## Testing
- Test the game by running `src/nethack` after building; it is an interative game and may need keyboard input to progress
- valgrind is installed and can be used to test for memory leaks and other issues
- Use the `tools/run_valgrind.sh` script to run the game under valgrind and capture logs to the workspace