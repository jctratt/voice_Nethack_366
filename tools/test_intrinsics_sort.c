#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/hack.h"

int main(void) {
    int count = 0;
    const struct propname *pn = propertynames;
    while (pn[count].prop_name) ++count;
    printf("original (count=%d):\n", count);
    for (int i=0;i<count;i++) printf("%s\n", propertynames[i].prop_name);

    int *order = malloc(count * sizeof(int));
    int idx = 0;
    for (int i=0; propertynames[i].prop_name; ++i) {
        if (propertynames[i].prop_num == HALLUC_RES) continue;
        order[idx++] = i;
    }
    int n = idx;
    for (int a = 1; a < n; ++a) {
        int key = order[a];
        int b = a - 1;
        while (b >= 0 && strcmp(propertynames[order[b]].prop_name, propertynames[key].prop_name) > 0) {
            order[b + 1] = order[b];
            --b;
        }
        order[b + 1] = key;
    }
    printf("\nsorted:\n");
    for (int i=0;i<n;i++) printf("%s\n", propertynames[order[i]].prop_name);
    return 0;
}
