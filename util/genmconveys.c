/* genmconveys.c -- one-off/occasionally-rerun generator, NOT part of the
 * normal build. Walks mons[] and, for every monster, derives a
 * comprehensive description of what eating its corpse does -- resistances
 * conferred, hazards (poison/acid/stoning/instadeath/slime), and the
 * hardcoded per-monster special cases in src/eat.c (nurse healing, newt
 * energy, wraith XP, quantum mechanic speed toggle, stalker invisibility,
 * mimic transformation, disenchanter attrcurse, mind flayer Int gain,
 * lycanthropy transmission, telepathy) -- using the exact same predicate
 * macros the game itself uses (mondata.h), so the output can't drift from
 * actual game behavior.
 *
 * Build (from the repo root, or from util/):
 *   cc -I../include -DNOTPARMDECL -o genmconveys genmconveys.c ../src/monst.o
 * Run:
 *   ./genmconveys > ../include/mconveys.h
 *
 * See the "Phase 2" section of the plan this was built from for the full
 * mechanics inventory and reasoning.
 */
#include "config.h"
#include "permonst.h"
#include "monsym.h"
#include "pm.h"
#include "mondata.h"
#include <stdio.h>
#include <string.h>

/* eat.c has these two as local macros (not exported to any header); it's
   simpler and safer to duplicate the two one-liners here than to try to
   standalone-include eat.c itself */
#define flesh_petrifies(pm) (touch_petrifies(pm) || (pm) == &mons[PM_MEDUSA])
#define nonrotting_corpse(mnum) \
    ((mnum) == PM_LIZARD || (mnum) == PM_LICHEN || is_rider(&mons[mnum]))

static char buf[4096];

static void
add(const char *frag)
{
    if (buf[0])
        strcat(buf, "; ");
    strcat(buf, frag);
}

static void
cescape(const char *in, char *out)
{
    for (; *in; in++) {
        if (*in == '"' || *in == '\\')
            *out++ = '\\';
        *out++ = *in;
    }
    *out = '\0';
}

int
main()
{
    int i, j;
    char escbuf[8192];

    printf("/* GENERATED FILE - regenerate via util/genmconveys.c.\n");
    printf("   Do not hand-edit. */\n");
    printf("#ifndef MCONVEYS_H\n#define MCONVEYS_H\n\n");
    printf("struct mconveys_entry {\n");
    printf("    const char *name;   /* matches mons[].mname, lowercase */\n");
    printf("    const char *text;   /* what eating this corpse does */\n");
    printf("};\n\n");
    printf("static const struct mconveys_entry mconveys_table[] = {\n");

    for (i = LOW_PM; i < NUMMONS; i++) {
        struct permonst *ptr = &mons[i];
        char namebuf[BUFSZ];
        int k;

        buf[0] = '\0';

        /* --- severe / instant hazards first --- */
        if (i == PM_DEATH || i == PM_FAMINE || i == PM_PESTILENCE)
            add("instant death (do not eat)");
        if (flesh_petrifies(ptr))
            add("instant stoning unless stoning-resistant");
        if (i == PM_GREEN_SLIME)
            add("turns you into green slime unless slime-proof");

        /* --- resistance conferred --- */
        {
            char rbuf[256];
            rbuf[0] = '\0';
            if (ptr->mconveys & MR_FIRE)
                strcat(rbuf, rbuf[0] ? ", fire resistance" : "fire resistance");
            if (ptr->mconveys & MR_COLD)
                strcat(rbuf, rbuf[0] ? ", cold resistance" : "cold resistance");
            if (ptr->mconveys & MR_SLEEP)
                strcat(rbuf, rbuf[0] ? ", sleep resistance" : "sleep resistance");
            if (ptr->mconveys & MR_DISINT)
                strcat(rbuf, rbuf[0] ? ", disintegration resistance"
                                      : "disintegration resistance");
            if (ptr->mconveys & MR_ELEC)
                strcat(rbuf, rbuf[0] ? ", shock resistance" : "shock resistance");
            if (ptr->mconveys & MR_POISON)
                strcat(rbuf, rbuf[0] ? ", poison resistance" : "poison resistance");
            if (rbuf[0]) {
                char withpfx[300];
                sprintf(withpfx, "may confer %s", rbuf);
                add(withpfx);
            }
        }
        if (telepathic(ptr))
            add("may confer telepathy");
        if (can_teleport(ptr))
            add("may confer teleportitis");
        if (control_teleport(ptr))
            add("may confer teleport control");
        if (is_giant(ptr))
            add("may confer strength gain");

        /* --- unique per-monster special cases --- */
        if (i == PM_NURSE)
            add("fully heals you and cures blindness");
        if (i == PM_NEWT)
            add("may restore a small amount of energy");
        if (i == PM_WRAITH)
            add("grants an experience level");
        if (i == PM_QUANTUM_MECHANIC)
            add("randomly toggles intrinsic speed (faster or slower)");
        if (i == PM_STALKER)
            add("grants temporary invisibility (permanent invisibility and "
                "see invisible if already invisible)");
        if (i == PM_SMALL_MIMIC || i == PM_LARGE_MIMIC || i == PM_GIANT_MIMIC)
            add("risk of turning into a mimic");
        if (i == PM_DISENCHANTER)
            add("risk of an attribute curse");
        if (i == PM_MIND_FLAYER || i == PM_MASTER_MIND_FLAYER)
            add("may raise Intelligence (otherwise as telepathy)");
        if (i == PM_HUMAN_WERERAT || i == PM_HUMAN_WEREJACKAL
            || i == PM_HUMAN_WEREWOLF)
            add("risk of lycanthropy");

        /* --- eating hazards --- */
        if (poisonous(ptr))
            add("poisonous to eat");
        if (acidic(ptr))
            add("acidic to eat");

        /* --- conditional notes --- */
        if (i == PM_LIZARD || acidic(ptr))
            add("cures stoning if eaten while petrifying");
        if (ptr->mflags2 & (M2_HUMAN | M2_ELF | M2_DWARF | M2_GNOME | M2_ORC)) {
            const char *race = (ptr->mflags2 & M2_HUMAN) ? "Human"
                              : (ptr->mflags2 & M2_ELF) ? "Elven"
                              : (ptr->mflags2 & M2_DWARF) ? "Dwarven"
                              : (ptr->mflags2 & M2_GNOME) ? "Gnomish"
                              : "Orcish";
            char cbuf[64];
            sprintf(cbuf, "counts as cannibalism for %s characters", race);
            add(cbuf);
        }
        if (nonrotting_corpse(i))
            add("never rots");

        /* --- stun / hallucination on eating --- */
        if (i == PM_STALKER || i == PM_YELLOW_LIGHT || i == PM_GIANT_BAT
            || i == PM_BAT)
            add("causes stunning");
        if (i == PM_LIZARD)
            add("cures stunning and confusion");
        {
            int halu = (i == PM_VIOLET_FUNGUS);
            for (k = 0; k < NATTK && !halu; k++)
                if (ptr->mattk[k].adtyp == AD_STUN
                    || ptr->mattk[k].adtyp == AD_HALU)
                    halu = 1;
            if (halu)
                add("causes hallucination");
        }

        if (!buf[0])
            strcpy(buf, "none");

        /* lowercase, trimmed name -- matches mons[].mname format already
           (all-lowercase in this fork's monst.c) */
        strcpy(namebuf, ptr->mname);
        for (j = 0; namebuf[j]; j++)
            if (namebuf[j] >= 'A' && namebuf[j] <= 'Z')
                namebuf[j] += 'a' - 'A';

        cescape(namebuf, escbuf);
        printf("    { \"%s\", ", escbuf);
        cescape(buf, escbuf);
        printf("\"%s\" },\n", escbuf);
    }

    printf("    { (const char *) 0, (const char *) 0 } /* sentinel */\n");
    printf("};\n\n");
    printf("/* %d entries */\n\n", NUMMONS - LOW_PM);
    printf("#endif /* MCONVEYS_H */\n");
    return 0;
}
