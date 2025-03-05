// changes to Nethack 3.6.6 to enable voice output with espeak
// .nethackrc needs:
// # Enable or disable voice output
// OPTIONS=voice_enabled
// # Command to use for voice output with espeak (hard coded for security reasons)
// OPTIONS=voice_command:-a 15 -v en-us -g 1 -k 10 -s 260 -z
//
// # Voice exceptions for Nethack 3.6.6
// # regex matching; no surrounding ""
// VOICE_EXCEPTION=welcome.*
// VOICE_EXCEPTION=Count.*
// VOICE_EXCEPTION=<.*>
// VOICE_EXCEPTION=


// File: include/flag.h
#ifdef VOICE_ENABLED
    boolean voice_enabled;   // New global boolean for voice enablement
    char voice_command[BUFSZ]; // New global buffer for voice command
#endif

// File: include/decl.h
#ifdef VOICE_ENABLED
struct voice_exception {
    struct voice_exception *next;
    char *pattern;
    int flags;
};
extern NEARDATA struct voice_exception *voicelist;
#endif

// File: src/pline.c
#ifdef VOICE_ENABLED
#include "config.h"
char voice_exceptions[MAX_VOICE_EXCEPTIONS][BUFSZ] = {0};
int num_voice_exceptions = 0;
void handle_voice_output(const char *message);
char *strip_patterns(const char *message);
#endif
#ifdef VOICE_ENABLED
    if (flags.voice_enabled) {
        handle_voice_output(line);
    }
#endif
#ifdef VOICE_ENABLED
void handle_voice_output(const char *message) {
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

    // Compile the regex for voice exceptions
    regex_t regex;
    int reti;
    char msg[BUFSZ];
    strncpy(msg, stripped_message, sizeof(msg) - 1); // Ensure null termination
    msg[sizeof(msg) - 1] = '\0';

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

    // If no exception was found, proceed with voice output
    snprintf(sayit, sizeof(sayit), "%s %s \"%s\"", "/usr/bin/espeak", flags.voice_command, escaped_message);
    (void)system(sayit);
    flushinp();
}
#endif
#ifdef VOICE_ENABLED
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
#endif

// File: src/files.c
#ifdef VOICE_ENABLED
    } else if (match_varname(buf, "VOICE_EXCEPTION", 5)) {
    add_voice_exception(bufp);
#endif

// File: src/decl.c
#ifdef VOICE_ENABLED
NEARDATA struct voice_exception *voicelist = (struct voice_exception *)0;
#endif

// File: src/options.c
#ifdef VOICE_ENABLED
#define MAX_VOICE_EXCEPTIONS 200  // Adjust based on your needs
char voice_exceptions[MAX_VOICE_EXCEPTIONS][BUFSZ];
int num_voice_exceptions = 0;
#endif
#ifdef VOICE_ENABLED
    { "voice_command", "command for voice output",
        sizeof flags.voice_command * 2, SET_IN_GAME },
#endif
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
#endif
