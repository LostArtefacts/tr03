#include "game.h"

#define BATON_STOP_ANIM 6
#define BATON_STOP 1

void InitialiseBaton(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    InitialiseCreature(item_number);
    item->anim_number = objects[MP1].anim_index + BATON_STOP_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = BATON_STOP;
}
