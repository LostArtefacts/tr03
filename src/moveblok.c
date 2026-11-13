#include "specific/standard.h"
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

extern sint16 MovingBlockBounds[12];

int TestLaraPosition(sint16 *bounds, ITEM_INFO *item, ITEM_INFO *laraitem);
void AnimateLara(ITEM_INFO *item);
int TestBlockPush(ITEM_INFO *item, int blokhite, uint16 quadrant);
int TestBlockPull(ITEM_INFO *item, int blokhite, uint16 quadrant);

void MovableBlockCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    uint16 quadrant;
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    sint16 room_number;

    item = &items[item_num];
    if (!(input & IN_ACTION) || item->status == ACTIVE || laraitem->gravity_status || laraitem->pos.y_pos != item->pos.y_pos)
        return;

    quadrant = (uint16)(laraitem->pos.y_rot + 0x2000) / 0x4000;
    if (laraitem->current_anim_state == AS_STOP) {
        if (lara.gun_status != LG_ARMLESS)
            return;

        switch (quadrant) {
        case 0:
            item->pos.y_rot = 0;
            break;
        case 1:
            item->pos.y_rot = 16384;
            break;
        case 2:
            item->pos.y_rot = -32768;
            break;
        case 3:
            item->pos.y_rot = -16384;
            break;
        }
        if (!TestLaraPosition(MovingBlockBounds, item, laraitem))
            return;

        room_number = laraitem->room_number;
        floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
        if (room_number != item->room_number)
            return;

        switch (quadrant) {
        case 0:
            laraitem->pos.z_pos &= -WALL_L;
            laraitem->pos.z_pos += WALL_L - LARA_RAD;
            break;
        case 2:
            laraitem->pos.z_pos &= -WALL_L;
            laraitem->pos.z_pos += LARA_RAD;
            break;
        case 1:
            laraitem->pos.x_pos &= -WALL_L;
            laraitem->pos.x_pos += WALL_L - LARA_RAD;
            break;
        case 3:
            laraitem->pos.x_pos &= -WALL_L;
            laraitem->pos.x_pos += LARA_RAD;
            break;
        }
        laraitem->pos.y_rot = item->pos.y_rot;
        laraitem->goal_anim_state = AS_PPREADY;
        AnimateLara(laraitem);
        if (laraitem->current_anim_state == AS_PPREADY)
            lara.gun_status = LG_HANDSBUSY;
    } else if (laraitem->current_anim_state == AS_PPREADY) {
        if (laraitem->frame_number != anims[120].frame_base + 19)
            return;
        if (!TestLaraPosition(MovingBlockBounds, item, laraitem))
            return;
        if (input & IN_FORWARD) {
            if (!TestBlockPush(item, 1024, quadrant))
                return;
            item->goal_anim_state = 2;
            laraitem->goal_anim_state = AS_PUSHBLOCK;
        } else if (input & IN_BACK) {
            if (!TestBlockPull(item, 1024, quadrant))
                return;
            item->goal_anim_state = 3;
            laraitem->goal_anim_state = AS_PULLBLOCK;
        } else {
            return;
        }

        AddActiveItem(item_num);
        AlterFloorHeight(item, 1024);
        AdjustStopperFlag(item, item->item_flags[0], 1);
        item->status = ACTIVE;
        AnimateItem(item);
        AnimateLara(laraitem);
        lara.head_x_rot = lara.head_y_rot = 0;
        lara.torso_x_rot = lara.torso_y_rot = 0;
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

void AlterFloorHeight(ITEM_INFO *item, int height)
{
    FLOOR_INFO *floor, *ceiling;
    sint16 room_num;

    room_num = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_num);
    ceiling = GetFloor(item->pos.x_pos, item->pos.y_pos + height - WALL_L, item->pos.z_pos, &room_num);
    if (floor->floor == NO_HEIGHT / 256) {
        floor->floor = ceiling->ceiling + height / 256;
    } else {
        floor->floor = height / 256 + floor->floor;
        if (floor->floor == ceiling->ceiling)
            floor->floor = NO_HEIGHT / 256;
    }
    if (boxes[floor->box].overlap_index & 0x8000) {
        if (height < 0)
            boxes[floor->box].overlap_index |= BLOCKED;
        else
            boxes[floor->box].overlap_index &= ~BLOCKED;
    }
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

void SetupCleanerFromSavegame(ITEM_INFO *item, sint32 block)
{
    ROOM_INFO *r;
    sint32 x, z, xadd, zadd;
    sint16 room_number;

    x = item->pos.x_pos;
    z = item->pos.z_pos;

    if (!block && (item->pos.z_pos & 0x3ff) == 512 && (item->pos.x_pos & 0x3ff) == 512) {
        room_number = item->room_number;
        GetFloor(x, item->pos.y_pos, z, &room_number);
        r = &room[room_number];
        r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size].stopper = 1;
        return;
    } else if (item->pos.y_rot == 0) {
        if (!block) {
            if ((z & 1023) < 512) {
                z &= ~1023;
                z -= 512;
            } else {
                z &= ~1023;
                z += 512;
            }
        }
        xadd = 0;
        zadd = 1024;
    } else if (item->pos.y_rot == 16384) {
        if (!block) {
            if ((x & 1023) < 512) {
                x &= ~1023;
                x -= 512;
            } else {
                x &= ~1023;
                x += 512;
            }
        }
        xadd = 1024;
        zadd = 0;
    } else if (item->pos.y_rot == -32768) {
        if (!block) {
            if ((z & 1023) > 512) {
                z |= 1023;
                z += 512;
            } else {
                z &= ~1023;
                z += 512;
            }
        }
        xadd = 0;
        zadd = -1024;
    } else {
        if (!block) {
            if ((x & 1023) > 512) {
                x |= 1023;
                x += 512;
            } else {
                x &= ~1023;
                x += 512;
            }
        }
        xadd = -1024;
        zadd = 0;
    }

    room_number = item->room_number;
    GetFloor(x, item->pos.y_pos, z, &room_number);
    r = &room[room_number];
    r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size].stopper = 1;

    x += xadd;
    z += zadd;
    room_number = item->room_number;
    GetFloor(x, item->pos.y_pos, z, &room_number);
    r = &room[room_number];
    r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size].stopper = 1;
}

#ifdef TR3_EXE
sint16 MovingBlockBounds[12] = {
    -300, 300, 0, 0, -692, -512, -1820, 1820, -5460, 5460, -1820, 1820,
};
#endif
