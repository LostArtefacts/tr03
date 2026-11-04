#include "game.h"

#define BOB_STOP_ANIM 6
#define BOB_STOP 1

void InitialisePrisoner(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    InitialiseCreature(item_number);
    item->anim_number = objects[BOB].anim_index + BOB_STOP_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = BOB_STOP;
}
