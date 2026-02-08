#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(void) {
    FILE *f = fopen("dat/dungeon", "rb");
    if (!f) { perror("dat/dungeon"); return 1; }
    unsigned char buf[40];
    if (fread(buf, 1, sizeof buf, f) < 24) { perror("read"); return 1; }

    printf("bytes: ");
    for (int i = 0; i < 32; i++) printf("%02x ", buf[i]);
    printf("\n");

    uint64_t w0 = *(uint64_t *)&buf[0];
    uint64_t w1 = *(uint64_t *)&buf[8];
    uint64_t w2 = *(uint64_t *)&buf[16];
    uint64_t w3 = *(uint64_t *)&buf[24];

    printf("word0: 0x%016llx\n", (unsigned long long)w0);
    printf("word1: 0x%016llx\n", (unsigned long long)w1);
    printf("word2: 0x%016llx\n", (unsigned long long)w2);
    printf("word3: 0x%016llx\n", (unsigned long long)w3);
    return 0;
}
