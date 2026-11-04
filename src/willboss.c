#include "game.h"

#define NO_AI_PATH -1

typedef struct {
    sint16 attack_count;
    sint16 death_count;
    uint8 attack_flag;
    uint8 attack_type;
    uint8 attack_head_count;
    uint8 ring_count;
    sint16 explode_count;
    sint16 LizmanItem;
    sint16 LizmanRoom;
    sint16 hp_counter;
    sint16 dropped_icon;
    uint8 charged;
    uint8 dead;
    PHD_VECTOR BeamTarget;
} BOSS_STRUCT;

extern BOSS_STRUCT bossdata;
extern sint32 closest_ai_path;

void DrawExplosionRings(void);
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
