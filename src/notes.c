/* NetHack 3.6    notes.c
 * Simple in-game personal notes feature.
 * Stores multiple short notes, editable via name/call-style input.
 */

#include "hack.h"
#include "lev.h" /* for FREE_SAVE macro */
#ifdef CURSES_GRAPHICS
#include "../win/curses/cursdial.h"
#endif

typedef struct player_note {
    struct player_note *next;
    char *text;
    unsigned lth;
} player_note;

static player_note *notes_head = (player_note *) 0;

/* Helper: escape newlines for single-line editor ("\n" sequences). */
STATIC_OVL void
escape_newlines_for_edit(src, dst, dstsize)
const char *src;
char *dst;
int dstsize;
{
    int si = 0, di = 0;
    if (!src || !dst || dstsize <= 0)
        return;
    while (src[si] != '\0' && di < dstsize - 1) {
        if (src[si] == '\n') {
            /* need two chars: '\\' 'n' */
            if (di + 2 >= dstsize - 1)
                break;
            dst[di++] = '\\';
            dst[di++] = 'n';
            ++si;
        } else {
            dst[di++] = src[si++];
        }
    }
    dst[di] = '\0';
}

/* Helper: unescape "\\n" sequences in-place to actual newline chars */
STATIC_OVL void
unescape_newlines_inplace(buf)
char *buf;
{
    int i = 0, j = 0;
    if (!buf)
        return;
    while (buf[i] != '\0') {
        if (buf[i] == '\\' && buf[i + 1] == 'n') {
            buf[j++] = '\n';
            i += 2;
        } else {
            buf[j++] = buf[i++];
        }
    }
    buf[j] = '\0';
}

/* Mungspaces variant that preserves newlines while collapsing spaces per-line */
STATIC_OVL void
mungspaces_preserve_newlines(bp)
char *bp;
{
    char c, *p, *p2;
    boolean was_space = TRUE;

    if (!bp)
        return;

    for (p = p2 = bp; (c = *p) != '\0'; p++) {
        if (c == '\t')
            c = ' ';
        if (c == '\n') {
            /* newline: copy it and reset space-state */
            *p2++ = '\n';
            was_space = TRUE;
            continue;
        }
        if (c != ' ' || !was_space)
            *p2++ = c;
        was_space = (c == ' ');
    }
    if (was_space && p2 > bp)
        p2--;
    *p2 = '\0';
}

/* Add a new note (dupstr'd) */
void
add_note(const char *s)
{
    player_note *n;
    if (!s || !*s)
        return;
    n = (player_note *) alloc((unsigned) sizeof *n);
    n->lth = strlen(s);
    n->text = dupstr(s);
    n->next = (player_note *) 0;

    /* Append to tail to preserve entered order */
    if (!notes_head) {
        notes_head = n;
    } else {
        player_note *t;
        for (t = notes_head; t->next; t = t->next)
            ;
        t->next = n;
    }
}

/* Remove and free a single note by index (0-based). Returns TRUE if removed */
boolean
remove_note_index(int idx)
{
    player_note *curr = notes_head, *prev = (player_note *) 0;
    int i = 0;

    for (; curr; prev = curr, curr = curr->next, ++i) {
        if (i == idx) {
            if (prev)
                prev->next = curr->next;
            else
                notes_head = curr->next;
            if (curr->text)
                free((genericptr_t) curr->text);
            free((genericptr_t) curr);
            return TRUE;
        }
    }
    return FALSE;
}

/* Free all notes */
void
clear_notes(void)
{
    player_note *curr = notes_head, *next;
    while (curr) {
        next = curr->next;
        if (curr->text)
            free((genericptr_t) curr->text);
        free((genericptr_t) curr);
        curr = next;
    }
    notes_head = (player_note *) 0;
}

/* Display notes in a simple numbered list */
void
show_notes(void)
{
    player_note *curr = notes_head;
    int i = 0;
    winid win;
    menu_item *pick_list = 0;
    anything any;

    if (!curr) {
        pline("No notes.");
        return;
    }

    win = create_nhwindow(NHW_MENU);
    start_menu(win);
    for (; curr; curr = curr->next, ++i) {
        char bufdisp[BUFSZ];
        Sprintf(bufdisp, "%d: %.60s", i + 1, curr->text);
        any = zeroany;
        any.a_int = i + 1; /* 1-based index so first item is selectable */
        add_menu(win, NO_GLYPH, &any, 0, 0, ATR_NONE, bufdisp, 0);
    }
    end_menu(win, "Select a note to edit (or Esc/Enter without choosing to cancel)");

    if (select_menu(win, PICK_ONE, &pick_list) > 0) {
        int idx = pick_list[0].item.a_int - 1; /* convert from 1-based to 0-based */
        char buf[BUFSZ];
        player_note *p = notes_head;
        int j;

        free((genericptr_t) pick_list);
        destroy_nhwindow(win);

        /* locate the note */
        for (j = 0; p && j < idx; p = p->next, ++j)
            ;
        if (!p)
            return;

        /* Prefill buffer. For curses we allow real newlines; otherwise escape them */
        buf[0] = '\0';
#ifdef CURSES_GRAPHICS
        if (WINDOWPORT("curses")) {
            if (p->text)
                Strcpy(buf, p->text);
        } else
#endif
        {
            if (p->text)
                escape_newlines_for_edit(p->text, buf, BUFSZ);
        }

        pline("(Arrows/Ctrl+B/F=move, Home/Ctrl+A, End/Ctrl+E, Del/Ctrl+D, Ctrl+W=word, Ctrl+U=line, Ctrl+O=insert \\n)");
#ifdef CURSES_GRAPHICS
        if (WINDOWPORT("curses")) {
            char prompt[BUFSZ];
            Sprintf(prompt, "Edit note %d (Ctrl+O inserts '\\n'):", idx + 1);
            curses_name_input_dialog(prompt, buf, BUFSZ);
        } else
#endif
        {
            /* Non-curses: instruct about \n sequence */
            getlin("Edit note (use '\\n' for newline):", buf);
        }

        /* If user pressed a literal ESC (non-curses), treat as cancel */
        if (buf[0] == '\033' && buf[1] == '\0') {
            pline("Edit cancelled.");
            return;
        }

        /* If non-curses, convert escaped '\\n' sequences into actual newlines */
#ifndef CURSES_GRAPHICS
        /* Unescape '\\n' sequences into actual newlines */
        unescape_newlines_inplace(buf);
#endif

        /* Collapse consecutive spaces per-line but preserve newlines */
        mungspaces_preserve_newlines(buf);

        /* Empty input means delete the note */
        if (!*buf) {
            if (remove_note_index(idx))
                pline("Note deleted.");
            else
                pline("Failed to delete note.");
            return;
        }

        /* If changed, update the note */
        if (!p->text || strcmp(p->text, buf) != 0) {
            if (p->text)
                free((genericptr_t) p->text);
            p->text = dupstr(buf);
            p->lth = strlen(p->text);
            pline("Note updated.");
        } else {
            pline("No change.");
        }
    } else {
        destroy_nhwindow(win);
        pline("No note selected.");
    }
}

/* Extended command driver */
int
donotes(void)
{
    menu_item *pick_list = 0;
    anything any;
    int done = 0;

    /* loop so user can add/edit multiple notes without reopening the menu */
    while (!done) {
        winid win = create_nhwindow(NHW_MENU);
        start_menu(win);

        /* Add actions: Add new note, Delete a note (appear at top) */
        any = zeroany;
        any.a_int = -1;
        add_menu(win, NO_GLYPH, &any, 'a', 0, ATR_NONE, "Add a new note", 0);
        any = zeroany;
        any.a_int = -3;
        add_menu(win, NO_GLYPH, &any, '-', 0, ATR_NONE, "Delete a note", 0);

        /* Add each note as a selectable item with an explicit selector */
        player_note *p = notes_head;
        int i = 0;
        for (p = notes_head; p; p = p->next, ++i) {
            char buf[BUFSZ];
            char disp[BUFSZ];
            int sel = 0;

            /* Build a one-line display by taking text up to first newline */
            if (p->text) {
                int k;
                for (k = 0; k < (int)sizeof disp - 1 && p->text[k] && p->text[k] != '\n'; ++k)
                    disp[k] = p->text[k];
                disp[k] = '\0';
            } else
                Strcpy(disp, "");

            Sprintf(buf, "%d: %.60s", i + 1, disp);
            any = zeroany;
            any.a_int = i + 1; /* 1-based index so zero-any checks don't suppress first item */
            /* Prefer to reserve 'a' for Add; use lowercase b-z, then uppercase A-Z, then digits 0-9 */
            if (i < 25)
                sel = 'b' + i;        /* 'b'..'z' for first 25 notes */
            else if (i < 51)
                sel = 'A' + (i - 25); /* 'A'..'Z' next */
            else if (i < 61)
                sel = '0' + (i - 51); /* '0'..'9' after that */
            else
                sel = 0; /* no selector available beyond that */

            add_menu(win, NO_GLYPH, &any, sel, 0, ATR_NONE, buf, 0);
        }

        end_menu(win, "Notes: select a letter to edit a note; 'a' to add; '-' to delete; Esc to exit");

        if (select_menu(win, PICK_ONE, &pick_list) > 0) {
            int code = pick_list[0].item.a_int;
            /* convert from 1-based index to 0-based for notes; keep -1/-2 for actions */
            if (code > 0)
                code -= 1;
            free((genericptr_t) pick_list);
            destroy_nhwindow(win);

            /* Note selected for editing */
            if (code >= 0) {
                char buf[BUFSZ];
                player_note *selp = notes_head;
                int j;

                for (j = 0; selp && j < code; selp = selp->next, ++j)
                    ;
                if (!selp)
                    continue;

                /* Prefill buffer. For curses we allow real newlines; otherwise escape them */
                buf[0] = '\0';
#ifdef CURSES_GRAPHICS
                if (WINDOWPORT("curses")) {
                    if (selp->text)
                        Strcpy(buf, selp->text);
                } else
#endif
                {
                    if (selp->text)
                        escape_newlines_for_edit(selp->text, buf, BUFSZ);
                }

                pline("(Arrows/Ctrl+B/F=move, Home/Ctrl+A, End/Ctrl+E, Del/Ctrl+D, Ctrl+W=word, Ctrl+U=line, Ctrl+O=insert \\n)");
#ifdef CURSES_GRAPHICS
                if (WINDOWPORT("curses")) {
                    char prompt[BUFSZ];
                    Sprintf(prompt, "Edit note %d (Ctrl+O inserts '\\n'):", code + 1);
                    curses_name_input_dialog(prompt, buf, BUFSZ);
                } else
#endif
                {
                    /* Non-curses: instruct about \n sequence */
                    getlin("Edit note (use '\\n' for newline):", buf);
                }

                /* Cancelled in non-curses if literal ESC */
                if (buf[0] == '\033' && buf[1] == '\0') {
                    pline("Edit cancelled.");
                    continue;
                }

#ifndef CURSES_GRAPHICS
                /* Unescape '\\n' sequences into actual newlines */
                unescape_newlines_inplace(buf);
#endif

                /* Collapse consecutive spaces per-line but preserve newlines */
                mungspaces_preserve_newlines(buf);

                /* Empty input deletes the note */
                if (!*buf) {
                    if (remove_note_index(code))
                        pline("Note deleted.");
                    else
                        pline("Failed to delete note.");
                    continue;
                }

                /* Update if changed */
                if (!selp->text || strcmp(selp->text, buf) != 0) {
                    if (selp->text)
                        free((genericptr_t) selp->text);
                    selp->text = dupstr(buf);
                    selp->lth = strlen(selp->text);
                    pline("Note updated.");
                } else {
                    pline("No change.");
                }
            } else if (code == -1) {
                /* Add a new note */
                char nbuf[BUFSZ];
                /* Prefill with empty buffer (curses supports real newlines via Ctrl+O) */
                Sprintf(nbuf, "");
                pline("(Arrows/Ctrl+B/F=move, Home/Ctrl+A, End/Ctrl+E, Del/Ctrl+D, Ctrl+W=word, Ctrl+U=line, Ctrl+O=insert \\n)");
#ifdef CURSES_GRAPHICS
                if (WINDOWPORT("curses")) {
                    curses_name_input_dialog("Add note (Ctrl+O inserts '\\n'):", nbuf, BUFSZ);
                } else
#endif
                {
                    getlin("Add note (use '\\n' for newline):", nbuf);
                }
#ifndef CURSES_GRAPHICS
                /* Convert '\\n' sequences to actual newlines */
                unescape_newlines_inplace(nbuf);
#endif
                /* Collapse spaces per-line but preserve newlines so multi-line notes survive */
                mungspaces_preserve_newlines(nbuf);
                if (!*nbuf || (*nbuf == '\033' && nbuf[1] == '\0')) {
                    pline("No note added.");
                } else {
                    if (*nbuf) {
                        add_note(nbuf);
                        pline("Note added.");
                    }
                }
            } else if (code == -3) {
                /* Delete a note: choose note to delete */
                if (!notes_head) {
                    pline("No notes.");
                    continue;
                }
                {
                    winid dwin = create_nhwindow(NHW_MENU);
                    menu_item *dlist = 0;
                    int j = 0;
                    player_note *q;

                    start_menu(dwin);
                    for (q = notes_head; q; q = q->next, ++j) {
                        char dbuf[BUFSZ];
                        any = zeroany;
                        any.a_int = j + 1;
                        Sprintf(dbuf, "%d: %.60s", j + 1, q->text ? q->text : "");
                        /* reuse same selector mapping for quick selection */
                        int sel = 0;
                        if (j < 25)
                            sel = 'b' + j;
                        else if (j < 51)
                            sel = 'A' + (j - 25);
                        else if (j < 61)
                            sel = '0' + (j - 51);
                        add_menu(dwin, NO_GLYPH, &any, sel, 0, ATR_NONE, dbuf, 0);
                    }
                    end_menu(dwin, "Select a note to DELETE (Esc to cancel)");
                    if (select_menu(dwin, PICK_ONE, &dlist) > 0) {
                        int idx = dlist[0].item.a_int - 1;
                        free((genericptr_t) dlist);
                        destroy_nhwindow(dwin);
                        char prompt[QBUFSZ];
                        Sprintf(prompt, "Delete note %d? (y/n)", idx + 1);
                        if (yn(prompt) == 'y') {
                            if (remove_note_index(idx)) {
                                pline("Note deleted.");
                                done = 1; /* close menu on confirm */
                            } else {
                                pline("Failed to delete note.");
                            }
                        } else {
                            pline("Delete cancelled.");
                        }
                    } else {
                        destroy_nhwindow(dwin);
                    }
                }
            }

            /* continue to redraw the menu */
        } else {
            destroy_nhwindow(win);
            /* Refresh display to match boomerang cleanup and avoid stuck modifier state */
            docrt();
            flush_screen(1);
            done = 1;
        }
    }

    return 0;
}

/* Save notes: write sequence of note lengths followed by note bytes, terminated by -1 */
void
save_notes(int fd, int mode)
{
    player_note *curr;
    int len;

#ifdef MFLOPPY
    if (mode & COUNT_SAVE) {
        /* counted by bytes scheme elsewhere; we don't strip separately here */
    }
#endif
    for (curr = notes_head; curr; curr = curr->next) {
        /* Convert internal LF to CRLF for portability when writing savefile */
        int i, j;
        /* worst-case CRLF expansion size: up to twice original length */
        int orig = (int) curr->lth;
        char *tmp = (char *) alloc((unsigned) (orig * 2 + 1));

        for (i = 0, j = 0; i < orig; ++i) {
            if (curr->text[i] == '\n') {
                tmp[j++] = '\r';
                tmp[j++] = '\n';
            } else {
                tmp[j++] = curr->text[i];
            }
        }
        len = j; /* length of converted data */
        bwrite(fd, (genericptr_t) &len, sizeof len);
        if (len)
            bwrite(fd, (genericptr_t) tmp, len);
        free((genericptr_t) tmp);
    }
    len = -1;
    bwrite(fd, (genericptr_t) &len, sizeof len);

    /* Free on release_data so save caller can pass mode */
    if (release_data(mode))
        clear_notes();
}

/* Restore notes */
void
restore_notes(int fd)
{
    int len;
    char *buf;

    while (1) {
        mread(fd, (genericptr_t) &len, sizeof len);
        if (len == -1)
            break;
        if (len <= 0) {
            /* zero-length note -- skip */
            continue;
        }
        /* read raw bytes (may contain CRLF), convert CRLF -> LF in place */
        buf = (char *) alloc((unsigned) len + 1);
        mread(fd, (genericptr_t) buf, len);
        buf[len] = '\0';

        /* Convert CRLF sequences to LF (in-place) */
        {
            int i = 0, j = 0;
            while (buf[i]) {
                if (buf[i] == '\r' && buf[i + 1] == '\n') {
                    buf[j++] = '\n';
                    i += 2;
                } else {
                    buf[j++] = buf[i++];
                }
            }
            buf[j] = '\0';
        }

        /* Sanity: collapse spaces per-line while preserving newlines */
        mungspaces_preserve_newlines(buf);

        add_note(buf);
        free((genericptr_t) buf);
    }
}
