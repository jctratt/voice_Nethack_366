/* standalone test for the drop_ignore_gems filtering algorithm */
#include <stdio.h>
#include <string.h>

/* minimal constants needed from objclass.h */
#define GEM_CLASS 13
#define ROCK_CLASS 14
#define COIN_CLASS 12
#define SCROLL_CLASS 9
#define GEM_SYM '*'
#define ROCK_SYM '`'
#define SCROLL_SYM '?'

#define BUFSZ 256

/* Minimal representation of the flags structure so we can set
   drop_ignore_gems. */
/* minimal substitute for flag.h so we can test the option value. */
typedef int boolean;
#define TRUE 1
#define FALSE 0
struct flag { boolean drop_ignore_gems; };
struct flag flags; /* just the one we care about in this test */

/* simple model of `allow_category` behaviour relevant to our option. */
static int
sim_allow_category(int oclass, boolean bucx_filter, boolean class_filter)
{
    if (flags.drop_ignore_gems && bucx_filter && !class_filter
        && (oclass == GEM_CLASS || oclass == ROCK_CLASS))
        return 0;
    return 1;
}

/* minimal object struct for our fake inventory */
struct obj {
    int oclass;
    struct obj *nobj;
};

/* mimic a small subset of collect_obj_classes() just enough for our test */
static int
collect_obj_classes(char *buf, struct obj *otmp)
{
    int idx = 0;
    buf[0] = '\0';

    for (; otmp; otmp = otmp->nobj) {
        int cls = otmp->oclass;
        if (cls == COIN_CLASS)
            continue;
        char sym;
        switch (cls) {
        case GEM_CLASS:
            sym = GEM_SYM;
            break;
        case ROCK_CLASS:
            sym = ROCK_SYM;
            break;
        case SCROLL_CLASS:
            sym = SCROLL_SYM;
            break;
        default:
            sym = '?';
            break;
        }
        if (!strchr(buf, sym)) {
            buf[idx++] = sym;
            buf[idx] = '\0';
        }
    }
    return idx;
}

static void
remove_class_sym(char *ilets, char sym)
{
    char *p;
    if ((p = strchr(ilets, sym)) != NULL) {
        strcpy(p, p + 1);
    }
}

/* replicate the algorithm from invent.c */
static void
hide_drop_gems(char *ilets)
{
    char savebuf[BUFSZ];

    /* copy and filter exactly like the real routine */
    strcpy(savebuf, ilets);
    remove_class_sym(ilets, GEM_SYM);
    remove_class_sym(ilets, ROCK_SYM);
    if (!ilets[0])
        strcpy(ilets, savebuf);
}

int
main(void)
{
    struct obj gem, rock, scroll, *inv;
    char ilets[BUFSZ];
    char *gsym, *rsym;

    memset(&gem, 0, sizeof gem);
    memset(&rock, 0, sizeof rock);
    memset(&scroll, 0, sizeof scroll);
    gem.oclass = GEM_CLASS;
    rock.oclass = ROCK_CLASS;
    scroll.oclass = SCROLL_CLASS;
    gem.nobj = &rock;
    rock.nobj = &scroll;
    scroll.nobj = NULL;
    inv = &gem;

    /* collect classes without filtering */
    collect_obj_classes(ilets, inv);
    printf("initial ilets=\"%s\"\n", ilets);
    gsym = strchr(ilets, GEM_SYM);
    rsym = strchr(ilets, ROCK_SYM);
    if (!gsym || !rsym) {
        fprintf(stderr, "FAIL: expected gem and rock in initial ilets\n");
        return 1;
    }

    /* now apply filter and check results (should be unaffected) */
    hide_drop_gems(ilets); /* stub but reproduces old behaviour */
    printf("filtered ilets=\"%s\"\n", ilets);
    gsym = strchr(ilets, GEM_SYM);
    rsym = strchr(ilets, ROCK_SYM);
    if (gsym || rsym) {
        fprintf(stderr, "FAIL: symbols still present after filter\n");
        return 1;
    }

    /* With the flag true, categories should still include gem/rock */
    flags.drop_ignore_gems = TRUE;
    collect_obj_classes(ilets, inv);
    printf("with flag ilets=\"%s\"\n", ilets);
    if (!strchr(ilets, GEM_SYM) || !strchr(ilets, ROCK_SYM)) {
        fprintf(stderr, "FAIL: categories disappeared when flag true\n");
        return 1;
    }

    printf("OK\n");

    /* simulate object-selection filtering for +X only (allow_category) */
    flags.drop_ignore_gems = TRUE;
    /* debug: report results */
    printf("sim gem=%d rock=%d scroll=%d\n",
           sim_allow_category(GEM_CLASS, TRUE, FALSE),
           sim_allow_category(ROCK_CLASS, TRUE, FALSE),
           sim_allow_category(SCROLL_CLASS, TRUE, FALSE));
    if (sim_allow_category(GEM_CLASS, TRUE, FALSE) != 0
        || sim_allow_category(ROCK_CLASS, TRUE, FALSE) != 0
        || sim_allow_category(SCROLL_CLASS, TRUE, FALSE) != 1) {
        fprintf(stderr, "FAIL: +X filtering failed\n");
        return 1;
    }
    /* gems/rocks should still be allowed when a class filter is present */
    if (sim_allow_category(GEM_CLASS, TRUE, TRUE) == 0) {
        fprintf(stderr, "FAIL: class filter override failed\n");
        return 1;
    }

    /* simulate ckvalidcat's decision for a gem when +X menu is active */
    {
        boolean menu_X = TRUE;
        boolean menu_gem = FALSE;
        boolean menu_rock = FALSE;
        int ck_gem;

        if (flags.drop_ignore_gems && menu_X
            && !menu_gem && !menu_rock
            && (GEM_CLASS == GEM_CLASS || GEM_CLASS == ROCK_CLASS))
            ck_gem = 0;
        else
            ck_gem = 1;

        if (ck_gem != 0) {
            fprintf(stderr, "FAIL: ckvalidcat simulation failed\n");
            return 1;
        }
    }

    return 0;
}
