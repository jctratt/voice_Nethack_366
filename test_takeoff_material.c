/* Simple tests for take-off material filters
 * - verifies is_worn_metal / is_worn_nonmetal callbacks
 */

#include "hack.h"
#include <stdio.h>

int
main(void)
{
    struct obj *o;

    /* metal armor -> metal filter */
    o = mksobj(PLATE_MAIL, TRUE, FALSE);
    o->owornmask = W_ARMOR;
    if (!is_worn_metal(o)) {
        printf("FAIL: PLATE_MAIL should be detected as metal\n");
        return 1;
    }

    /* robe -> non-metal */
    o = mksobj(ROBE, TRUE, FALSE);
    o->owornmask = W_ARMOR;
    if (!is_worn_nonmetal(o)) {
        printf("FAIL: ROBE should be detected as non-metal\n");
        return 1;
    }

    /* wooden staff wielded -> non-metal */
    o = mksobj(QUARTERSTAFF, TRUE, FALSE);
    o->owornmask = W_WEP;
    if (!is_worn_nonmetal(o)) {
        printf("FAIL: QUARTERSTAFF should be detected as non-metal\n");
        return 1;
    }

    printf("OK\n");
    return 0;
}
