#include "game.h"

#define FIREHEAD_REAR 1

void InitialiseFireHead(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->anim_number = objects[item->object_number].anim_index + 1;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = FIREHEAD_REAR;
    item->goal_anim_state = FIREHEAD_REAR;
}

void ControlRotateyThing(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (TriggerActive(item))
        AnimateItem(item);
    else
        KillItem(item_number);
}
