#include "specific/standard.h"
#include <io.h>
#include <direct.h>
#include "game.h"

#define MAX_CD_TRACKS 128

void MovableBlock(sint16);
void AlterFloorHeight(ITEM_INFO *item, int height);
#define KILLED_ITEM (-0x8000)

typedef struct {
    sint16 goal_anim_state;
    sint16 number_ranges;
    sint16 range_index;
} CHANGE_STRUCT;

typedef struct {
    sint16 start_frame;
    sint16 end_frame;
    sint16 link_anim_num;
    sint16 link_frame_num;
} RANGE_STRUCT;

extern CHANGE_STRUCT *changes;
extern RANGE_STRUCT *ranges;

void RemoveDrawnItem(sint16);
void TriggerNormalCDTrack(sint16 value, sint16 flags, sint16 type);

int GetChange(ITEM_INFO *item, ANIM_STRUCT *anim)
{
    int i, j;
    CHANGE_STRUCT *change;
    RANGE_STRUCT *range;

    if (item->current_anim_state == item->goal_anim_state)
        return 0;
    change = &changes[anim->change_index];
    for (i = 0; i < anim->number_changes; i++, change++)
    {
        if (change->goal_anim_state == item->goal_anim_state)
        {
            range = &ranges[change->range_index];
            for (j = 0; j < change->number_ranges; j++, range++)
            {
                if (item->frame_number >= range->start_frame && item->frame_number <= range->end_frame)
                {
                    item->anim_number = range->link_anim_num;
                    item->frame_number = range->link_frame_num;
                    return 1;
                }
            }
        }
    }
    return 0;
}

void RefreshCamera(sint16 type, sint16 *data)
{
    sint16 trigger, value, target_ok;

    target_ok = 2;
    do {
        trigger = *(data++);
        value = trigger & 0x3ff;
        switch ((trigger & 0x3fff) >> 10) {
        case 1:
            data++;
            if (value == camera.last) {
                camera.number = value;
                if (camera.timer < 0 || camera.type == LOOK_CAMERA || camera.type == COMBAT_CAMERA) {
                    camera.timer = -1;
                    target_ok = 0;
                    break;
                }
                camera.type = FIXED_CAMERA;
                target_ok = 1;
            } else {
                target_ok = 0;
            }
            break;
        case 6:
            if (camera.type == LOOK_CAMERA || camera.type == COMBAT_CAMERA)
                break;
            camera.item = &items[value];
            break;
        }
    } while (!(trigger & 0x8000));

    if (camera.item)
        if (!target_ok || (target_ok == 2 && camera.item->looked_at && camera.item != camera.last_item))
            camera.item = NULL;

    if (camera.number == -1 && camera.timer > 0)
        camera.timer = -1;
}

int TriggerActive(ITEM_INFO *item)
{
    int ok;

    ok = (item->flags & REVERSE) ? 0 : 1;
    if ((item->flags & CODE_BITS) != CODE_BITS)
        return !ok;
    if (!item->timer)
        return ok;
    if (item->timer == -1)
        return !ok;
    item->timer--;
    if (!item->timer)
        item->timer = -1;
    return ok;
}

#define DATA_TYPE 0x1f
#define END_BIT 0x8000
#define DOOR_TYPE 1
#define TILT_TYPE 2
#define ROOF_TYPE 3
#define SPLIT1 7
#define SPLIT2 8
#define SPLIT3 9
#define SPLIT4 10
#define NOCOLF1T 11
#define NOCOLF1B 12
#define NOCOLF2T 13
#define NOCOLF2B 14
#define NOCOLC1T 15
#define NOCOLC1B 16
#define NOCOLC2T 17
#define NOCOLC2B 18

extern sint16 *floor_data;

sint16 GetDoor(FLOOR_INFO *floor)
{
    sint16 *data, type;

    if (!floor->index)
        return NO_ROOM;
    data = &floor_data[floor->index];
    type = *data++;
    if ((type & DATA_TYPE) == TILT_TYPE || (type & DATA_TYPE) == SPLIT1 || (type & DATA_TYPE) == SPLIT2 || (type & DATA_TYPE) == NOCOLF1B || (type & DATA_TYPE) == NOCOLF1T || (type & DATA_TYPE) == NOCOLF2B || (type & DATA_TYPE) == NOCOLF2T) {
        if (type & END_BIT)
            return NO_ROOM;
        data++;
        type = *data++;
    }
    if ((type & DATA_TYPE) == ROOF_TYPE || (type & DATA_TYPE) == SPLIT3 || (type & DATA_TYPE) == SPLIT4 || (type & DATA_TYPE) == NOCOLC1B || (type & DATA_TYPE) == NOCOLC1T || (type & DATA_TYPE) == NOCOLC2B || (type & DATA_TYPE) == NOCOLC2T) {
        if (type & END_BIT)
            return NO_ROOM;
        data++;
        type = *data++;
    }
    if ((type & DATA_TYPE) == DOOR_TYPE)
        return *data;
    return NO_ROOM;
}

int xLOS(GAME_VECTOR *start, GAME_VECTOR *target);
int zLOS(GAME_VECTOR *start, GAME_VECTOR *target);
int ClipTarget(GAME_VECTOR *start, GAME_VECTOR *target);

int LOS(GAME_VECTOR *start, GAME_VECTOR *target)
{
    int los1, los2;

    if (ABS(target->z - start->z) > ABS(target->x - start->x)) {
        los1 = xLOS(start, target);
        los2 = zLOS(start, target);
    } else {
        los1 = zLOS(start, target);
        los2 = xLOS(start, target);
    }
    if (!los2)
        return 0;
    GetFloor(target->x, target->y, target->z, &target->room_number);
    if (!ClipTarget(start, target) || los1 != 1 || los2 != 1)
        return 0;
    return 1;
}

void RemoveRoomFlipItems(ROOM_INFO *r);
void AddRoomFlipItems(ROOM_INFO *r);

void FlipMap(void)
{
    int i;
    ROOM_INFO *r, *flipped, temp;

    for (i = 0, r = room; i < number_rooms; i++, r++) {
        if (r->flipped_room < 0)
            continue;
        RemoveRoomFlipItems(r);
        flipped = &room[r->flipped_room];
        memcpy(&temp, r, sizeof(ROOM_INFO));
        memcpy(r, flipped, sizeof(ROOM_INFO));
        memcpy(flipped, &temp, sizeof(ROOM_INFO));
        r->flipped_room = flipped->flipped_room;
        flipped->flipped_room = -1;
        r->item_number = flipped->item_number;
        r->fx_number = flipped->fx_number;
        AddRoomFlipItems(r);
    }
    flip_status = !flip_status;
}

void RemoveRoomFlipItems(ROOM_INFO *r)
{
    ITEM_INFO *item;
    sint16 item_num;

    for (item_num = r->item_number; item_num != NO_ITEM; item_num = items[item_num].next_item)
    {
        item = &items[item_num];
        if (objects[item->object_number].control == MovableBlock)
            AlterFloorHeight(item, 1024);
        else if ((item->flags & ONESHOT) && objects[item->object_number].intelligent && item->hit_points <= 0)
        {
            RemoveDrawnItem(item_num);
            item->flags |= KILLED_ITEM;
        }
    }
}

void AddRoomFlipItems(ROOM_INFO *r)
{
    ITEM_INFO *item;
    sint16 item_num;

    for (item_num = r->item_number; item_num != NO_ITEM; item_num = items[item_num].next_item)
    {
        item = &items[item_num];
        if (objects[item->object_number].control == MovableBlock)
            AlterFloorHeight(item, -1024);
    }
}

void TriggerCDTrack(sint16 value, sint16 flags, sint16 type)
{
    if (value <= 1 || value >= MAX_CD_TRACKS)
        return;
    TriggerNormalCDTrack(value, flags, type);
}

extern char cd_flags[];
extern uint8 CurrentAtmosphere;
extern uint8 IsAtmospherePlaying;
extern uint32 CurrentLevel;

int printf(const char *format, ...);
void S_CDPlay(sint16 track, int mode);

void TriggerNormalCDTrack(sint16 value, sint16 flags, sint16 type)
{
    int code;

    if ((value >= 26 && value <= 36) || (value >= 73 && value <= 78) || value == 117) {
        if (CurrentAtmosphere != value) {
            CurrentAtmosphere = (uint8)value;
            if (IsAtmospherePlaying)
                S_CDPlay(value, 1);
            printf("Got new atmosphere %d\n", value);
        }
    } else {
        if (value != 2 || CurrentLevel != 0) {
            code = (flags & (CODE_BITS | ONESHOT)) >> 8;
            if ((cd_flags[value] & code) == code)
                return;
            cd_flags[value] |= code;
        }
        S_CDPlay(value, 0);
        IsAtmospherePlaying = 0;
    }
}

sint32 CheckNoColFloorTriangle(FLOOR_INFO *floor, sint32 x, sint32 z)
{
    sint16 *data, type, dx, dz;

    if (!floor->index)
        return 0;
    data = &floor_data[floor->index];
    type = *data & DATA_TYPE;
    if (type == NOCOLF1T || type == NOCOLF1B || type == NOCOLF2T || type == NOCOLF2B) {
        dx = x & 1023;
        dz = z & 1023;
        if (type == NOCOLF1T && dx <= 1024 - dz)
            return -1;
        else if (type == NOCOLF1B && dx > 1024 - dz)
            return -1;
        else if (type == NOCOLF2T && dx <= dz)
            return -1;
        else if (type == NOCOLF2B && dx > dz)
            return -1;
        return 1;
    }
    return 0;
}

sint32 CheckNoColCeilingTriangle(FLOOR_INFO *floor, sint32 x, sint32 z)
{
    sint16 *data, type, dx, dz;

    if (!floor->index)
        return 0;
    data = &floor_data[floor->index];
    type = *data & DATA_TYPE;
    if (type == TILT_TYPE || type == SPLIT1 || type == SPLIT2 || type == NOCOLF1T || type == NOCOLF1B
        || type == NOCOLF2T || type == NOCOLF2B) {
        if (*data & END_BIT)
            return 0;
        type = *(data + 2) & DATA_TYPE;
    }
    if (type == NOCOLC1T || type == NOCOLC1B || type == NOCOLC2T || type == NOCOLC2B) {
        dx = x & 1023;
        dz = z & 1023;
        if (type == NOCOLC1T && dx <= 1024 - dz)
            return -1;
        else if (type == NOCOLC1B && dx > 1024 - dz)
            return -1;
        else if (type == NOCOLC2T && dx <= dz)
            return -1;
        else if (type == NOCOLC2B && dx > dz)
            return -1;
        return 1;
    }
    return 0;
}


sint32 GetWaterHeight(sint32 x, sint32 y, sint32 z, sint16 room_number)
{
    sint32 x_floor, y_floor;
    ROOM_INFO *r;
    FLOOR_INFO *floor;
    sint16 data;

    r = &room[room_number];
    do {
        x_floor = (z - r->z) >> WALL_SHIFT;
        y_floor = (x - r->x) >> WALL_SHIFT;
        if (x_floor <= 0) {
            x_floor = 0;
            if (y_floor < 1)
                y_floor = 1;
            else if (y_floor > r->y_size - 2)
                y_floor = r->y_size - 2;
        } else if (x_floor >= r->x_size - 1) {
            x_floor = r->x_size - 1;
            if (y_floor < 1)
                y_floor = 1;
            else if (y_floor > r->y_size - 2)
                y_floor = r->y_size - 2;
        } else if (y_floor < 0)
            y_floor = 0;
        else if (y_floor >= r->y_size)
            y_floor = r->y_size - 1;
        floor = &r->floor[x_floor + y_floor * r->x_size];
        data = GetDoor(floor);
        if (data != NO_ROOM) {
            room_number = data;
            r = &room[data];
        }
    } while (data != NO_ROOM);
    if (r->flags & (UNDERWATER | 0x80)) {
        while (floor->sky_room != NO_ROOM) {
            if (CheckNoColCeilingTriangle(floor, x, z) == 1)
                break;
            r = &room[floor->sky_room];
            if (!(r->flags & (UNDERWATER | 0x80)))
                return r->minfloor;
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        }
        return r->maxceiling;
    } else {
        while (floor->pit_room != NO_ROOM) {
            if (CheckNoColFloorTriangle(floor, x, z) == 1)
                break;
            r = &room[floor->pit_room];
            if (r->flags & (UNDERWATER | 0x80))
                return r->maxceiling;
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        }
    }
    return NO_HEIGHT;
}

FLOOR_INFO *GetFloor(sint32 x, sint32 y, sint32 z, sint16 *room_number)
{
    sint16 data;
    ROOM_INFO *r;
    FLOOR_INFO *floor;
    sint32 x_floor, y_floor, retval;

    r = &room[*room_number];
    do {
        x_floor = (z - r->z) >> WALL_SHIFT;
        y_floor = (x - r->x) >> WALL_SHIFT;
        if (x_floor <= 0) {
            x_floor = 0;
            if (y_floor < 1)
                y_floor = 1;
            else if (y_floor > r->y_size - 2)
                y_floor = r->y_size - 2;
        } else if (x_floor >= r->x_size - 1) {
            x_floor = r->x_size - 1;
            if (y_floor < 1)
                y_floor = 1;
            else if (y_floor > r->y_size - 2)
                y_floor = r->y_size - 2;
        } else if (y_floor < 0)
            y_floor = 0;
        else if (y_floor >= r->y_size)
            y_floor = r->y_size - 1;
        floor = &r->floor[x_floor + y_floor * r->x_size];
        data = GetDoor(floor);
        if (data != NO_ROOM) {
            *room_number = data;
            r = &room[data];
        }
    } while (data != NO_ROOM);
    if (y >= floor->floor << 8) {
        do {
            if (floor->pit_room == NO_ROOM)
                return floor;
            if ((retval = CheckNoColFloorTriangle(floor, x, z)) == 1)
                break;
            if (retval == -1) {
                if (y < r->minfloor)
                    break;
            }
            *room_number = floor->pit_room;
            r = &room[floor->pit_room];
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        } while (y >= floor->floor << 8);
    } else if (y < floor->ceiling << 8) {
        do {
            if (floor->sky_room == NO_ROOM)
                return floor;
            if ((retval = CheckNoColCeilingTriangle(floor, x, z)) == 1)
                break;
            if (retval == -1) {
                if (y >= r->maxceiling)
                    break;
            }
            *room_number = floor->sky_room;
            r = &room[floor->sky_room];
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        } while (y < floor->ceiling << 8);
    }
    return floor;
}

extern int los_rooms[20];
extern int number_los_rooms;

int zLOS(GAME_VECTOR *start, GAME_VECTOR *target)
{
    sint32 x, y, z;
    sint32 dx, dy, dz;
    sint16 room_number, last_room;
    FLOOR_INFO *floor;

    dz = target->z - start->z;
    if (dz == 0)
        return 1;
    dx = ((target->x - start->x) << WALL_SHIFT) / dz;
    dy = ((target->y - start->y) << WALL_SHIFT) / dz;
    los_rooms[0] = room_number = last_room = start->room_number;
    number_los_rooms = 1;
    if (dz < 0) {
        z = start->z & ~(WALL_L - 1);
        x = start->x + (dx * (z - start->z) >> WALL_SHIFT);
        y = start->y + (dy * (z - start->z) >> WALL_SHIFT);
        while (z > target->z) {
            floor = GetFloor(x, y, z, &room_number);
            if (y > GetHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z)) {
                target->x = x;
                target->y = y;
                target->z = z;
                target->room_number = room_number;
                return -1;
            }
            if (room_number != last_room) {
                los_rooms[number_los_rooms++] = room_number;
                last_room = room_number;
            }
            floor = GetFloor(x, y, z - 1, &room_number);
            if (y > GetHeight(floor, x, y, z - 1) || y < GetCeiling(floor, x, y, z - 1)) {
                target->x = x;
                target->y = y;
                target->z = z;
                target->room_number = last_room;
                return 0;
            }
            z -= WALL_L;
            x -= dx;
            y -= dy;
        }
    } else {
        z = start->z | (WALL_L - 1);
        x = start->x + (dx * (z - start->z) >> WALL_SHIFT);
        y = start->y + (dy * (z - start->z) >> WALL_SHIFT);
        while (z < target->z) {
            floor = GetFloor(x, y, z, &room_number);
            if (y > GetHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z)) {
                target->x = x;
                target->y = y;
                target->z = z;
                target->room_number = room_number;
                return -1;
            }
            if (room_number != last_room) {
                los_rooms[number_los_rooms++] = room_number;
                last_room = room_number;
            }
            floor = GetFloor(x, y, z + 1, &room_number);
            if (y > GetHeight(floor, x, y, z + 1) || y < GetCeiling(floor, x, y, z + 1)) {
                target->x = x;
                target->y = y;
                target->z = z;
                target->room_number = last_room;
                return 0;
            }
            z += WALL_L;
            x += dx;
            y += dy;
        }
    }
    target->room_number = room_number;
    return 1;
}

int xLOS(GAME_VECTOR *start, GAME_VECTOR *target)
{
    sint32 x, y, z;
    sint32 dx, dy, dz;
    sint16 room_number, last_room;
    FLOOR_INFO *floor;

    dx = target->x - start->x;
    if (dx == 0)
        return 1;
    dy = ((target->y - start->y) << WALL_SHIFT) / dx;
    dz = ((target->z - start->z) << WALL_SHIFT) / dx;
    los_rooms[0] = room_number = last_room = start->room_number;
    number_los_rooms = 1;
    if (dx < 0) {
        x = start->x & ~(WALL_L - 1);
        y = start->y + (dy * (x - start->x) >> WALL_SHIFT);
        z = start->z + (dz * (x - start->x) >> WALL_SHIFT);
        while (x > target->x) {
            floor = GetFloor(x, y, z, &room_number);
            if (y > GetHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z)) {
                target->x = x;
                target->y = y;
                target->z = z;
                target->room_number = room_number;
                return -1;
            }
            if (room_number != last_room) {
                los_rooms[number_los_rooms++] = room_number;
                last_room = room_number;
            }
            floor = GetFloor(x - 1, y, z, &room_number);
            if (y > GetHeight(floor, x - 1, y, z) || y < GetCeiling(floor, x - 1, y, z)) {
                target->x = x;
                target->y = y;
                target->z = z;
                target->room_number = last_room;
                return 0;
            }
            x -= WALL_L;
            y -= dy;
            z -= dz;
        }
    } else {
        x = start->x | (WALL_L - 1);
        y = start->y + (dy * (x - start->x) >> WALL_SHIFT);
        z = start->z + (dz * (x - start->x) >> WALL_SHIFT);
        while (x < target->x) {
            floor = GetFloor(x, y, z, &room_number);
            if (y > GetHeight(floor, x, y, z) || y < GetCeiling(floor, x, y, z)) {
                target->x = x;
                target->y = y;
                target->z = z;
                target->room_number = room_number;
                return -1;
            }
            if (room_number != last_room) {
                los_rooms[number_los_rooms++] = room_number;
                last_room = room_number;
            }
            floor = GetFloor(x + 1, y, z, &room_number);
            if (y > GetHeight(floor, x + 1, y, z) || y < GetCeiling(floor, x + 1, y, z)) {
                target->x = x;
                target->y = y;
                target->z = z;
                target->room_number = last_room;
                return 0;
            }
            x += WALL_L;
            y += dy;
            z += dz;
        }
    }
    target->room_number = room_number;
    return 1;
}

int ClipTarget(GAME_VECTOR *start, GAME_VECTOR *target)
{
    sint32 dx, dy, dz, height, lp;
    sint16 room_no;
    FLOOR_INFO *floor;
    GAME_VECTOR src;

    room_no = target->room_number;
    floor = GetFloor(target->x, target->y, target->z, &room_no);
    height = GetHeight(floor, target->x, target->y, target->z);
    if (target->y > height) {
        src.x = start->x + (((target->x - start->x) * 7) >> 3);
        src.y = start->y + (((target->y - start->y) * 7) >> 3);
        src.z = start->z + (((target->z - start->z) * 7) >> 3);
        for (lp = 3; lp > 0; lp--) {
            dx = src.x + (((target->x - src.x) * lp) >> 2);
            dy = src.y + (((target->y - src.y) * lp) >> 2);
            dz = src.z + (((target->z - src.z) * lp) >> 2);
            floor = GetFloor(dx, dy, dz, &room_no);
            height = GetHeight(floor, dx, dy, dz);
            if (dy < height) {
                target->x = dx;
                target->y = dy;
                target->z = dz;
                target->room_number = room_no;
                return 0;
            }
        }
        target->x = dx;
        target->y = dy;
        target->z = dz;
        target->room_number = room_no;
        return 0;
    }
    room_no = target->room_number;
    floor = GetFloor(target->x, target->y, target->z, &room_no);
    height = GetCeiling(floor, target->x, target->y, target->z);
    if (target->y < height) {
        src.x = start->x + (((target->x - start->x) * 7) >> 3);
        src.y = start->y + (((target->y - start->y) * 7) >> 3);
        src.z = start->z + (((target->z - start->z) * 7) >> 3);
        for (lp = 3; lp > 0; lp--) {
            dx = src.x + (((target->x - src.x) * lp) >> 2);
            dy = src.y + (((target->y - src.y) * lp) >> 2);
            dz = src.z + (((target->z - src.z) * lp) >> 2);
            floor = GetFloor(dx, dy, dz, &room_no);
            height = GetCeiling(floor, dx, dy, dz);
            if (dy > height) {
                target->x = dx;
                target->y = dy;
                target->z = dz;
                target->room_number = room_no;
                return 0;
            }
        }
        target->x = dx;
        target->y = dy;
        target->z = dz;
        target->room_number = room_no;
        return 0;
    }
    return 1;
}

extern sint16 *commands;
extern sint16 FXType;
extern void (*effect_routines[])(ITEM_INFO *item);

void TranslateItem(ITEM_INFO *item, sint32 x, sint32 y, sint32 z);

void AnimateItem(ITEM_INFO *item)
{
    int i;
    ANIM_STRUCT *anim;
    sint16 *command;
    sint32 speed;
    uint16 num;
    uint16 type;

    item->touch_bits = 0;
    item->hit_status = 0;
    anim = &anims[item->anim_number];
    item->frame_number++;
    if (anim->number_changes > 0) {
        if (GetChange(item, anim)) {
            anim = &anims[item->anim_number];
            item->current_anim_state = anim->current_anim_state;
            if (item->required_anim_state == item->current_anim_state)
                item->required_anim_state = 0;
        }
    }

    if (item->frame_number > anim->frame_end) {
        if (anim->number_commands > 0) {
            command = &commands[anim->command_index];
            for (i = anim->number_commands; i > 0; i--) {
                switch (*(command++)) {
                case 1:
                    TranslateItem(item, (sint32)*command, (sint32)*(command + 1), (sint32)*(command + 2));
                    command += 3;
                    break;
                case 2:
                    item->fallspeed = *(command++);
                    item->speed = *(command++);
                    item->gravity_status = 1;
                    break;
                case 3:
                    break;
                case 4:
                    if (objects[item->object_number].intelligent)
                        item->after_death = 1;
                    else
                        item->after_death = 64;
                    item->status = DEACTIVATED;
                    break;
                case 5:
                case 6:
                    command += 2;
                    break;
                }
            }
        }
        item->anim_number = anim->jump_anim_num;
        item->frame_number = anim->jump_frame_num;
        anim = &anims[item->anim_number];
        if (item->current_anim_state != anim->current_anim_state) {
            item->current_anim_state = anim->current_anim_state;
            item->goal_anim_state = item->current_anim_state;
        }
        if (item->required_anim_state == item->current_anim_state)
            item->required_anim_state = 0;
    }

    if (anim->number_commands > 0) {
        command = &commands[anim->command_index];
        for (i = anim->number_commands; i > 0; i--) {
            switch (*(command++)) {
            case 1:
                command += 3;
                break;
            case 2:
                command += 2;
                break;
            case 3:
                break;
            case 4:
                break;
            case 5:
                if (item->frame_number == *command) {
                    type = *(command + 1) & 0xc000;
                    num = *(command + 1) & 0x3fff;
                    if (objects[item->object_number].water_creature)
                        SoundEffect((int)num, &item->pos, 1);
                    else if (item->room_number == 255) {
                        item->pos.x_pos = lara_item->pos.x_pos;
                        item->pos.y_pos = lara_item->pos.y_pos - 762;
                        item->pos.z_pos = lara_item->pos.z_pos;
                        if (item->object_number == 9)
                            SoundEffect((int)num, &item->pos, 2);
                        else
                            SoundEffect((int)num, &item->pos, 0);
                    } else if (room[item->room_number].flags & UNDERWATER) {
                        if (type == 0 || type == 0x8000)
                            SoundEffect((int)num, &item->pos, 0);
                    } else if (type == 0 || type == 0x4000)
                        SoundEffect((int)num, &item->pos, 0);
                }
                command += 2;
                break;
            case 6:
                if (item->frame_number == *command) {
                    FXType = command[1] & 0xc000;
                    num = *(command + 1) & 0x3fff;
                    (*effect_routines[(int)num])(item);
                }
                command += 2;
                break;
            }
        }
    }

    if (!item->gravity_status) {
        speed = anim->velocity;
        if (anim->acceleration != 0)
            speed += anim->acceleration * (item->frame_number - anim->frame_base);
        item->speed = (sint16)(speed >> 16);
    } else {
        item->fallspeed += (item->fallspeed < FASTFALL_SPEED) ? 6 : 1;
        item->pos.y_pos += item->fallspeed;
    }

    item->pos.x_pos += (phd_sin(item->pos.y_rot) * item->speed) >> W2V_SHIFT;
    item->pos.z_pos += (phd_cos(item->pos.y_rot) * item->speed) >> W2V_SHIFT;
}


int ObjectOnLOS(GAME_VECTOR *start, GAME_VECTOR *target)
{
    int i, j, failure;
    sint32 x, y, z, dx, dy, dz, distance;
    sint32 direction;
    sint16 item_number;
    sint16 *bounds, *zextent, *xextent;
    ITEM_INFO *item;

    dx = target->x - start->x;
    dy = target->y - start->y;
    dz = target->z - start->z;
    for (i = 0; i < number_los_rooms; i++) {
        for (item_number = room[los_rooms[i]].item_number; item_number != NO_ITEM;
             item_number = items[item_number].next_item) {
            item = &items[item_number];
            if (item->status == DEACTIVATED)
                continue;
            if (item->object_number != 101 && item->object_number != 102 && item->object_number != 103 &&
                item->object_number != 104 && item->object_number != 38 && item->object_number != 373)
                continue;
            bounds = GetBoundsAccurate(item);
            direction = (uint16)(item->pos.y_rot + 0x2000) / 0x4000;
            if (direction & 1) {
                zextent = &bounds[0];
                xextent = &bounds[4];
            } else {
                zextent = &bounds[4];
                xextent = &bounds[0];
            }
            failure = 0;
            if ((dz < 0 ? -dz : dz) > (dx < 0 ? -dx : dx)) {
                distance = item->pos.z_pos + zextent[0] - start->z;
                for (j = 0; j < 2; j++) {
                    if ((distance & 0x80000000) == (dz & 0x80000000)) {
                        y = dy * distance / dz;
                        if (y > item->pos.y_pos + bounds[2] - start->y && y < item->pos.y_pos + bounds[3] - start->y) {
                            x = dx * distance / dz;
                            if (x < item->pos.x_pos + xextent[0] - start->x)
                                failure |= 1;
                            else if (x > item->pos.x_pos + xextent[1] - start->x)
                                failure |= 2;
                            else
                                return item_number;
                        }
                    }
                    distance = item->pos.z_pos + zextent[1] - start->z;
                }
                if (failure == 3)
                    return item_number;
            } else {
                distance = item->pos.x_pos + xextent[0] - start->x;
                for (j = 0; j < 2; j++) {
                    if ((distance & 0x80000000) == (dx & 0x80000000)) {
                        y = dy * distance / dx;
                        if (y > item->pos.y_pos + bounds[2] - start->y && y < item->pos.y_pos + bounds[3] - start->y) {
                            z = dz * distance / dx;
                            if (z < item->pos.z_pos + zextent[0] - start->z)
                                failure |= 1;
                            else if (z > item->pos.z_pos + zextent[1] - start->z)
                                failure |= 2;
                            else
                                return item_number;
                        }
                    }
                    distance = item->pos.x_pos + xextent[1] - start->x;
                }
                if (failure == 3)
                    return item_number;
            }
        }
    }
    return NO_ITEM;
}

void TranslateItem(ITEM_INFO *item, sint32 x, sint32 y, sint32 z)
{
    sint32 cosy, siny;

    cosy = phd_cos(item->pos.y_rot);
    siny = phd_sin(item->pos.y_rot);
    item->pos.x_pos += (x * cosy + z * siny) >> W2V_SHIFT;
    item->pos.y_pos += y;
    item->pos.z_pos += (z * cosy - x * siny) >> W2V_SHIFT;
}

#define LAVA_TYPE 5
#define CLIMB_TYPE 6
#define MONKEY_TYPE 19
#define MINEL_TYPE 20
#define MINER_TYPE 21
#define VALUE_BITS 0x03ff
#define TRIG_BITS(T) ((T & 0x3fff) >> 10)
#define ACTIVE 1
#define SWONESHOT 0x40
#define ATONESHOT 0x80

#define TRIGGER 0
#define PAD 1
#define SWITCH 2
#define KEY 3
#define PICKUP 4
#define HEAVY 5
#define ANTIPAD 6
#define COMBAT 7
#define DUMMY 8
#define ANTITRIGGER 9

#define TO_OBJECT 0
#define TO_CAMERA 1
#define TO_SINK 2
#define TO_FLIPMAP 3
#define TO_FLIPON 4
#define TO_FLIPOFF 5
#define TO_TARGET 6
#define TO_FINISH 7
#define TO_CD 8
#define TO_FLIPEFFECT 9
#define TO_SECRET 10
#define TO_BODYBAG 11

typedef struct {
    sint16 angle;
    uint8 speed;
    uint8 on;
    sint16 angle_time;
    sint16 speed_time;
    sint16 Xrange;
    sint16 Zrange;
    sint16 Yrange;
} LEADER_INFO;

extern LEADER_INFO lead_info[];
extern char HeavyTriggered;
extern char LaraOnPad;
extern int flipmap[];
extern int flip_status;
extern int flipeffect;
extern int fliptimer;
extern int level_complete;

void LavaBurn(ITEM_INFO *item);
int SwitchTrigger(sint16 item_num, sint16 timer);
int KeyTrigger(sint16 item_num);
int PickupTrigger(sint16 item_num);
void SetupShoal(sint32 shoal_number);
int EnableBaddieAI(sint16 item_number, int always);
void ClearBodyBag(void);
void S_CDPlay(sint16 track, int mode);

void TestTriggers(sint16 *data, int heavy)
{
    ITEM_INFO *item, *camera_item;
    sint16 camera_flags, camera_timer;
    sint16 type, trigger, value, timer, flags;
    int switch_off = 0, flip = 0, flip_available = 0, neweffect = -1;

    HeavyTriggered = 0;
    if (!heavy) {
        lara.climb_status = 0;
        lara.CanMonkeySwing = 0;
        lara.MineL = 0;
        lara.MineR = 0;
    }
    if (data == NULL)
        return;

    if ((*data & DATA_TYPE) == LAVA_TYPE) {
        if (!heavy && (lara_item->pos.y_pos == lara_item->floor || lara.water_status != LARA_ABOVEWATER))
            LavaBurn(lara_item);
        if (*data & END_BIT)
            return;
        data++;
    }

    if ((*data & DATA_TYPE) == CLIMB_TYPE) {
        if (!heavy) {
            int quad;

            quad = (uint16)(lara_item->pos.y_rot + 0x2000) / 0x4000;
            if ((1 << (quad + 8)) & *data)
                lara.climb_status = 1;
        }
        if (*data & END_BIT)
            return;
        data++;
    }

    if ((*data & DATA_TYPE) == MONKEY_TYPE) {
        if (!heavy)
            lara.CanMonkeySwing = 1;
        if (*data & END_BIT)
            return;
        data++;
    }

    if ((*data & DATA_TYPE) == MINEL_TYPE) {
        if (!heavy)
            lara.MineL = 1;
        if (*data & END_BIT)
            return;
        data++;
    }

    if ((*data & DATA_TYPE) == MINER_TYPE) {
        if (!heavy)
            lara.MineR = 1;
        if (*data & END_BIT)
            return;
        data++;
    }

    type = (*(data++) >> 8) & 0x3f;
    flags = *(data++);
    timer = flags & 0xff;

    if (camera.type != HEAVY_CAMERA)
        RefreshCamera(type, data);

    if (heavy) {
        if (type != HEAVY)
            return;
    } else {
        switch (type) {
        case SWITCH:
            value = *(data++) & VALUE_BITS;
            if (flags & ONESHOT)
                items[value].item_flags[0] = 1;
            if (!SwitchTrigger(value, timer))
                return;
            switch_off = (items[value].current_anim_state == 1);
            break;
        case PAD:
        case ANTIPAD:
            LaraOnPad = 1;
            if (lara_item->pos.y_pos != lara_item->floor || lara_item->item_flags[0])
                return;
            if (CurrentLevel == 0) {
                if (lara_item->item_flags[1] == 1)
                    return;
                lara_item->item_flags[1] = 1;
            }
            break;
        case KEY:
            value = *(data++) & VALUE_BITS;
            if (!KeyTrigger(value))
                return;
            break;
        case PICKUP:
            value = *(data++) & VALUE_BITS;
            if (!PickupTrigger(value))
                return;
            break;
        case HEAVY:
            return;
        case COMBAT:
            if (lara.gun_status != LG_READY)
                return;
            break;
        case DUMMY:
            return;
        }
    }

    camera_item = NULL;
    do {
        trigger = *(data++);
        value = trigger & VALUE_BITS;
        switch (TRIG_BITS(trigger)) {
        case TO_OBJECT:
            item = &items[value];
            if ((type == ANTIPAD || type == ANTITRIGGER) && (item->flags & ATONESHOT))
                break;
            if (type == SWITCH && (item->flags & SWONESHOT))
                break;
            if (type != SWITCH && type != ANTIPAD && type != ANTITRIGGER && (item->flags & ONESHOT))
                break;

            if (type != ANTIPAD && type != ANTITRIGGER) {
                if ((item->object_number == 91 || item->object_number == 92) && item->active)
                    break;
                if (item->object_number == 339 || item->object_number == 338) {
                    item->hit_points = timer & 7;
                    SetupShoal(item->hit_points);
                    timer = 0;
                } else if (item->object_number >= 66 && item->object_number <= 68) {
                    timer &= 7;
                    if (timer == 0)
                        timer = 1;
                    item->hit_points = timer;
                    timer = 0;
                } else if (item->object_number == 116 || item->object_number == 86 || item->object_number == 282) {
                    item->item_flags[0] = timer;
                    if (item->object_number == 282)
                        item->hit_points = timer;
                    timer = 0;
                }
            }

            item->timer = timer;
            if (timer != 1)
                item->timer *= 30;

            if (type == SWITCH) {
                item->flags ^= (flags & CODE_BITS);
                if (flags & ONESHOT)
                    item->flags |= SWONESHOT;
            } else if (type == ANTIPAD || type == ANTITRIGGER) {
                if (item->object_number == 339 || item->object_number == 338) {
                    sint32 leader;

                    leader = item->hit_points;
                    lead_info[leader].on = 0;
                }
                if (item->object_number == 365) {
                    item->item_flags[0] = 0;
                    item->item_flags[1] = 100;
                }
                item->flags &= ~(CODE_BITS | REVERSE);
                if (flags & ONESHOT)
                    item->flags |= ATONESHOT;
            } else if (flags & CODE_BITS)
                item->flags |= (flags & CODE_BITS);

            if ((item->flags & CODE_BITS) != CODE_BITS)
                break;
            if (flags & ONESHOT)
                item->flags |= ONESHOT;

            if (!item->active) {
                if (objects[item->object_number].intelligent) {
                    if (item->status == NOT_ACTIVE) {
                        item->touch_bits = 0;
                        item->status = ACTIVE;
                        AddActiveItem(value);
                        if (item->object_number != 30)
                            EnableBaddieAI(value, 1);
                    } else if (item->status == INVISIBLE) {
                        item->touch_bits = 0;
                        if (EnableBaddieAI(value, 0))
                            item->status = ACTIVE;
                        else
                            item->status = INVISIBLE;
                        AddActiveItem(value);
                    }
                } else {
                    item->touch_bits = 0;
                    AddActiveItem(value);
                    item->status = ACTIVE;
                    HeavyTriggered = heavy;
                }
            }
            break;

        case TO_CAMERA:
            trigger = *(data++);
            camera_flags = trigger;
            camera_timer = trigger & 0xff;
            if (camera.fixed[value].flags & ONESHOT)
                break;
            camera.number = value;
            if (camera.type == LOOK_CAMERA || camera.type == COMBAT_CAMERA)
                break;
            if (type == COMBAT)
                break;
            if (type == SWITCH && timer && switch_off)
                break;
            if (camera.number != camera.last || type == SWITCH) {
                camera.timer = camera_timer * 30;
                if (camera_flags & ONESHOT)
                    camera.fixed[camera.number].flags |= ONESHOT;
                camera.speed = ((camera_flags & CODE_BITS) >> 6) + 1;
                camera.type = heavy ? HEAVY_CAMERA : FIXED_CAMERA;
            }
            break;

        case TO_TARGET:
            camera_item = &items[value];
            break;

        case TO_SINK:
            lara.current_active = value + 1;
            break;

        case TO_FLIPMAP:
            flip_available = 1;
            if (flipmap[value] & ONESHOT)
                break;
            if (type == SWITCH)
                flipmap[value] ^= (flags & CODE_BITS);
            else if (flags & CODE_BITS)
                flipmap[value] |= (flags & CODE_BITS);
            if ((flipmap[value] & CODE_BITS) == CODE_BITS) {
                if (flags & ONESHOT)
                    flipmap[value] |= ONESHOT;
                if (!flip_status)
                    flip = 1;
            } else if (flip_status)
                flip = 1;
            break;

        case TO_FLIPON:
            flip_available = 1;
            if ((flipmap[value] & CODE_BITS) == CODE_BITS && !flip_status)
                flip = 1;
            break;

        case TO_FLIPOFF:
            flip_available = 1;
            if ((flipmap[value] & CODE_BITS) == CODE_BITS && flip_status)
                flip = 1;
            break;

        case TO_FLIPEFFECT:
            neweffect = value;
            break;

        case TO_FINISH:
            level_complete = 1;
            break;

        case TO_CD:
            TriggerCDTrack(value, flags, type);
            break;

        case TO_SECRET:
            if (!(savegame.secrets & (1 << value))) {
                S_CDPlay(122, 0);
                savegame.secrets |= (1 << value);
            }
            break;

        case TO_BODYBAG:
            ClearBodyBag();
            break;
        }
    } while (!(trigger & END_BIT));

    if (camera_item && (camera.type == FIXED_CAMERA || camera.type == HEAVY_CAMERA))
        camera.item = camera_item;
    if (flip)
        FlipMap();
    if (neweffect != -1 && (flip || !flip_available)) {
        flipeffect = neweffect;
        fliptimer = 0;
    }
}

#define TRIGGER_TYPE 4

extern sint32 OnObject;
extern sint32 height_type;
extern sint32 tiltxoff;
extern sint32 tiltyoff;
extern sint32 chunky_flag;
extern sint16 GF_NoFloor;
void S_ExitSystem(char *message);

sint32 GetHeight(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z)
{
    sint32 xoff, yoff, height, height2;
    ROOM_INFO *r;
    ITEM_INFO *item;
    sint16 *data, type, trigger;
    sint16 tilts, dx, dz, t0, t1, t2, t3, hadj;

    OnObject = 0;
    height_type = 0;
    while (floor->pit_room != NO_ROOM) {
        if (CheckNoColFloorTriangle(floor, x, z) == 1)
            break;
        r = &room[floor->pit_room];
        floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
    }
    height = floor->floor << 8;
    if (height == NO_HEIGHT)
        return height;
    if (GF_NoFloor && GF_NoFloor == height)
        height = 0x4000;
    height2 = height;
    trigger_index = NULL;
    if (!floor->index)
        return height;
    tiltxoff = tiltyoff = 0;
    data = &floor_data[floor->index];
    do {
        type = *(data++);
        switch (type & DATA_TYPE) {
        case SPLIT1:
        case SPLIT2:
        case NOCOLF1T:
        case NOCOLF1B:
        case NOCOLF2T:
        case NOCOLF2B:
            tilts = *data;
            t0 = tilts & 15;
            t1 = (tilts >> 4) & 15;
            t2 = (tilts >> 8) & 15;
            t3 = (tilts >> 12) & 15;
            dx = x & 1023;
            dz = z & 1023;
            xoff = yoff = 0;
            height_type = 4;
            if ((type & DATA_TYPE) == SPLIT1 || (type & DATA_TYPE) == NOCOLF1T || (type & DATA_TYPE) == NOCOLF1B) {
                if (dx <= (1024 - dz)) {
                    hadj = (type >> 10) & 0x1f;
                    if (hadj & 0x10)
                        hadj |= 0xfff0;
                    height += hadj << 8;
                    xoff = t2 - t1;
                    yoff = t0 - t1;
                } else {
                    hadj = (type >> 5) & 0x1f;
                    if (hadj & 0x10)
                        hadj |= 0xfff0;
                    height += hadj << 8;
                    xoff = t3 - t0;
                    yoff = t3 - t2;
                }
            } else {
                if (dx <= dz) {
                    hadj = (type >> 10) & 0x1f;
                    if (hadj & 0x10)
                        hadj |= 0xfff0;
                    height += hadj << 8;
                    xoff = t2 - t1;
                    yoff = t3 - t2;
                } else {
                    hadj = (type >> 5) & 0x1f;
                    if (hadj & 0x10)
                        hadj |= 0xfff0;
                    height += hadj << 8;
                    xoff = t3 - t0;
                    yoff = t0 - t1;
                }
            }
            tiltxoff = xoff;
            tiltyoff = yoff;
            if (chunky_flag) {
                long h1, h2;
                hadj = (hadj >> 10) & 0x1f;
                if (hadj & 0x10)
                    hadj |= 0xfff0;
                h1 = h2 = height2;
                h1 += hadj << 8;
                hadj = (hadj >> 5) & 0x1f;
                if (hadj & 0x10)
                    hadj |= 0xfff0;
                h2 += hadj << 8;
                if (h1 < h2)
                    height = h1;
                else
                    height = h2;
            } else {
                if (ABS(xoff) > 2 || ABS(yoff) > 2)
                    height_type = 3;
                else if (height_type != 4)
                    height_type = 1;
                if (xoff < 0)
                    height -= (z & (WALL_L - 1)) * xoff >> 2;
                else
                    height += ((WALL_L - 1 - z) & (WALL_L - 1)) * xoff >> 2;
                if (yoff < 0)
                    height -= (x & (WALL_L - 1)) * yoff >> 2;
                else
                    height += ((WALL_L - 1 - x) & (WALL_L - 1)) * yoff >> 2;
            }
            data++;
            break;
        case TILT_TYPE:
            tiltxoff = xoff = (*data) >> 8;
            tiltyoff = yoff = (signed char)(*data);
            if (!chunky_flag || (ABS(xoff) <= 2 && ABS(yoff) <= 2)) {
                if (ABS(xoff) > 2 || ABS(yoff) > 2)
                    height_type = 2;
                else
                    height_type = 1;
                if (xoff < 0)
                    height -= (z & (WALL_L - 1)) * xoff >> 2;
                else
                    height += ((WALL_L - 1 - z) & (WALL_L - 1)) * xoff >> 2;
                if (yoff < 0)
                    height -= (x & (WALL_L - 1)) * yoff >> 2;
                else
                    height += ((WALL_L - 1 - x) & (WALL_L - 1)) * yoff >> 2;
            }
            data++;
            break;
        case SPLIT3:
        case SPLIT4:
        case NOCOLC1T:
        case NOCOLC1B:
        case NOCOLC2T:
        case NOCOLC2B:
        case ROOF_TYPE:
        case DOOR_TYPE:
            data++;
            break;
        case LAVA_TYPE:
            trigger_index = data - 1;
            break;
        case CLIMB_TYPE:
        case MONKEY_TYPE:
        case MINEL_TYPE:
        case MINER_TYPE:
            if (!trigger_index)
                trigger_index = data - 1;
            break;
        case TRIGGER_TYPE:
            if (!trigger_index)
                trigger_index = data - 1;
            data++;
            do {
                trigger = *(data++);
                if (TRIG_BITS(trigger) != TO_OBJECT) {
                    if (TRIG_BITS(trigger) == TO_CAMERA)
                        trigger = *(data++);
                    continue;
                }
                item = &items[trigger & VALUE_BITS];
                if (objects[item->object_number].floor)
                    (*objects[item->object_number].floor)(item, x, y, z, &height);
            } while (!(trigger & END_BIT));
            break;
        default:
            S_ExitSystem("GetHeight(): Unknown type");
        }
    } while (!(type & END_BIT));
    return height;
}

extern ITEM_INFO *CeilingObject;

sint32 GetCeiling(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z)
{
    sint32 xoff, yoff, height;
    ROOM_INFO *r;
    ITEM_INFO *item;
    FLOOR_INFO *f;
    sint16 *data, type, trigger;
    sint16 tilts, dx, dz, t0, t1, t2, t3, hadj, ended;

    CeilingObject = NULL;
    f = floor;
    while (f->sky_room != NO_ROOM) {
        if (CheckNoColCeilingTriangle(floor, x, z) == 1)
            break;
        r = &room[f->sky_room];
        f = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
    }
    height = f->ceiling << 8;
    if (height == NO_HEIGHT)
        return height;
    if (f->index) {
        data = &floor_data[f->index];
        hadj = *(data++);
        type = hadj & DATA_TYPE;
        ended = 0;
        if (type == TILT_TYPE || type == SPLIT1 || type == SPLIT2 || type == NOCOLF1T || type == NOCOLF1B || type == NOCOLF2T || type == NOCOLF2B) {
            ended = hadj & END_BIT;
            data++;
            hadj = *(data++);
            type = hadj & DATA_TYPE;
        }
        if (ended == 0 && (type == ROOF_TYPE || type == SPLIT3 || type == SPLIT4 || type == NOCOLC1T || type == NOCOLC1B || type == NOCOLC2T || type == NOCOLC2B)) {
            if (type != ROOF_TYPE) {
                tilts = *data;
                t0 = -(tilts & 15);
                t1 = -((tilts >> 4) & 15);
                t2 = -((tilts >> 8) & 15);
                t3 = -((tilts >> 12) & 15);
                dx = x & 1023;
                dz = z & 1023;
                xoff = yoff = 0;
                if (type == SPLIT3 || type == NOCOLC1T || type == NOCOLC1B) {
                    if (dx <= (1024 - dz)) {
                        hadj = (hadj >> 10) & 0x1f;
                        if (hadj & 0x10)
                            hadj |= 0xfff0;
                        height += hadj << 8;
                        xoff = t2 - t1;
                        yoff = t3 - t2;
                    } else {
                        hadj = (hadj >> 5) & 0x1f;
                        if (hadj & 0x10)
                            hadj |= 0xfff0;
                        height += hadj << 8;
                        xoff = t3 - t0;
                        yoff = t0 - t1;
                    }
                } else {
                    if (dx <= dz) {
                        hadj = (hadj >> 10) & 0x1f;
                        if (hadj & 0x10)
                            hadj |= 0xfff0;
                        height += hadj << 8;
                        xoff = t2 - t1;
                        yoff = t0 - t1;
                    } else {
                        hadj = (hadj >> 5) & 0x1f;
                        if (hadj & 0x10)
                            hadj |= 0xfff0;
                        height += hadj << 8;
                        xoff = t3 - t0;
                        yoff = t3 - t2;
                    }
                }
                if (chunky_flag) {
                    long h1, h2;
                    hadj = (hadj >> 10) & 0x1f;
                    if (hadj & 0x10)
                        hadj |= 0xfff0;
                    h1 = h2 = f->ceiling << 8;
                    h1 += hadj << 8;
                    hadj = (hadj >> 5) & 0x1f;
                    if (hadj & 0x10)
                        hadj |= 0xfff0;
                    h2 += hadj << 8;
                    if (h1 > h2)
                        height = h1;
                    else
                        height = h2;
                }
            } else {
                xoff = (*data) >> 8;
                yoff = (signed char)(*data);
            }
            if (!chunky_flag) {
                if (xoff < 0)
                    height += (z & (WALL_L - 1)) * xoff >> 2;
                else
                    height -= ((WALL_L - 1 - z) & (WALL_L - 1)) * xoff >> 2;
                if (yoff >= 0)
                    height -= (x & (WALL_L - 1)) * yoff >> 2;
                else
                    height += ((WALL_L - 1 - x) & (WALL_L - 1)) * yoff >> 2;
            }
        }
    }
    while (floor->pit_room != NO_ROOM) {
        if (CheckNoColFloorTriangle(floor, x, z) == 1)
            break;
        r = &room[floor->pit_room];
        floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
    }
    if (!floor->index)
        return height;
    data = &floor_data[floor->index];
    do {
        type = *(data++);
        switch (type & DATA_TYPE) {
        case SPLIT1:
        case SPLIT2:
        case SPLIT3:
        case SPLIT4:
        case NOCOLF1T:
        case NOCOLF1B:
        case NOCOLF2T:
        case NOCOLF2B:
        case NOCOLC1T:
        case NOCOLC1B:
        case NOCOLC2T:
        case NOCOLC2B:
        case DOOR_TYPE:
        case TILT_TYPE:
        case ROOF_TYPE:
            data++;
            break;
        case LAVA_TYPE:
        case CLIMB_TYPE:
        case MONKEY_TYPE:
        case MINEL_TYPE:
        case MINER_TYPE:
            break;
        case TRIGGER_TYPE:
            data++;
            do {
                trigger = *(data++);
                if (TRIG_BITS(trigger) != TO_OBJECT) {
                    if (TRIG_BITS(trigger) == TO_CAMERA)
                        trigger = *(data++);
                    continue;
                }
                item = &items[trigger & VALUE_BITS];
                if (objects[item->object_number].ceiling)
                    (*objects[item->object_number].ceiling)(item, x, y, z, &height);
            } while (!(trigger & END_BIT));
            break;
        default:
            S_ExitSystem("GetCeiling(): Unknown type");
        }
    } while (!(type & END_BIT));
    return height;
}

extern uint16 OutsideRoomOffsets[];
extern uint8 *OutsideRoomTable;
extern sint16 IsRoomOutsideNo;

sint32 IsRoomOutside(sint32 x, sint32 y, sint32 z)
{
    uint16 offset;
    ROOM_INFO *r;
    FLOOR_INFO *floor;
    sint16 room_number;
    long height;

    if ((offset = OutsideRoomOffsets[((x >> 12) * 27) + (z >> 12)]) == -1)
        return -2;

    if (offset < 0) {
        r = &room[(offset & 0x7fff)];

        if ((y > r->maxceiling) && (y < r->minfloor)
            && ((z > (r->z + 1024)) && (z < (r->z + ((r->x_size - 1) * 1024))))
            && ((x > (r->x + 1024)) && (x < (r->x + ((r->y_size - 1) * 1024))))) {
            room_number = offset & 0x7fff;
            floor = GetFloor(x, y, z, &room_number);
            height = GetHeight(floor, x, y, z);
            if (height == NO_HEIGHT || y > height)
                return -2;
            height = GetCeiling(floor, x, y, z);
            if (y < height)
                return -2;

            if (!(r->flags & (32 | UNDERWATER)))
                return -3;

            IsRoomOutsideNo = offset & 0x7fff;
            return 1;
        } else
            return -2;
    } else {
        unsigned char *s = &OutsideRoomTable[offset];

        while (*s != 0xff) {
            r = &room[*s];

            if ((y > r->maxceiling && y < r->minfloor)
                && ((z > (r->z + 1024)) && (z < (r->z + ((r->x_size - 1) * 1024))))
                && ((x > (r->x + 1024)) && (x < (r->x + ((r->y_size - 1) * 1024))))) {
                room_number = *s;

                floor = GetFloor(x, y, z, &room_number);
                height = GetHeight(floor, x, y, z);
                if (height == NO_HEIGHT || y > height)
                    return -2;
                height = GetCeiling(floor, x, y, z);
                if (y < height)
                    return -2;

                if (!(r->flags & (32 | UNDERWATER)))
                    return -3;

                IsRoomOutsideNo = *s;
                return 1;
            }
            s++;
        }
        return -2;
    }
}
