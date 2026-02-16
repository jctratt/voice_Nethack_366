/* test_overview_pets_levels.c - ensure recalc_mapseen includes pet levels */
#include "hack.h"
#include <stdio.h>

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

    if (!strstr(mptr->petnames, "(Lv 5)")) {
        fprintf(stderr, "FAILED: petnames='%s' (missing level)\n", mptr->petnames);
        return 3;
    }

    printf("OK: petnames='%s'\n", mptr->petnames);
    return 0;
}
