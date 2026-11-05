#include "specific/standard.h"
#include "game.h"

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

int abs(int x);
#pragma intrinsic(abs)

void AnimateLara(ITEM_INFO *item);

void ControlDeathSlide(sint16 item_number)
{
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    sint32 c, s, x, y, z;
    sint16 room_number;
    int lara_on_deathslide;
    GAME_VECTOR *old;

    item = &items[item_number];
    if (item->status == ACTIVE) {
        if (!(item->flags & ONESHOT)) {
            old = (GAME_VECTOR *)item->data;
            item->pos.x_pos = old->x;
            item->pos.y_pos = old->y;
            item->pos.z_pos = old->z;
            if (old->room_number != item->room_number)
                ItemNewRoom(item_number, old->room_number);
            item->status = NOT_ACTIVE;
            item->current_anim_state = item->goal_anim_state = 1;
            item->anim_number = objects[item->object_number].anim_index;
            item->frame_number = anims[item->anim_number].frame_base;
            RemoveActiveItem(item_number);
            return;
        }

        if (item->current_anim_state == 1) {
            AnimateItem(item);
            return;
        }

        AnimateItem(item);

        if (item->fallspeed < 100)
            item->fallspeed += 5;

        c = phd_cos(item->pos.y_rot);
        s = phd_sin(item->pos.y_rot);

        item->pos.z_pos += item->fallspeed * c >> W2V_SHIFT;
        item->pos.x_pos += item->fallspeed * s >> W2V_SHIFT;
        item->pos.y_pos += item->fallspeed >> 2;
        room_number = item->room_number;
        GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
        if (room_number != item->room_number)
            ItemNewRoom(item_number, room_number);

        lara_on_deathslide = (lara_item->current_anim_state == AS_DEATHSLIDE);
        if (lara_on_deathslide) {
            lara_item->pos.x_pos = item->pos.x_pos;
            lara_item->pos.y_pos = item->pos.y_pos;
            lara_item->pos.z_pos = item->pos.z_pos;
        }

        z = item->pos.z_pos + (WALL_L * c >> W2V_SHIFT);
        x = item->pos.x_pos + (WALL_L * s >> W2V_SHIFT);
        y = item->pos.y_pos + (STEP_L >> 2);
        floor = GetFloor(x, y, z, &room_number);
        if (GetHeight(floor, x, y, z) <= y + STEP_L || GetCeiling(floor, x, y, z) >= y - STEP_L) {
            if (lara_on_deathslide) {
                lara_item->goal_anim_state = AS_FORWARDJUMP;
                AnimateLara(lara_item);
                lara_item->gravity_status = 1;
                lara_item->speed = item->fallspeed;
                lara_item->fallspeed = item->fallspeed >> 2;
            }

            SoundEffect(281, &item->pos, 0);
            RemoveActiveItem(item_number);
            item->status = NOT_ACTIVE;
            item->flags -= ONESHOT;
        } else {
            SoundEffect(280, &item->pos, 0);
        }
    }
}

void EarthQuake(sint16 item_number)
{
    sint16 earth_item;
    int want, pitch;
    ITEM_INFO *item;

    item = &items[item_number];
    if (!TriggerActive(item))
        return;
    if (item->item_flags[1] == 0)
        item->item_flags[1] = 100;
    if (item->item_flags[2] == 0 && abs(item->item_flags[0] - item->item_flags[1]) < 16) {
        if (item->item_flags[1] == 20) {
            item->item_flags[1] = 100;
            item->item_flags[2] = (GetRandomControl() & 127) + 90;
        } else {
            item->item_flags[1] = 20;
            item->item_flags[2] = (GetRandomControl() & 127) + 30;
        }
    }
    if (item->item_flags[2])
        item->item_flags[2]--;
    if (item->item_flags[0] > item->item_flags[1])
        item->item_flags[0] -= (GetRandomControl() & 7) + 2;
    else
        item->item_flags[0] += (GetRandomControl() & 7) + 2;
    pitch = (0x10000 + (item->item_flags[0] << 8)) << 8;
    SoundEffect(107, NULL, 4 + pitch);
    camera.bounce = -item->item_flags[0];
    want = GetRandomControl();
    if (want < 0x400) {
        want = (want < 0x200) ? 330 : 107;
        for (earth_item = room[item->room_number].item_number; earth_item != NO_ITEM; earth_item = items[earth_item].next_item) {
            item = &items[earth_item];
            if (item->object_number != want)
                continue;
            if (item->status == ACTIVE || item->status == DEACTIVATED)
                continue;
            AddActiveItem(earth_item);
            item->status = ACTIVE;
            item->flags = 0x3e00;
            item->timer = 0;
            return;
        }
    }
}

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

extern sint16 final_boss[5];
extern sint16 cine_frame;
extern PHD_3DPOS cinematic_pos;

int LOS(GAME_VECTOR *start, GAME_VECTOR *target);
void CreatureKill(ITEM_INFO *item, int kill_anim, int kill_state, int lara_kill_state);

void FinalLevelCounter(sint16 item_number)
{
    sint16 best;
    ITEM_INFO *item;
    GAME_VECTOR start, target;
    int i, best_distance, distance, x, y, z;

    if (savegame.kills == final_level_count && !final_boss_active) {
        best_distance = 0x7fffffff;
        best = final_boss[0];
        for (i = 0; i < final_boss_count; i++) {
            item = &items[final_boss[i]];

            start.x = lara_item->pos.x_pos;
            start.y = lara_item->pos.y_pos - 512;
            start.z = lara_item->pos.z_pos;
            start.room_number = lara_item->room_number;

            target.x = item->pos.x_pos;
            target.y = item->pos.y_pos - 512;
            target.z = item->pos.z_pos;
            if (!LOS(&start, &target)) {
                x = (lara_item->pos.x_pos - item->pos.x_pos) >> 6;
                y = (lara_item->pos.y_pos - item->pos.y_pos) >> 6;
                z = (lara_item->pos.z_pos - item->pos.z_pos) >> 6;
                distance = x * x + y * y + z * z;
                if (distance < best_distance) {
                    best_distance = distance;
                    best = final_boss[i];
                }
            }
        }

        item = &items[best];
        item->touch_bits = 0;
        item->status = ACTIVE;
        AddActiveItem(best);
        EnableBaddieAI(best, 1);
        item->mesh_bits = 0xffff1fff;

        final_boss_active = 1;
    } else if (savegame.kills > final_level_count) {
        final_boss_active++;
        if (final_boss_active == FINAL_COUNTDOWN) {
            cine_frame = 428;
            item = &items[item_number];
            CreatureKill(item, 0, 0, 10);
            lara.mesh_ptrs[HAND_R] = meshes[objects[LARA].mesh_index + HAND_R];

            camera.type = CINEMATIC_CAMERA;
            memcpy(&cinematic_pos, &item->pos, sizeof(PHD_3DPOS));
        }
    }
}

void MiniCopterControl(sint16 item_number)
{
    ITEM_INFO *item;
    sint16 room_number;
    PHD_3DPOS pos;

    item = &items[item_number];
    item->pos.z_pos += 100;
    pos.x_pos = lara_item->pos.x_pos + ((item->pos.x_pos - lara_item->pos.x_pos) >> 2);
    pos.y_pos = lara_item->pos.y_pos + ((item->pos.y_pos - lara_item->pos.y_pos) >> 2);
    pos.z_pos = lara_item->pos.z_pos + ((item->pos.z_pos - lara_item->pos.z_pos) >> 2);
    SoundEffect(297, &pos, 0);
    if (ABS(item->pos.z_pos - lara_item->pos.z_pos) > WALL_L * 30)
        KillItem(item_number);
    AnimateItem(item);
    room_number = item->room_number;
    GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (room_number != item->room_number)
        ItemNewRoom(item_number, room_number);
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

int ExplodingDeath(sint16 item_number, sint32 mesh_bits, sint16 damage);

void SmashWindow(sint16 item_number)
{
    ROOM_INFO *r;
    FLOOR_INFO *floor;
    ITEM_INFO *item;

    item = &items[item_number];
    r = &room[item->room_number];
    floor = &r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size];
    if (boxes[floor->box].overlap_index & 0x8000)
        boxes[floor->box].overlap_index &= ~BLOCKED;

    if (item->object_number == SMASH_WINDOW) {
        SoundEffect(214, &item->pos, 0);
    } else {
        SoundEffect(105, &item->pos, 0);
        SoundEffect(106, &item->pos, 0);
    }

    item->mesh_bits = 0x0fffe;
    item->collidable = 0;
    ExplodingDeath(item_number, 0x0fefe, 0);
    item->flags |= ONESHOT;
    if (item->status == ACTIVE)
        RemoveActiveItem(item_number);
    item->status = DEACTIVATED;
}

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

sint16 GetDoor(FLOOR_INFO *floor);

void InitialiseDoor(sint16 item_number)
{
    int dx, dy;
    ITEM_INFO *item;
    ROOM_INFO *r, *b;
    DOOR_DATA *door;
    sint16 two_room, box_number, room_number;

    item = &items[item_number];
    door = item->data = (DOOR_DATA *)game_malloc(sizeof(DOOR_DATA), 31);
    dx = dy = 0;
    if (item->pos.y_rot == 0)
        dx--;
    else if (item->pos.y_rot == -0x8000)
        dx++;
    else if (item->pos.y_rot == 0x4000)
        dy--;
    else
        dy++;

    r = &room[item->room_number];
    door->d1.floor = &r->floor[(((item->pos.z_pos - r->z) >> WALL_SHIFT) + dx) + (((item->pos.x_pos - r->x) >> WALL_SHIFT) + dy) * r->x_size];
    room_number = GetDoor(door->d1.floor);
    if (room_number == NO_ROOM)
        box_number = door->d1.floor->box;
    else {
        b = &room[room_number];
        box_number = b->floor[(((item->pos.z_pos - b->z) >> WALL_SHIFT) + dx) + (((item->pos.x_pos - b->x) >> WALL_SHIFT) + dy) * b->x_size].box;
    }
    door->d1.block = (boxes[box_number].overlap_index & 0x8000) ? box_number : NO_BOX;
    memcpy(&door->d1.data, door->d1.floor, sizeof(FLOOR_INFO));

    if (r->flipped_room != -1) {
        r = &room[r->flipped_room];
        door->d1flip.floor = &r->floor[(((item->pos.z_pos - r->z) >> WALL_SHIFT) + dx) + (((item->pos.x_pos - r->x) >> WALL_SHIFT) + dy) * r->x_size];
        room_number = GetDoor(door->d1flip.floor);
        if (room_number == NO_ROOM)
            box_number = door->d1flip.floor->box;
        else {
            b = &room[room_number];
            box_number = b->floor[(((item->pos.z_pos - b->z) >> WALL_SHIFT) + dx) + (((item->pos.x_pos - b->x) >> WALL_SHIFT) + dy) * b->x_size].box;
        }
        door->d1flip.block = (boxes[box_number].overlap_index & 0x8000) ? box_number : NO_BOX;
        memcpy(&door->d1flip.data, door->d1flip.floor, sizeof(FLOOR_INFO));
    } else
        door->d1flip.floor = NULL;

    two_room = GetDoor(door->d1.floor);
    ShutThatDoor(&door->d1);
    ShutThatDoor(&door->d1flip);

    if (two_room == NO_ROOM) {
        door->d2.floor = NULL;
        door->d2flip.floor = NULL;
    } else {
        r = &room[two_room];
        door->d2.floor = &r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size];
        room_number = GetDoor(door->d2.floor);
        if (room_number == NO_ROOM)
            box_number = door->d2.floor->box;
        else {
            b = &room[room_number];
            box_number = b->floor[((item->pos.z_pos - b->z) >> WALL_SHIFT) + ((item->pos.x_pos - b->x) >> WALL_SHIFT) * b->x_size].box;
        }
        door->d2.block = (boxes[box_number].overlap_index & 0x8000) ? box_number : NO_BOX;
        memcpy(&door->d2.data, door->d2.floor, sizeof(FLOOR_INFO));

        if (r->flipped_room != -1) {
            r = &room[r->flipped_room];
            door->d2flip.floor = &r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size];
            room_number = GetDoor(door->d2flip.floor);
            if (room_number == NO_ROOM)
                box_number = door->d2flip.floor->box;
            else {
                b = &room[room_number];
                box_number = b->floor[((item->pos.z_pos - b->z) >> WALL_SHIFT) + ((item->pos.x_pos - b->x) >> WALL_SHIFT) * b->x_size].box;
            }
            door->d2flip.block = (boxes[box_number].overlap_index & 0x8000) ? box_number : NO_BOX;
            memcpy(&door->d2flip.data, door->d2flip.floor, sizeof(FLOOR_INFO));
        } else
            door->d2flip.floor = NULL;

        ShutThatDoor(&door->d2);
        ShutThatDoor(&door->d2flip);
        room_number = item->room_number;
        ItemNewRoom(item_number, two_room);
        item->room_number = room_number;
    }
}

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

void LiftFloorCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *floor, sint32 *ceiling)
{
    int ix, iz, lx, lz, point_in_shaft;
    int lift_floor, lift_ceiling;

    ix = item->pos.x_pos >> WALL_SHIFT;
    iz = item->pos.z_pos >> WALL_SHIFT;
    lx = lara_item->pos.x_pos >> WALL_SHIFT;
    lz = lara_item->pos.z_pos >> WALL_SHIFT;
    x >>= WALL_SHIFT;
    z >>= WALL_SHIFT;

    point_in_shaft = ((x == ix || x + 1 == ix) && (z == iz || z - 1 == iz));
    lift_floor = item->pos.y_pos;
    lift_ceiling = item->pos.y_pos - STEP_L * 5;
    *floor = 0x7fff;
    *ceiling = -0x7fff;

    if ((lx == ix || lx + 1 == ix) && (lz == iz || lz - 1 == iz)) {
        if (item->current_anim_state == DOOR_CLOSED && lara_item->pos.y_pos < lift_floor + STEP_L
            && lara_item->pos.y_pos > lift_ceiling + STEP_L) {
            if (point_in_shaft) {
                *floor = lift_floor;
                *ceiling = lift_ceiling + STEP_L;
            } else {
                *floor = NO_HEIGHT;
                *ceiling = 0x7fff;
            }
        } else if (point_in_shaft) {
            if (lara_item->pos.y_pos < lift_ceiling + STEP_L) {
                *floor = lift_ceiling;
            } else if (lara_item->pos.y_pos < lift_floor + STEP_L) {
                *floor = lift_floor;
                *ceiling = lift_ceiling + STEP_L;
            } else {
                *ceiling = lift_floor + STEP_L;
            }
        }
    } else if (point_in_shaft) {
        if (y <= lift_ceiling) {
            *floor = lift_ceiling;
        } else if (y < lift_floor + STEP_L) {
            if (item->current_anim_state == DOOR_CLOSED) {
                *floor = NO_HEIGHT;
                *ceiling = 0x7fff;
            } else {
                *floor = lift_floor;
                *ceiling = lift_ceiling + STEP_L;
            }
        } else {
            *ceiling = lift_floor + STEP_L;
        }
    }
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
