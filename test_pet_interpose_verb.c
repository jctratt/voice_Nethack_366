/* test_pet_interpose_verb.c - verify pet interpose wording by monster type */
#include "hack.h"
#include <stdio.h>
#include <string.h>

extern const char *pet_interpose_verb(struct monst *);

int
main(void)
{
    struct monst mtmp;

    /* dog -> growls */
    mtmp.data = &mons[PM_DOG];
    if (strcmp(pet_interpose_verb(&mtmp), "growls") != 0) {
        fprintf(stderr, "FAILED: dog -> %s\n", pet_interpose_verb(&mtmp));
        return 1;
    }

    /* housecat -> hisses */
    mtmp.data = &mons[PM_HOUSECAT];
    if (strcmp(pet_interpose_verb(&mtmp), "hisses") != 0) {
        fprintf(stderr, "FAILED: housecat -> %s\n", pet_interpose_verb(&mtmp));
        return 2;
    }

    /* raven (bird) -> screeches */
    mtmp.data = &mons[PM_RAVEN];
    if (strcmp(pet_interpose_verb(&mtmp), "screeches") != 0) {
        fprintf(stderr, "FAILED: raven -> %s\n", pet_interpose_verb(&mtmp));
        return 3;
    }

    /* giant rat (rodent) -> chitters */
    mtmp.data = &mons[PM_GIANT_RAT];
    if (strcmp(pet_interpose_verb(&mtmp), "chitters") != 0) {
        fprintf(stderr, "FAILED: giant rat -> %s\n", pet_interpose_verb(&mtmp));
        return 4;
    }

    /* giant spider -> chitters */
    mtmp.data = &mons[PM_GIANT_SPIDER];
    if (strcmp(pet_interpose_verb(&mtmp), "chitters") != 0) {
        fprintf(stderr, "FAILED: giant spider -> %s\n", pet_interpose_verb(&mtmp));
        return 5;
    }

    /* snake -> hisses */
    mtmp.data = &mons[PM_SNAKE];
    if (strcmp(pet_interpose_verb(&mtmp), "hisses") != 0) {
        fprintf(stderr, "FAILED: snake -> %s\n", pet_interpose_verb(&mtmp));
        return 6;
    }

    printf("OK: pet_interpose_verb mappings\n");
    return 0;
}
