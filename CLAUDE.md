# NetHack Voice Commands Project

## Build Commands
- Setup and Compile: `cd sys/unix/ && sh setup.sh hints/linux && cd ../.. && make all`
- Clean Build: `make clean && make all`
- Voice Feature Script: `./voice.sh` (Extracts VOICE_ENABLED sections from code)

## Code Information
- This is a modified NetHack 3.6.6 with voice command functionality
- Voice commands are wrapped in `#ifdef VOICE_ENABLED` blocks
- Key files are listed in `file_list.txt`: flag.h, decl.h, pline.c, files.c, decl.c, options.c

## Code Style
- Follow existing NetHack coding conventions
- Use K&R style with 4-space indentation
- Keep the VOICE_ENABLED sections clearly marked and isolated
- Don't modify core NetHack functionality outside of voice features

## Testing
- Test the game by running `src/nethack` after building
- Verify voice commands work in-game by testing various voice interactions