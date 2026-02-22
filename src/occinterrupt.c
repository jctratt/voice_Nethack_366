/* NetHack 3.6    occinterrupt.c    $NHDT-Date: 2025-12-27 $ */
/* Copyright (c) NetHack Development Team 2025 */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Occupation Interruption Control System
 *
 * Allows players to configure how digging and eating occupations respond
 * to approaching monsters. Supports prompting for action limits and HP
 * loss thresholds.
 */

#include "hack.h"

/* Forward declarations */
STATIC_DCL boolean FDECL(prompt_occ_interrupt, (struct monst *));

/*
 * init_occ_interrupt()
 * Initialize occupation interruption control when an occupation starts
 */
void
init_occ_interrupt(VOID_ARGS)
{
    context.occ_interrupt.active = 1;
    context.occ_interrupt.last_threat_id = 0;
    context.occ_interrupt.start_hp = Upolyd ? u.mh : u.uhp;
    context.occ_interrupt.hp_budget = 0;
    context.occ_interrupt.actions_remaining = 0;
    context.occ_interrupt.continue_mode = 0;
}

/*
 * reset_occ_interrupt()
 * Clear occupation interruption state when occupation ends or is interrupted
 */
void
reset_occ_interrupt(VOID_ARGS)
{
    context.occ_interrupt.active = 0;
    context.occ_interrupt.last_threat_id = 0;
    context.occ_interrupt.start_hp = 0;
    context.occ_interrupt.hp_budget = 0;
    context.occ_interrupt.actions_remaining = 0;
    context.occ_interrupt.continue_mode = 0;
}

/*
 * check_occ_interrupt()
 * Called when a threat is detected during an occupation
 * Returns TRUE if occupation should continue, FALSE if it should stop
 */
boolean
check_occ_interrupt(mtmp)
struct monst *mtmp;
{
    if (!context.occ_interrupt.active)
        return FALSE; /* interruption control not active */

    /* Check if this is the same threat as before */
    if (context.occ_interrupt.last_threat_id == mtmp->m_id)
        return TRUE; /* same monster, continue without re-prompting */

    /* Check if user chose to continue uninterrupted */
    if (context.occ_interrupt.continue_mode)
        return TRUE; /* continue without prompting */

    /* New threat detected - prompt user */
    boolean result = prompt_occ_interrupt(mtmp);

    /* For digging, explicitly clear occupation if user said no,
     * since stop_occupation() has special handling for digging that skips clearing it */
    if (!result && is_digging()) {
        occupation = 0;
    }

    return result;
}

/*
 * prompt_occ_interrupt()
 * Display a prompt asking the user how they want to handle an approaching monster
 * Returns TRUE if occupation should continue, FALSE if it should stop
 */
STATIC_OVL boolean
prompt_occ_interrupt(mtmp)
struct monst *mtmp;
{
    char ans[BUFSZ];
    char *endp;
    long val;
    int choice;

    /* Remember this threat */
    context.occ_interrupt.last_threat_id = mtmp->m_id;

    /* Display the threat */
    pline("%s is approaching!", Monnam(mtmp));

    for (;;) {
        /* Show the options: yes interrupts, no continues */
        choice = yn_function("Interrupt your occupation? [y/n/a:actions/h:hp/?]", "ynah?", 'y');

        switch (choice) {
        case 'y':
            if (occtxt && *occtxt)
                You("stop %s.", occtxt);
            else
                You("stop.");
            return FALSE; /* stop occupation */

        case 'n':
            return TRUE; /* continue occupation */

        case 'a':
            /* Set action limit */
            ans[0] = '\0';
            getlin("How many more actions?", ans);
            if (ans[0] && ans[0] != '\033') {
                val = strtol(ans, &endp, 10);
                if (endp > ans && *endp == '\0' && val > 0L && val <= LARGEST_INT) {
                    context.occ_interrupt.actions_remaining = (int) val;
                    pline("Will stop after %d more actions.",
                          context.occ_interrupt.actions_remaining);
                    return TRUE; /* continue */
                }
                pline("That's not valid.");
            }
            break;

        case 'h':
            /* Set HP loss limit */
            ans[0] = '\0';
            getlin("How much hp do you want to lose before occupation ends?", ans);
            if (ans[0] && ans[0] != '\033') {
                val = strtol(ans, &endp, 10);
                if (endp > ans && *endp == '\0' && val > 0L && val <= LARGEST_INT) {
                    context.occ_interrupt.hp_budget = (int) val;
                    context.occ_interrupt.start_hp = Upolyd ? u.mh : u.uhp;
                    pline("Will stop if you lose more than %d HP.",
                          context.occ_interrupt.hp_budget);
                    return TRUE; /* continue */
                }
                pline("That's not valid.");
            }
            break;

        case '?':
            pline("y: interrupt now, n: keep going, a: set action limit, h: set HP-loss limit");
            break;

        default:
            break;
        }
    }
}

/*
 * handle_occ_limits()
 * Called each turn during occupation to check if limits have been reached
 * Returns TRUE if occupation should stop, FALSE if it should continue
 */
boolean
handle_occ_limits(VOID_ARGS)
{
    int current_hp, hp_lost;

    if (!context.occ_interrupt.active)
        return FALSE; /* interruption control not active */

    /* Check action limit */
    if (context.occ_interrupt.actions_remaining > 0) {
        context.occ_interrupt.actions_remaining--;
        if (context.occ_interrupt.actions_remaining == 0) {
            pline("Action limit reached.");
            /* Reset to prompt mode so next threat will prompt */
            context.occ_interrupt.continue_mode = 0;
            context.occ_interrupt.last_threat_id = 0;
            return TRUE; /* stop occupation */
        }
    }

    /* Check HP loss limit */
    if (context.occ_interrupt.hp_budget > 0) {
        current_hp = Upolyd ? u.mh : u.uhp;
        hp_lost = context.occ_interrupt.start_hp - current_hp;

        if (hp_lost >= context.occ_interrupt.hp_budget) {
            pline("HP loss limit reached (lost %d HP).", hp_lost);
            /* Reset to prompt mode so next threat will prompt */
            context.occ_interrupt.continue_mode = 0;
            context.occ_interrupt.last_threat_id = 0;
            return TRUE; /* stop occupation */
        }
    }

    return FALSE; /* continue */
}
