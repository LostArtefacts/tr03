#include "game.h"

#define NO_AI_PATH -1

extern sint32 closest_ai_path;

void DrawWillBossShield(ITEM_INFO *item);

void S_DrawWillBoss(ITEM_INFO *item)
{
    DrawAnimatingItem(item);
    if (bossdata.explode_count)
        DrawExplosionRings();
    if (bossdata.explode_count && bossdata.explode_count <= 64)
        DrawWillBossShield(item);
}

void InitialiseWillBoss(sint16 item_number)
{
    closest_ai_path = NO_AI_PATH;
    items[item_number].item_flags[1] = 0;
    bossdata.dead = bossdata.dropped_icon = bossdata.explode_count = 0;
}
