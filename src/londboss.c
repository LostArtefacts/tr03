#include "game.h"

void DrawLondonBossShield(ITEM_INFO *item);
void DrawSummonRings(void);
void DrawKnockBackRings(void);
void UpdateElectricityPoints(void);
void LaraElectricDeath(sint32 copy, ITEM_INFO *item);

typedef struct {
    sint16 x;
    sint16 y;
    sint16 z;
    uint8 rsub;
    uint8 gsub;
    uint8 bsub;
    uint8 pad[3];
    sint32 rgb;
} SHIELD_POINTS;

extern sint16 rcossin_tbl[];
extern sint32 LondonBossRadii[5];
extern sint32 LondonBossHeights[5];
extern SHIELD_POINTS LondonBossShield[40];

void S_DrawLondonBoss(ITEM_INFO *item)
{
    DrawAnimatingItem(item);
    if (bossdata.explode_count)
    {
        DrawLondonBossShield(item);
        DrawExplosionRings();
    }
    else
    {
        DrawSummonRings();
        DrawKnockBackRings();
    }
    if (item->hit_points <= 0 && bossdata.explode_count == 0)
    {
        UpdateElectricityPoints();
        LaraElectricDeath(0, item);
        LaraElectricDeath(1, item);
    }
}

void InitialiseLondonBoss(sint16 item_number)
{
    SHIELD_POINTS *shptr;
    sint32 lp, lp1, angle, y;

    bossdata.explode_count = bossdata.ring_count = bossdata.dead = bossdata.dropped_icon = 0;
    shptr = &LondonBossShield[0];
    for (lp = 0; lp < 5; lp++) {
        y = LondonBossHeights[lp];
        angle = 0;
        for (lp1 = 0; lp1 < 8; lp1++) {
            shptr->x = (rcossin_tbl[angle << 1] * LondonBossRadii[lp]) >> 11;
            shptr->y = y;
            shptr->z = (rcossin_tbl[(angle << 1) + 1] * LondonBossRadii[lp]) >> 11;
            shptr->rgb = 0;
            angle += 512;
            shptr++;
        }
    }
}
