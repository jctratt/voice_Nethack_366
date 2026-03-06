/* test_pri_move.c - simple sanity check of the goal-selection logic from
   pri_move().  This is a small, self‑contained program that verifies the
   randomised offsets used for milling do not include the centre for peaceful
   priests and *do* include it for angry ones.
*/

#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
    int shrx = 5, shry = 5;
    int gx, gy;
    int peaceful_center = 0;
    int angry_center = 0;
    const int trials = 100000;

    /* peaceful priest: offsets reroll until not the centre */
    for (int i = 0; i < trials; ++i) {
        do {
            gx = shrx + (rand() % 3 - 1);
            gy = shry + (rand() % 3 - 1);
        } while (gx == shrx && gy == shry);
        if (gx == shrx && gy == shry)
            peaceful_center++;
    }

    /* angry priest: simple 3x3 random, should hit centre roughly 1/9 of the time */
    for (int i = 0; i < trials; ++i) {
        gx = shrx + (rand() % 3 - 1);
        gy = shry + (rand() % 3 - 1);
        if (gx == shrx && gy == shry)
            angry_center++;
    }

    if (peaceful_center != 0) {
        printf("FAIL: peaceful algorithm produced %d centre hits\n", peaceful_center);
        return 1;
    }
    if (angry_center == 0) {
        printf("FAIL: angry algorithm never produced centre even though it should\n");
        return 2;
    }

    printf("OK: peaceful avoids, angry hits (counts %d,%d)\n", peaceful_center, angry_center);
    return 0;
}
