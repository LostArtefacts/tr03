#include "game.h"

void ClearMovableBlockSplitters(sint32 x, sint32 y, sint32 z, sint16 room_number);
void AlterFloorHeight(ITEM_INFO *item, int height);

extern sint32 phd_left, phd_right, phd_top, phd_bottom;
extern sint16 phd_winxmax, phd_winymax;

void InitialiseMovingBlock(sint16 item_num)
{
    ITEM_INFO *item;

    item = &items[item_num];
    ClearMovableBlockSplitters(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
    if (item->status != INVISIBLE)
        AlterFloorHeight(item, -1024);
}

void floor_shake_effect(ITEM_INFO *item);
void AdjustStopperFlag(ITEM_INFO *item, sint32 dir, sint32 set);

void ClearMovableBlockSplitters(sint32 x, sint32 y, sint32 z, sint16 room_number)
{
    sint16 height, base_room_number;
    FLOOR_INFO *floor;

    floor = GetFloor(x, y, z, &room_number);
    boxes[floor->box].overlap_index &= ~BLOCKED;
    height = boxes[floor->box].height;
    base_room_number = room_number;
    floor = GetFloor(x + WALL_L, y, z, &room_number);
    if (floor->box != NO_BOX) {
        if (boxes[floor->box].height == height && (boxes[floor->box].overlap_index & 0x8000) && (boxes[floor->box].overlap_index & BLOCKED))
            ClearMovableBlockSplitters(x + WALL_L, y, z, room_number);
    }
    room_number = base_room_number;
    floor = GetFloor(x - WALL_L, y, z, &room_number);
    if (floor->box != NO_BOX) {
        if (boxes[floor->box].height == height && (boxes[floor->box].overlap_index & 0x8000) && (boxes[floor->box].overlap_index & BLOCKED))
            ClearMovableBlockSplitters(x - WALL_L, y, z, room_number);
    }
    room_number = base_room_number;
    floor = GetFloor(x, y, z + WALL_L, &room_number);
    if (floor->box != NO_BOX) {
        if (boxes[floor->box].height == height && (boxes[floor->box].overlap_index & 0x8000) && (boxes[floor->box].overlap_index & BLOCKED))
            ClearMovableBlockSplitters(x, y, z + WALL_L, room_number);
    }
    room_number = base_room_number;
    floor = GetFloor(x, y, z - WALL_L, &room_number);
    if (floor->box != NO_BOX) {
        if (boxes[floor->box].height == height && (boxes[floor->box].overlap_index & 0x8000) && (boxes[floor->box].overlap_index & BLOCKED))
            ClearMovableBlockSplitters(x, y, z - WALL_L, room_number);
    }
}

void MovableBlock(sint16 item_number)
{
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    int height;
    sint16 room_number;

    item = &items[item_number];
    if (item->flags & ONESHOT) {
        AlterFloorHeight(item, 1024);
        KillItem(item_number);
        return;
    }
    AnimateItem(item);
    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    height = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    if (item->pos.y_pos < height)
        item->gravity_status = 1;
    else if (item->gravity_status) {
        item->gravity_status = 0;
        item->pos.y_pos = height;
        item->status = DEACTIVATED;
        floor_shake_effect(item);
        SoundEffect(70, &item->pos, 0);
    }
    if (item->room_number != room_number)
        ItemNewRoom(item_number, room_number);
    if (item->status == DEACTIVATED) {
        item->status = NOT_ACTIVE;
        RemoveActiveItem(item_number);
        AlterFloorHeight(item, -1024);
        AdjustStopperFlag(item, item->item_flags[0] + 0x8000, 0);
        room_number = item->room_number;
        floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
        GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
        TestTriggers(trigger_index, 1);
    }
}

int TestBlockMovable(ITEM_INFO *item, int blokhite)
{
    sint16 room_number;
    FLOOR_INFO *floor;

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (floor->floor == NO_HEIGHT / 256)
        return 1;
    if (floor->floor * 256 != item->pos.y_pos - blokhite)
        return 0;
    return 1;
}

extern int height_type;

int CollideStaticObjects(COLL_INFO *coll, sint32 x, sint32 y, sint32 z, sint16 room_number, sint32 hite);

int TestBlockPush(ITEM_INFO *item, int blokhite, uint16 quadrant)
{
    COLL_INFO scoll;
    FLOOR_INFO *floor;
    ROOM_INFO *r;
    sint32 x, y, z, cmax;
    sint16 room_num;

    if (!TestBlockMovable(item, blokhite))
        return 0;
    x = item->pos.x_pos;
    y = item->pos.y_pos;
    z = item->pos.z_pos;
    room_num = item->room_number;
    switch (quadrant) {
    case 0:
        z += WALL_L;
        break;
    case 1:
        x += WALL_L;
        break;
    case 2:
        z -= WALL_L;
        break;
    case 3:
        x -= WALL_L;
        break;
    }
    floor = GetFloor(x, y, z, &room_num);
    r = &room[room_num];
    if (r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size].stopper)
        return 0;
    scoll.quadrant = quadrant;
    scoll.radius = 500;
    if (CollideStaticObjects(&scoll, x, y, z, room_num, 1000))
        return 0;
    if (((sint32)floor->floor << 8) != y)
        return 0;
    GetHeight(floor, x, y, z);
    if (height_type != 0)
        return 0;
    cmax = y - (blokhite - 100);
    floor = GetFloor(x, cmax, z, &room_num);
    if (GetCeiling(floor, x, cmax, z) > cmax)
        return 0;
    item->item_flags[0] = lara_item->pos.y_rot;
    return 1;
}

int TestBlockPull(ITEM_INFO *item, int blokhite, uint16 quadrant)
{
    COLL_INFO scoll;
    FLOOR_INFO *floor;
    ROOM_INFO *r;
    sint32 x, y, z, zadd, xadd, cmax;
    sint16 room_num;

    if (!TestBlockMovable(item, blokhite))
        return 0;
    xadd = zadd = 0;
    switch (quadrant) {
    case 0:
        zadd = -WALL_L;
        break;
    case 1:
        xadd = -WALL_L;
        break;
    case 2:
        zadd = WALL_L;
        break;
    case 3:
        xadd = WALL_L;
        break;
    }
    x = item->pos.x_pos + xadd;
    y = item->pos.y_pos;
    z = item->pos.z_pos + zadd;
    room_num = item->room_number;
    floor = GetFloor(x, y, z, &room_num);
    r = &room[room_num];
    if (r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size].stopper)
        return 0;
    scoll.quadrant = quadrant;
    scoll.radius = 500;
    if (CollideStaticObjects(&scoll, x, y, z, room_num, 1000))
        return 0;
    if (((sint32)floor->floor << 8) != y)
        return 0;
    cmax = y - blokhite;
    floor = GetFloor(x, cmax, z, &room_num);
    if (((sint32)floor->ceiling << 8) > cmax)
        return 0;
    x += xadd;
    z += zadd;
    room_num = item->room_number;
    floor = GetFloor(x, y, z, &room_num);
    if (((sint32)floor->floor << 8) != y)
        return 0;
    cmax = y - LARA_HITE;
    floor = GetFloor(x, cmax, z, &room_num);
    if (((sint32)floor->ceiling << 8) > cmax)
        return 0;
    x = lara_item->pos.x_pos + xadd;
    y = lara_item->pos.y_pos;
    z = lara_item->pos.z_pos + zadd;
    room_num = lara_item->room_number;
    floor = GetFloor(x, y, z, &room_num);
    scoll.quadrant = (quadrant + 2) & 3;
    scoll.radius = LARA_RAD;
    if (CollideStaticObjects(&scoll, x, y, z, room_num, LARA_HITE))
        return 0;
    item->item_flags[0] = lara_item->pos.y_rot + 0x8000;
    return 1;
}

void DrawMovableBlock(ITEM_INFO *item)
{
    if (item->status != ACTIVE)
        DrawAnimatingItem(item);
    else
        DrawUnclippedItem(item);
}

void DrawUnclippedItem(ITEM_INFO *item)
{
    sint32 left, right, top, bottom;

    left = phd_left;
    right = phd_right;
    top = phd_top;
    bottom = phd_bottom;
    phd_left = phd_top = 0;
    phd_bottom = phd_winymax;
    phd_right = phd_winxmax;
    DrawAnimatingItem(item);
    phd_left = left;
    phd_right = right;
    phd_top = top;
    phd_bottom = bottom;
}
