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

int TestBoundsCollide(ITEM_INFO *item, ITEM_INFO *laraitem, sint32 radius);
int TestCollision(ITEM_INFO *item, ITEM_INFO *laraitem);

int GetInMineCart(ITEM_INFO *v, ITEM_INFO *l, COLL_INFO *coll)
{
    int dist;
    sint32 x, z;
    FLOOR_INFO *floor;
    sint16 room_number;

    if (!(input & IN_ACTION) || lara.gun_status != LG_ARMLESS || l->gravity_status)
        return 0;
    if (!TestBoundsCollide(v, l, coll->radius))
        return 0;
    if (!TestCollision(v, l))
        return 0;
    x = l->pos.x_pos - v->pos.x_pos;
    z = l->pos.z_pos - v->pos.z_pos;
    dist = x * x + z * z;
    if (dist > 200000)
        return 0;
    room_number = v->room_number;
    floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);
    if (GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos) < -32000)
        return 0;
    return 1;
}
