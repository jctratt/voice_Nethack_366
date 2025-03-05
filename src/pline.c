/* NetHack 3.6	pline.c	$NHDT-Date: 1549327495 2019/02/05 00:44:55 $  $NHDT-Branch: NetHack-3.6.2-beta01 $:$NHDT-Revision: 1.73 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2018. */
/* NetHack may be freely redistributed.  See license for details. */

#define NEED_VARARGS /* Uses ... */ /* comment line for pre-compiled headers */
#include "hack.h"
#ifdef CURSES_GRAPHICS
#include <curses.h>
#endif
#include "flag.h"
#include <regex.h>
#include <string.h>

#define BIGBUFSZ (5 * BUFSZ) /* big enough to format a 4*BUFSZ string (from
                              * config file parsing) with modest decoration;
                              * result will then be truncated to BUFSZ-1 */

#ifdef VOICE_ENABLED
#include "config.h"
void handle_voice_output(const char *message);
char *strip_patterns(const char *message);
void sanitize_message(const char *src, char *dest);
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

static unsigned pline_flags = 0;
static char prevmsg[BUFSZ];

static void FDECL(putmesg, (const char *));
static char *FDECL(You_buf, (int));
#if defined(MSGHANDLER) && (defined(POSIX_TYPES) || defined(__GNUC__))
static void FDECL(execplinehandler, (const char *));
#endif

#ifdef DUMPLOG
/* also used in end.c */
unsigned saved_pline_index = 0; /* slot in saved_plines[] to use next */
char *saved_plines[DUMPLOG_MSG_COUNT] = { (char *) 0 };

/* keep the most recent DUMPLOG_MSG_COUNT messages */
void
dumplogmsg(line)
const char *line;
{
    /*
     * TODO:
     *  This essentially duplicates message history, which is
     *  currently implemented in an interface-specific manner.
     *  The core should take responsibility for that and have
     *  this share it.
     */
    unsigned indx = saved_pline_index; /* next slot to use */
    char *oldest = saved_plines[indx]; /* current content of that slot */

    if (oldest && strlen(oldest) >= strlen(line)) {
        /* this buffer will gradually shrink until the 'else' is needed;
           there's no pressing need to track allocation size instead */
        Strcpy(oldest, line);
    } else {
        if (oldest)
            free((genericptr_t) oldest);
        saved_plines[indx] = dupstr(line);
    }
    saved_pline_index = (indx + 1) % DUMPLOG_MSG_COUNT;
}

/* called during save (unlike the interface-specific message history,
   this data isn't saved and restored); end-of-game releases saved_pline[]
   while writing its contents to the final dump log */
void
dumplogfreemessages()
{
    unsigned indx;

    for (indx = 0; indx < DUMPLOG_MSG_COUNT; ++indx)
        if (saved_plines[indx])
            free((genericptr_t) saved_plines[indx]), saved_plines[indx] = 0;
    saved_pline_index = 0;
}
#endif

/* keeps windowprocs usage out of pline() */
static void
putmesg(line)
const char *line;
{
    int attr = ATR_NONE;

    if ((pline_flags & URGENT_MESSAGE) != 0
        && (windowprocs.wincap2 & WC2_URGENT_MESG) != 0)
        attr |= ATR_URGENT;
    if ((pline_flags & SUPPRESS_HISTORY) != 0
        && (windowprocs.wincap2 & WC2_SUPPRESS_HIST) != 0)
        attr |= ATR_NOHISTORY;

    putstr(WIN_MESSAGE, attr, line);
}

/* Note that these declarations rely on knowledge of the internals
 * of the variable argument handling stuff in "tradstdc.h"
 */

#if defined(USE_STDARG) || defined(USE_VARARGS)
static void FDECL(vpline, (const char *, va_list));

/*VARARGS1*/
void
pline
VA_DECL(const char *, line)
{
    VA_START(line);
    VA_INIT(line, char *);
    vpline(line, VA_ARGS);
    VA_END();
}

# ifdef USE_STDARG
static void
vpline(const char *line, va_list the_args)
# else
static void
vpline(line, the_args)
const char *line;
va_list the_args;
# endif

#else /* USE_STDARG | USE_VARARG */

# define vpline pline

/*VARARGS1*/
void
pline
VA_DECL(const char *, line)
#endif /* USE_STDARG | USE_VARARG */
{       /* start of vpline() or of nested block in USE_OLDARG's pline() */
    static int in_pline = 0;
    char pbuf[BIGBUFSZ]; /* will get chopped down to BUFSZ-1 if longer */
    int ln;
    int msgtyp;
#if !defined(NO_VSNPRINTF)
    int vlen = 0;
#endif
    boolean no_repeat;
    /* Do NOT use VA_START and VA_END in here... see above */

    if (!line || !*line)
        return;
#ifdef HANGUPHANDLING
    if (program_state.done_hup)
        return;
#endif
    if (program_state.wizkit_wishing)
        return;

    if (index(line, '%')) {
#if !defined(NO_VSNPRINTF)
        vlen = vsnprintf(pbuf, sizeof pbuf, line, VA_ARGS);
#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED) && defined(DEBUG)
        if (vlen >= (int) sizeof pbuf)
            panic("%s: truncation of buffer at %zu of %d bytes",
                  "pline", sizeof pbuf, vlen);
#endif
#else
        Vsprintf(pbuf, line, VA_ARGS);
#endif
        line = pbuf;
    }
    if ((ln = (int) strlen(line)) > BUFSZ - 1) {
        if (line != pbuf)                          /* no '%' was present */
            (void) strncpy(pbuf, line, BUFSZ - 1); /* caveat: unterminated */
        /* truncate, preserving the final 3 characters:
           "___ extremely long text" -> "___ extremely l...ext"
           (this may be suboptimal if overflow is less than 3) */
        (void) strncpy(pbuf + BUFSZ - 1 - 6, "...", 3);
        /* avoid strncpy; buffers could overlap if excess is small */
        pbuf[BUFSZ - 1 - 3] = line[ln - 3];
        pbuf[BUFSZ - 1 - 2] = line[ln - 2];
        pbuf[BUFSZ - 1 - 1] = line[ln - 1];
        pbuf[BUFSZ - 1] = '\0';
        line = pbuf;
    }

#ifdef DUMPLOG
    /* We hook here early to have options-agnostic output.
     * Unfortunately, that means Norep() isn't honored (general issue) and
     * that short lines aren't combined into one longer one (tty behavior).
     */
    if ((pline_flags & SUPPRESS_HISTORY) == 0)
        dumplogmsg(line);
#endif
    /* use raw_print() if we're called too early (or perhaps too late
       during shutdown) or if we're being called recursively (probably
       via debugpline() in the interface code) */
    if (in_pline++ || !iflags.window_inited) {
        /* [we should probably be using raw_printf("\n%s", line) here] */
        raw_print(line);
        iflags.last_msg = PLNMSG_UNKNOWN;
        goto pline_done;
    }

    msgtyp = MSGTYP_NORMAL;
    no_repeat = (pline_flags & PLINE_NOREPEAT) ? TRUE : FALSE;
    if ((pline_flags & OVERRIDE_MSGTYPE) == 0) {
        msgtyp = msgtype_type(line, no_repeat);
        if ((pline_flags & URGENT_MESSAGE) == 0
            && (msgtyp == MSGTYP_NOSHOW
                || (msgtyp == MSGTYP_NOREP && !strcmp(line, prevmsg))))
            /* FIXME: we need a way to tell our caller that this message
             * was suppressed so that caller doesn't set iflags.last_msg
             * for something that hasn't been shown, otherwise a subsequent
             * message which uses alternate wording based on that would be
             * doing so out of context and probably end up seeming silly.
             * (Not an issue for no-repeat but matters for no-show.)
             */
            goto pline_done;
    }

    if (vision_full_recalc)
        vision_recalc(0);
    if (u.ux)
        flush_screen(1); /* %% */

    putmesg(line);

#ifdef VOICE_ENABLED
    if (flags.voice_enabled) {
        handle_voice_output(line);
    }
#endif /* VOICE_ENABLED */

#if defined(MSGHANDLER) && (defined(POSIX_TYPES) || defined(__GNUC__))
    execplinehandler(line);
#endif

    /* this gets cleared after every pline message */
    iflags.last_msg = PLNMSG_UNKNOWN;
    (void) strncpy(prevmsg, line, BUFSZ), prevmsg[BUFSZ - 1] = '\0';
    if (msgtyp == MSGTYP_STOP)
        display_nhwindow(WIN_MESSAGE, TRUE); /* --more-- */

 pline_done:
    --in_pline;
    return;

#if !(defined(USE_STDARG) || defined(USE_VARARGS))
    /* provide closing brace for the nested block
       which immediately follows USE_OLDARGS's VA_DECL() */
    VA_END();
#endif
}

/* pline() variant which can override MSGTYPE handling or suppress
   message history (tty interface uses pline() to issue prompts and
   they shouldn't be blockable via MSGTYPE=hide) */
/*VARARGS2*/
void custompline
VA_DECL2(unsigned, pflags, const char *, line)
{
    VA_START(line);
    VA_INIT(line, const char *);
    pline_flags = pflags;
    vpline(line, VA_ARGS);
    pline_flags = 0;
    VA_END();
    return;
}

/*VARARGS1*/
void Norep
VA_DECL(const char *, line)
{
    VA_START(line);
    VA_INIT(line, const char *);
    pline_flags = PLINE_NOREPEAT;
    vpline(line, VA_ARGS);
    pline_flags = 0;
    VA_END();
    return;
}

/* work buffer for You(), &c and verbalize() */
static char *you_buf = 0;
static int you_buf_siz = 0;

static char *
You_buf(siz)
int siz;
{
    if (siz > you_buf_siz) {
        if (you_buf)
            free((genericptr_t) you_buf);
        you_buf_siz = siz + 10;
        you_buf = (char *) alloc((unsigned) you_buf_siz);
    }
    return you_buf;
}

void
free_youbuf()
{
    if (you_buf)
        free((genericptr_t) you_buf), you_buf = (char *) 0;
    you_buf_siz = 0;
}

/* `prefix' must be a string literal, not a pointer */
#define YouPrefix(pointer, prefix, text) \
    Strcpy((pointer = You_buf((int) (strlen(text) + sizeof prefix))), prefix)

#define YouMessage(pointer, prefix, text) \
    strcat((YouPrefix(pointer, prefix, text), pointer), text)

/*VARARGS1*/
void You
VA_DECL(const char *, line)
{
    char *tmp;

    VA_START(line);
    VA_INIT(line, const char *);
    vpline(YouMessage(tmp, "You ", line), VA_ARGS);
    VA_END();
}

/*VARARGS1*/
void Your
VA_DECL(const char *, line)
{
    char *tmp;

    VA_START(line);
    VA_INIT(line, const char *);
    vpline(YouMessage(tmp, "Your ", line), VA_ARGS);
    VA_END();
}

/*VARARGS1*/
void You_feel
VA_DECL(const char *, line)
{
    char *tmp;

    VA_START(line);
    VA_INIT(line, const char *);
    if (Unaware)
        YouPrefix(tmp, "You dream that you feel ", line);
    else
        YouPrefix(tmp, "You feel ", line);
    vpline(strcat(tmp, line), VA_ARGS);
    VA_END();
}

/*VARARGS1*/
void You_cant
VA_DECL(const char *, line)
{
    char *tmp;

    VA_START(line);
    VA_INIT(line, const char *);
    vpline(YouMessage(tmp, "You can't ", line), VA_ARGS);
    VA_END();
}

/*VARARGS1*/
void pline_The
VA_DECL(const char *, line)
{
    char *tmp;

    VA_START(line);
    VA_INIT(line, const char *);
    vpline(YouMessage(tmp, "The ", line), VA_ARGS);
    VA_END();
}

/*VARARGS1*/
void There
VA_DECL(const char *, line)
{
    char *tmp;

    VA_START(line);
    VA_INIT(line, const char *);
    vpline(YouMessage(tmp, "There ", line), VA_ARGS);
    VA_END();
}

/*VARARGS1*/
void You_hear
VA_DECL(const char *, line)
{
    char *tmp;

    if (Deaf || !flags.acoustics)
        return;
    VA_START(line);
    VA_INIT(line, const char *);
    if (Underwater)
        YouPrefix(tmp, "You barely hear ", line);
    else if (Unaware)
        YouPrefix(tmp, "You dream that you hear ", line);
    else
        YouPrefix(tmp, "You hear ", line);  /* Deaf-aware */
    vpline(strcat(tmp, line), VA_ARGS);
    VA_END();
}

/*VARARGS1*/
void You_see
VA_DECL(const char *, line)
{
    char *tmp;

    VA_START(line);
    VA_INIT(line, const char *);
    if (Unaware)
        YouPrefix(tmp, "You dream that you see ", line);
    else if (Blind) /* caller should have caught this... */
        YouPrefix(tmp, "You sense ", line);
    else
        YouPrefix(tmp, "You see ", line);
    vpline(strcat(tmp, line), VA_ARGS);
    VA_END();
}

/* Print a message inside double-quotes.
 * The caller is responsible for checking deafness.
 * Gods can speak directly to you in spite of deafness.
 */
/*VARARGS1*/
void verbalize
VA_DECL(const char *, line)
{
    char *tmp;

    VA_START(line);
    VA_INIT(line, const char *);
    tmp = You_buf((int) strlen(line) + sizeof "\"\"");
    Strcpy(tmp, "\"");
    Strcat(tmp, line);
    Strcat(tmp, "\"");
    vpline(tmp, VA_ARGS);
    VA_END();
}

/*VARARGS1*/
/* Note that these declarations rely on knowledge of the internals
 * of the variable argument handling stuff in "tradstdc.h"
 */

#if defined(USE_STDARG) || defined(USE_VARARGS)
static void FDECL(vraw_printf, (const char *, va_list));

void raw_printf
VA_DECL(const char *, line)
{
    VA_START(line);
    VA_INIT(line, char *);
    vraw_printf(line, VA_ARGS);
    VA_END();
}

# ifdef USE_STDARG
static void
vraw_printf(const char *line, va_list the_args)
# else
static void
vraw_printf(line, the_args)
const char *line;
va_list the_args;
# endif

#else /* USE_STDARG | USE_VARARG */

void raw_printf
VA_DECL(const char *, line)
#endif
{
    char pbuf[BIGBUFSZ]; /* will be chopped down to BUFSZ-1 if longer */
    /* Do NOT use VA_START and VA_END in here... see above */

    if (index(line, '%')) {
#if !defined(NO_VSNPRINTF)
        (void) vsnprintf(pbuf, sizeof pbuf, line, VA_ARGS);
#else
        Vsprintf(pbuf, line, VA_ARGS);
#endif
        line = pbuf;
    }
    if ((int) strlen(line) > BUFSZ - 1) {
        if (line != pbuf)
            line = strncpy(pbuf, line, BUFSZ - 1);
        /* unlike pline, we don't futz around to keep last few chars */
        pbuf[BUFSZ - 1] = '\0'; /* terminate strncpy or truncate vsprintf */
    }
    raw_print(line);
#if defined(MSGHANDLER) && (defined(POSIX_TYPES) || defined(__GNUC__))
    execplinehandler(line);
#endif
#if !(defined(USE_STDARG) || defined(USE_VARARGS))
    VA_END(); /* (see vpline) */
#endif
}

/*VARARGS1*/
void impossible
VA_DECL(const char *, s)
{
    char pbuf[BIGBUFSZ]; /* will be chopped down to BUFSZ-1 if longer */

    VA_START(s);
    VA_INIT(s, const char *);
    if (program_state.in_impossible)
        panic("impossible called impossible");

    program_state.in_impossible = 1;
#if !defined(NO_VSNPRINTF)
    (void) vsnprintf(pbuf, sizeof pbuf, s, VA_ARGS);
#else
    Vsprintf(pbuf, s, VA_ARGS);
#endif
    pbuf[BUFSZ - 1] = '\0'; /* sanity */
    paniclog("impossible", pbuf);
    if (iflags.debug_fuzzer)
        panic("%s", pbuf);
    pline("%s", VA_PASS1(pbuf));
    /* reuse pbuf[] */
    Strcpy(pbuf, "Program in disorder!");
    if (program_state.something_worth_saving)
        Strcat(pbuf, "  (Saving and reloading may fix this problem.)");
    pline("%s", VA_PASS1(pbuf));

    program_state.in_impossible = 0;
    VA_END();
}

#if defined(MSGHANDLER) && (defined(POSIX_TYPES) || defined(__GNUC__))
static boolean use_pline_handler = TRUE;

static void
execplinehandler(line)
const char *line;
{
    int f;
    const char *args[3];
    char *env;

    if (!use_pline_handler)
        return;

    if (!(env = nh_getenv("NETHACK_MSGHANDLER"))) {
        use_pline_handler = FALSE;
        return;
    }

    f = fork();
    if (f == 0) { /* child */
        args[0] = env;
        args[1] = line;
        args[2] = NULL;
        (void) setgid(getgid());
        (void) setuid(getuid());
        (void) execv(args[0], (char *const *) args);
        perror((char *) 0);
        (void) fprintf(stderr, "Exec to message handler %s failed.\n", env);
        nh_terminate(EXIT_FAILURE);
    } else if (f > 0) {
        int status;

        waitpid(f, &status, 0);
    } else if (f == -1) {
        perror((char *) 0);
        use_pline_handler = FALSE;
        pline("%s", VA_PASS1("Fork to message handler failed."));
    }
}
#endif /* MSGHANDLER && (POSIX_TYPES || __GNUC__) */

/*
 * varargs handling for files.c
 */
#if defined(USE_STDARG) || defined(USE_VARARGS)
static void FDECL(vconfig_error_add, (const char *, va_list));

/*VARARGS1*/
void
config_error_add
VA_DECL(const char *, str)
{
    VA_START(str);
    VA_INIT(str, char *);
    vconfig_error_add(str, VA_ARGS);
    VA_END();
}

# ifdef USE_STDARG
static void
vconfig_error_add(const char *str, va_list the_args)
# else
static void
vconfig_error_add(str, the_args)
const char *str;
va_list the_args;
# endif

#else /* !(USE_STDARG || USE_VARARG) => USE_OLDARGS */

/*VARARGS1*/
void
config_error_add
VA_DECL(const char *, str)
#endif /* ?(USE_STDARG || USE_VARARG) */
{       /* start of vconf...() or of nested block in USE_OLDARG's conf...() */
#if !defined(NO_VSNPRINTF)
    int vlen = 0;
#endif
    char buf[BIGBUFSZ]; /* will be chopped down to BUFSZ-1 if longer */

#if !defined(NO_VSNPRINTF)
    vlen = vsnprintf(buf, sizeof buf, str, VA_ARGS);
#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED) && defined(DEBUG)
    if (vlen >= (int) sizeof buf)
        panic("%s: truncation of buffer at %zu of %d bytes",
              "config_error_add", sizeof buf, vlen);
#endif
#else
    Vsprintf(buf, str, VA_ARGS);
#endif
    buf[BUFSZ - 1] = '\0';
    config_erradd(buf);

#if !(defined(USE_STDARG) || defined(USE_VARARGS))
    VA_END(); /* (see pline/vpline -- ends nested block for USE_OLDARGS) */
#endif
}

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

/*pline.c*/
