#include "game.h"

#define MONKEY_SIT_ANIM 2
#define MONKEY_SIT 6

void InitialiseMonkey(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    InitialiseCreature(item_number);
    item->anim_number = objects[MONKEY].anim_index + MONKEY_SIT_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = MONKEY_SIT;
}
