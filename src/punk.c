#include "game.h"

#define PUNK_STOP_ANIM 6
#define PUNK_STOP 1

void InitialisePunk(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    InitialiseCreature(item_number);
    item->anim_number = objects[PUNK1].anim_index + PUNK_STOP_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = PUNK_STOP;
}
