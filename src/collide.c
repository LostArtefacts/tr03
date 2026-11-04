#include "game.h"

extern sint16 draw_rooms[];
extern int number_draw_rooms;

void GetNewRoom(sint32 x, sint32 y, sint32 z, sint16 room_number);
int CollideStaticObjects(COLL_INFO *coll, sint32 x, sint32 y, sint32 z, sint16 room_number, sint32 hite);
void UpdateLaraRoom(ITEM_INFO *item, int height);
int TestBoundsCollide(ITEM_INFO *item, ITEM_INFO *laraitem, sint32 radius);
int TestCollision(ITEM_INFO *item, ITEM_INFO *laraitem);
void ItemPushLara(ITEM_INFO *item, ITEM_INFO *laraitem, COLL_INFO *coll, int spaz, int big_push);
sint16 GetTiltType(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);
void LaraBaddieCollision(ITEM_INFO *laraitem, COLL_INFO *coll);
void CreatureCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);

sint32 FindGridShift(sint32 src, sint32 dst)
{
    sint32 srcw, dstw;

    srcw = src >> WALL_SHIFT;
    dstw = dst >> WALL_SHIFT;
    if (srcw == dstw)
        return 0;

    src &= WALL_L - 1;
    if (dstw > srcw)
        return WALL_L - (src - 1);
    else
        return -(src + 1);
}

void GetNearByRooms(sint32 x, sint32 y, sint32 z, sint32 r, sint32 h, sint16 room_number)
{
    draw_rooms[0] = room_number;
    number_draw_rooms = 1;
    GetNewRoom(x + r, y, z + r, room_number);
    GetNewRoom(x - r, y, z + r, room_number);
    GetNewRoom(x + r, y, z - r, room_number);
    GetNewRoom(x - r, y, z - r, room_number);
    GetNewRoom(x + r, y - h, z + r, room_number);
    GetNewRoom(x - r, y - h, z + r, room_number);
    GetNewRoom(x + r, y - h, z - r, room_number);
    GetNewRoom(x - r, y - h, z - r, room_number);
}

void GetNewRoom(sint32 x, sint32 y, sint32 z, sint16 room_number)
{
    int i;

    GetFloor(x, y, z, &room_number);
    for (i = 0; i < number_draw_rooms; i++)
    {
        if (draw_rooms[i] == room_number)
            break;
    }
    if (i == number_draw_rooms)
        draw_rooms[number_draw_rooms++] = room_number;
}

void ShiftItem(ITEM_INFO *item, COLL_INFO *coll)
{
    item->pos.x_pos += coll->shift.x;
    item->pos.y_pos += coll->shift.y;
    item->pos.z_pos += coll->shift.z;
    coll->shift.x = coll->shift.y = coll->shift.z = 0;
}
