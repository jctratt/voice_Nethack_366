/* Simple compile-time test harness for quiver-order scaffolding */
#include "hack.h"
#include <stdio.h>

int
main(void)
{
    struct obj *o = select_quiver_candidate();
    if (!o) {
        printf("select_quiver_candidate: OK (NULL as expected for stub)\n");
        return 0;
    }
    printf("select_quiver_candidate: returned object o_id=%u\n", o->o_id);
    return 0;
}
