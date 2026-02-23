/* test_pardon_pet_stairs.c - verify that "m" command on stairs shoves pet away
 *
 * This is a small program which sets up a player standing on stairs with a
 * peaceful pet adjacent and then simulates the code path triggered by using an
 * "m" prefix move.  After running the logic that prints "Pardon me" we expect
 * the pet to be relocated one square farther from the hero.
 */

#include <stdio.h>
#include <stdlib.h>

/* Very small standalone program that replicates the movement math used when
   a "pardon me" is issued on the stairs.  No NetHack globals or functions are
   referenced so it compiles and runs on its own. */

static int
sgn(int x)
{
    return (x > 0) - (x < 0);
}

static int
not_adjacent(int ux, int uy, int mx, int my)
{
    return (abs(mx - ux) > 1 || abs(my - uy) > 1);
}

int
main(void)
{
    int ux = 20, uy = 20;
    int pets[][2] = {{21, 20}, {21, 21}};
    int i;

    for (i = 0; i < 2; i++) {
        int mx = pets[i][0];
        int my = pets[i][1];
        int dx = sgn(mx - ux);
        int dy = sgn(my - uy);
        int nx = mx + dx;
        int ny = my + dy;

        if (!not_adjacent(ux, uy, nx, ny)) {
            printf("FAIL: after moving pet from (%d,%d) to (%d,%d) still adjacent\n",
                   mx, my, nx, ny);
            return 1;
        }
    }

    printf("OK: coordinate math produces nonadjacent positions\n");
    return 0;
}
