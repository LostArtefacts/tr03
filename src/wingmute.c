#include "game.h"

#define WING_START_ANIM 2
#define WING_WAIT 2

void InitialiseWingmute(sint16 item_number)
{
    ITEM_INFO *item;

    InitialiseCreature(item_number);
    item = &items[item_number];
    item->anim_number = objects[MUTANT1].anim_index + WING_START_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = WING_WAIT;
    item->item_flags[1] = GetRandomControl() & 0x7f;
}
