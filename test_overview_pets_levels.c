/* test_overview_pets_levels.c - ensure recalc_mapseen includes pet HP/status */
#include "hack.h"
#include <stdio.h>

extern mapseen *mapseenchn; /* from dungeon.c */
extern void sanitize_petnames(char *);

/* mapseen chain head defined in dungeon.c */
extern mapseen *mapseenchn; 

int
main(void)
{
    mapseen *mptr;
    struct monst *mtmp;

    /* Ensure a mapseen for current level exists */
    init_mapseen(&u.uz);

    if (!mapseenchn) {
        fprintf(stderr, "FAILED: mapseenchn not initialized\n");
        return 1;
    }

    mptr = mapseenchn; /* newly created entry for current level */

    /* sanity: ensure sanitize_petnames behaves as expected */
    {
        char tmp[PETNAMES_MAX+1] = "Zippy (Lv 3)";
        sanitize_petnames(tmp);
        if (!strstr(tmp, "(HP 3)")) {
            fprintf(stderr, "FAILED: sanitize_petnames didn't convert '%s'\n", tmp);
            return 4;
        }
    }

    /* ensure no monsters are present */
    level.monlist = (struct monst *) 0;

    /* create a fake tame pet and add it to the level.monlist */
    mtmp = newmonst();
    *mtmp = zeromonst;
    set_mon_data(mtmp, &mons[PM_LITTLE_DOG]);
    mtmp->mhp = 20;
    mtmp->m_lev = 5; /* expected level to appear */
    mtmp->mtame = 1;
    mtmp->isminion = 0;
    mtmp->nmon = fmon;
    fmon = mtmp;

    /* recalc mapseen should pick up the pet and include level */
    recalc_mapseen();

    if (!mptr->petnames) {
        fprintf(stderr, "FAILED: petnames not set by recalc_mapseen\n");
        return 2;
    }

    if (!strstr(mptr->petnames, "(HP 5)")) {
        fprintf(stderr, "FAILED: petnames='%s' (missing HP)\n", mptr->petnames);
        return 3;
    }

    printf("OK: petnames='%s'\n", mptr->petnames);
    return 0;
}
