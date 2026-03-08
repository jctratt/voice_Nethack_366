/* test_mapseen_migrating_pets.c - ensure migrating/mydogs pets appear in mapseen */
#include "hack.h"
#include <stdio.h>

/* mapseen chain head defined in dungeon.c */
extern mapseen *mapseenchn;
/* helper added by recent patch */
extern void sanitize_petnames(char *);

/* mapseen chain head defined in dungeon.c */
extern mapseen *mapseenchn; 

int
main(void)
{
    mapseen *mptr;
    struct monst *mtmp;
    d_level dest = { 0, 2 }; /* pick a different level number for test */

    /* ensure a mapseen entry exists for the destination */
    init_mapseen(&dest);

    /* find that mapseen entry */
    for (mptr = mapseenchn; mptr; mptr = mptr->next)
        if (mptr->lev.dnum == dest.dnum && mptr->lev.dlevel == dest.dlevel)
            break;
    if (!mptr) {
        fprintf(stderr, "FAILED: could not create/find mapseen for dest\n");
        return 1;
    }

    /* create a fake tame pet and add it to the destination mapseen */
    mtmp = newmonst();
    *mtmp = zeromonst;
    set_mon_data(mtmp, &mons[PM_LITTLE_DOG]);
    mtmp->mtame = 1;
    mtmp->isminion = 0;
    mtmp->m_lev = 4;

    /* call the new API to append the pet to the mapseen entry */
    add_pet_to_mapseen(&dest, mtmp);

    if (!mptr->petnames) {
        fprintf(stderr, "FAILED: petnames not set by add_pet_to_mapseen\n");
        return 2;
    }

    if (!strstr(mptr->petnames, "(HP 4)")) {
        fprintf(stderr, "FAILED: petnames='%s' (missing HP)\n", mptr->petnames);
        return 3;
    }

    printf("OK: mapseen for %d:%d petnames='%s'\n", dest.dnum, dest.dlevel, mptr->petnames);

    /* verify sanitize_petnames fixes legacy text */
    {
        char buf[PETNAMES_MAX+1];
        strcpy(buf, "Fido (Lv 4), Spot (Lv 1)");
        sanitize_petnames(buf);
        if (!strstr(buf, "(HP 4)") || !strstr(buf, "(HP 1)")) {
            fprintf(stderr, "FAILED: sanitize_petnames('%s') -> '%s'\n",
                    "Fido (Lv 4), Spot (Lv 1)", buf);
            return 4;
        }
    }
    return 0;
}
