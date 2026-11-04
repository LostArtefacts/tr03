#include "game.h"

void DrawTonyBossShield(ITEM_INFO *item);

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
extern sint32 TonyBossRadii[5];
extern sint32 TonyBossHeights[5];
extern SHIELD_POINTS TonyBossShield[40];

void S_DrawTonyBoss(ITEM_INFO *item)
{
    DrawAnimatingItem(item);
    if (bossdata.explode_count && item->hit_points <= 0)
        DrawExplosionRings();
    if (bossdata.explode_count && bossdata.explode_count <= 64)
        DrawTonyBossShield(item);
}

void TonyBossDie(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->collidable = 0;
    item->hit_points = DONT_TARGET;
    KillItem(item_number);
    DisableBaddieAI(item_number);
    item->flags |= ONESHOT;
}

void InitialiseTonyBoss(sint16 item_number)
{
    SHIELD_POINTS *shptr;
    sint32 lp, lp1, angle, y;

    items[item_number].item_flags[3] = bossdata.explode_count = bossdata.ring_count = bossdata.dropped_icon = bossdata.dead = 0;
    shptr = &TonyBossShield[0];
    for (lp = 0; lp < 5; lp++) {
        y = TonyBossHeights[lp];
        angle = 0;
        for (lp1 = 0; lp1 < 8; lp1++) {
            shptr->x = (rcossin_tbl[angle << 1] * TonyBossRadii[lp]) >> 11;
            shptr->y = y;
            shptr->z = (rcossin_tbl[(angle << 1) + 1] * TonyBossRadii[lp]) >> 11;
            shptr->rgb = 0;
            angle += 512;
            shptr++;
        }
    }
}
