#include "game.h"

void DrawLondonBossShield(ITEM_INFO *item);
void DrawSummonRings(void);
void DrawKnockBackRings(void);
void UpdateElectricityPoints(void);
void LaraElectricDeath(sint32 copy, ITEM_INFO *item);

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
