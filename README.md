# Voice-Enabled NetHack 3.6.6

A modified version of NetHack 3.6.6 with text-to-speech capabilities using espeak, making the game more accessible and immersive through voice output.

## Features

### Voice Commands (Primary Feature)
- **Text-to-Speech Integration**: Game messages are spoken aloud using espeak
- **Customizable Voice Settings**: Configure speech rate, pitch, volume, and language
- **Smart Filtering**: Regex-based system to control which messages are spoken
  - `VOICE_EXCEPTION`: Define patterns for messages that should NOT be spoken
  - `VOICE_FORCE`: Define patterns for messages that should be spoken with custom formatting
- **Cross-Platform**: Supports Linux (primary), with preparation for Windows and MacOS

### Additional Enhancements
- **Special Colors Patch**: Enhanced visual distinction in TTY mode
  - Special areas (beehives, wizard tower, mines, etc.) have distinct colors
  - Different altar alignments are color-coded
  - Visual differentiation for special levels
  - Originally by "L" for NetHack 3.4.3, ported to 3.6.6
- **Sticky Objects Patch**: Keep inventory letters consistent
  - Mark items as "sticky" to maintain their inventory position
  - Toggle with `#sticky` command or Meta-y
  - Sticky items show with `=` instead of `-` in inventory listings

- **Quiver ordering & autoswap**: `#setquiver` to reorder quiver priority; persistent
  `OPTIONS=quiverorder:<letters>` (e.g. `quiverorder:DejL`); enable
  `quiver_autoswap` to automatically switch your quivered item to a
  higher-priority candidate when picked up.

- **Showlines (sight-lines helper)** 🟡
  - Adds an extended command `#showlines` (also bound by default to a Ctrl key) that draws transient sight/ray lines out from the player to help judge line-of-sight and zap direction.
  - Adds an extended command `#showboomerang` (no default separate key binding) that draws transient boomerang trajectories for the 8 directions, using the same visibility and stopping rules as `#showlines`.
  - Visuals (curses): **red** for tiles the player can see (stops at real obstacles), **yellow** for tiles unknown to the player (projects to window edge). For non-curses ports it uses the `tmp_at` temporary glyph mechanism with similar behavior.
  - Non-destructive and non-cheating: it does not reveal hidden monsters or objects and will not overwrite visible monsters/objects.
  - Persistence & clearing: the rays persist as a no-move visual until the player moves (they are cleared automatically), and they are also cleared if you cancel an action (Esc) or after performing an action that consumes a move.
  - Integration: invoking `z` (zap wand) or `Z` (cast spell) will showlines automatically before prompting for wand/spell selection so you can judge direction; Esc cancels selection and clears the rays without performing the action.
  - Tagged: this feature was added as `v0.1-showlines`.


### More Additional Enhancements
  - A lot of minor visual changes, some of which might be considered cheats (inventory information providing various stats; sorry I made this for myself--just sharing)
  - Some changes to items (silver elven arrows)
  - Changes to (ynq) to make them all more consistent
  - Hitting a peaceful is no longer a prompt, but must use 'F' first otherwise ignored if it would prompt (e.g., not wielding Stormbringer)

## Installation

### Prerequisites
- C compiler (GCC recommended)
- Make
- espeak text-to-speech engine (`sudo apt install espeak` on Debian/Ubuntu)

### Building from Source
```bash
# Clone the repository
git clone https://github.com/yourusername/voice-nethack.git
cd voice-nethack

# Configure and build
cd sys/unix/
sh setup.sh hints/linux
cd ../..
make all
```

## Configuration

Add these lines to your `.nethackrc` file:

```
# Enable voice output
OPTIONS=voice_enabled

# Configure espeak parameters (example)
OPTIONS=voice_command:-a 15 -v en-us -g 1 -k 10 -s 260 -z

# Example regex patterns to not speak
VOICE_EXCEPTION=^--More--$
VOICE_EXCEPTION=^You see here
VOICE_EXCEPTION=\[ynq\]

# Special handling for important messages
VOICE_FORCE=^You feel much better
```

## Usage

- Launch the game with `src/nethack`
- Voice output will automatically speak game messages
- Toggle voice on/off in-game through options menu
- Customize voice parameters in your `.nethackrc` file

## Voice Feature Implementation

Voice-related code is isolated in `#ifdef VOICE_ENABLED` blocks to maintain compatibility with standard NetHack. Key implementation files:

- `include/config.h`: Main feature definitions and limits
  ```c
  #ifndef NO_VOICE /* compile-time option enabled--skip this */
  #define VOICE_ENABLED
  #endif /* NO_VOICE */
  
  #ifdef VOICE_ENABLED
  #define MAX_VOICE_EXCEPTIONS 200                           // Adjust as needed
  extern char voice_exceptions[MAX_VOICE_EXCEPTIONS][BUFSZ]; // VOICE_EXCEPTIONS patterns
  extern int num_voice_exceptions;                           // VOICE_EXCEPTIONS counter
  #define MAX_VOICE_FORCES 200                               // VOICE_FORCE patterns
  extern char voice_forces[MAX_VOICE_FORCES][BUFSZ];         // VOICE_FORCE patterns
  extern int num_voice_forces;                               // VOICE_FORCE counter
  #endif /* VOICE_ENABLED */
  ```
- `include/flag.h`: Voice feature flags
- `include/decl.h`: Voice-related data structures
- `src/pline.c`: Main voice output integration
- `src/files.c`: Configuration file parsing
- `src/decl.c`: Data initialization
- `src/options.c`: User-configurable options

Run `./voice.sh` to extract all voice-related code sections into a single file for review.

## License

This project is licensed under the NetHack General Public License - see the original NetHack license file for details.

## Acknowledgments

- The NetHack DevTeam for the original game
- "L" for the original specialcolors patch (from NetHack 3.4.3)
- Contributors of the sticky patch
- The espeak project for text-to-speech capabilities

---

## Changelog 📜

Recent work is summarized in `CHANGELOG.md` (commits since the last README update). Notable recent additions include:

- **Showlines**: transient sight-lines visual aid with curses highlighting and tmp_at fallback; persists until move; integrated into `z` (wand) and `Z` (spell cast) flows; tagged as `v0.1-showlines`.
- **Status line tile**: `BL_TILE` status field and related display/voice improvements.
- **Pricing & shop UX**: price identification and shop UI improvements (priceid, #name price ranges, shop hints).

For the full commit list and dates, see `CHANGELOG.md`.
