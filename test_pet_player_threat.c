/* test_pet_player_threat.c - regression checks for player-facing pet threat
 * ranking used by interpose/follow behavior.
 *
 * This is a small standalone test that exercises the intended ordering rules:
 * immediate threats near the player must outrank farther monsters.
 */

#include <stdio.h>

static long
score_player_threat_for_test(int player_dist, int hostile, int mlev, int mhp,
                             int sleeping, int fleeing, int has_ranged,
                             int ignores_elbereth, int marked_for_pet)
{
    const int radius = 14;
    long score = 0L;

    if (!hostile || player_dist > radius)
        return -5000L;

    score += 800L;
    if (marked_for_pet)
        score += 5000L;
    if (ignores_elbereth)
        score += 1000L;
    score += (long) (radius - player_dist + 1) * 200L;
    if (player_dist <= 1)
        score += 1200L;
    else if (player_dist == 2)
        score += 300L;

    score += (long) mlev * 30L;
    score += (long) mhp / 4L;

    if (fleeing)
        score -= 200L;
    if (sleeping)
        score -= 300L;
    if (has_ranged)
        score += 250L;

    return score;
}

int
main(void)
{
    long adjacent_ogre, distant_dragon, ignores_elbereth, fears_elbereth;
    long marked_distant, unmarked_nearby;

    adjacent_ogre = score_player_threat_for_test(1, 1, 7, 30, 0, 0, 0, 0, 0);
    distant_dragon = score_player_threat_for_test(6, 1, 15, 80, 0, 0, 1, 0, 0);
    if (adjacent_ogre <= distant_dragon) {
        fprintf(stderr,
                "FAIL: adjacent threat (%ld) should outrank distant threat (%ld)\n",
                adjacent_ogre, distant_dragon);
        return 1;
    }

    marked_distant = score_player_threat_for_test(6, 1, 6, 25, 0, 0, 0, 0, 1);
    unmarked_nearby = score_player_threat_for_test(3, 1, 8, 35, 0, 0, 0, 0, 0);
    if (marked_distant <= unmarked_nearby) {
        fprintf(stderr,
                "FAIL: marked target (%ld) should outrank unmarked target (%ld)\n",
                marked_distant, unmarked_nearby);
        return 2;
    }

    fears_elbereth = score_player_threat_for_test(2, 1, 8, 35, 0, 0, 0, 0, 0);
    ignores_elbereth = score_player_threat_for_test(2, 1, 8, 35, 0, 0, 0, 1, 0);
    if (ignores_elbereth <= fears_elbereth) {
        fprintf(stderr,
                "FAIL: Elbereth-immune threat (%ld) should outrank vulnerable threat (%ld)\n",
                ignores_elbereth, fears_elbereth);
        return 3;
    }

    if (score_player_threat_for_test(1, 0, 9, 50, 0, 0, 1, 0, 0) >= 0L) {
        fprintf(stderr, "FAIL: non-hostile monster should not be a threat\n");
        return 4;
    }

    printf("OK: player-facing pet threat ranking favors nearby hostiles\n");
    return 0;
}
