/* test_cursdial_keys.c - unit test for escape-sequence mapping in cursdial.c */
#include "hack.h"
#include <stdio.h>

/* function exposed for testing in cursdial.c */
extern int curses_translate_escape_sequence_for_test(const char *seq, int len);

int
main(void)
{
    if (curses_translate_escape_sequence_for_test("OP", 2) != KEY_F(1)) {
        fprintf(stderr, "FAILED: ESC O P -> expected KEY_F(1)\n");
        return 1;
    }

    if (curses_translate_escape_sequence_for_test("OQ", 2) != KEY_F(2)) {
        fprintf(stderr, "FAILED: ESC O Q -> expected KEY_F(2)\n");
        return 2;
    }

    /* numeric CSI F-key encodings (ESC[11~/ESC[12~) should map to F1/F2 */
    if (curses_translate_escape_sequence_for_test("[11~", 4) != KEY_F(1)) {
        fprintf(stderr, "FAILED: CSI [11~ -> expected KEY_F(1)\n");
        return 4;
    }

    if (curses_translate_escape_sequence_for_test("[12~", 4) != KEY_F(2)) {
        fprintf(stderr, "FAILED: CSI [12~ -> expected KEY_F(2)\n");
        return 5;
    }

    /* sanity: unknown/partial sequences should return 0 */
    if (curses_translate_escape_sequence_for_test("[1;5C", 5) != 0) {
        fprintf(stderr, "FAILED: CSI modifier sequence should not map to KEY_F() here\n");
        return 3;
    }

    printf("OK: cursdial escape-sequence mapping\n");
    return 0;
}
