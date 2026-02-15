/* test_overview_pets.c - basic checks for mapseen petnames field */
#include "hack.h"
#include <stdio.h>

extern mapseen *mapseenchn; /* defined in src/dungeon.c */

int
main(void)
{
    mapseen *mptr;

    /* Ensure a mapseen for current level exists */
    init_mapseen(&u.uz);

    if (!mapseenchn) {
        fprintf(stderr, "FAILED: mapseenchn not initialized\n");
        return 1;
    }

    mptr = mapseenchn; /* newly created entry for current level */

    /* set petnames manually and verify length handling */
    if (mptr->petnames) {
        free((genericptr_t) mptr->petnames);
        mptr->petnames = 0;
        mptr->petnames_lth = 0;
    }

    mptr->petnames = dupstr("Fido, Spot");
    mptr->petnames_lth = strlen(mptr->petnames);

    if (mptr->petnames_lth != 10) {
        fprintf(stderr, "FAILED: unexpected petnames_lth=%u\n", mptr->petnames_lth);
        return 2;
    }

    printf("OK: petnames='%s' len=%u\n", mptr->petnames, mptr->petnames_lth);
    return 0;
}
