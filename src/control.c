#include "game.h"
#include <string.h>

#define MAX_CD_TRACKS 128

void MovableBlock(sint16 item_number);
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

void RemoveDrawnItem(sint16 item_num);
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
