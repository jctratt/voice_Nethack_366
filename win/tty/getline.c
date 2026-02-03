/* NetHack 3.6	getline.c	$NHDT-Date: 1543830347 2018/12/03 09:45:47 $  $NHDT-Branch: NetHack-3.6.2-beta01 $:$NHDT-Revision: 1.37 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Michael Allison, 2006. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#ifdef TTY_GRAPHICS

#if !defined(MAC)
#define NEWAUTOCOMP
#endif

#include "wintty.h"
#include "func_tab.h"

/* Macro for control characters */
#ifndef C
#define C(c) (0x1f & (c))
#endif

char morc = 0; /* tell the outside world what char you chose */
STATIC_VAR boolean suppress_history;
STATIC_DCL boolean FDECL(ext_cmd_getlin_hook, (char *));

typedef boolean FDECL((*getlin_hook_proc), (char *));

STATIC_DCL void FDECL(hooked_tty_getlin,
                      (const char *, char *, getlin_hook_proc));
extern int NDECL(extcmd_via_menu); /* cmd.c */

extern char erase_char, kill_char; /* from appropriate tty.c file */

/*
 * Read a line closed with '\n' into the array char bufp[BUFSZ].
 * (The '\n' is not stored. The string is closed with a '\0'.)
 * Reading can be interrupted by an escape ('\033') - now the
 * resulting string is "\033".
 */
void
tty_getlin(query, bufp)
const char *query;
register char *bufp;
{
    suppress_history = FALSE;
    hooked_tty_getlin(query, bufp, (getlin_hook_proc) 0);
}

STATIC_OVL void
hooked_tty_getlin(query, bufp, hook)
const char *query;
register char *bufp;
getlin_hook_proc hook;
{
    register char *obufp = bufp;
    register char *curpos = bufp;  /* cursor position in buffer */
    register int c;
    struct WinDesc *cw = wins[WIN_MESSAGE];
    boolean doprev = 0;

    if (ttyDisplay->toplin == 1 && !(cw->flags & WIN_STOP))
        more();
    cw->flags &= ~WIN_STOP;
    ttyDisplay->toplin = 3; /* special prompt state */
    ttyDisplay->inread++;

    /* issue the prompt */
    custompline(OVERRIDE_MSGTYPE | SUPPRESS_HISTORY, "%s ", query);
#ifdef EDIT_GETLIN
    /* bufp is input/output; treat current contents (presumed to be from
       previous getlin()) as default input */
    addtopl(obufp);
    bufp = eos(obufp);
    curpos = bufp;  /* cursor starts at end */
#else
    /* !EDIT_GETLIN: bufp is output only; init it to empty */
    *bufp = '\0';
    curpos = bufp;
#endif

    for (;;) {
        (void) fflush(stdout);
        Strcat(strcat(strcpy(toplines, query), " "), obufp);
        c = pgetchar();
        if (c == '\033' || c == EOF) {
            if (c == '\033' && obufp[0] != '\0') {
                obufp[0] = '\0';
                bufp = obufp;
                curpos = bufp;
                tty_clear_nhwindow(WIN_MESSAGE);
                cw->maxcol = cw->maxrow;
                addtopl(query);
                addtopl(" ");
                addtopl(obufp);
            } else {
                obufp[0] = '\033';
                obufp[1] = '\0';
                break;
            }
        }
        if (ttyDisplay->intr) {
            ttyDisplay->intr--;
            *bufp = 0;
            curpos = bufp;
        }
        if (c == '\020') { /* ctrl-P */
            if (iflags.prevmsg_window != 's') {
                int sav = ttyDisplay->inread;

                ttyDisplay->inread = 0;
                (void) tty_doprev_message();
                ttyDisplay->inread = sav;
                tty_clear_nhwindow(WIN_MESSAGE);
                cw->maxcol = cw->maxrow;
                addtopl(query);
                addtopl(" ");
                *bufp = 0;
                addtopl(obufp);
                curpos = bufp;
            } else {
                if (!doprev)
                    (void) tty_doprev_message(); /* need two initially */
                (void) tty_doprev_message();
                doprev = 1;
                continue;
            }
        } else if (doprev && iflags.prevmsg_window == 's') {
            tty_clear_nhwindow(WIN_MESSAGE);
            cw->maxcol = cw->maxrow;
            doprev = 0;
            addtopl(query);
            addtopl(" ");
            *bufp = 0;
            addtopl(obufp);
            curpos = bufp;
        }
        if (c == erase_char || c == '\b') {
            if (curpos != obufp) {
#ifdef NEWAUTOCOMP
                char *i;

#endif /* NEWAUTOCOMP */
                curpos--;
                /* Delete character and shift remaining text left */
                for (char *p = curpos; *p; ++p)
                    *p = *(p + 1);
                bufp = eos(obufp);
#ifndef NEWAUTOCOMP
                putsyms("\b \b"); /* putsym converts \b */
#else                             /* NEWAUTOCOMP */
                putsyms("\b");
                for (i = curpos; *i; ++i)
                    putsyms(" ");
                for (; i > curpos; --i)
                    putsyms("\b");
#endif                            /* NEWAUTOCOMP */
            } else
                tty_nhbell();
        } else if (c == '\n' || c == '\r') {
#ifndef NEWAUTOCOMP
            *bufp = 0;
#endif /* not NEWAUTOCOMP */
            break;
        } else if (c == C('h') || c == C('b')) {
            /* Ctrl+H or Ctrl+B: move cursor left */
            if (curpos > obufp) {
                curpos--;
                putsyms("\b");
            } else
                tty_nhbell();
        } else if (c == C('l') || c == C('f')) {
            /* Ctrl+L or Ctrl+F: move cursor right */
            if (*curpos) {
                putsyms(curpos);
                curpos++;
            } else
                tty_nhbell();
        } else if (c == C('a')) {
            /* Ctrl+A: move to beginning */
            while (curpos > obufp) {
                curpos--;
                putsyms("\b");
            }
        } else if (c == C('e')) {
            /* Ctrl+E: move to end */
            while (*curpos) {
                putsyms(curpos);
                curpos++;
            }
        } else if (c == C('d')) {
            /* Ctrl+D: delete character at cursor */
            if (*curpos) {
                char *p = curpos;
                while (*p) {
                    *p = *(p + 1);
                    ++p;
                }
                bufp = eos(obufp);
                /* Redraw from cursor to end */
                for (p = curpos; *p; ++p)
                    putsyms(" ");
                for (p = eos(obufp); p > curpos; --p)
                    putsyms("\b");
            }
        } else if (c == C('w')) {
            /* Ctrl+W: delete word before cursor */
            if (curpos > obufp) {
                char *p = curpos;
                /* Skip spaces before cursor */
                while (p > obufp && *(p - 1) == ' ')
                    p--;
                /* Skip word characters */
                while (p > obufp && *(p - 1) != ' ')
                    p--;
                /* Delete from p to curpos */
                char *q = p;
                while (*curpos) {
                    *q = *curpos;
                    ++q;
                    ++curpos;
                }
                *q = '\0';
                bufp = eos(obufp);
                curpos = p;
                /* Redraw from new position */
                for (int i = 0; i < (int)(eos(obufp) - p); ++i)
                    putsyms(" ");
                for (int i = 0; i < (int)(eos(obufp) - p); ++i)
                    putsyms("\b");
                for (p = curpos; *p; ++p)
                    putsyms(p);
                for (p = eos(obufp); p > curpos; --p)
                    putsyms("\b");
            }
        } else if (' ' <= (unsigned char) c && c != '\177'
                   /* avoid isprint() - some people don't have it
                      ' ' is not always a printing char */
                   && (bufp - obufp < BUFSZ - 1 && bufp - obufp < COLNO)) {
#ifdef NEWAUTOCOMP
            char *i = eos(bufp);

#endif /* NEWAUTOCOMP */
            /* Insert character at cursor position */
            char *p = eos(obufp);
            while (p > curpos) {
                *p = *(p - 1);
                --p;
            }
            *curpos = c;
            curpos++;
            bufp = eos(obufp);
            putsyms(curpos - 1);
            if (hook && (*hook)(obufp)) {
                putsyms(curpos);
#ifndef NEWAUTOCOMP
                bufp = eos(bufp);
#else  /* NEWAUTOCOMP */
                /* pointer and cursor left where they were */
                for (i = curpos; *i; ++i)
                    putsyms("\b");
            } else if (i > curpos) {
                char *s = i;

                /* erase rest of prior guess */
                for (; i > curpos; --i)
                    putsyms(" ");
                for (; s > curpos; --s)
                    putsyms("\b");
#endif /* NEWAUTOCOMP */
            }
        } else if (c == kill_char || c == '\177') { /* Robert Viduya */
            /* this test last - @ might be the kill_char */
#ifndef NEWAUTOCOMP
            while (curpos != obufp) {
                curpos--;
                putsyms("\b \b");
            }
#else  /* NEWAUTOCOMP */
            for (; *curpos; ++curpos)
                putsyms(" ");
            for (; curpos != obufp; --curpos)
                putsyms("\b \b");
            *curpos = 0;
#endif /* NEWAUTOCOMP */
            bufp = curpos;
        } else if (c == C('?')) {
            /* Ctrl+? - show help */
            pline("Editing keys: Arrows/Ctrl+B/F=move, Home/Ctrl+A=start, End/Ctrl+E=end, Del/Ctrl+D=delete, Ctrl+W=word, Ctrl+U=line");
            tty_clear_nhwindow(WIN_MESSAGE);
            cw->maxcol = cw->maxrow;
            addtopl(query);
            addtopl(" ");
            *bufp = 0;
            addtopl(obufp);
            curpos = bufp;
            continue;
        } else
            tty_nhbell();
    }
    ttyDisplay->toplin = 2; /* nonempty, no --More-- required */
    ttyDisplay->inread--;
    clear_nhwindow(WIN_MESSAGE); /* clean up after ourselves */

    if (suppress_history) {
        /* prevent next message from pushing current query+answer into
           tty message history */
        *toplines = '\0';
#ifdef DUMPLOG
    } else {
        /* needed because we've bypassed pline() */
        dumplogmsg(toplines);
#endif
    }
}

void
xwaitforspace(s)
register const char *s; /* chars allowed besides return */
{
    register int c, x = ttyDisplay ? (int) ttyDisplay->dismiss_more : '\n';

    morc = 0;
    while (
#ifdef HANGUPHANDLING
        !program_state.done_hup &&
#endif
        (c = tty_nhgetch()) != EOF) {
        if (c == '\n' || c == '\r')
            break;

        if (iflags.cbreak) {
            if (c == '\033') {
                if (ttyDisplay)
                    ttyDisplay->dismiss_more = 1;
                morc = '\033';
                break;
            }
            if ((s && index(s, c)) || c == x || (x == '\n' && c == '\r')) {
                morc = (char) c;
                break;
            }
            tty_nhbell();
        }
    }
}

/*
 * Implement extended command completion by using this hook into
 * tty_getlin.  Check the characters already typed, if they uniquely
 * identify an extended command, expand the string to the whole
 * command.
 *
 * Return TRUE if we've extended the string at base.  Otherwise return FALSE.
 * Assumptions:
 *
 *	+ we don't change the characters that are already in base
 *	+ base has enough room to hold our string
 */
STATIC_OVL boolean
ext_cmd_getlin_hook(base)
char *base;
{
    int oindex, com_index;

    com_index = -1;
    for (oindex = 0; extcmdlist[oindex].ef_txt != (char *) 0; oindex++) {
        if (extcmdlist[oindex].flags & CMD_NOT_AVAILABLE)
            continue;
        if ((extcmdlist[oindex].flags & AUTOCOMPLETE)
            && !(!wizard && (extcmdlist[oindex].flags & WIZMODECMD))
            && !strncmpi(base, extcmdlist[oindex].ef_txt, strlen(base))) {
            if (com_index == -1) /* no matches yet */
                com_index = oindex;
            else /* more than 1 match */
                return FALSE;
        }
    }
    if (com_index >= 0) {
        Strcpy(base, extcmdlist[com_index].ef_txt);
        return TRUE;
    }

    return FALSE; /* didn't match anything */
}

/*
 * Read in an extended command, doing command line completion.  We
 * stop when we have found enough characters to make a unique command.
 */
int
tty_get_ext_cmd()
{
    int i;
    char buf[BUFSZ];

    if (iflags.extmenu)
        return extcmd_via_menu();

    suppress_history = TRUE;
    /* maybe a runtime option?
     * hooked_tty_getlin("#", buf,
     *                   (flags.cmd_comp && !in_doagain)
     *                      ? ext_cmd_getlin_hook
     *                      : (getlin_hook_proc) 0);
     */
    buf[0] = '\0';
    hooked_tty_getlin("#", buf, in_doagain ? (getlin_hook_proc) 0
                                           : ext_cmd_getlin_hook);
    (void) mungspaces(buf);
    if (buf[0] == 0 || buf[0] == '\033')
        return -1;

    for (i = 0; extcmdlist[i].ef_txt != (char *) 0; i++)
        if (!strcmpi(buf, extcmdlist[i].ef_txt))
            break;

    if (!in_doagain) {
        int j;
        for (j = 0; buf[j]; j++)
            savech(buf[j]);
        savech('\n');
    }

    if (extcmdlist[i].ef_txt == (char *) 0) {
        pline("%s: unknown extended command.", buf);
        i = -1;
    }

    return i;
}

#endif /* TTY_GRAPHICS */

/*getline.c*/
