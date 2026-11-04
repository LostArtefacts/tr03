#include "game.h"

#define COBRA_DOWN_ANIM 2
#define COBRA_DOWN 3

void InitialiseCobra(sint16 item_number)
{
    ITEM_INFO *item;

    InitialiseCreature(item_number);
    item = &items[item_number];
    item->anim_number = objects[item->object_number].anim_index + COBRA_DOWN_ANIM;
    item->frame_number = anims[item->anim_number].frame_base + 45;
    item->current_anim_state = item->goal_anim_state = COBRA_DOWN;
    item->item_flags[2] = item->hit_points;
    item->hit_points = DONT_TARGET;
}
