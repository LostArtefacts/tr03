#include "game.h"
#include <string.h>

#define DOOR_CLOSED 0
#define DOOR_OPEN 1
#define FINAL_COUNTDOWN 150

typedef struct {
    sint32 start_height;
    sint32 wait_time;
} LIFT_INFO;

enum height_types { WALL, SMALL_SLOPE, BIG_SLOPE, DIAGONAL, SPLIT_TRI };

extern int height_type;
extern int OnObject;
extern sint16 final_boss_active, final_boss_count, final_level_count;
extern sint32 level_items;

int OnDrawBridge(ITEM_INFO *item, sint32 z, sint32 x);
void LiftFloorCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *floor, sint32 *ceiling);

void ControlCutShotgun(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (final_boss_active >= FINAL_COUNTDOWN)
    {
        item->status = ACTIVE;
        AnimateItem(item);
    }
    else
        item->status = INVISIBLE;
}

void InitialiseFinalLevel(void)
{
    int i;
    ITEM_INFO *item;

    final_level_count = final_boss_count = final_boss_active = 0;
    for (i = 0, item = items; i < level_items; i++, item++)
    {
        switch (item->object_number)
        {
        case DOG:
            final_level_count++;
            break;
        }
    }
}

void InitialiseWindow(sint16 item_number)
{
    ITEM_INFO *item;
    ROOM_INFO *r;
    FLOOR_INFO *floor;

    item = &items[item_number];
    item->flags = 0;
    item->mesh_bits = 1;
    r = &room[item->room_number];
    floor = &r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size];
    if (boxes[floor->box].overlap_index & 0x8000)
        boxes[floor->box].overlap_index |= BLOCKED;
}

int ItemNearLara(PHD_3DPOS *pos, sint32 distance);
void SmashWindow(sint16 item_number);

void WindowControl(sint16 item_number)
{
    ITEM_INFO *item;
    sint32 speed;

    item = &items[item_number];
    if (item->flags & ONESHOT)
        return;
    if (lara.skidoo == NO_ITEM) {
        if (!item->touch_bits)
            return;
        item->touch_bits = 0;
        speed = lara_item->speed * phd_cos((sint16)(lara_item->pos.y_rot - item->pos.y_rot)) >> W2V_SHIFT;
        if (ABS(speed) < 50)
            return;
    } else if (!ItemNearLara(&item->pos, WALL_L / 2)) {
        return;
    }
    SmashWindow(item_number);
}

void ShutThatDoor(DOORPOS_DATA *d)
{
    FLOOR_INFO *floor;

    if (d->floor)
    {
        floor = d->floor;
        floor->index = 0;
        floor->box = NO_BOX;
        floor->floor = floor->ceiling = NO_HEIGHT >> 8;
        floor->pit_room = floor->sky_room = NO_ROOM;
        if (d->block != NO_BOX)
            boxes[d->block].overlap_index |= BLOCKED;
    }
}

void OpenThatDoor(DOORPOS_DATA *d)
{
    if (d->floor)
    {
        memcpy(d->floor, &d->data, sizeof(FLOOR_INFO));
        if (d->block != NO_BOX)
            boxes[d->block].overlap_index &= ~BLOCKED;
    }
}

typedef struct {
    DOORPOS_DATA d1;
    DOORPOS_DATA d1flip;
    DOORPOS_DATA d2;
    DOORPOS_DATA d2flip;
} DOOR_DATA;

void DoorControl(sint16 item_number)
{
    ITEM_INFO *item;
    DOOR_DATA *door;

    item = &items[item_number];
    door = (DOOR_DATA *)item->data;
    if (TriggerActive(item)) {
        if (item->current_anim_state == DOOR_CLOSED) {
            item->goal_anim_state = DOOR_OPEN;
        } else {
            OpenThatDoor(&door->d1);
            OpenThatDoor(&door->d2);
            OpenThatDoor(&door->d1flip);
            OpenThatDoor(&door->d2flip);
        }
    } else {
        if (item->current_anim_state == DOOR_OPEN) {
            item->goal_anim_state = DOOR_CLOSED;
        } else {
            ShutThatDoor(&door->d1);
            ShutThatDoor(&door->d2);
            ShutThatDoor(&door->d1flip);
            ShutThatDoor(&door->d2flip);
        }
    }
    AnimateItem(item);
}

int OnDrawBridge(ITEM_INFO *item, sint32 x, sint32 y)
{
    int ix, iy;

    ix = item->pos.z_pos >> WALL_SHIFT;
    iy = item->pos.x_pos >> WALL_SHIFT;
    x >>= WALL_SHIFT;
    y >>= WALL_SHIFT;
    if (item->pos.y_rot == 0 && y == iy && (x == ix - 1 || x == ix - 2))
        return 1;
    else if (item->pos.y_rot == -0x8000 && y == iy && (x == ix + 1 || x == ix + 2))
        return 1;
    else if (item->pos.y_rot == 0x4000 && x == ix && (y == iy - 1 || y == iy - 2))
        return 1;
    else if (item->pos.y_rot == -0x4000 && x == ix && (y == iy + 1 || y == iy + 2))
        return 1;
    return 0;
}

void DrawBridgeFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    if (item->current_anim_state == DOOR_OPEN)
    {
        if (!OnDrawBridge(item, z, x))
            return;
        if (y <= item->pos.y_pos)
        {
            OnObject = 1;
            *height = item->pos.y_pos;
            if (item == lara_item)
                lara_item->item_flags[0] = 1;
        }
    }
}

void DrawBridgeCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    if (item->current_anim_state == DOOR_OPEN)
    {
        if (!OnDrawBridge(item, z, x))
            return;
        if (y > item->pos.y_pos)
            *height = item->pos.y_pos + STEP_L;
    }
}

void DrawBridgeCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (item->current_anim_state == DOOR_CLOSED)
        DoorCollision(item_num, laraitem, coll);
}

void InitialiseLift(sint16 item_number)
{
    LIFT_INFO *lift;

    items[item_number].data = lift = (LIFT_INFO *)game_malloc(sizeof(LIFT_INFO), 0);
    lift->start_height = items[item_number].pos.y_pos;
    lift->wait_time = 0;
}

void LiftControl(sint16 item_number)
{
    ITEM_INFO *item;
    LIFT_INFO *lift;
    sint16 room_number;

    item = &items[item_number];
    lift = (LIFT_INFO *)item->data;
    if (TriggerActive(item)) {
        if (item->pos.y_pos < lift->start_height + STEP_L * 22 - 16) {
            if (lift->wait_time < 90) {
                item->goal_anim_state = DOOR_OPEN;
                lift->wait_time++;
            } else {
                item->goal_anim_state = DOOR_CLOSED;
                item->pos.y_pos += 16;
            }
        } else {
            item->goal_anim_state = DOOR_OPEN;
            lift->wait_time = 0;
        }
    } else {
        if (item->pos.y_pos > lift->start_height + 16) {
            if (lift->wait_time < 90) {
                item->goal_anim_state = DOOR_OPEN;
                lift->wait_time++;
            } else {
                item->goal_anim_state = DOOR_CLOSED;
                item->pos.y_pos -= 16;
            }
        } else {
            item->goal_anim_state = DOOR_OPEN;
            lift->wait_time = 0;
        }
    }
    AnimateItem(item);
    room_number = item->room_number;
    GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (item->room_number != room_number)
        ItemNewRoom(item_number, room_number);
}

void LiftFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    sint32 floor, ceiling;

    LiftFloorCeiling(item, x, y, z, &floor, &ceiling);
    if (floor < *height)
    {
        OnObject = 1;
        *height = floor;
    }
}

void LiftCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    sint32 floor, ceiling;

    LiftFloorCeiling(item, x, y, z, &floor, &ceiling);
    if (ceiling > *height)
        *height = ceiling;
}

void BridgeFlatFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    if (y <= item->pos.y_pos)
    {
        *height = item->pos.y_pos;
        height_type = WALL;
        OnObject = 1;
        if (item == lara_item)
            lara_item->item_flags[0] = 1;
    }
}

void BridgeFlatCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    if (y > item->pos.y_pos)
        *height = item->pos.y_pos + STEP_L;
}

sint32 GetOffset(ITEM_INFO *item, sint32 x, sint32 z)
{
    if (item->pos.y_rot == 0)
        return (WALL_L - x) & (WALL_L - 1);
    else if (item->pos.y_rot == -0x8000)
        return x & (WALL_L - 1);
    else if (item->pos.y_rot == 0x4000)
        return z & (WALL_L - 1);
    return (WALL_L - z) & (WALL_L - 1);
}

void BridgeTilt1Floor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    sint32 level;

    level = item->pos.y_pos + (GetOffset(item, x, z) >> 2);
    if (y <= level)
    {
        *height = level;
        height_type = WALL;
        OnObject = 1;
        if (item == lara_item)
            lara_item->item_flags[0] = 1;
    }
}

void BridgeTilt1Ceiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    sint32 level;

    level = item->pos.y_pos + (GetOffset(item, x, z) >> 2);
    if (y > level)
        *height = level + STEP_L;
}

void BridgeTilt2Floor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    sint32 level;

    level = item->pos.y_pos + (GetOffset(item, x, z) >> 1);
    if (y <= level)
    {
        *height = level;
        height_type = WALL;
        OnObject = 1;
        if (item == lara_item)
            lara_item->item_flags[0] = 1;
    }
}

void BridgeTilt2Ceiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    sint32 level;

    level = item->pos.y_pos + (GetOffset(item, x, z) >> 1);
    if (y > level)
        *height = level + STEP_L;
}

void GeneralControl(sint16 item_number)
{
    ITEM_INFO *item;
    sint16 room_number;

    item = &items[item_number];
    if (TriggerActive(item))
        item->goal_anim_state = DOOR_OPEN;
    else
        item->goal_anim_state = DOOR_CLOSED;
    AnimateItem(item);
    room_number = item->room_number;
    GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (room_number != item->room_number)
        ItemNewRoom(item_number, room_number);
    if (item->status == DEACTIVATED) {
        RemoveActiveItem(item_number);
        item->flags |= ONESHOT;
    }
}

void DetonatorControl(sint16 item_number)
{
    int frame_number;
    ITEM_INFO *item;

    item = &items[item_number];
    AnimateItem(item);
    frame_number = item->frame_number - anims[item->anim_number].frame_base;
    if (frame_number == 80)
    {
        camera.bounce = -150;
        SoundEffect(105, NULL, 0);
    }
    if (item->status == DEACTIVATED)
        RemoveActiveItem(item_number);
}

void ControlAnimating_1_4(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (TriggerActive(item))
        AnimateItem(item);
}
