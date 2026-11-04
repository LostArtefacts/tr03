#include "game.h"

void DrawTonyBossShield(ITEM_INFO *item);

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
