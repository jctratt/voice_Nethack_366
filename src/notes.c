/* notes.c - Player note management */

#include "hack.h"
#include <ctype.h>

/* Sanitize text by removing non-printable control characters (except newline/tab).
   Returns an allocated string (caller must free). */
static char *
sanitize_text(const char *src)
{
    size_t len = strlen(src);
    char *temp = (char *) alloc(len + 1);
    size_t i, j = 0;
    unsigned char c;

    for (i = 0; i < len; i++) {
        c = (unsigned char) src[i];
        if (isprint(c) || c == '\n' || c == '\t') {
            temp[j++] = src[i];
        }
    }
    temp[j] = '\0';

    if (j == len) {
        char *res = dupstr(src);
        free((genericptr_t) temp);
        return res;
    } else {
        char *res = (char *) alloc(j + 1);
        memcpy(res, temp, j + 1);
        free((genericptr_t) temp);
        return res;
    }
}

/* Add a new note (sanitize control characters) */
void
add_note(const char *text)
{
    char **new_list;
    int i;
    char *clean;

    if (!text || !*text)
        return;

    clean = sanitize_text(text);
    if (!clean || !*clean) {
        if (clean)
            free((genericptr_t) clean);
        return;
    }

    new_list = (char **) alloc((u.note_count + 1) * sizeof(char *));

    for (i = 0; i < u.note_count; i++) {
        new_list[i] = u.note_list[i];
    }

    /* ownership of clean is transferred to the list */
    new_list[u.note_count] = clean;

    if (u.note_list) {
        free((genericptr_t) u.note_list);
    }

    u.note_list = new_list;
    u.note_count++;
}

/* Delete a note by index */
void
delete_note(int index)
{
    char **new_list;
    int i, j;

    if (index < 0 || index >= u.note_count)
        return;

    if (u.note_list[index])
        free((genericptr_t) u.note_list[index]);

    if (u.note_count == 1) {
        if (u.note_list)
            free((genericptr_t) u.note_list);
        u.note_list = (char **) 0;
        u.note_count = 0;
        return;
    }

    new_list = (char **) alloc((u.note_count - 1) * sizeof(char *));

    for (i = 0, j = 0; i < u.note_count; i++) {
        if (i != index)
            new_list[j++] = u.note_list[i];
    }

    free((genericptr_t) u.note_list);
    u.note_list = new_list;
    u.note_count--;
}

/* Get a note by index (for display) */
const char *
get_note(int index)
{
    if (index < 0 || index >= u.note_count)
        return "";
    return u.note_list[index];
}

/* Set/update a note at index (replace existing text). */
void
set_note(int index, const char *text)
{
    char *clean;

    if (index < 0 || index >= u.note_count || !text)
        return;

    clean = sanitize_text(text);
    if (!clean) return;

    if (u.note_list[index])
        free((genericptr_t) u.note_list[index]);
    u.note_list[index] = clean;
}

/* Free all notes (for game cleanup) */
void
free_notes(void)
{
    int i;

    if (u.note_list) {
        for (i = 0; i < u.note_count; i++) {
            if (u.note_list[i])
                free((genericptr_t) u.note_list[i]);
        }
        free((genericptr_t) u.note_list);
        u.note_list = (char **) 0;
    }
    u.note_count = 0;
}

/* Save player notes */
void
save_notes(int fd)
{
    int i;
    int len;

    /* note_count is already saved as part of u struct, don't write it again */
    for (i = 0; i < u.note_count; i++) {
        if (u.note_list[i]) {
            len = strlen(u.note_list[i]);
            bwrite(fd, (genericptr_t) &len, sizeof(len));
            bwrite(fd, (genericptr_t) u.note_list[i], len + 1);
        } else {
            len = 0;
            bwrite(fd, (genericptr_t) &len, sizeof(len));
            bwrite(fd, (genericptr_t) "", 1);
        }
    }
}

/* Restore player notes */
void
restore_notes(int fd)
{
    int i;
    int len;

    /* note_count was already restored as part of u struct, don't read it again */
    if (u.note_count > 0) {
        u.note_list = (char **) alloc(u.note_count * sizeof(char *));
        for (i = 0; i < u.note_count; i++) {
            mread(fd, (genericptr_t) &len, sizeof(len));
            if (len > 0) {
                char *buf = (char *) alloc(len + 1);
                mread(fd, (genericptr_t) buf, len + 1);
                u.note_list[i] = sanitize_text(buf);
                free((genericptr_t) buf);
            } else {
                char tmp[1];
                mread(fd, (genericptr_t) tmp, 1);
                u.note_list[i] = sanitize_text(tmp);
            }
        }
    } else {
        u.note_list = (char **) 0;
    }
}

/* Simple notes UI: list, add, delete */

#ifdef CURSES_GRAPHICS
/* Curses-based multiline input dialog prototype (implemented in cursdial.c) */
extern void curses_multiline_input_dialog(const char *prompt, char *buf, int bufsize);
#endif

/* Collect multi-line input using repeated getlin() calls.
   End input by entering a single '.' on its own line, or cancel with ESC.
   Returns 1 if text was collected and placed into `out` (NUL-terminated),
   or 0 if cancelled or nothing entered. */
static int
get_multiline_input(char *out, int outsize)
{
    char line[BUFSZ];
    size_t used = 0;

    if (!out || outsize <= 0)
        return 0;

    out[0] = '\0';

    for (;;) {
        line[0] = '\0';
        getlin("(Enter '.' on its own line to finish, ESC to cancel)", line);

        if (line[0] == '\033') /* ESC */
            return 0;

        if (line[0] == '.' && line[1] == '\0')
            break; /* finished */

        /* ensure we have room for the line plus possible newline */
        if ((int)(used + strlen(line) + 2) > outsize - 1) {
            pline("Note too long; truncating.");
            /* append as much as fits and finish */
            int can = outsize - 1 - used;
            if (can > 0) {
                if (used > 0) {
                    out[used++] = '\n';
                    can--;
                }
                strncpy(out + used, line, can);
                used += can;
            }
            out[used] = '\0';
            break;
        }

        if (used > 0) {
            out[used++] = '\n';
            out[used] = '\0';
        }
        Strcpy(out + used, line);
        used += strlen(line);
    }

    return out[0] != '\0';
}

int
donotes(void)
{
#ifdef CURSES_GRAPHICS
    /* Use a menu-style dialog similar to #enhance when curses is available */
    winid win;
    anything any;
    menu_item *selected = NULL;
    int n;
    int i;
    char acc;

    do {
        acc = 'b'; /* 'a' reserved for Add, '-' reserved for Delete */
        win = create_nhwindow(NHW_MENU);
        start_menu(win);

        /* Add option */
        any = zeroany;
        any.a_int = -1;
        add_menu(win, NO_GLYPH, &any, 'a', 0, ATR_NONE,
                 "Add note", MENU_UNSELECTED);

        /* Delete option (use '-' as accelerator) */
        any = zeroany;
        any.a_int = -2;
        add_menu(win, NO_GLYPH, &any, '-', 0, ATR_NONE,
                 "Delete a note", MENU_UNSELECTED);

        /* Blank separator */
        add_menu(win, NO_GLYPH, &zeroany, 0, 0, ATR_NONE, "", MENU_UNSELECTED);

        /* List existing notes with accelerators b,c,d... */
        for (i = 0; i < u.note_count; i++) {
            any = zeroany;
            any.a_int = i + 1; /* index + 1 (so 0 index doesn't make identifier NULL) */
            char accch = acc;
            /* cycle accelerators: b..z, A..Z */
            if (accch > 'z')
                accch = 'A' + (accch - 'z' - 1);

            /* Display a one-line summary (first line) so accelerator is visible */
            char summary[BUFSZ];
            const char *full = get_note(i);
            int j = 0;
            /* Copy first line character by character */
            while (full[j] && full[j] != '\n' && j < (int)sizeof(summary) - 5) {
                summary[j] = full[j];
                j++;
            }
            summary[j] = '\0';
            if (full[j] == '\n') {
                Strcat(summary, " ...");
            }

            add_menu(win, NO_GLYPH, &any, accch, 0, ATR_NONE,
                     summary, MENU_UNSELECTED);
            acc++;
            if (acc == 'z' + 1)
                acc = 'A';
            else if (acc == 'Z' + 1)
                acc = 'a'; /* wrap around */
        }

        end_menu(win, "Notes");

        n = select_menu(win, PICK_ONE, &selected);
        destroy_nhwindow(win);

        if (n > 0) {
            int choice = selected[0].item.a_int;
            free((genericptr_t) selected);

            if (choice == -1) {
                /* Add note: open curses multiline editor */
                char buf[4096];
                buf[0] = '\0';
#ifdef CURSES_GRAPHICS
                curses_multiline_input_dialog("New note (F1=Help, F2=Save)", buf, sizeof buf);
                if (buf[0]) {
                    add_note(buf);
                    pline("Note added.");
                }
#else
                if (get_multiline_input(buf, sizeof buf)) {
                    add_note(buf);
                    pline("Note added.");
                }
#endif
            } else if (choice == -2) {
                /* Delete which note - show submenu */
                winid dwin = create_nhwindow(NHW_MENU);
                start_menu(dwin);
                for (i = 0; i < u.note_count; i++) {
                    any = zeroany;
                    any.a_int = i + 1; /* index + 1 to avoid NULL identifier */
                    add_menu(dwin, NO_GLYPH, &any, 'a' + (i % 52), 0, ATR_NONE,
                             get_note(i), MENU_UNSELECTED);
                }
                end_menu(dwin, "Delete which note?");
                n = select_menu(dwin, PICK_ONE, &selected);
                destroy_nhwindow(dwin);
                if (n > 0) {
                    delete_note(selected[0].item.a_int - 1); /* convert back to real index */
                    pline("Note deleted.");
                    free((genericptr_t) selected);
                }
            } else {
                /* Edit selected note (multiline) - choice is index+1, convert back */
                int note_idx = choice - 1;
                char buf[4096];
                Strcpy(buf, get_note(note_idx));
#ifdef CURSES_GRAPHICS
                curses_multiline_input_dialog("Edit note (F1=Help, F2=Save)", buf, sizeof buf);
                if (buf[0]) {
                    set_note(note_idx, buf);
                    pline("Note updated.");
                }
#else
                if (get_multiline_input(buf, sizeof buf)) {
                    set_note(note_idx, buf);
                    pline("Note updated.");
                }
#endif
            }
        } else {
            break; /* user quit */
        }
    } while (1);

    return 0;
#else
    /* Non-curses fallback: use previous text-based menu */
    char inbuf[BUFSZ];
    char notebuf[BUFSZ];
    char bigbuf[4096];
    int i;

    for (;;) {
        if (u.note_count == 0) {
            pline("No notes.");
        } else {
            for (i = 0; i < u.note_count; i++)
                pline("%d) %s", i + 1, u.note_list[i] ? u.note_list[i] : "");
        }

        getlin("Notes: (a)dd, (d)elete <#>, (q)uit", inbuf);
        if (!*inbuf || inbuf[0] == 'q' || inbuf[0] == 'Q')
            break;

        if (inbuf[0] == 'a' || inbuf[0] == 'A') {
            pline("Add: (s)ingle-line, (m)ulti-line (ESC to cancel)");
            int ch = readchar();
            if (ch == 'm' || ch == 'M') {
                /* collect multi-line input */
                bigbuf[0] = '\0';
                if (get_multiline_input(bigbuf, sizeof bigbuf)) {
                    add_note(bigbuf);
                    pline("Note added.");
                } else {
                    pline("Add cancelled.");
                }
            } else if (ch == 's' || ch == 'S' || ch == '\n' || ch == '\r') {
                notebuf[0] = '\0';
                getlin("New note: ", notebuf);
                if (*notebuf && *notebuf != '\033') {
                    add_note(notebuf);
                    pline("Note added.");
                }
            } else if (ch == '\033') {
                pline("Add cancelled.");
            } else {
                pline("Unknown add option.");
            }
        } else if (inbuf[0] == 'd' || inbuf[0] == 'D') {
            int idx = 0;
            /* allow e.g. "d 2" or just "d" and then prompt */
            if (sscanf(inbuf + 1, "%d", &idx) != 1) {
                getlin("Delete which note #? ", notebuf);
                idx = atoi(notebuf);
            }
            if (idx >= 1 && idx <= u.note_count) {
                delete_note(idx - 1);
                pline("Note deleted.");
            } else {
                pline("Invalid note number.");
            }
        } else {
            pline("Unknown notes command.");
        }
    }

    return 0;
#endif
}
