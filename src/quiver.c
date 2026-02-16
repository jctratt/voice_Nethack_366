/* quiver.c -- quiver ordering and selection helpers
 *
 * Initial implementation:
 * - `select_quiver_candidate()` picks the best candidate from inventory
 *   using a simple scoring function that prefers matched ammo, missiles,
 *   throwing weapons, sticky-marked objects and higher player skill.
 * - `dosetquiver()` is implemented as a first-step: it opens the
 *   existing `ready` picker (dowieldquiver) so the player can explicitly
 *   choose which object to ready into the quiver.  A full reorder UI
 *   and persistent preference list will follow in later commits.
 */

#include "hack.h"
#include <ctype.h>

/* Persistent ordering for quiver preference.
   - `quiver_orderindx` is a per-savelist ordering of `otyp` values.
   - `quiver_ordercnt` is number of entries in `quiver_orderindx`.
   The array is consulted by `select_quiver_candidate()`; higher entries
   are preferred.  It is saved/restored with the game state. */
int *quiver_orderindx = 0; /* array of object types (otyp) */
int quiver_ordercnt = 0;
int *quiver_ignoreindx = 0; /* array of ignored object types (otyp) */
int quiver_ignorecnt = 0;

/* Persisted split option channels for quiver ordering. */
char *quiverorder_otypes = 0;
char *quiverorder_invlet = 0;
char *quiverorder_ignore_type = 0;
char *quiverorder_ignore_invlet = 0;

/* Helper to look up position of an object's otyp in the saved order; -1 if
   not found. */
STATIC_DCL int FDECL(quiver_order_pos, (int));
STATIC_DCL boolean FDECL(quiver_is_ignored_otyp, (int));
STATIC_DCL int FDECL(quiver_invlet_pos, (char, const char *));
STATIC_DCL int
quiver_order_pos(otyp)
int otyp;
{
    int i;
    if (!quiver_orderindx || quiver_ordercnt <= 0)
        return -1;
    for (i = 0; i < quiver_ordercnt; ++i)
        if (quiver_orderindx[i] == otyp)
            return i;
    return -1;
}

STATIC_DCL boolean
quiver_is_ignored_otyp(otyp)
int otyp;
{
    int i;

    if (!quiver_ignoreindx || quiver_ignorecnt <= 0)
        return FALSE;
    for (i = 0; i < quiver_ignorecnt; ++i)
        if (quiver_ignoreindx[i] == otyp)
            return TRUE;
    return FALSE;
}

boolean
quiver_obj_is_excluded(obj)
struct obj *obj;
{
    if (!obj)
        return FALSE;
    return (boolean) (quiver_is_ignored_otyp(obj->otyp)
                      || quiver_invlet_pos(obj->invlet,
                                           quiverorder_ignore_invlet) >= 0);
}

boolean
quiver_sync_readied_slot()
{
    if (!uquiver)
        return FALSE;
    if (quiver_obj_is_excluded(uquiver)) {
        uquiver->owornmask &= ~W_QUIVER;
        uquiver = (struct obj *) 0;
        return TRUE;
    }
    return FALSE;
}

STATIC_DCL int
quiver_invlet_pos(invlet, spec)
char invlet;
const char *spec;
{
    int pos = 0;

    if (!spec || !*spec || !isalpha((uchar) invlet))
        return -1;
    for (; *spec; ++spec) {
        if (!isalpha((uchar) *spec))
            continue;
        if (*spec == invlet)
            return pos;
        ++pos;
    }
    return -1;
}

/* Forward to the existing interactive 'ready quiver' command for the
   immediate pick action; the full reorder UI is implemented below. */
extern int NDECL(dowieldquiver);

/* Extended command: #setquiver — interactive reorder UI for quiver
   preferences.  The resulting ordering is stored in `quiver_orderindx`
   and will be saved with the game. */
int
dosetquiver(VOID_ARGS)
{
    struct obj *otmp;
    int cnt = 0, i = 0, chidx = 0, allow_gem_fallback = 0;
    char lets[BUFSZ], entrylet[BUFSZ];
    char qbuf[QBUFSZ];

    /* Build a list of unique quiver-candidate otypes present in inventory.
       Pass 1: strict (exclude GEM_CLASS unless matched sling ammo).
       Pass 2 (fallback): if none found, allow GEM_CLASS ammo as
       desperation throwables. */
    for (allow_gem_fallback = 0; allow_gem_fallback <= 1; ++allow_gem_fallback) {
        cnt = 0;
        for (otmp = invent; otmp; otmp = otmp->nobj) {
            if (otmp->owornmask || otmp->oartifact || !otmp->dknown)
                continue;
            if (!((is_ammo(otmp) && (otmp->oclass != GEM_CLASS
                                     || ammo_and_launcher(otmp, uwep)
                                     || allow_gem_fallback))
                  || is_missile(otmp)
                  || (otmp->oclass == WEAPON_CLASS && throwing_weapon(otmp))))
                continue;
            /* add unique otyp to quiver_orderindx buffer (temporary) */
            for (i = 0; i < cnt; ++i)
                if (quiver_orderindx && quiver_orderindx[i] == otmp->otyp)
                    break;
            if (i < cnt)
                continue; /* already present */
            /* append */
            quiver_orderindx = (int *) realloc(quiver_orderindx,
                                               (cnt + 1) * sizeof(int));
            quiver_orderindx[cnt++] = otmp->otyp;
        }
        if (cnt > 0)
            break;
    }

    if (cnt == 0) {
        pline("You have no items suitable for quiver ordering.");
        return 0;
    }

    quiver_ordercnt = cnt; /* adopt prepared list */

    for (;;) {
        /* Show current ordered preference list (also show a representative
           inventory letter if that otype is present). */
        int lcnt = 0;
        lets[0] = '\0';
        for (i = 0; i < cnt; ++i) {
            const char *oname = obj_typename(quiver_orderindx[i]);
            char repinv = 0;
            struct obj *ot;
            for (ot = invent; ot; ot = ot->nobj)
                if (ot->otyp == quiver_orderindx[i]) { repinv = ot->invlet; break; }
            entrylet[i] = repinv ? repinv : '?';
            if (repinv && !index(lets, repinv) && lcnt < (int) sizeof lets - 2) {
                lets[lcnt++] = repinv;
                lets[lcnt] = '\0';
            }
            pline("%c: %s", entrylet[i], oname ? oname : "(unknown)");
        }
        entrylet[cnt] = '\0';

        Sprintf(qbuf, "Quiver priority: [%.40s] swap which entry? (ESC to finish)",
                lets[0] ? lets : "");

        chidx = yn_function(qbuf, (char *) 0, '\0');
        if (chidx == '\0' || index(quitchars, chidx))
            break; /* finish */

        /* invlet-only selector: map typed inventory letter to list index */
        {
            int mapped = -1;
            for (i = 0; i < cnt; ++i)
                if (entrylet[i] == chidx) {
                    mapped = i;
                    break;
                }
            if (mapped < 0) {
                You("don't have that entry.");
                continue;
            }
            chidx = mapped;
        }

        /* ask for swap target */
        {
            int src = chidx;
            int dst;
            int mapped = -1;
                Sprintf(qbuf, "Swap '%c' with which entry? [%.40s] (ESC to cancel)",
                    entrylet[src], lets[0] ? lets : "");
            dst = yn_function(qbuf, (char *) 0, '\0');
            if (dst == '\0' || index(quitchars, dst))
                continue;

            for (i = 0; i < cnt; ++i)
                if (entrylet[i] == dst) {
                    mapped = i;
                    break;
                }
            if (mapped < 0) {
                    You("don't have that entry.");
                    continue;
            }
            /* perform swap in the order array */
            {
                int tmp = quiver_orderindx[src];
                quiver_orderindx[src] = quiver_orderindx[mapped];
                quiver_orderindx[mapped] = tmp;
            }
        }
    }

    /* Feedback */
    pline("Quiver order updated.");
    return 0;
}

/* Helper: compute a heurisitic score for how desirable `obj` is to be
   the quiver candidate.  Higher is better. */
STATIC_OVL int
quiver_score(obj, allow_gem_fallback)
struct obj *obj;
int allow_gem_fallback;
{
    int score = 0;
    int skill = objects[obj->otyp].oc_skill;
    int psk = P_SKILL(weapon_type(obj));
    int ivpos;

    if (quiver_is_ignored_otyp(obj->otyp))
        return 0;
    if (quiver_invlet_pos(obj->invlet, quiverorder_ignore_invlet) >= 0)
        return 0;

    /* GEM_CLASS ammo (rocks/flint/gems) is only considered when the
       currently wielded launcher is a sling. */
    if (obj->oclass == GEM_CLASS) {
        if (!ammo_and_launcher(obj, uwep) && !allow_gem_fallback)
            return 0;
    }

    /* If player has a launcher equipped, prefer matching ammo *before*
       other heuristics (user requested).  This makes bow/xbow/ sling
       selection dominant when a launcher is present. */
    if (uwep && ammo_and_launcher(obj, uwep)) {
        score += 800; /* overwhelming boost for primary launcher match */
    } else if (uswapwep && ammo_and_launcher(obj, uswapwep)) {
        score += 500; /* significant boost for alternate weapon launcher */
    }

    /* If object type is in the user quiver-order list, add a bonus based on
       its position (earlier entries get larger bonus). */
    {
        int pos = quiver_order_pos(obj->otyp);
        if (pos >= 0)
            score += (quiver_ordercnt - pos) * 60; /* tunable weight */
    }

    /* Invlet channel has precedence over otype ordering and should work
       even when the letter wasn't present when options were set. */
    ivpos = quiver_invlet_pos(obj->invlet, quiverorder_invlet);
    if (ivpos >= 0)
        score += 5000 - (ivpos * 50);

    /* Prefer skillable items next (still important for weapon training). */
    if (skill != P_NONE) {
        score += 400;
    }

    /* Favor weapons whose `oc_skill` is a positive weapon skill
       (for example `P_DAGGER`) when the player already has training in
       that weapon.  Scale the bonus by proficiency so EXPERT training
       strongly dominates generic missiles, SKILLED gets a solid edge,
       and BASIC gets a modest preference. */
    if (objects[obj->otyp].oc_skill > 0) {
        int wskill = objects[obj->otyp].oc_skill;
        int skl = P_SKILL(wskill);
        if (skl >= P_BASIC) {
            /* Tuned weights: BASIC < SKILLED < EXPERT to strongly favor
               high-skill throwing weapons when the player has trained. */
            if (skl >= P_EXPERT)
                score += 420; /* strong bias for expert */
            else if (skl >= P_SKILLED)
                score += 260; /* clear preference for skilled */
            else
                score += 120; /* modest advantage for basic */
        }
    }

    /* Base preference by type */
    if (is_ammo(obj)) {
        /* matched-launcher already received a large boost above; give
           additional smaller bonuses for class & role specifics */
        if (ammo_and_launcher(obj, uwep))
            score += 200;
        else if (ammo_and_launcher(obj, uswapwep))
            score += 120;
        else
            score += 60;

        if (obj->oclass == GEM_CLASS && !ammo_and_launcher(obj, uwep))
            score -= 120; /* fallback-only, prefer real missiles/weapons */

        /* Race-specific preference when using bows */
        if (Role_if(PM_RANGER) || Race_if(PM_ELF) || Race_if(PM_GNOME)) {
            if (Race_if(PM_ELF) && obj->otyp == ELVEN_ARROW)
                score += 80; /* elves prefer elven arrows */
            if (Race_if(PM_GNOME) && obj->otyp == CROSSBOW_BOLT)
                score += 40; /* gnomes like bolts */
        }
    } else if (is_missile(obj)) {
        score += 160;     /* darts/shuriken/etc. */
    } else if (obj->oclass == WEAPON_CLASS && throwing_weapon(obj)) {
        /* If no launcher present, prefer daggers & high-damage throwables */
        if (!uwep && !uswapwep && (obj->otyp == DAGGER || obj->otyp == ELVEN_DAGGER
                                   || obj->otyp == SILVER_DAGGER || obj->otyp == ORCISH_DAGGER))
            score += 220; /* strong preference for daggers when unlaunched */
        else
            score += 110; /* ordinary throwable */
    } else if (obj->otyp == ROCK || obj->otyp == FLINT) {
        score += 50;      /* rocks/flints usable as ammo */
    } else {
        /* not a sensible quiver candidate */
        return 0;
    }

#ifdef STICKY_OBJECTS
    if (obj->sticky)
        score += 40; /* respect sticky-marked inventory items */
#endif

    /* Quantity matters a bit */
    if (obj->quan > 1L)
        score += (obj->quan > 5L) ? 5 : (int) obj->quan;

    /* Player proficiency still contributes */
    switch (psk) {
    case P_EXPERT:
        score += 50;
        break;
    case P_SKILLED:
        score += 22;
        break;
    case P_BASIC:
        score += 8;
        break;
    default:
        break;
    }

    /* Role-based encouragements (additional) */
    switch (Role_switch) {
    case PM_RANGER:
        if (skill == P_BOW || skill == P_CROSSBOW || skill == P_SLING)
            score += 12;
        break;
    default:
        break;
    }

    /* Damage-based tiebreaker: prefer higher potential damage when
       other factors don't separate candidates. */
    {
        int wsd = objects[obj->otyp].oc_wsdam;
        int wld = objects[obj->otyp].oc_wldam;
        int dam = (wsd > wld) ? wsd : wld;
        score += dam;
    }

    return score;
}

/* Selection helper used by dofire()/autoquiver().  Scans inventory and
   returns the highest-scoring usable object, or NULL if none found. */
struct obj *
select_quiver_candidate()
{
    struct obj *otmp, *best = (struct obj *) 0;
    int bestscore = 0;
    int allow_gem_fallback;

    for (allow_gem_fallback = 0; allow_gem_fallback <= 1; ++allow_gem_fallback) {
        best = (struct obj *) 0;
        bestscore = 0;
        for (otmp = invent; otmp; otmp = otmp->nobj) {
            int s;

            /* skip worn, artifact or unknown items (same policy as autoquiver) */
            if (otmp->owornmask || otmp->oartifact || !otmp->dknown)
                continue;

            /* compute desirability */
            s = quiver_score(otmp, allow_gem_fallback);
            if (s <= 0)
                continue;

            /* prefer the currently-readied quiver item unless something
               substantially better exists */
            if (otmp == uquiver)
                s += 10;

            if (!best || s > bestscore) {
                best = otmp;
                bestscore = s;
            }
        }
        if (best)
            break;
    }

    return best;
}
