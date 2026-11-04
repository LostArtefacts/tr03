#include "game.h"

#define CIVVY_STOP_ANIM 6
#define CIVVY_STOP 1

void InitialiseCivvy(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    InitialiseCreature(item_number);
    item->anim_number = objects[CIVVIE].anim_index + CIVVY_STOP_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = CIVVY_STOP;
}
