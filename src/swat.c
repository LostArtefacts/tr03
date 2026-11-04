#include "game.h"

#define SWAT_STOP_ANIM 12
#define SWAT_STOP 1

void InitialiseSwat(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    InitialiseCreature(item_number);
    item->anim_number = objects[item->object_number].anim_index + SWAT_STOP_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = SWAT_STOP;
}
