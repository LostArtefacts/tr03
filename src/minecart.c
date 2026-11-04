#include "game.h"

typedef struct {
    sint32 Speed;
    sint32 MidPos;
    sint32 FrontPos;
    sint32 TurnX;
    sint32 TurnZ;
    sint16 TurnLen;
    sint16 TurnRot;
    sint16 YVel;
    sint16 Gradient;
    char Flags;
    char StopDelay;
} CARTINFO;

sint16 GetCollision(ITEM_INFO *v, sint16 ang, sint32 dist, sint16 *ceiling)
{
    sint16 room_number;
    FLOOR_INFO *floor;
    sint32 x, y, z, height, cheight;

    x = v->pos.x_pos + ((phd_sin(ang) * dist) >> W2V_SHIFT);
    y = v->pos.y_pos - LARA_HITE;
    z = v->pos.z_pos + ((phd_cos(ang) * dist) >> W2V_SHIFT);
    room_number = v->room_number;
    floor = GetFloor(x, y, z, &room_number);
    height = GetHeight(floor, x, y, z);
    cheight = GetCeiling(floor, x, y, z);
    *ceiling = (sint16)cheight;
    if (height != NO_HEIGHT)
        height -= v->pos.y_pos;
    return (sint16)height;
}

void MineCartInitialise(sint16 item_number)
{
    ITEM_INFO *v;

    v = &items[item_number];
    v->data = game_malloc(sizeof(CARTINFO), 0);
}
