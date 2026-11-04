#include "game.h"

void Richochet(GAME_VECTOR *pos);

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
