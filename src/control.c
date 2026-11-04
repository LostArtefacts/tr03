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
