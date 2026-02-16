/* NetHack 3.6	dogmove.c	$NHDT-Date: 1557094801 2019/05/05 22:20:01 $  $NHDT-Branch: NetHack-3.6.2-beta01 $:$NHDT-Revision: 1.74 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

#include "mfndpos.h"

extern boolean notonhead;

STATIC_DCL boolean FDECL(dog_hunger, (struct monst *, struct edog *));
STATIC_DCL int FDECL(dog_invent, (struct monst *, struct edog *, int));
STATIC_DCL int FDECL(dog_goal, (struct monst *, struct edog *, int, int, int));
STATIC_DCL struct monst *FDECL(find_targ, (struct monst *, int, int, int));
STATIC_OVL int FDECL(find_friends, (struct monst *, struct monst *, int));
STATIC_DCL struct monst *FDECL(best_target, (struct monst *));
STATIC_DCL long FDECL(score_targ, (struct monst *, struct monst *));
STATIC_DCL boolean FDECL(can_reach_location, (struct monst *, XCHAR_P,
                                              XCHAR_P, XCHAR_P, XCHAR_P));
STATIC_DCL boolean FDECL(could_reach_item, (struct monst *, XCHAR_P, XCHAR_P));
STATIC_DCL void FDECL(quickmimic, (struct monst *));

/* choose an appropriate verb for a pet interpose announcement
   (returns 3rd-person singular present tense, e.g. "growls") */
const char *
pet_interpose_verb(mtmp)
    struct monst *mtmp;
{
    /* Prefer monster class (mlet) for short, obvious mappings; fall back
       to msound when appropriate.  Keep the returned verb in simple
       3rd-person-singular present form for direct insertion into
       pline("%s %s at %s.", Monnam(mtmp), verb, mon_nam(target)); */
    switch (mtmp->data->mlet) {
    case S_DOG:
        return "growls";
    case S_FELINE:
        return "hisses";
    case S_UNICORN:
        return "snorts";
    case S_RODENT:
        return "chitters";
    case S_SNAKE:
    case S_LIZARD:
        return "hisses";
    case S_ANT:
    case S_SPIDER:
        return "chitters";
    default:
        break;
    }

    /* birds are represented with the S_BAT class in this tree; use helper */
    if (is_bird(mtmp->data))
        return "screeches";

    /* fallback based on monster sound class */
    switch (mtmp->data->msound) {
    case MS_BARK:
        return "barks";
    case MS_MEW:
        return "mews";
    case MS_HISS:
        return "hisses";
    case MS_SQAWK:
        return "screeches";
    case MS_SQEEK:
        return "squeaks";
    case MS_BUZZ:
        return "buzzes";
    case MS_GRUNT:
        return "grunts";
    case MS_NEIGH:
        return "snorts";
    default:
        return "growls";
    }
}

/* pick a carried item for pet to drop */
struct obj *
droppables(mon)
struct monst *mon;
{
    struct obj *obj, *wep, dummy, *pickaxe, *unihorn, *key;

    dummy = zeroobj;
    dummy.otyp = GOLD_PIECE; /* not STRANGE_OBJECT or tools of interest */
    dummy.oartifact = 1; /* so real artifact won't override "don't keep it" */
    pickaxe = unihorn = key = (struct obj *) 0;
    wep = MON_WEP(mon);

    if (is_animal(mon->data) || mindless(mon->data)) {
        /* won't hang on to any objects of these types */
        pickaxe = unihorn = key = &dummy; /* act as if already have them */
    } else {
        /* don't hang on to pick-axe if can't use one or don't need one */
        if (!tunnels(mon->data) || !needspick(mon->data))
            pickaxe = &dummy;
        /* don't hang on to key if can't open doors */
        if (nohands(mon->data) || verysmall(mon->data))
            key = &dummy;
    }
    if (wep) {
        if (is_pick(wep))
            pickaxe = wep;
        if (wep->otyp == UNICORN_HORN)
            unihorn = wep;
        /* don't need any wielded check for keys... */
    }

    for (obj = mon->minvent; obj; obj = obj->nobj) {
        switch (obj->otyp) {
        case DWARVISH_MATTOCK:
            /* reject mattock if couldn't wield it */
            if (which_armor(mon, W_ARMS))
                break;
            /* keep mattock in preference to pick unless pick is already
               wielded or is an artifact and mattock isn't */
            if (pickaxe && pickaxe->otyp == PICK_AXE && pickaxe != wep
                && (!pickaxe->oartifact || obj->oartifact))
                return pickaxe; /* drop the one we earlier decided to keep */
        /*FALLTHRU*/
        case PICK_AXE:
            if (!pickaxe || (obj->oartifact && !pickaxe->oartifact)) {
                if (pickaxe)
                    return pickaxe;
                pickaxe = obj; /* keep this digging tool */
                continue;
            }
            break;

        case UNICORN_HORN:
            /* reject cursed unicorn horns */
            if (obj->cursed)
                break;
            /* keep artifact unihorn in preference to ordinary one */
            if (!unihorn || (obj->oartifact && !unihorn->oartifact)) {
                if (unihorn)
                    return unihorn;
                unihorn = obj; /* keep this unicorn horn */
                continue;
            }
            break;

        case SKELETON_KEY:
            /* keep key in preference to lock-pick */
            if (key && key->otyp == LOCK_PICK
                && (!key->oartifact || obj->oartifact))
                return key; /* drop the one we earlier decided to keep */
        /*FALLTHRU*/
        case LOCK_PICK:
            /* keep lock-pick in preference to credit card */
            if (key && key->otyp == CREDIT_CARD
                && (!key->oartifact || obj->oartifact))
                return key;
        /*FALLTHRU*/
        case CREDIT_CARD:
            if (!key || (obj->oartifact && !key->oartifact)) {
                if (key)
                    return key;
                key = obj; /* keep this unlocking tool */
                continue;
            }
            break;

        default:
            break;
        }

        if (!obj->owornmask && obj != wep)
            return obj;
    }

    return (struct obj *) 0; /* don't drop anything */
}

static NEARDATA const char nofetch[] = { BALL_CLASS, CHAIN_CLASS, ROCK_CLASS,
                                         0 };

STATIC_VAR xchar gtyp, gx, gy; /* type and position of dog's current goal */

/* Pet AI tuning */
#define PET_TARGET_RADIUS 14 /* tiles around player to search for hostile targets (increased from 7) */
#define MAX_PET_DISTANCE 5  /* max allowed distance (tiles) pet may move away from player */

/* Player-centered search helpers (prefer player-origin targets first) */
STATIC_DCL struct monst *FDECL(best_target_from_origin, (struct monst *, int, int, int));
STATIC_DCL struct monst *FDECL(find_targ_from_origin, (struct monst *, int, int, int, int, int));

STATIC_PTR void FDECL(wantdoor, (int, int, genericptr_t));

boolean
cursed_object_at(x, y)
int x, y;
{
    struct obj *otmp;

    for (otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere)
        if (otmp->cursed)
            return TRUE;
    return FALSE;
}

int
dog_nutrition(mtmp, obj)
struct monst *mtmp;
struct obj *obj;
{
    int nutrit;

    /*
     * It is arbitrary that the pet takes the same length of time to eat
     * as a human, but gets more nutritional value.
     */
    if (obj->oclass == FOOD_CLASS) {
        if (obj->otyp == CORPSE) {
            mtmp->meating = 3 + (mons[obj->corpsenm].cwt >> 6);
            nutrit = mons[obj->corpsenm].cnutrit;
        } else {
            mtmp->meating = objects[obj->otyp].oc_delay;
            nutrit = objects[obj->otyp].oc_nutrition;
        }
        switch (mtmp->data->msize) {
        case MZ_TINY:
            nutrit *= 8;
            break;
        case MZ_SMALL:
            nutrit *= 6;
            break;
        default:
        case MZ_MEDIUM:
            nutrit *= 5;
            break;
        case MZ_LARGE:
            nutrit *= 4;
            break;
        case MZ_HUGE:
            nutrit *= 3;
            break;
        case MZ_GIGANTIC:
            nutrit *= 2;
            break;
        }
        if (obj->oeaten) {
            mtmp->meating = eaten_stat(mtmp->meating, obj);
            nutrit = eaten_stat(nutrit, obj);
        }
    } else if (obj->oclass == COIN_CLASS) {
        mtmp->meating = (int) (obj->quan / 2000) + 1;
        if (mtmp->meating < 0)
            mtmp->meating = 1;
        nutrit = (int) (obj->quan / 20);
        if (nutrit < 0)
            nutrit = 0;
    } else {
        /* Unusual pet such as gelatinous cube eating odd stuff.
         * meating made consistent with wild monsters in mon.c.
         * nutrit made consistent with polymorphed player nutrit in
         * eat.c.  (This also applies to pets eating gold.)
         */
        mtmp->meating = obj->owt / 20 + 1;
        nutrit = 5 * objects[obj->otyp].oc_nutrition;
    }
    return nutrit;
}

/* returns 2 if pet dies, otherwise 1 */
int
dog_eat(mtmp, obj, x, y, devour)
register struct monst *mtmp;
register struct obj *obj; /* if unpaid, then thrown or kicked by hero */
int x, y; /* dog's starting location, might be different from current */
boolean devour;
{
    register struct edog *edog = EDOG(mtmp);
    boolean poly, grow, heal, eyes, slimer, deadmimic;
    int nutrit;
    long oprice;
    char objnambuf[BUFSZ];

    objnambuf[0] = '\0';
    if (edog->hungrytime < monstermoves)
        edog->hungrytime = monstermoves;
    nutrit = dog_nutrition(mtmp, obj);

    deadmimic = (obj->otyp == CORPSE && (obj->corpsenm == PM_SMALL_MIMIC
                                         || obj->corpsenm == PM_LARGE_MIMIC
                                         || obj->corpsenm == PM_GIANT_MIMIC));
    slimer = (obj->otyp == CORPSE && obj->corpsenm == PM_GREEN_SLIME);
    poly = polyfodder(obj);
    grow = mlevelgain(obj);
    heal = mhealup(obj);
    eyes = (obj->otyp == CARROT);

    if (devour) {
        if (mtmp->meating > 1)
            mtmp->meating /= 2;
        if (nutrit > 1)
            nutrit = (nutrit * 3) / 4;
    }
    edog->hungrytime += nutrit;
    mtmp->mconf = 0;
    if (edog->mhpmax_penalty) {
        /* no longer starving */
        mtmp->mhpmax += edog->mhpmax_penalty;
        edog->mhpmax_penalty = 0;
    }
    if (mtmp->mflee && mtmp->mfleetim > 1)
        mtmp->mfleetim /= 2;
    if (mtmp->mtame < 20)
        mtmp->mtame++;
    if (x != mtmp->mx || y != mtmp->my) { /* moved & ate on same turn */
        newsym(x, y);
        newsym(mtmp->mx, mtmp->my);
    }

    /* food items are eaten one at a time; entire stack for other stuff */
    if (obj->quan > 1L && obj->oclass == FOOD_CLASS)
        obj = splitobj(obj, 1L);
    if (obj->unpaid)
        iflags.suppress_price++;
    if (is_pool(x, y) && !Underwater) {
        /* Don't print obj */
        /* TODO: Reveal presence of sea monster (especially sharks) */
    } else {
        /* food is at monster's current location, <mx,my>;
           <x,y> was monster's location at start of this turn;
           they might be the same but will be different when
           the monster is moving+eating on same turn */
        boolean seeobj = cansee(mtmp->mx, mtmp->my),
                sawpet = cansee(x, y) && mon_visible(mtmp);

        /* Observe the action if either the food location or the pet
           itself is in view.  When pet which was in view moves to an
           unseen spot to eat the food there, avoid referring to that
           pet as "it".  However, we want "it" if invisible/unsensed
           pet eats visible food. */
        if (sawpet || (seeobj && canspotmon(mtmp))) {
            if (tunnels(mtmp->data))
                pline("%s digs in.", noit_Monnam(mtmp));
            else
                pline("%s %s %s.", noit_Monnam(mtmp),
                      devour ? "devours" : "eats", distant_name(obj, doname));
        } else if (seeobj)
            pline("It %s %s.", devour ? "devours" : "eats",
                  distant_name(obj, doname));
    }
    if (obj->unpaid) {
        Strcpy(objnambuf, xname(obj));
        iflags.suppress_price--;
    }
    /* It's a reward if it's DOGFOOD and the player dropped/threw it.
       We know the player had it if invlet is set. -dlc */
    if (dogfood(mtmp, obj) == DOGFOOD && obj->invlet)
#ifdef LINT
        edog->apport = 0;
#else
        edog->apport += (int) (200L / ((long) edog->dropdist + monstermoves
                                       - edog->droptime));
#endif
    if (mtmp->data == &mons[PM_RUST_MONSTER] && obj->oerodeproof) {
        /* The object's rustproofing is gone now */
        if (obj->unpaid)
            costly_alteration(obj, COST_DEGRD);
        obj->oerodeproof = 0;
        mtmp->mstun = 1;
        if (canseemon(mtmp) && flags.verbose) {
            pline("%s spits %s out in disgust!", Monnam(mtmp),
                  distant_name(obj, doname));
        }
    } else if (obj == uball) {
        unpunish();
        delobj(obj); /* we assume this can't be unpaid */
    } else if (obj == uchain) {
        unpunish();
    } else {
        if (obj->unpaid) {
            /* edible item owned by shop has been thrown or kicked
               by hero and caught by tame or food-tameable monst */
            oprice = unpaid_cost(obj, TRUE);
            pline("That %s will cost you %ld %s.", objnambuf, oprice,
                  currency(oprice));
            /* delobj->obfree will handle actual shop billing update */
        }
        delobj(obj);
    }

#if 0 /* pet is eating, so slime recovery is not feasible... */
    /* turning into slime might be cureable */
    if (slimer && munslime(mtmp, FALSE)) {
        /* but the cure (fire directed at self) might be fatal */
        if (DEADMONSTER(mtmp))
            return 2;
        slimer = FALSE; /* sliming is avoided, skip polymorph */
    }
#endif

    if (poly || slimer) {
        struct permonst *ptr = slimer ? &mons[PM_GREEN_SLIME] : 0;

        (void) newcham(mtmp, ptr, FALSE, cansee(mtmp->mx, mtmp->my));
    }

    /* limit "instant" growth to prevent potential abuse */
    if (grow && (int) mtmp->m_lev < (int) mtmp->data->mlevel + 15) {
        if (!grow_up(mtmp, (struct monst *) 0))
            return 2;
    }
    if (heal)
        mtmp->mhp = mtmp->mhpmax;
    if ((eyes || heal) && !mtmp->mcansee)
        mcureblindness(mtmp, canseemon(mtmp));
    if (deadmimic)
        quickmimic(mtmp);
    return 1;
}

/* hunger effects -- returns TRUE on starvation */
STATIC_OVL boolean
dog_hunger(mtmp, edog)
struct monst *mtmp;
struct edog *edog;
{
    if (monstermoves > edog->hungrytime + 500) {
        if (!carnivorous(mtmp->data) && !herbivorous(mtmp->data)) {
            edog->hungrytime = monstermoves + 500;
            /* but not too high; it might polymorph */
        } else if (!edog->mhpmax_penalty) {
            /* starving pets are limited in healing */
            int newmhpmax = mtmp->mhpmax / 3;
            mtmp->mconf = 1;
            edog->mhpmax_penalty = mtmp->mhpmax - newmhpmax;
            mtmp->mhpmax = newmhpmax;
            if (mtmp->mhp > mtmp->mhpmax)
                mtmp->mhp = mtmp->mhpmax;
            if (DEADMONSTER(mtmp))
                goto dog_died;
            if (cansee(mtmp->mx, mtmp->my))
                pline("%s is confused from hunger.", Monnam(mtmp));
            else if (couldsee(mtmp->mx, mtmp->my))
                beg(mtmp);
            else
                You_feel("worried about %s.", y_monnam(mtmp));
            stop_occupation();
        } else if (monstermoves > edog->hungrytime + 750
                   || DEADMONSTER(mtmp)) {
 dog_died:
            if (mtmp->mleashed && mtmp != u.usteed)
                Your("leash goes slack.");
            else if (cansee(mtmp->mx, mtmp->my))
                pline("%s starves.", Monnam(mtmp));
            else
                You_feel("%s for a moment.",
                         Hallucination ? "bummed" : "sad");
            mondied(mtmp);
            return  TRUE;
        }
    }
    return FALSE;
}

/* do something with object (drop, pick up, eat) at current position
 * returns 1 if object eaten (since that counts as dog's move), 2 if died
 */
STATIC_OVL int
dog_invent(mtmp, edog, udist)
register struct monst *mtmp;
register struct edog *edog;
int udist;
{
    register int omx, omy, carryamt = 0;
    struct obj *obj, *otmp;

    if (mtmp->msleeping || !mtmp->mcanmove)
        return 0;

    omx = mtmp->mx;
    omy = mtmp->my;

    /* If we are carrying something then we drop it (perhaps near @).
     * Note: if apport == 1 then our behaviour is independent of udist.
     * Use udist+1 so steed won't cause divide by zero.
     */
    if (droppables(mtmp)) {
        if (!rn2(udist + 1) || !rn2(edog->apport))
            if (rn2(10) < edog->apport) {
                relobj(mtmp, (int) mtmp->minvis, TRUE);
                if (edog->apport > 1)
                    edog->apport--;
                edog->dropdist = udist; /* hpscdi!jon */
                edog->droptime = monstermoves;
            }
    } else {
        if ((obj = level.objects[omx][omy]) != 0
            && !index(nofetch, obj->oclass)
#ifdef MAIL
            && obj->otyp != SCR_MAIL
#endif
            ) {
            int edible = dogfood(mtmp, obj);

            if ((edible <= CADAVER
                 /* starving pet is more aggressive about eating */
                 || (edog->mhpmax_penalty && edible == ACCFOOD))
                && could_reach_item(mtmp, obj->ox, obj->oy))
                return dog_eat(mtmp, obj, omx, omy, FALSE);

            carryamt = can_carry(mtmp, obj);
            if (carryamt > 0 && !obj->cursed
                && could_reach_item(mtmp, obj->ox, obj->oy)) {
                if (rn2(20) < edog->apport + 3) {
                    if (rn2(udist) || !rn2(edog->apport)) {
                        otmp = obj;
                        if (carryamt != obj->quan)
                            otmp = splitobj(obj, carryamt);
                        if (cansee(omx, omy) && flags.verbose)
                            pline("%s picks up %s.", Monnam(mtmp),
                                  distant_name(otmp, doname));
                        obj_extract_self(otmp);
                        newsym(omx, omy);
                        (void) mpickobj(mtmp, otmp);
                        if (attacktype(mtmp->data, AT_WEAP)
                            && mtmp->weapon_check == NEED_WEAPON) {
                            mtmp->weapon_check = NEED_HTH_WEAPON;
                            (void) mon_wield_item(mtmp);
                        }
                        m_dowear(mtmp, FALSE);
                    }
                }
            }
        }
    }
    return 0;
}

/* set dog's goal -- gtyp, gx, gy;
   returns -1/0/1 (dog's desire to approach player) or -2 (abort move) */
STATIC_OVL int
dog_goal(mtmp, edog, after, udist, whappr)
register struct monst *mtmp;
struct edog *edog;
int after, udist, whappr;
{
    register int omx, omy;
    boolean in_masters_sight, dog_has_minvent;
    register struct obj *obj;
    xchar otyp;
    int appr;

    /* Steeds don't move on their own will */
    if (mtmp == u.usteed)
        return -2;

    omx = mtmp->mx;
    omy = mtmp->my;

    in_masters_sight = couldsee(omx, omy);
    dog_has_minvent = (droppables(mtmp) != 0);

    if (!edog || mtmp->mleashed) { /* he's not going anywhere... */
        gtyp = APPORT;
        gx = u.ux;
        gy = u.uy;
    } else {
#define DDIST(x, y) (dist2(x, y, omx, omy))
#define SQSRCHRADIUS 5
        int min_x, max_x, min_y, max_y;
        register int nx, ny;

        gtyp = UNDEF; /* no goal as yet */
        gx = gy = 0;  /* suppress 'used before set' message */

        if ((min_x = omx - SQSRCHRADIUS) < 1)
            min_x = 1;
        if ((max_x = omx + SQSRCHRADIUS) >= COLNO)
            max_x = COLNO - 1;
        if ((min_y = omy - SQSRCHRADIUS) < 0)
            min_y = 0;
        if ((max_y = omy + SQSRCHRADIUS) >= ROWNO)
            max_y = ROWNO - 1;

        /* nearby food is the first choice, then other objects */
        for (obj = fobj; obj; obj = obj->nobj) {
            nx = obj->ox;
            ny = obj->oy;
            if (nx >= min_x && nx <= max_x && ny >= min_y && ny <= max_y) {
                otyp = dogfood(mtmp, obj);
                /* skip inferior goals */
                if (otyp > gtyp || otyp == UNDEF)
                    continue;
                /* avoid cursed items unless starving */
                if (cursed_object_at(nx, ny)
                    && !(edog->mhpmax_penalty && otyp < MANFOOD))
                    continue;
                /* skip completely unreachable goals */
                if (!could_reach_item(mtmp, nx, ny)
                    || !can_reach_location(mtmp, mtmp->mx, mtmp->my, nx, ny))
                    continue;
                if (otyp < MANFOOD) {
                    if (otyp < gtyp || DDIST(nx, ny) < DDIST(gx, gy)) {
                        gx = nx;
                        gy = ny;
                        gtyp = otyp;
                    }
                } else if (gtyp == UNDEF && in_masters_sight
                           && !dog_has_minvent
                           && (!levl[omx][omy].lit || levl[u.ux][u.uy].lit)
                           && (otyp == MANFOOD || m_cansee(mtmp, nx, ny))
                           && can_carry(mtmp, obj) > 0) {
                    /* De-prioritise objects that are far from the player so
                       the pet prefers to follow/assist the player first.
                       Also: don't let stair tiles ('<'/'>' ground) act as
                       fetch goals unless the player is standing on them. */
                    int pdist = distmin(nx, ny, u.ux, u.uy);
                    int extra = (pdist > MAX_PET_DISTANCE) ? (pdist - MAX_PET_DISTANCE) : 0;
                    int apport_thresh = 8 + extra; /* farther objects are harder */
                    if (edog->apport > rn2(apport_thresh)) {
                        /* avoid stair tiles unless player is on them */
                        if (On_stairs(nx, ny) && (nx != u.ux || ny != u.uy))
                            continue;
                        /* If the player can see a hostile within PET_TARGET_RADIUS,
                           don't let APPORT beat that hostile. */
                        if (best_target_from_origin(mtmp, u.ux, u.uy, PET_TARGET_RADIUS))
                            continue;
                        gx = nx;
                        gy = ny;
                        gtyp = APPORT;
                    }
                }
            }
        }
    }

    /* follow player if appropriate */
    if (gtyp == UNDEF || (gtyp != DOGFOOD && gtyp != APPORT
                          && monstermoves < edog->hungrytime)) {
        gx = u.ux;
        gy = u.uy;
        if (after && udist <= 4 && gx == u.ux && gy == u.uy)
            return -2;
        appr = (udist >= 9) ? 1 : (mtmp->mflee) ? -1 : 0;
        if (udist > 1) {
            if (udist >= 4
                || !IS_ROOM(levl[u.ux][u.uy].typ) || !rn2(4) || whappr
                || (dog_has_minvent && rn2(edog->apport)))
                appr = 1;
        }
        /* if you have dog food it'll follow you more closely */
        if (appr == 0)
            for (obj = invent; obj; obj = obj->nobj)
                if (dogfood(mtmp, obj) == DOGFOOD) {
                    appr = 1;
                    break;
                }
    } else
        appr = 1; /* gtyp != UNDEF */
    if (mtmp->mconf)
        appr = 0;

#define FARAWAY (COLNO + 2) /* position outside screen */
    if (gx == u.ux && gy == u.uy && !in_masters_sight) {
        register coord *cp;

        cp = gettrack(omx, omy);
        /* ignore stair-track points only when already close to the player;
           when far away, allow stair-adjacent tracking so pets can rejoin */
        if (cp && On_stairs(cp->x, cp->y)
            && (cp->x != u.ux || cp->y != u.uy)
            && udist <= 4)
            cp = (coord *) 0;
        if (cp) {
            gx = cp->x;
            gy = cp->y;
            if (edog)
                edog->ogoal.x = 0;
        } else {
            /* assume master hasn't moved far, and reuse previous goal */
            if (edog && edog->ogoal.x
                && (edog->ogoal.x != omx || edog->ogoal.y != omy)) {
                gx = edog->ogoal.x;
                gy = edog->ogoal.y;
                edog->ogoal.x = 0;
            } else {
                int fardist = FARAWAY * FARAWAY;
                gx = gy = FARAWAY; /* random */
                do_clear_area(omx, omy, 9, wantdoor, (genericptr_t) &fardist);

                /* here gx == FARAWAY e.g. when dog is in a vault */
                if (gx == FARAWAY || (gx == omx && gy == omy)) {
                    gx = u.ux;
                    gy = u.uy;
                } else if (edog) {
                    edog->ogoal.x = gx;
                    edog->ogoal.y = gy;
                }
            }
        }
    } else if (edog) {
        edog->ogoal.x = 0;
    }
    return appr;
}

STATIC_OVL struct monst *
find_targ(mtmp, dx, dy, maxdist)
register struct monst *mtmp;
int dx, dy;
int maxdist;
{
    struct monst *targ = 0;
    int curx = mtmp->mx, cury = mtmp->my;
    int dist = 0;

    /* Walk outwards */
    for ( ; dist < maxdist; ++dist) {
        curx += dx;
        cury += dy;
        if (!isok(curx, cury))
            break;

        /* FIXME: Check if we hit a wall/door/boulder to
         *        short-circuit unnecessary subsequent checks
         */

        /* If we can't see up to here, forget it - will this
         * mean pets in corridors don't breathe at monsters
         * in rooms? If so, is that necessarily bad?
         */
        if (!m_cansee(mtmp, curx, cury))
            break;

        if (curx == mtmp->mux && cury == mtmp->muy)
            return &youmonst;

        if ((targ = m_at(curx, cury)) != 0) {
            /* Is the monster visible to the pet? */
            if ((!targ->minvis || perceives(mtmp->data))
                && !targ->mundetected)
                break;
            /* If the pet can't see it, it assumes it aint there */
            targ = 0;
        }
    }
    return targ;
}

/* like find_targ() but starts scanning from an arbitrary origin (ox,oy)
   while using the pet's visibility (m_cansee) to determine whether the
   pet would detect a monster along that line */
STATIC_OVL struct monst *
find_targ_from_origin(mtmp, ox, oy, dx, dy, maxdist)
struct monst *mtmp;
int ox, oy, dx, dy, maxdist;
{
    struct monst *targ = 0;
    int curx = ox, cury = oy;
    int dist = 0;

    for ( ; dist < maxdist; ++dist) {
        curx += dx;
        cury += dy;
        if (!isok(curx, cury))
            break;

        /* allow the player's vision to extend the scan: continue
           scanning if either the pet or the player can see this square */
        if (!m_cansee(mtmp, curx, cury) && !cansee(curx, cury))
            break;

        if (curx == mtmp->mux && cury == mtmp->muy)
            return &youmonst;

        if ((targ = m_at(curx, cury)) != 0) {
            /* accept the monster if either the pet or the player can see it */
            boolean pet_sees_mon = ((!targ->minvis || perceives(mtmp->data))
                                    && !targ->mundetected);
            boolean player_sees_mon = canspotmon(targ);
            if (pet_sees_mon || player_sees_mon)
                break;
            targ = 0;
        }
    }
    return targ;
}

/* best_target_from_origin: same scoring as best_target() but scan lines
   starting from (ox,oy) instead of the pet's square (useful to prefer
   player-centered targets). */
STATIC_OVL struct monst *
best_target_from_origin(mtmp, ox, oy, radius)
struct monst *mtmp;
int ox, oy, radius;
{
    int dx, dy;
    long bestscore = -40000L, currscore;
    struct monst *best_targ = 0, *temp_targ = 0;

    if (!mtmp)
        return 0;

    if (!mtmp->mcansee)
        return 0;

    for (dy = -1; dy < 2; ++dy) {
        for (dx = -1; dx < 2; ++dx) {
            if (!dx && !dy)
                continue;
            temp_targ = find_targ_from_origin(mtmp, ox, oy, dx, dy, radius);
            if (!temp_targ)
                continue;

            currscore = score_targ(mtmp, temp_targ);
            if (currscore > bestscore) {
                bestscore = currscore;
                best_targ = temp_targ;
            }
        }
    }

    if (bestscore < 0L)
        return 0;
    return best_targ;
}

STATIC_OVL int
find_friends(mtmp, mtarg, maxdist)
struct monst *mtmp, *mtarg;
int    maxdist;
{
    struct monst *pal;
    int dx = sgn(mtarg->mx - mtmp->mx),
        dy = sgn(mtarg->my - mtmp->my);
    int curx = mtarg->mx, cury = mtarg->my;
    int dist = distmin(mtarg->mx, mtarg->my, mtmp->mx, mtmp->my);

    for ( ; dist <= maxdist; ++dist) {
        curx += dx;
        cury += dy;

        if (!isok(curx, cury))
            return 0;

        /* If the pet can't see beyond this point, don't
         * check any farther
         */
        if (!m_cansee(mtmp, curx, cury))
            return 0;

        /* Does pet think you're here? */
        if (mtmp->mux == curx && mtmp->muy == cury)
            return 1;

        pal = m_at(curx, cury);

        if (pal) {
            if (pal->mtame) {
                /* Pet won't notice invisible pets */
                if (!pal->minvis || perceives(mtmp->data))
                    return 1;
            } else {
                /* Quest leaders and guardians are always seen */
                if (pal->data->msound == MS_LEADER
                    || pal->data->msound == MS_GUARDIAN)
                    return 1;
            }
        }
    }
    return 0;
}

STATIC_OVL long
score_targ(mtmp, mtarg)
struct monst *mtmp, *mtarg;
{
    long score = 0L;

    /* If the monster is confused, normal scoring is disrupted -
     * anything may happen
     */

    /* Give 1 in 3 chance of safe breathing even if pet is confused or
     * if you're on the quest start level */
    if (!mtmp->mconf || !rn2(3) || Is_qstart(&u.uz)) {
        int mtmp_lev;
        aligntyp align1 = A_NONE, align2 = A_NONE; /* For priests, minions */
        boolean faith1 = TRUE,  faith2 = TRUE;

        if (mtmp->isminion)
            align1 = EMIN(mtmp)->min_align;
        else if (mtmp->ispriest)
            align1 = EPRI(mtmp)->shralign;
        else
            faith1 = FALSE;
        if (mtarg->isminion)
            align2 = EMIN(mtarg)->min_align; /* MAR */
        else if (mtarg->ispriest)
            align2 = EPRI(mtarg)->shralign; /* MAR */
        else
            faith2 = FALSE;

        /* Never target quest friendlies */
        if (mtarg->data->msound == MS_LEADER
            || mtarg->data->msound == MS_GUARDIAN)
            return -5000L;
        /* D: Fixed angelic beings using gaze attacks on coaligned priests */
        if (faith1 && faith2 && align1 == align2 && mtarg->mpeaceful) {
            score -= 5000L;
            return score;
        }
        /* Is monster adjacent? */
        if (distmin(mtmp->mx, mtmp->my, mtarg->mx, mtarg->my) <= 1) {
            score -= 3000L;
            return score;
        }
        /* Is the monster peaceful or tame? */
        if (/*mtarg->mpeaceful ||*/ mtarg->mtame || mtarg == &youmonst) {
            /* Pets will never be targeted */
            score -= 3000L;
            return score;
        }
        /* Is master/pet behind monster? Check up to 15 squares beyond pet. */
        if (find_friends(mtmp, mtarg, 15)) {
            score -= 3000L;
            return score;
        }
        /* Target hostile monsters in preference to peaceful ones */
        if (!mtarg->mpeaceful)
            score += 10;
        /* Is the monster passive? Don't waste energy on it, if so */
        if (mtarg->data->mattk[0].aatyp == AT_NONE)
            score -= 1000;
        /* Even weak pets with breath attacks shouldn't take on very
           low-level monsters. Wasting breath on lichens is ridiculous. */
        if ((mtarg->m_lev < 2 && mtmp->m_lev > 5)
            || (mtmp->m_lev > 12 && mtarg->m_lev < mtmp->m_lev - 9
                && u.ulevel > 8 && mtarg->m_lev < u.ulevel - 7))
            score -= 25;
        /* for strength purposes, a vampshifter in weak form (vampire bat,
           fog cloud, maybe wolf) will attack as if in vampire form;
           otherwise if won't do much and usually wouldn't suffer enough
           damage (from counterattacks) to switch back to vampire form;
           make it be more aggressive by behaving as if stronger */
        mtmp_lev = mtmp->m_lev;
        if (is_vampshifter(mtmp) && mtmp->data->mlet != S_VAMPIRE) {
            /* is_vampshifter() implies (mtmp->cham >= LOW_PM) */
            mtmp_lev = mons[mtmp->cham].mlevel;
            /* actual vampire level would range from 1.0*mlvl to 1.5*mlvl */
            mtmp_lev += rn2(mtmp_lev / 2 + 1);
            /* we don't expect actual level in weak form to exceed
               base level of strong form, but handle that if it happens */
            if (mtmp->m_lev > mtmp_lev)
                mtmp_lev = mtmp->m_lev;
        }
        /* And pets will hesitate to attack vastly stronger foes.
           This penalty will be discarded if master's in trouble. */
        if (mtarg->m_lev > mtmp_lev + 4L)
            score -= (mtarg->m_lev - mtmp_lev) * 20L;
        /* All things being the same, go for the beefiest monster. This
           bonus should not be large enough to override the pet's aversion
           to attacking much stronger monsters. */
        score += mtarg->m_lev * 2 + mtarg->mhp / 3;
    }
    /* Fuzz factor to make things less predictable when very
       similar targets are abundant. */
    score += rnd(5);
    /* Pet may decide not to use ranged attack when confused */
    if (mtmp->mconf && !rn2(3))
        score -= 1000;
    return score;
}

STATIC_OVL struct monst *
best_target(mtmp)
struct monst *mtmp;   /* Pet */
{
    int dx, dy;
    long bestscore = -40000L, currscore;
    struct monst *best_targ = 0, *temp_targ = 0;

    /* Help! */
    if (!mtmp)
        return 0;

    /* If the pet is blind, it's not going to see any target */
    if (!mtmp->mcansee)
        return 0;

    /* Search for any monsters lined up with the pet, within an arbitrary
     * distance from the pet (7 squares, even along diagonals). Monsters
     * are assigned scores and the best score is chosen.
     */
    for (dy = -1; dy < 2; ++dy) {
        for (dx = -1; dx < 2; ++dx) {
            if (!dx && !dy)
                continue;
            /* Traverse the line to find the first monster within 7
             * squares. Invisible monsters are skipped (if the
             * pet doesn't have see invisible).
             */
            temp_targ = find_targ(mtmp, dx, dy, 7);

            /* Nothing in this line? */
            if (!temp_targ)
                continue;

            /* Decide how attractive the target is */
            currscore = score_targ(mtmp, temp_targ);

            if (currscore > bestscore) {
                bestscore = currscore;
                best_targ = temp_targ;
            }
        }
    }

    /* Filter out targets the pet doesn't like */
    if (bestscore < 0L)
        best_targ = 0;

    return best_targ;
}

/* return 0 (no move), 1 (move) or 2 (dead) */
int
dog_move(mtmp, after)
register struct monst *mtmp;
int after; /* this is extra fast monster movement */
{
    int omx, omy; /* original mtmp position */
    int appr, whappr, udist;
    int i, j, k;
    register struct edog *edog = EDOG(mtmp);
    struct obj *obj = (struct obj *) 0;
    xchar otyp;
    boolean has_edog, cursemsg[9], do_eat = FALSE;
    boolean better_with_displacing = FALSE;
    boolean did_interpose = FALSE; /* set when pet explicitly interposes */
    boolean have_interpose_goal = FALSE;
    boolean force_rejoin = FALSE;
    xchar nix, niy;      /* position mtmp is (considering) moving to */
    register int nx, ny; /* temporary coordinates */
    xchar cnt, uncursedcnt, chcnt;
    int chi = -1, nidist, ndist;
    int interpose_goal_x = 0, interpose_goal_y = 0;
    coord poss[9];
    long info[9], allowflags;
    struct monst *pursue_mon = (struct monst *) 0; /* player-centered pursuit candidate */
#define GDIST(x, y) (dist2(x, y, gx, gy))

    /*
     * Tame Angels have isminion set and an ispriest structure instead of
     * an edog structure.  Fortunately, guardian Angels need not worry
     * about mundane things like eating and fetching objects, and can
     * spend all their energy defending the player.  (They are the only
     * monsters with other structures that can be tame.)
     */
    has_edog = !mtmp->isminion;

    omx = mtmp->mx;
    omy = mtmp->my;
    if (has_edog && dog_hunger(mtmp, edog))
        return 2; /* starved */

    udist = distu(omx, omy);
    /* Let steeds eat and maybe throw rider during Conflict */
    if (mtmp == u.usteed) {
        if (Conflict && !resist(mtmp, RING_CLASS, 0, 0)) {
            dismount_steed(DISMOUNT_THROWN);
            return 1;
        }
        udist = 1;
    } else if (!udist)
        /* maybe we tamed him while being swallowed --jgm */
        return 0;

    nix = omx; /* set before newdogpos */
    niy = omy;
    cursemsg[0] = FALSE; /* lint suppression */
    info[0] = 0;         /* ditto */

    if (has_edog) {
        j = dog_invent(mtmp, edog, udist);
        if (j == 2)
            return 2; /* died */
        else if (j == 1)
            goto newdogpos; /* eating something */

        whappr = (monstermoves - edog->whistletime < 5);
    } else
        whappr = 0;

    /* Hard rejoin mode: when a tame pet is more than 7 tiles away,
       suppress tactical behavior and force movement back toward player. */
    if (mtmp->mtame && !mtmp->mleashed
        && distmin(omx, omy, u.ux, u.uy) > 7)
        force_rejoin = TRUE;

    /* choose a monster to pursue (player-centered preferred)
       - prefer straight-line player-origin targets (existing behavior)
       - if none found on those lines, search radially around the player
         (within PET_TARGET_RADIUS) for any player-visible hostile and pick
         the best-scoring one so pets don't ignore nearby threats that are
         off-axis relative to the player. */
    pursue_mon = best_target_from_origin(mtmp, u.ux, u.uy, PET_TARGET_RADIUS);

    if (!pursue_mon) {
        long bestscore = -40000L;
        struct monst *bestm = (struct monst *)0;
        int rx, ry, r2 = PET_TARGET_RADIUS * PET_TARGET_RADIUS;

        for (rx = u.ux - PET_TARGET_RADIUS; rx <= u.ux + PET_TARGET_RADIUS; ++rx) {
            for (ry = u.uy - PET_TARGET_RADIUS; ry <= u.uy + PET_TARGET_RADIUS; ++ry) {
                if (!isok(rx, ry))
                    continue;
                int dx = rx - u.ux, dy = ry - u.uy;
                if (dx * dx + dy * dy > r2)
                    continue;
                if (!(cansee(rx, ry) || m_cansee(mtmp, rx, ry)))
                    continue; /* neither player nor pet can see this square */
                {
                    struct monst *mt = m_at(rx, ry);
                    if (!mt)
                        continue;
                    long sc = score_targ(mtmp, mt);
                    if (sc > bestscore) {
                        bestscore = sc;
                        bestm = mt;
                    }
                }
            }
        }
        if (bestscore >= 0L)
            pursue_mon = bestm;
    }

    if (!pursue_mon) {
        /* For tame pets, avoid selecting pet-local targets when already
           separated from the player; this keeps them from wandering off.
           Non-tame monsters keep the old fallback behavior. */
        if (!mtmp->mtame || udist <= (MAX_PET_DISTANCE * MAX_PET_DISTANCE))
            pursue_mon = best_target(mtmp); /* fallback to pet-local target */
    }

    if (pursue_mon && mtmp->mtame) {
        int pthreat = dist2(pursue_mon->mx, pursue_mon->my, u.ux, u.uy);
        /* Only let tame pets pursue targets that are actually near the
           player.  Otherwise, prioritize rejoining the player. */
        if (pthreat > 16)
            pursue_mon = (struct monst *) 0;
    }

    /* Hard player-first guard for tame pets: if separated by more than
       2 tiles, rejoin the player area before doing tactical pursuit.
       This prevents side-to-side interpose oscillation while the player
       is under immediate threat. */
    if (force_rejoin || (mtmp->mtame && udist > 4))
        pursue_mon = (struct monst *) 0;

    appr = dog_goal(mtmp, has_edog ? edog : (struct edog *) 0, after, udist,
                    whappr);
    if (appr == -2)
        return 0;

    if (force_rejoin) {
        gx = u.ux;
        gy = u.uy;
        appr = 1;
    }

    /* If we have a player-centered monster target, make the movement goal
       point toward that monster (clamped to MAX_PET_DISTANCE from the player)
       so the pet positions between the player and the hostile instead of
       lingering on the opposite side.  Also: if the hostile is very close
       to the player, attempt an immediate safe interpose square next to the
       player on the target side (urgent override to help the player). */
    if (pursue_mon) {
        int pmx = pursue_mon->mx, pmy = pursue_mon->my;
        int maxd2 = MAX_PET_DISTANCE * MAX_PET_DISTANCE;

          /* urgent interpose: if target is near player, prefer an adjacent
              square beside the player on the target side; only force an
              immediate move when that square is adjacent to the pet. */
        {
            int tpdist2 = dist2(pursue_mon->mx, pursue_mon->my, u.ux, u.uy);
            if (tpdist2 <= (PET_TARGET_RADIUS * PET_TARGET_RADIUS)
                && !pursue_mon->mflee) {
                int vx = sgn(pursue_mon->mx - u.ux);
                int vy = sgn(pursue_mon->my - u.uy);
                int ix, iy;
                int bestdot = -9999;
                int bestdist = 9999;
                int bestx = 0, besty = 0;

                /* choose a legal adjacent square around the player on the
                   threat-facing side (dot > 0), preferring strongest side
                   alignment then shortest distance from pet */
                for (ix = u.ux - 1; ix <= u.ux + 1; ++ix) {
                    for (iy = u.uy - 1; iy <= u.uy + 1; ++iy) {
                        int uxv, uyv, dot, pd;
                        if (!isok(ix, iy) || (ix == u.ux && iy == u.uy))
                            continue;
                        if (!goodpos(ix, iy, mtmp, 0))
                            continue;
                        uxv = ix - u.ux;
                        uyv = iy - u.uy;
                        dot = vx * uxv + vy * uyv;
                        if (dot <= 0)
                            continue;
                        pd = dist2(omx, omy, ix, iy);
                        if (dot > bestdot || (dot == bestdot && pd < bestdist)) {
                            bestdot = dot;
                            bestdist = pd;
                            bestx = ix;
                            besty = iy;
                        }
                    }
                }

                if (bestdot > -9999) {
                    have_interpose_goal = TRUE;
                    interpose_goal_x = bestx;
                    interpose_goal_y = besty;

                    if (abs(bestx - omx) <= 1 && abs(besty - omy) <= 1) {
                        /* announce interpose when visible */
                        if (pursue_mon && pursue_mon != &youmonst
                            && !(abs(omx - u.ux) <= 1 && abs(omy - u.uy) <= 1)
                            && (canseemon(mtmp) || canspotmon(pursue_mon))) {
                            pline("%s %s at %s.", Monnam(mtmp), pet_interpose_verb(mtmp), mon_nam(pursue_mon));
                        }
                        did_interpose = TRUE;
                        /* set immediate move target and skip normal navigation */
                        nix = bestx;
                        niy = besty;
                        goto newdogpos;
                    } else {
                        /* let normal movement choose a legal one-step path
                           toward the interpose square */
                        gx = bestx;
                        gy = besty;
                        gtyp = UNDEF;
                    }
                }
            }
        }

        if (!have_interpose_goal) {
            if (distu(pmx, pmy) <= maxd2) {
                gx = pmx;
                gy = pmy;
            } else {
                /* Step from player toward monster up to MAX_PET_DISTANCE cells
                   using an integer Bresenham-like advance so we avoid floating
                   math and macro collisions with the platform math headers. */
                int x0 = u.ux, y0 = u.uy, x1 = pmx, y1 = pmy;
                int tx = x0, ty = y0;
                int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
                int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
                int err = dx + dy;
                int steps = MAX_PET_DISTANCE;
                int i;
                for (i = 0; i < steps; ++i) {
                    int e2 = 2 * err;
                    if (e2 >= dy) { err += dy; tx += sx; }
                    if (e2 <= dx) { err += dx; ty += sy; }
                    if (!isok(tx, ty)) { tx = x0; ty = y0; break; }
                }
                gx = tx;
                gy = ty;
            }
        }
    }
    gtyp = UNDEF; /* treat as transient navigation goal */

    allowflags = ALLOW_M | ALLOW_TRAPS | ALLOW_SSM | ALLOW_SANCT;
    if (passes_walls(mtmp->data))
        allowflags |= (ALLOW_ROCK | ALLOW_WALL);
    if (passes_bars(mtmp->data))
        allowflags |= ALLOW_BARS;
    if (throws_rocks(mtmp->data))
        allowflags |= ALLOW_ROCK;
    if (is_displacer(mtmp->data))
        allowflags |= ALLOW_MDISP;
    if (Conflict && !resist(mtmp, RING_CLASS, 0, 0)) {
        allowflags |= ALLOW_U;
        if (!has_edog) {
            /* Guardian angel refuses to be conflicted; rather,
             * it disappears, angrily, and sends in some nasties
             */
            lose_guardian_angel(mtmp);
            return 2; /* current monster is gone */
        }
    }
#if 0 /* [this is now handled in dochug()] */
    if (!Conflict && !mtmp->mconf
        && mtmp == u.ustuck && !sticks(youmonst.data)) {
        unstuck(mtmp); /* swallowed case handled above */
        You("get released!");
    }
#endif
    if (!nohands(mtmp->data) && !verysmall(mtmp->data)) {
        allowflags |= OPENDOOR;
        if (monhaskey(mtmp, TRUE))
            allowflags |= UNLOCKDOOR;
        /* note:  the Wizard and Riders can unlock doors without a key;
           they won't use that ability if someone manages to tame them */
    }
    if (is_giant(mtmp->data))
        allowflags |= BUSTDOOR;
    if (tunnels(mtmp->data)
        && !Is_rogue_level(&u.uz)) /* same restriction as m_move() */
        allowflags |= ALLOW_DIG;
    cnt = mfndpos(mtmp, poss, info, allowflags);

    /* Normally dogs don't step on cursed items, but if they have no
     * other choice they will.  This requires checking ahead of time
     * to see how many uncursed item squares are around.
     */
    uncursedcnt = 0;
    for (i = 0; i < cnt; i++) {
        nx = poss[i].x;
        ny = poss[i].y;
        if (MON_AT(nx, ny) && !((info[i] & ALLOW_M) || info[i] & ALLOW_MDISP))
            continue;
        if (cursed_object_at(nx, ny))
            continue;
        uncursedcnt++;
    }

    better_with_displacing = should_displace(mtmp, poss, info, cnt, gx, gy);

    chcnt = 0;
    chi = -1;
    nidist = GDIST(nix, niy);

    for (i = 0; i < cnt; i++) {
        nx = poss[i].x;
        ny = poss[i].y;
        cursemsg[i] = FALSE;

        /* Never consider moving onto the player's square; that results in
           the pet 'attacking' the player (mattacku).  Allow only when the
           movement flags explicitly permit (e.g. ALLOW_U under Conflict).
        */
        if (nx == u.ux && ny == u.uy && !(allowflags & ALLOW_U))
            continue;

        /* Avoid stepping onto stairs when already close to player, but
           allow it while far away so the pet can route back to you. */
        if (On_stairs(nx, ny) && (nx != u.ux || ny != u.uy)
            && udist <= 4)
            continue;

        /* if leashed, we drag him along. */
        if (mtmp->mleashed && distu(nx, ny) > 4)
            continue;

        /* never allow an un-leashed pet to move so far from the player that
           it exceeds MAX_PET_DISTANCE. */
        if (!mtmp->mleashed) {
            int allowed_dist_sq = MAX_PET_DISTANCE * MAX_PET_DISTANCE;
            int old_player_dist = udist;
            int new_player_dist = distu(nx, ny);

            /* Keep pets within MAX_PET_DISTANCE once they're near, but if a
               pet is already farther out, allow moves that bring it closer
               so it won't get stuck permanently out of range. */
            if (new_player_dist > allowed_dist_sq
                && new_player_dist >= old_player_dist)
                continue;
        }

        /* if a guardian, try to stay close by choice */
        if (!has_edog && (j = distu(nx, ny)) > 16 && j >= udist)
            continue;

        if ((info[i] & ALLOW_M) && MON_AT(nx, ny)) {
            int mstatus;
            register struct monst *mtmp2 = m_at(nx, ny);

            if ((int) mtmp2->m_lev >= (int) mtmp->m_lev + 2
                || (mtmp2->data == &mons[PM_FLOATING_EYE] && rn2(10)
                    && mtmp->mcansee && haseyes(mtmp->data) && mtmp2->mcansee
                    && (perceives(mtmp->data) || !mtmp2->minvis))
                || (mtmp2->data == &mons[PM_GELATINOUS_CUBE] && rn2(10))
                || (max_passive_dmg(mtmp2, mtmp) >= mtmp->mhp)
                || ((mtmp->mhp * 4 < mtmp->mhpmax
                     || mtmp2->data->msound == MS_GUARDIAN
                     || mtmp2->data->msound == MS_LEADER) && mtmp2->mpeaceful
                    && !Conflict)
                || (touch_petrifies(mtmp2->data) && !resists_ston(mtmp)))
                continue;

            if (after)
                return 0; /* hit only once each move */

            notonhead = 0;
            mstatus = mattackm(mtmp, mtmp2);

            /* aggressor (pet) died */
            if (mstatus & MM_AGR_DIED)
                return 2;

            if ((mstatus & MM_HIT) && !(mstatus & MM_DEF_DIED) && rn2(4)
                && mtmp2->mlstmv != monstermoves
                && !onscary(mtmp->mx, mtmp->my, mtmp2)
                /* monnear check needed: long worms hit on tail */
                && monnear(mtmp2, mtmp->mx, mtmp->my)) {
                mstatus = mattackm(mtmp2, mtmp); /* return attack */
                if (mstatus & MM_DEF_DIED)
                    return 2;
            }
            return 0;
        }
        if ((info[i] & ALLOW_MDISP) && MON_AT(nx, ny)
            && better_with_displacing && !undesirable_disp(mtmp, nx, ny)) {
            int mstatus;
            register struct monst *mtmp2 = m_at(nx, ny);

            mstatus = mdisplacem(mtmp, mtmp2, FALSE); /* displace monster */
            if (mstatus & MM_DEF_DIED)
                return 2;
            return 0;
        }

        {
            /* Dog avoids harmful traps, but perhaps it has to pass one
             * in order to follow player.  (Non-harmful traps do not
             * have ALLOW_TRAPS in info[].)  The dog only avoids the
             * trap if you've seen it, unlike enemies who avoid traps
             * if they've seen some trap of that type sometime in the
             * past.  (Neither behavior is really realistic.)
             */
            struct trap *trap;

            if ((info[i] & ALLOW_TRAPS) && (trap = t_at(nx, ny))) {
                if (mtmp->mleashed) {
                    if (!Deaf)
                        whimper(mtmp);
                } else {
                    /* 1/40 chance of stepping on it anyway, in case
                     * it has to pass one to follow the player...
                     */
                    if (trap->tseen && rn2(40))
                        continue;
                }
            }
        }

        /* dog eschews cursed objects, but likes dog food */
        /* (minion isn't interested; `cursemsg' stays FALSE) */
        if (has_edog)
            for (obj = level.objects[nx][ny]; obj; obj = obj->nexthere) {
                if (obj->cursed) {
                    cursemsg[i] = TRUE;
                } else if ((otyp = dogfood(mtmp, obj)) < MANFOOD
                         && (otyp < ACCFOOD
                             || edog->hungrytime <= monstermoves)) {
                    /* Note: our dog likes the food so much that he
                     * might eat it even when it conceals a cursed object */
                    nix = nx;
                    niy = ny;
                    chi = i;
                    do_eat = TRUE;
                    cursemsg[i] = FALSE; /* not reluctant */
                    goto newdogpos;
                }
            }
        /* didn't find something to eat; if we saw a cursed item and
           aren't being forced to walk on it, usually keep looking */
        if (cursemsg[i] && !mtmp->mleashed && uncursedcnt > 0
            && rn2(13 * uncursedcnt))
            continue;

        /* lessen the chance of backtracking to previous position(s) */
        /* This causes unintended issues for pets trying to follow
           the hero. Thus, only run it if not leashed and >5 tiles
           away. */
        if (!mtmp->mleashed && distmin(mtmp->mx, mtmp->my, u.ux, u.uy) > 5) {
            k = has_edog ? uncursedcnt : cnt;
            for (j = 0; j < MTSZ && j < k - 1; j++)
                if (nx == mtmp->mtrack[j].x && ny == mtmp->mtrack[j].y)
                    if (rn2(MTSZ * (k - j)))
                        goto nxti;
        }

        ndist = GDIST(nx, ny);
        /* base goal delta (relative to gx,gy from dog_goal) */
        int goalDelta = ndist - nidist; /* squared-distance delta */
        int jtmp = goalDelta * appr;

        /* if pursuing a monster, bias moves toward that monster and
           also avoid moving away from the player.  When the target is
           player-centered (within PET_TARGET_RADIUS) be more aggressive
           about closing on the monster and *allow* temporary movement
           away from the player so the pet can position between you and
           the hostile. */
        if (pursue_mon) {
            int old_mon_dist = dist2(omx, omy, pursue_mon->mx, pursue_mon->my);
            int new_mon_dist = dist2(nx, ny, pursue_mon->mx, pursue_mon->my);
            int monDelta = new_mon_dist - old_mon_dist; /* >0 means moving away */

            /* stronger bias toward closing on a player-centered target */
            int MON_WEIGHT = 3; /* default moderately aggressive */
            int tpdist2 = dist2(pursue_mon->mx, pursue_mon->my, u.ux, u.uy);
            if (tpdist2 <= PET_TARGET_RADIUS * PET_TARGET_RADIUS)
                MON_WEIGHT = 7; /* stronger, but don't overwhelm follow behavior */
            jtmp -= monDelta * MON_WEIGHT;

            /* prefer candidate squares that are on the same side of the player
               as the target (encourages the pet to position between @ and x) */
            int vx = pursue_mon->mx - u.ux;
            int vy = pursue_mon->my - u.uy;
            int uxv = nx - u.ux;
            int uyv = ny - u.uy;
            int dot = vx * uxv + vy * uyv;
            const int SAME_SIDE_BONUS = 6;
            const int OPP_SIDE_PENALTY = 3;
            if (dot > 0)
                jtmp += SAME_SIDE_BONUS;
            else
                jtmp -= OPP_SIDE_PENALTY;

            /* if the hostile is very close to the player, prefer squares
               adjacent to the player on the target side so the pet will
               interpose quickly */
            if (tpdist2 <= 4) { /* target adjacent or nearly adjacent to player */
                if (abs(nx - u.ux) <= 1 && abs(ny - u.uy) <= 1 && dot > 0)
                    jtmp += 20; /* strong preference for interposing */
            }

            /* penalize moves that increase distance to the player, but
               relax that penalty for player-centered targets so the pet
               can cross to the player's side when needed */
            int old_player_dist = udist; /* squared */
            int new_player_dist = distu(nx, ny);
            int playerDelta = new_player_dist - old_player_dist; /* >0 moving away */
            int PLAYER_WEIGHT = 2;
            if (tpdist2 <= PET_TARGET_RADIUS * PET_TARGET_RADIUS)
                PLAYER_WEIGHT = 1;
            jtmp -= playerDelta * PLAYER_WEIGHT;

            /* If we selected a concrete interpose-side square, bias strongly
               toward moves that close distance to it. */
            if (have_interpose_goal) {
                int old_ip_dist = dist2(omx, omy, interpose_goal_x, interpose_goal_y);
                int new_ip_dist = dist2(nx, ny, interpose_goal_x, interpose_goal_y);
                jtmp += (new_ip_dist - old_ip_dist) * 8;
            }

            /* Anti-oscillation: discourage immediate backtracking and
               side-steps which don't improve either player or target
               distance.  This prevents NE<->SW ping-pong loops when
               several candidate squares score similarly. */
            if (nx == mtmp->mtrack[0].x && ny == mtmp->mtrack[0].y)
                jtmp += 25;
            if (monDelta >= 0 && playerDelta >= 0)
                jtmp += 20;
        }

        j = jtmp;

        if ((j == 0 && !rn2(++chcnt)) || j < 0
            || (j > 0 && !whappr
                && ((omx == nix && omy == niy && !rn2(3)) || !rn2(12)))) {
            nix = nx;
            niy = ny;
            nidist = ndist;
            if (j < 0)
                chcnt = 0;
            chi = i;
        }
 nxti:
        ;
    }

    /* Pet hasn't attacked anything but is considering moving -
     * now's the time for ranged attacks. Note that the pet can move
     * after it performs its ranged attack. Should this be changed?
     */
    {
        struct monst *mtarg;
        int hungry = 0;

        /* How hungry is the pet? */
        if (!mtmp->isminion) {
            struct edog *dog = EDOG(mtmp);

            hungry = (monstermoves > (dog->hungrytime + 300));
        }

        /* Identify the best target in a straight line from the pet;
         * if there is such a target, we'll let the pet attempt an
         * attack.
         */
        mtarg = best_target_from_origin(mtmp, u.ux, u.uy, PET_TARGET_RADIUS);
        if (!mtarg)
            mtarg = best_target(mtmp);

        /* Hungry pets are unlikely to use breath/spit attacks */
        if (mtarg && (!hungry || !rn2(5))) {
            int mstatus;

            /* Do NOT let a *tame* pet decide to attack the hero here.
               Tame pets should never treat the hero as a ranged target.
               (If the pet is hostile or the player is under Conflict, the
               appropriate code paths will handle that.) */
            if (mtarg == &youmonst) {
                if (!mtmp->mtame) {
                    if (mattacku(mtmp))
                        return 2;
                } else {
                    /* pet is tame — ignore hero as ranged attack target */
                }
            } else {
                mstatus = mattackm(mtmp, mtarg);

                /* Shouldn't happen, really */
                if (mstatus & MM_AGR_DIED)
                    return 2;

                /* Allow the targeted nasty to strike back - if
                 * the targeted beast doesn't have a ranged attack,
                 * nothing will happen.
                 */
                if ((mstatus & MM_HIT) && !(mstatus & MM_DEF_DIED)
                    && rn2(4) && mtarg != &youmonst) {

                    /* Can monster see? If it can, it can retaliate
                     * even if the pet is invisible, since it'll see
                     * the direction from which the ranged attack came;
                     * if it's blind or unseeing, it can't retaliate
                     */
                    if (mtarg->mcansee && haseyes(mtarg->data)) {
                        mstatus = mattackm(mtarg, mtmp);
                        if (mstatus & MM_DEF_DIED)
                            return 2;
                    }
                }
            }
        }
    }

 newdogpos:
    if (nix != omx || niy != omy) {
        boolean wasseen;

        /* EXTRA SAFETY: never move a pet onto the hero's square by mistake.
           This duplicates the candidate filtering earlier and prevents any
           remaining code-paths from producing an illegal swap/jump. */
        if (nix == u.ux && niy == u.uy && !(info[chi] & ALLOW_U)) {
            /* shouldn't happen; treat as 'no move' for this turn */
            impossible("pet tried to move onto the hero's square and was blocked");
            nix = omx;
            niy = omy;
        }

        if (info[chi] & ALLOW_U) {
            if (mtmp->mleashed) { /* play it safe */
                pline("%s breaks loose of %s leash!", Monnam(mtmp),
                      mhis(mtmp));
                m_unleash(mtmp, FALSE);
            }
            /* Never allow a tame pet to attack the player via this path. */
            if (mtmp->mtame) {
                if (canseemon(mtmp))
                    pline("%s paws at you but won't pass through.", Monnam(mtmp));
                return 0;
            }
            (void) mattacku(mtmp);
            return 0;
        }
        if (!m_in_out_region(mtmp, nix, niy))
            return 1;
        if (m_digweapon_check(mtmp, nix,niy))
            return 0;

        /* insert a worm_move() if worms ever begin to eat things */
        wasseen = canseemon(mtmp);
        remove_monster(omx, omy);
        place_monster(mtmp, nix, niy);

        /* Defensive: block any move that would cross *through* the hero in
           a single monster step (this was causing pets to "jump over" the
           player).  If detected, revert the pet to its original square and
           report an informative message. */
        if ((omx != nix || omy != niy)
            && ((omx - u.ux) * (nix - u.ux) < 0 && omy == u.uy && niy == u.uy
                /* horizontal crossing */
                || (omy - u.uy) * (niy - u.uy) < 0 && omx == u.ux && nix == u.ux
                /* vertical crossing */
                || (omx - u.ux) * (nix - u.ux) < 0 && (omy - u.uy) * (niy - u.uy) < 0)) {
            /* illegal cross detected; revert pet to original location */
            if (canseemon(mtmp))
                pline("%s paws at you but won't pass through.", Monnam(mtmp));
            remove_monster(nix, niy);
            place_monster(mtmp, omx, omy);
            newsym(omx, omy);
            newsym(nix, niy);
            return 1; /* treat as a non-move this turn */
        }

        /* announce interpose if not already announced by the urgent case */
        if (!did_interpose && pursue_mon && pursue_mon != &youmonst
            && !(abs(omx - u.ux) <= 1 && abs(omy - u.uy) <= 1)
            && abs(nix - u.ux) <= 1 && abs(niy - u.uy) <= 1) {
            int vx = pursue_mon->mx - u.ux;
            int vy = pursue_mon->my - u.uy;
            int uxv = nix - u.ux;
            int uyv = niy - u.uy;
            if (vx * uxv + vy * uyv > 0
                && (canseemon(mtmp) || canspotmon(pursue_mon))) {
                pline("%s %s at %s.", Monnam(mtmp), pet_interpose_verb(mtmp), mon_nam(pursue_mon));
            }
        }
        /* leash feedback is handled once-per-player-input in check_leash_end_of_turn() */
        if (cursemsg[chi] && (wasseen || canseemon(mtmp))) {
            /* describe top item of pile, not necessarily cursed item itself;
               don't use glyph_at() here--it would return the pet but we want
               to know whether an object is remembered at this map location */
            struct obj *o = (!Hallucination && level.flags.hero_memory
                             && glyph_is_object(levl[nix][niy].glyph))
                               ? vobj_at(nix, niy) : 0;
            const char *what = o ? distant_name(o, doname) : something;

            pline("%s %s reluctantly over %s.", noit_Monnam(mtmp),
                  vtense((char *) 0, locomotion(mtmp->data, "step")), what);
        }
        for (j = MTSZ - 1; j > 0; j--)
            mtmp->mtrack[j] = mtmp->mtrack[j - 1];
        mtmp->mtrack[0].x = omx;
        mtmp->mtrack[0].y = omy;
        /* We have to know if the pet's going to do a combined eat and
         * move before moving it, but it can't eat until after being
         * moved.  Thus the do_eat flag.
         */
        if (do_eat) {
            if (dog_eat(mtmp, obj, omx, omy, FALSE) == 2)
                return 2;
        }
    } else if (mtmp->mleashed && distu(omx, omy) > 4) {
        /* an incredible kludge, but the only way to keep pooch near
         * after it spends time eating or in a trap, etc.
         */
        coord cc;

        nx = sgn(omx - u.ux);
        ny = sgn(omy - u.uy);
        cc.x = u.ux + nx;
        cc.y = u.uy + ny;
        if (goodpos(cc.x, cc.y, mtmp, 0))
            goto dognext;

        i = xytod(nx, ny);
        for (j = (i + 7) % 8; j < (i + 1) % 8; j++) {
            dtoxy(&cc, j);
            if (goodpos(cc.x, cc.y, mtmp, 0))
                goto dognext;
        }
        for (j = (i + 6) % 8; j < (i + 2) % 8; j++) {
            dtoxy(&cc, j);
            if (goodpos(cc.x, cc.y, mtmp, 0))
                goto dognext;
        }
        cc.x = mtmp->mx;
        cc.y = mtmp->my;
 dognext:
        if (!m_in_out_region(mtmp, nix, niy))
            return 1;
        remove_monster(mtmp->mx, mtmp->my);
        place_monster(mtmp, cc.x, cc.y);
        newsym(cc.x, cc.y);
        set_apparxy(mtmp);
    }
    return 1;
}

/* check if a monster could pick up objects from a location */
STATIC_OVL boolean
could_reach_item(mon, nx, ny)
struct monst *mon;
xchar nx, ny;
{
    if ((!is_pool(nx, ny) || is_swimmer(mon->data))
        && (!is_lava(nx, ny) || likes_lava(mon->data))
        && (!sobj_at(BOULDER, nx, ny) || throws_rocks(mon->data)))
        return TRUE;
    return FALSE;
}

/* Hack to prevent a dog from being endlessly stuck near an object that
 * it can't reach, such as caught in a teleport scroll niche.  It recursively
 * checks to see if the squares in between are good.  The checking could be
 * a little smarter; a full check would probably be useful in m_move() too.
 * Since the maximum food distance is 5, this should never be more than 5
 * calls deep.
 */
STATIC_OVL boolean
can_reach_location(mon, mx, my, fx, fy)
struct monst *mon;
xchar mx, my, fx, fy;
{
    int i, j;
    int dist;

    if (mx == fx && my == fy)
        return TRUE;
    if (!isok(mx, my))
        return FALSE; /* should not happen */

    dist = dist2(mx, my, fx, fy);
    for (i = mx - 1; i <= mx + 1; i++) {
        for (j = my - 1; j <= my + 1; j++) {
            if (!isok(i, j))
                continue;
            if (dist2(i, j, fx, fy) >= dist)
                continue;
            if (IS_ROCK(levl[i][j].typ) && !passes_walls(mon->data)
                && (!may_dig(i, j) || !tunnels(mon->data)))
                continue;
            if (IS_DOOR(levl[i][j].typ)
                && (levl[i][j].doormask & (D_CLOSED | D_LOCKED)))
                continue;
            if (!could_reach_item(mon, i, j))
                continue;
            if (can_reach_location(mon, i, j, fx, fy))
                return TRUE;
        }
    }
    return FALSE;
}

/* do_clear_area client */
STATIC_PTR void
wantdoor(x, y, distance)
int x, y;
genericptr_t distance;
{
    int ndist, *dist_ptr = (int *) distance;

    /* Avoid stair targets only when close to player; if far away,
       stair-adjacent targets are allowed to help reunification. */
    if (On_stairs(x, y) && (x != u.ux || y != u.uy)
        && distu(x, y) <= 4)
        return;
    if (*dist_ptr > (ndist = distu(x, y))) {
        gx = x;
        gy = y;
        *dist_ptr = ndist;
    }
}

static struct qmchoices {
    int mndx;             /* type of pet, 0 means any  */
    char mlet;            /* symbol of pet, 0 means any */
    unsigned mappearance; /* mimic this */
    uchar m_ap_type;      /* what is the thing it is mimicing? */
} qm[] = {
    /* Things that some pets might be thinking about at the time */
    { PM_LITTLE_DOG, 0, PM_KITTEN, M_AP_MONSTER },
    { PM_DOG, 0, PM_HOUSECAT, M_AP_MONSTER },
    { PM_LARGE_DOG, 0, PM_LARGE_CAT, M_AP_MONSTER },
    { PM_KITTEN, 0, PM_LITTLE_DOG, M_AP_MONSTER },
    { PM_HOUSECAT, 0, PM_DOG, M_AP_MONSTER },
    { PM_LARGE_CAT, 0, PM_LARGE_DOG, M_AP_MONSTER },
    { PM_HOUSECAT, 0, PM_GIANT_RAT, M_AP_MONSTER },
    { 0, S_DOG, SINK,
      M_AP_FURNITURE }, /* sorry, no fire hydrants in NetHack */
    { 0, 0, TRIPE_RATION, M_AP_OBJECT }, /* leave this at end */
};

void
finish_meating(mtmp)
struct monst *mtmp;
{
    mtmp->meating = 0;
    if (M_AP_TYPE(mtmp) && mtmp->mappearance && mtmp->cham == NON_PM) {
        /* was eating a mimic and now appearance needs resetting */
        mtmp->m_ap_type = 0;
        mtmp->mappearance = 0;
        newsym(mtmp->mx, mtmp->my);
    }
}

STATIC_OVL void
quickmimic(mtmp)
struct monst *mtmp;
{
    int idx = 0, trycnt = 5, spotted;
    char buf[BUFSZ];

    if (Protection_from_shape_changers || !mtmp->meating)
        return;

    /* with polymorph, the steed's equipment would be re-checked and its
       saddle would come off, triggering DISMOUNT_FELL, but mimicking
       doesn't impact monster's equipment; normally DISMOUNT_POLY is for
       rider taking on an unsuitable shape, but its message works fine
       for this and also avoids inflicting damage during forced dismount;
       do this before changing so that dismount refers to original shape */
    if (mtmp == u.usteed)
        dismount_steed(DISMOUNT_POLY);

    do {
        idx = rn2(SIZE(qm));
        if (qm[idx].mndx != 0 && monsndx(mtmp->data) == qm[idx].mndx)
            break;
        if (qm[idx].mlet != 0 && mtmp->data->mlet == qm[idx].mlet)
            break;
        if (qm[idx].mndx == 0 && qm[idx].mlet == 0)
            break;
    } while (--trycnt > 0);
    if (trycnt == 0)
        idx = SIZE(qm) - 1;

    Strcpy(buf, mon_nam(mtmp));
    spotted = canspotmon(mtmp);

    mtmp->m_ap_type = qm[idx].m_ap_type;
    mtmp->mappearance = qm[idx].mappearance;

    if (spotted || cansee(mtmp->mx, mtmp->my) || canspotmon(mtmp)) {
        /* this isn't quite right; if sensing a monster without being
           able to see its location, you really shouldn't be told you
           sense it becoming furniture or an object that you can't see
           (on the other hand, perhaps you're sensing a brief glimpse
           of its mind as it changes form) */
        newsym(mtmp->mx, mtmp->my);
        You("%s %s %sappear%s where %s was!",
            cansee(mtmp->mx, mtmp->my) ? "see" : "sense that",
            (M_AP_TYPE(mtmp) == M_AP_FURNITURE)
                ? an(defsyms[mtmp->mappearance].explanation)
                : (M_AP_TYPE(mtmp) == M_AP_OBJECT
                   && OBJ_DESCR(objects[mtmp->mappearance]))
                      ? an(OBJ_DESCR(objects[mtmp->mappearance]))
                      : (M_AP_TYPE(mtmp) == M_AP_OBJECT
                         && OBJ_NAME(objects[mtmp->mappearance]))
                            ? an(OBJ_NAME(objects[mtmp->mappearance]))
                            : (M_AP_TYPE(mtmp) == M_AP_MONSTER)
                                  ? an(mons[mtmp->mappearance].mname)
                                  : something,
            cansee(mtmp->mx, mtmp->my) ? "" : "has ",
            cansee(mtmp->mx, mtmp->my) ? "" : "ed",
            buf);
        display_nhwindow(WIN_MAP, TRUE);
    }
}

/*dogmove.c*/
