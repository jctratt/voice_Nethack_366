#include <stdio.h>
#include <stdlib.h>
#include "../include/hack.h"

int main(void) {
    FILE *f = fopen("dat/dungeon", "rb");
    if (!f) { perror("dat/dungeon"); return 1; }
    struct version_info v;
    if (fread(&v, sizeof v, 1, f) != 1) { perror("read"); return 1; }
    printf("struct sizes: incarnation=0x%lx feature_set=0x%lx entity_count=0x%lx struct_sizes1=0x%lx struct_sizes2=0x%lx\n",
           v.incarnation, v.feature_set, v.entity_count, v.struct_sizes1, v.struct_sizes2);
    unsigned char *p = (unsigned char *)&v;
    printf("bytes: ");
    for (size_t i=0;i<sizeof v;i++) printf("%02x ", p[i]);
    printf("\n");
    printf("sizeof(struct version_info)=%zu\n", sizeof v);
    return 0;
}
