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

sint16 CreateEffect(sint16 room_num);

void TriggerPlasmaBall_willboss(PHD_VECTOR *pos, sint16 room_number, sint16 angle, sint16 type)
{
    FX_INFO *fx;
    sint16 fx_number;

    fx_number = CreateEffect(room_number);
    if (fx_number != NO_ITEM) {
        fx = &effects[fx_number];
        fx->pos.x_pos = pos->x;
        fx->pos.y_pos = pos->y;
        fx->pos.z_pos = pos->z;
        fx->pos.x_rot = 0;
        fx->pos.y_rot = angle;
        fx->object_number = 369;
        fx->speed = 16 + type ? (GetRandomControl() & 31) + 16 : 0;
        fx->fallspeed = -(type << 4);
        fx->flag1 = type;
    }
}
