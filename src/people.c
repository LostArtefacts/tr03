#include "game.h"

void Richochet(GAME_VECTOR *pos);

typedef struct {
    sint16 zone_number;
    sint16 enemy_zone;
    sint32 distance;
    sint32 ahead;
    sint32 bite;
    sint16 angle;
    sint16 x_angle;
    sint16 enemy_facing;
} AI_INFO;

sint16 *GetBestFrame(ITEM_INFO *item);
int LOS(GAME_VECTOR *start, GAME_VECTOR *target);

int Targetable(ITEM_INFO *item, AI_INFO *info)
{
    GAME_VECTOR start, target;
    CREATURE_INFO *creature;
    ITEM_INFO *enemy;
    sint16 *bounds;

    creature = (CREATURE_INFO *)item->data;
    if ((enemy = creature->enemy) == NULL)
        return 0;
    if (enemy->hit_points <= 0 || enemy->data == NULL)
        return 0;
    if (info->ahead && info->distance < 0x4000000) {
        start.x = item->pos.x_pos;
        bounds = GetBestFrame(item);
        start.y = item->pos.y_pos + (sint32)(((bounds[2] << 1) + bounds[2] + bounds[3]) >> 2);
        start.z = item->pos.z_pos;
        start.room_number = item->room_number;
        target.x = enemy->pos.x_pos;
        bounds = GetBestFrame(enemy);
        target.y = enemy->pos.y_pos + (sint32)(((bounds[2] << 1) + bounds[2] + bounds[3]) >> 2);
        target.z = enemy->pos.z_pos;
        return LOS(&start, &target);
    }
    return 0;
}

sint16 GunShot(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number)
{
    return NO_ITEM;
}

sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);

sint16 GunHit(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number)
{
    PHD_VECTOR pos;

    pos.x = pos.y = pos.z = 0;
    GetJointAbsPosition(lara_item, &pos, GetRandomControl() * 25 / 0x7fff);
    DoBloodSplat(pos.x, pos.y, pos.z, lara_item->speed, lara_item->pos.y_rot, lara_item->room_number);
    SoundEffect(50, &lara_item->pos, 0);
    return GunShot(x, y, z, speed, yrot, room_number);
}

sint16 GunMiss(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number)
{
    GAME_VECTOR pos;

    pos.x = lara_item->pos.x_pos + (GetRandomControl() - 0x4000) * (WALL_L / 2) / 0x7fff;
    pos.y = lara_item->floor;
    pos.z = lara_item->pos.z_pos + (GetRandomControl() - 0x4000) * (WALL_L / 2) / 0x7fff;
    pos.room_number = lara_item->room_number;
    Richochet(&pos);
    return GunShot(x, y, z, speed, yrot, room_number);
}
