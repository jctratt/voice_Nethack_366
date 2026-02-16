/* test_overview_pet_on_other_level.c
 * Verify that a level which contains pets (petnames) is considered
 * "interesting" for #overview even when the player is on a different
 * dungeon level.
 */
#include "hack.h"
#include <stdio.h>

int
main(void)
{
    mapseen *mptr, *found = 0;
    d_level lvl2, lvl3;

    /* set up two levels in same dungeon (use u.uz.dnum) */
    lvl2.dnum = lvl3.dnum = u.uz.dnum;
    lvl2.dlevel = 2;
    lvl3.dlevel = 3;

    init_mapseen(&lvl2);
    init_mapseen(&lvl3);

    /* find the mapseen entry for lvl2 */
    for (mptr = mapseenchn; mptr; mptr = mptr->next) {
        if (mptr->lev.dnum == lvl2.dnum && mptr->lev.dlevel == lvl2.dlevel) {
            found = mptr;
            break;
        }
    }
    if (!found) {
        fprintf(stderr, "FAILED: could not find mapseen for level 2\n");
        return 1;
    }

    /* ensure player is on a different level (lvl3) */
    assign_level(&u.uz, &lvl3);

    /* annotate lvl2 with a pet summary */
    if (found->petnames)
        free((genericptr_t) found->petnames);
    found->petnames = dupstr("Fido (Lv 4)");
    found->petnames_lth = strlen(found->petnames);

    /* now the level with petnames must be considered interesting */
    if (!mapseen_is_interesting(found)) {
        fprintf(stderr, "FAILED: mapseen_is_interesting() returned FALSE for level with petnames\n");
        return 2;
    }

    printf("OK: level %d considered interesting when pet present\n", (int) found->lev.dlevel);
    return 0;
}
