#include "game.h"

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
