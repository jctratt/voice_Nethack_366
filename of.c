File: include/flag.h
--- Previous Block Context -------------------------------------------------------------------------------------------------
    char    sortloot; /* 'n'=none, 'l'=loot (pickup), 'f'=full ('l'+invent) */
#endif
    boolean sortpack;        /* sorted inventory */
    boolean sparkle;         /* show "resisting" special FX (Scott Bigham) */
    boolean standout;        /* use standout for --More-- */
    boolean time;            /* display elapsed 'time' */
    boolean tombstone;       /* print tombstone */
    boolean verbose;         /* max battle info */
--- Start of VOICE_ENABLED -------------------------------------------------------------------------------------------------
#ifdef VOICE_ENABLED
    boolean voice_enabled;   // New global boolean for voice enablement
    char voice_command[BUFSZ]; // New global buffer for voice command
#endif /* VOICE_ENABLED */
--- End of VOICE_ENABLED -------------------------------------------------------------------------------------------------
    int end_top, end_around; /* describe desired score list */
    unsigned moonphase;
    unsigned long suppress_alert;
#define NEW_MOON 0
#define FULL_MOON 4
    unsigned paranoia_bits; /* alternate confirmation prompting */
#define PARANOID_CONFIRM    0x0001
#define PARANOID_QUIT       0x0002
#define PARANOID_DIE        0x0004
--- After Block Context -------------------------------------------------------------------------------------------------


File: include/decl.h
--- Previous Block Context -------------------------------------------------------------------------------------------------
struct plinemsg_type {
    xchar msgtype;  /* one of MSGTYP_foo */
    struct nhregex *regex;
    char *pattern;
    struct plinemsg_type *next;
};
--- Start of VOICE_ENABLED -------------------------------------------------------------------------------------------------
#ifdef VOICE_ENABLED
#define MAX_VOICE_EXCEPTIONS 200
#define MAX_VOICE_FORCES 200
extern char voice_exceptions[MAX_VOICE_EXCEPTIONS][BUFSZ];
extern int num_voice_exceptions;
extern char voice_forces[MAX_VOICE_FORCES][BUFSZ];
extern int num_voice_forces;

/* Structures for VOICE_EXCEPTION and VOICE_FORCE */
struct voice_exception {
    struct voice_exception *next;
    char *pattern;
    int flags;
};
extern NEARDATA struct voice_exception *voicelist;

struct voice_force {
    struct voice_force *next;
    char *pattern;
    char *speak_text;     /* e.g., "\1" or "corpse." */
    int flags;
};
extern NEARDATA struct voice_force *forcelist;
/* Function prototypes */
int add_voice_exception(const char *pattern);
void free_voice_exceptions(void);
struct voice_exception *check_voice_exceptions(const char *text);
int add_voice_force(const char *pattern);
void free_voice_forces(void);
struct voice_force *check_voice_forces(const char *text);
#endif /* VOICE_ENABLED */
--- End of VOICE_ENABLED -------------------------------------------------------------------------------------------------
#define MSGTYP_NORMAL   0
#define MSGTYP_NOREP    1
#define MSGTYP_NOSHOW   2
#define MSGTYP_STOP     3
/* bitmask for callers of hide_unhide_msgtypes() */
#define MSGTYP_MASK_REP_SHOW ((1 << MSGTYP_NOREP) | (1 << MSGTYP_NOSHOW))
E struct plinemsg_type *plinemsg_types;
enum bcargs {override_restriction = -1};
struct breadcrumbs {
--- After Block Context -------------------------------------------------------------------------------------------------


File: src/pline.c
--- Previous Block Context -------------------------------------------------------------------------------------------------
#include "flag.h"
#include <regex.h>
#include <string.h>
#define BIGBUFSZ (5 * BUFSZ) /* big enough to format a 4*BUFSZ string (from
                              * config file parsing) with modest decoration;
                              * result will then be truncated to BUFSZ-1 */
--- Start of VOICE_ENABLED -------------------------------------------------------------------------------------------------
#ifdef VOICE_ENABLED
#include "config.h"
void handle_voice_output(const char *message);
char *strip_patterns(const char *message);
#ifdef __linux__
//#define TTS_CMD "/usr/bin/espeak"
#define TTS_CMD "~/.local/bin/gtts-cli"
#elif defined(_WIN32) || defined(_WIN64)
#define TTS_CMD "powershell"
#elif defined(__APPLE__) && defined(__MACH__)
#define TTS_CMD "/usr/bin/say"
#endif

int check_command_available(const char *command_to_check) {
    char check_command[256];
    snprintf(check_command, sizeof(check_command), "command -v %s >/dev/null 2>&1 || { echo 'not found'; exit 1; }", command_to_check);

    #ifdef _WIN32
    snprintf(check_command, sizeof(check_command), "where %s >nul 2>nul && (echo found) || (echo not found)", command_to_check);
    #endif

    FILE *fp = popen(check_command, "r");
    if (!fp) {
        fprintf(stderr, "Failed to run command\n");
        return 0;
    }

    char result[10];
    if (fgets(result, sizeof(result), fp) != NULL) {
        pclose(fp);
        return strcmp(result, "not found\n") != 0;
    }

    pclose(fp);
    return 0;
}

void check_tts_availability() {
    if (!check_command_available(TTS_CMD)) {
        fprintf(stderr, "\nVoice output is not available. Please ensure '%s' is installed:\n", TTS_CMD);
        // Platform-specific installation instructions
        fprintf(stderr, "Voice output has been disabled.\n\n");
        flags.voice_enabled = 0; // Disable voice output at runtime if TTS not available
    }
}

void check_tts_availability();
#endif /* VOICE_ENABLED */
--- End of VOICE_ENABLED -------------------------------------------------------------------------------------------------
static unsigned pline_flags = 0;
static char prevmsg[BUFSZ];
static void FDECL(putmesg, (const char *));
static char *FDECL(You_buf, (int));
#if defined(MSGHANDLER) && (defined(POSIX_TYPES) || defined(__GNUC__))
static void FDECL(execplinehandler, (const char *));
#endif
#ifdef DUMPLOG
/* also used in end.c */
--- After Block Context -------------------------------------------------------------------------------------------------
--- Previous Block Context -------------------------------------------------------------------------------------------------
    if (vision_full_recalc)
        vision_recalc(0);
    if (u.ux)
        flush_screen(1); /* %% */
    putmesg(line);
--- Start of VOICE_ENABLED -------------------------------------------------------------------------------------------------
#ifdef VOICE_ENABLED
    if (flags.voice_enabled) {
        handle_voice_output(line);
    }
#endif /* VOICE_ENABLED */
--- End of VOICE_ENABLED -------------------------------------------------------------------------------------------------
#if defined(MSGHANDLER) && (defined(POSIX_TYPES) || defined(__GNUC__))
    execplinehandler(line);
#endif
    /* this gets cleared after every pline message */
    iflags.last_msg = PLNMSG_UNKNOWN;
    (void) strncpy(prevmsg, line, BUFSZ), prevmsg[BUFSZ - 1] = '\0';
    if (msgtyp == MSGTYP_STOP)
        display_nhwindow(WIN_MESSAGE, TRUE); /* --more-- */
 pline_done:
--- After Block Context -------------------------------------------------------------------------------------------------
--- Previous Block Context -------------------------------------------------------------------------------------------------
    buf[BUFSZ - 1] = '\0';
    config_erradd(buf);
#if !(defined(USE_STDARG) || defined(USE_VARARGS))
    VA_END(); /* (see pline/vpline -- ends nested block for USE_OLDARGS) */
#endif
}
--- Start of VOICE_ENABLED -------------------------------------------------------------------------------------------------
#ifdef VOICE_ENABLED
void handle_voice_output(const char *message) {
    if (flags.voice_enabled) {
    char sayit[BUFSZ * 2];
    char escaped_message[BUFSZ * 2] = {0}; // Increased size to account for backslashes
    int i, j = 0;

    const char *stripped_message = strip_patterns(message);

    // Escape double quotes in the stripped message
    for (i = 0; stripped_message[i] != '\0' && j < sizeof(escaped_message) - 1; i++) {
        if (stripped_message[i] == '"') {
            escaped_message[j++] = '\\'; // Escape the quote
        }
        escaped_message[j++] = stripped_message[i];
    }
    escaped_message[j] = '\0'; // Null-terminate the string

    regex_t regex;
    int reti;
    char msg[BUFSZ];
    char rep[BUFSZ] = {0};
    strncpy(msg, stripped_message, sizeof(msg) - 1); // Ensure null termination
    msg[sizeof(msg) - 1] = '\0';

    // Check VOICE_FORCE patterns first
    struct voice_force *vf;
    for (vf = forcelist; vf; vf = vf->next) {
        regmatch_t pmatch[2]; // 2 for full match (0) and \1 (1)
        reti = regcomp(&regex, vf->pattern, REG_EXTENDED); // Remove REG_NOSUB for capture groups
        if (reti) {
            pline("Could not compile VOICE_FORCE regex");
            regfree(&regex);
            return;
        }

        reti = regexec(&regex, msg, 2, pmatch, 0); // Capture up to 1 group
        if (!reti) { // Match found in VOICE_FORCE
            regfree(&regex);
            // Proceed directly to voice output, skipping VOICE_EXCEPTION
        if (vf->speak_text && strcmp(vf->speak_text, "\\1") == 0 && pmatch[1].rm_so != -1) {
            // Extract and escape captured group \1
            size_t len = pmatch[1].rm_eo - pmatch[1].rm_so;
            char captured[BUFSZ];
            strncpy(captured, msg + pmatch[1].rm_so, len);
            captured[len] = '\0';
            char escaped[BUFSZ * 2] = {0};
            int k, m = 0;
            for (k = 0; captured[k] && m < sizeof(escaped) - 1; k++) {
                if (captured[k] == '"') escaped[m++] = '\\';
                escaped[m++] = captured[k];
            }
            escaped[m] = '\0';
            snprintf(sayit, sizeof(sayit), "%s %s \"%s\"", "/usr/bin/espeak", flags.voice_command, escaped);
        } else if (vf->speak_text) {
            // Use custom speak_text as-is (assumes no quotes needed escaping here)
            snprintf(sayit, sizeof(sayit), "%s %s \"%s\"", "/usr/bin/espeak", flags.voice_command, vf->speak_text);
        } else {
            // Default to full message
            snprintf(sayit, sizeof(sayit), "%s %s \"%s\"", "/usr/bin/espeak", flags.voice_command, escaped_message);
        }
            (void)system(sayit);
            flushinp();
            return;
        }
        regfree(&regex);
    }

    // Check VOICE_EXCEPTION patterns (only if no VOICE_FORCE match)
    // Compile the regex for voice exceptions
    // Loop through all possible voice exceptions
    struct voice_exception *ve;
    for (ve = voicelist; ve; ve = ve->next) {
        reti = regcomp(&regex, ve->pattern, REG_EXTENDED | REG_NOSUB);
        if (reti) {
            pline("Could not compile regex");
            regfree(&regex);
            return;
        }

        // Perform regex match
        reti = regexec(&regex, msg, 0, NULL, 0);
        if (!reti) { // Match found
            //pline("exception");
            regfree(&regex);
            return; // Skip speaking if there's a match
        }
        regfree(&regex); // Free the compiled regex before moving to the next one
    }
    sanitize_message(escaped_message, rep);

    #ifdef _WIN32
        snprintf(sayit, sizeof(sayit), "%s -Command \"%s\"", TTS_CMD, TTS_FLAGS, rep);
    #else
        // gTTS yuck; too slow for me
        //pline("%s \"%s\" %s", TTS_CMD, rep, flags.voice_command); // testing
        // gTTS
        // snprintf(sayit, sizeof(sayit), "%s \"%s\" %s", TTS_CMD, rep, flags.voice_command);
        //
        // espeak
        //pline("%s %s \"%s\"", "/usr/bin/espeak", flags.voice_command, rep);
        snprintf(sayit, sizeof(sayit), "%s %s \"%s\"", "/usr/bin/espeak", flags.voice_command, rep);
    #endif
    // If no exception was found, proceed with voice output
    (void)system(sayit);
    flushinp();
    }
}

char *strip_patterns(const char *message) {
    static char result[BUFSZ];
    regex_t regex;
    regmatch_t pmatch[1];
    const char *pattern = "\\[.*\\)|\\[.*\\]"; // matches "[...]" or "[...)" for input questions "[ynq] (n)"
    char error_message[100];

    // Compile the regex
    int reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti) {
        regerror(reti, &regex, error_message, sizeof(error_message));
        snprintf(result, BUFSZ, "Error compiling regex: %s", error_message);
        pline("%s",result);
        return result;
    }

    // Copy the original message
    strcpy(result, message);

    // Try to match the pattern at the end of the string
    if (regexec(&regex, result, 1, pmatch, 0) == 0) {
        // If there's a match, truncate the string at the start of the match
        result[pmatch[0].rm_so] = '\0';
    }

    // Clean up
    regfree(&regex);
    return result;
}

void sanitize_message(const char *src, char *dest) {
    int i, j;
    for (i = 0, j = 0; src[i] != '\0' && j < BUFSZ - 1; i++) {
        if (/*(src[i] == '\'' || src[i] == '"' || src[i] == '`' ||
            src[i] == '|' || src[i] == '>' || src[i] == '<' ||
            src[i] == '(' || src[i] == ')' || src[i] == ';' || */
            src[i] == '_') {
            if (i < BUFSZ - 1) {
                dest[j++] = ' ';
            }
        } else {
            dest[j++] = src[i];
        }
    }
    dest[j] = '\0';
}
#endif /* VOICE_ENABLED */
--- End of VOICE_ENABLED -------------------------------------------------------------------------------------------------
/*pline.c*/
--- After Block Context -------------------------------------------------------------------------------------------------


File: src/files.c
--- Previous Block Context -------------------------------------------------------------------------------------------------
        }
#endif /*AMIGA*/
#ifdef USER_SOUNDS
    } else if (match_varname(buf, "SOUNDDIR", 8)) {
        sounddir = dupstr(bufp);
    } else if (match_varname(buf, "SOUND", 5)) {
        add_sound_mapping(bufp);
#endif
--- Start of VOICE_ENABLED -------------------------------------------------------------------------------------------------
#ifdef VOICE_ENABLED
    } else if (match_varname(buf, "VOICE_EXCEPTION", 13)) {
        add_voice_exception(bufp); // New function to add VOICE_EXCEPTION patterns /* defined in options.c */
    } else if (match_varname(buf, "VOICE_FORCE", 11)) {
        if (add_voice_force(bufp) < 0) { /* defined in options.c */
            config_error_add("Could not compile VOICE_FORCE regex: '%s'", bufp);
        } else {
            ;//config_error_add("Successfully compiled VOICE_FORCE: '%s'", bufp); /* Optional debug */
        }
#endif /* VOICE_ENABLED */
--- End of VOICE_ENABLED -------------------------------------------------------------------------------------------------
    } else if (match_varname(buf, "QT_TILEWIDTH", 12)) {
#ifdef QT_GRAPHICS
        extern char *qt_tilewidth;
        if (qt_tilewidth == NULL)
            qt_tilewidth = dupstr(bufp);
#endif
    } else if (match_varname(buf, "QT_TILEHEIGHT", 13)) {
#ifdef QT_GRAPHICS
        extern char *qt_tileheight;
--- After Block Context -------------------------------------------------------------------------------------------------


File: src/decl.c
--- Previous Block Context -------------------------------------------------------------------------------------------------
    0x00000000UL, 0x00000000UL
#else
    0x00000000L, 0x00000000L
#endif
};
struct plinemsg_type *plinemsg_types = (struct plinemsg_type *) 0;
--- Start of VOICE_ENABLED -------------------------------------------------------------------------------------------------
#ifdef VOICE_ENABLED
NEARDATA char voice_exceptions[MAX_VOICE_EXCEPTIONS][BUFSZ] = { { 0 } };
NEARDATA int num_voice_exceptions = 0;
NEARDATA char voice_forces[MAX_VOICE_FORCES][BUFSZ] = { { 0 } };
NEARDATA int num_voice_forces = 0;
NEARDATA struct voice_exception *voicelist = (struct voice_exception *) 0; // New linked list for VOICE_EXCEPTION
NEARDATA struct voice_force *forcelist = (struct voice_force *) 0; // New linked list for VOICE_FORCE
#endif /* VOICE_ENABLED */
--- End of VOICE_ENABLED -------------------------------------------------------------------------------------------------
#ifdef PANICTRACE
const char *ARGV0;
#endif
/* support for lint.h */
unsigned nhUse_dummy = 0;
/* dummy routine used to force linkage */
void
decl_init()
{
--- After Block Context -------------------------------------------------------------------------------------------------


File: src/options.c
--- Previous Block Context -------------------------------------------------------------------------------------------------
NEARDATA struct instance_flags iflags; /* provide linkage */
#define static
#else
#include "hack.h"
#include "tcap.h"
#include <ctype.h>
#endif
--- Start of VOICE_ENABLED -------------------------------------------------------------------------------------------------
#ifdef VOICE_ENABLED
#include <regex.h>  /* For regex_t, regcomp, etc. in add_voice_force */
#endif /* VOICE_ENABLED */
--- End of VOICE_ENABLED -------------------------------------------------------------------------------------------------
#define BACKWARD_COMPAT
#ifdef DEFAULT_WC_TILED_MAP
#define PREFER_TILED TRUE
#else
#define PREFER_TILED FALSE
#endif
#ifdef CURSES_GRAPHICS
extern int curses_read_attrs(const char *attrs);
extern char *curses_fmt_attrs(char *);
--- After Block Context -------------------------------------------------------------------------------------------------
--- Previous Block Context -------------------------------------------------------------------------------------------------
#endif
    { "use_darkgray", &iflags.wc2_darkgray, TRUE, SET_IN_FILE }, /*WC2*/
#ifdef WIN32
    { "use_inverse", &iflags.wc_inverse, TRUE, SET_IN_GAME }, /*WC*/
#else
    { "use_inverse", &iflags.wc_inverse, FALSE, SET_IN_GAME }, /*WC*/
#endif
    { "verbose", &flags.verbose, TRUE, SET_IN_GAME },
--- Start of VOICE_ENABLED -------------------------------------------------------------------------------------------------
#ifdef VOICE_ENABLED
    { "voice_enabled", &flags.voice_enabled, FALSE, SET_IN_GAME },
#endif /* VOICE_ENABLED */
--- End of VOICE_ENABLED -------------------------------------------------------------------------------------------------
#ifdef TTY_TILES_ESCCODES
    { "vt_tiledata", &iflags.vt_tiledata, FALSE, SET_IN_FILE },
#else
    { "vt_tiledata", (boolean *) 0, FALSE, SET_IN_FILE },
#endif
    { "whatis_menu", &iflags.getloc_usemenu, FALSE, SET_IN_GAME },
    { "whatis_moveskip", &iflags.getloc_moveskip, FALSE, SET_IN_GAME },
    { "wizweight", &iflags.wizweight, FALSE, SET_IN_WIZGAME },
    { "wraptext", &iflags.wc2_wraptext, FALSE, SET_IN_GAME }, /*WC2*/
--- After Block Context -------------------------------------------------------------------------------------------------
--- Previous Block Context -------------------------------------------------------------------------------------------------
    { "video", "method of video updating", 20, SET_IN_FILE },
#endif
#ifdef VIDEOSHADES
    { "videocolors", "color mappings for internal screen routines", 40,
      DISP_IN_GAME },
    { "videoshades", "gray shades to map to black/gray/white", 32,
      DISP_IN_GAME },
#endif
--- Start of VOICE_ENABLED -------------------------------------------------------------------------------------------------
#ifdef VOICE_ENABLED
    { "voice_command", "command for voice output",
        sizeof flags.voice_command * 2, SET_IN_GAME },
#endif /* VOICE_ENABLED */
--- End of VOICE_ENABLED -------------------------------------------------------------------------------------------------
    { "whatis_coord", "show coordinates when auto-describing cursor position",
      1, SET_IN_GAME },
    { "whatis_filter",
      "filter coordinate locations when targeting next or previous",
      1, SET_IN_GAME },
    { "windowborders", "0 (off), 1 (on), 2 (auto)", 9, SET_IN_GAME }, /*WC2*/
    { "windowcolors", "the foreground/background colors of windows", /*WC*/
      80, DISP_IN_GAME },
    { "windowtype", "windowing system to use", WINTYPELEN, DISP_IN_GAME },
--- After Block Context -------------------------------------------------------------------------------------------------
--- Previous Block Context -------------------------------------------------------------------------------------------------
        /* If initial, then initoptions is allowed to do it instead
         * of here (initoptions always has to do it even if there's
         * no fruit option at all.  Also, we don't want people
         * setting multiple fruits in their options.)
         */
        return retval;
    }
--- Start of VOICE_ENABLED -------------------------------------------------------------------------------------------------
#ifdef VOICE_ENABLED
    fullname = "VOICE_EXCEPTION"; /* suppress voice output for pattern */
    if (match_optname(opts, fullname, 13, TRUE)) {
        if (duplicate)
            complain_about_duplicate(opts, 1);
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else if ((op = string_for_opt(opts, FALSE)) != empty_optstr) {
            if (add_voice_exception(op) < 0) {
                config_error_add("Failed to add VOICE_EXCEPTION '%s'", op);
                return FALSE;
            }
        } else {
            return FALSE;
        }
        return retval;
    }

    fullname = "VOICE_FORCE"; /* force voice output for pattern */
    if (match_optname(opts, fullname, 11, TRUE)) {
        if (duplicate)
            complain_about_duplicate(opts, 1);
        if (negated) {
            bad_negation(fullname, FALSE);
            return FALSE;
        } else if ((op = string_for_opt(opts, FALSE)) != empty_optstr) {
            int result = add_voice_force(op);
            if (result < 0) {
                config_error_add("Could not compile VOICE_FORCE regex: '%s'", op);
                return FALSE;
            } else if (!initial && !tfrom_file) {
                ;//config_error_add("Successfully compiled VOICE_FORCE: '%s'", op); /* Optional debug */
                }
        } else {
            return FALSE;
        }
        return retval;
    }

    fullname = "voice_command";
    if (match_optname(opts, fullname, 12, TRUE)) {
        if (duplicate)
            complain_about_duplicate(opts, 1);
        op = string_for_opt(opts, negated);
        if ((negated && op == empty_optstr) || (!negated && op != empty_optstr)) {
            if (negated) {
                flags.voice_command[0] = '\0';  // Reset to empty if negated
            } else {
                strncpy(flags.voice_command, op, sizeof(flags.voice_command) - 1);
                flags.voice_command[sizeof(flags.voice_command) - 1] = '\0'; // Ensure null termination
            }
        } else if (negated) {
            bad_negation(fullname, TRUE);
            return FALSE;
        } else { // op == empty_optstr
            flags.voice_command[0] = '\0';  // Set to empty if no value provided
        }
        return retval;
    }
#endif /* VOICE_ENABLED */
--- End of VOICE_ENABLED -------------------------------------------------------------------------------------------------
    fullname = "whatis_coord";
    if (match_optname(opts, fullname, 8, TRUE)) {
        if (duplicate)
            complain_about_duplicate(opts, 1);
        if (negated) {
            iflags.getpos_coords = GPCOORDS_NONE;
            return retval;
        } else if ((op = string_for_env_opt(fullname, opts, FALSE))
                                            != empty_optstr) {
--- After Block Context -------------------------------------------------------------------------------------------------
--- Previous Block Context -------------------------------------------------------------------------------------------------
        discover = !wizard;
        iflags.deferred_X = FALSE;
    }
    /* don't need to do anything special for explore mode or normal play */
}
#endif /* OPTION_LISTS_ONLY */
--- Start of VOICE_ENABLED -------------------------------------------------------------------------------------------------
#ifdef VOICE_ENABLED
int add_voice_exception(const char *pattern) {
    struct voice_exception *newve = (struct voice_exception *) alloc(sizeof *newve);
    newve->pattern = dupstr(pattern);
    newve->flags = 0; // Or set some initial flags if needed
    newve->next = voicelist;
    voicelist = newve;
    return 0; // Return success or error code
}

void free_voice_exceptions() {
    struct voice_exception *ve, *nextve;
    for (ve = voicelist; ve; ve = nextve) {
        nextve = ve->next;
        free((genericptr_t) ve->pattern);
        free((genericptr_t) ve);
    }
    voicelist = (struct voice_exception *)0;
}

struct voice_exception *check_voice_exceptions(const char *text) {
    struct voice_exception *ve;
    for (ve = voicelist; ve; ve = ve->next) {
        if (pmatch(ve->pattern, text)) {
            return ve; // Found a match
        }
    }
    return (struct voice_exception *)0; // No match found
}

int add_voice_force(const char *pattern) {
    struct voice_force *newvf;
    char *sep;
    char *match_pattern = NULL;
    char *speak_text = NULL;
    regex_t regex;
    int reti;

    newvf = (struct voice_force *)alloc(sizeof *newvf);
    if (!newvf) {
        config_error_add("Memory allocation failed for VOICE_FORCE");
        return -1;
    }

    sep = strstr(pattern, "=>");
    if (sep) {
        size_t match_len = sep - pattern;
        match_pattern = (char *)alloc(match_len + 1);
        if (!match_pattern) {
            config_error_add("Memory allocation failed for VOICE_FORCE pattern");
            free(newvf);
            return -1;
        }
        strncpy(match_pattern, pattern, match_len);
        match_pattern[match_len] = '\0';
        speak_text = dupstr(sep + 2); // Skip "=>"
        if (!speak_text) {
            config_error_add("Memory allocation failed for VOICE_FORCE speak_text");
            free(match_pattern);
            free(newvf);
            return -1;
        }
    } else {
        match_pattern = dupstr(pattern);
        if (!match_pattern) {
            config_error_add("Memory allocation failed for VOICE_FORCE pattern");
            free(newvf);
            return -1;
        }
        speak_text = NULL; // NULL means use full message
    }

    // Validate the regex pattern
    reti = regcomp(&regex, match_pattern, REG_EXTENDED);
    if (reti) {
        char errbuf[BUFSZ];
        regerror(reti, &regex, errbuf, sizeof(errbuf));
        config_error_add("Could not compile VOICE_FORCE regex '%s': %s", match_pattern, errbuf);
        free(match_pattern);
        free(speak_text);
        free(newvf);
        regfree(&regex);
        return -1;
    }
    regfree(&regex);

    newvf->pattern = match_pattern;
    newvf->speak_text = speak_text;
    newvf->flags = 0;
    newvf->next = forcelist;
    forcelist = newvf;
    return 0;
}

void free_voice_forces() {
    struct voice_force *vf, *nextvf;
    for (vf = forcelist; vf; vf = nextvf) {
        nextvf = vf->next;
        free((genericptr_t) vf->pattern);
        if (vf->speak_text) free((genericptr_t) vf->speak_text);
        free((genericptr_t) vf);
    }
    forcelist = (struct voice_force *)0;
}

struct voice_force *check_voice_forces(const char *text) {
    struct voice_force *vf;
    for (vf = forcelist; vf; vf = vf->next) {
        if (pmatch(vf->pattern, text)) {
            return vf;
        }
    }
    return (struct voice_force *)0;
}
#endif /* VOICE_ENABLED */
--- End of VOICE_ENABLED -------------------------------------------------------------------------------------------------
/*options.c*/
--- After Block Context -------------------------------------------------------------------------------------------------


