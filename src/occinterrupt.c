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
    char buf[BUFSZ], ans[BUFSZ];
    int choice;

    /* Remember this threat */
    context.occ_interrupt.last_threat_id = mtmp->m_id;

    /* Display the threat */
    pline("%s is approaching!", Monnam(mtmp));

    /* Show the options */
    choice = yn_function("Continue your occupation? [(y)es (n)o (a)ction limit (h)p limit (q)uit]",
                        "ynah", 'n');

    switch (choice) {
    case 'y':
        /* Continue uninterrupted from now on */
        context.occ_interrupt.continue_mode = 1;
        pline("You will continue your occupation uninterrupted.");
        return TRUE; /* continue */

    case 'n':
        /* Stop now */
        pline("You stop.");
        return FALSE; /* stop occupation */

    case 'a':
        /* Set action limit */
        pline("How many more actions? ");
        getlin(buf, ans);
        if (ans[0] && ans[0] != '\033') {
            int actions = atoi(ans);
            if (actions > 0) {
                context.occ_interrupt.actions_remaining = actions;
                pline("Will stop after %d more actions.", actions);
                return TRUE; /* continue */
            } else if (actions <= 0) {
                pline("That's not valid.");
                return prompt_occ_interrupt(mtmp);
            }
        }
        return prompt_occ_interrupt(mtmp); /* Cancelled, ask again */

    case 'h':
        /* Set HP loss limit */
        pline("Maximum HP loss allowed? ");
        getlin(buf, ans);
        if (ans[0] && ans[0] != '\033') {
            int hp_limit = atoi(ans);
            if (hp_limit > 0) {
                context.occ_interrupt.hp_budget = hp_limit;
                pline("Will stop if you lose more than %d HP.", hp_limit);
                return TRUE; /* continue */
            } else if (hp_limit <= 0) {
                pline("That's not valid.");
                return prompt_occ_interrupt(mtmp);
            }
        }
        return prompt_occ_interrupt(mtmp); /* Cancelled, ask again */

    case 'q':
    default:
        /* Stop */
        return FALSE;
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
