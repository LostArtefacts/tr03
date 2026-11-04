#include "game.h"

#define ARMY_STOP_ANIM 12
#define ARMY_STOP 1

void InitialiseArmySMG(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    InitialiseCreature(item_number);
    item->anim_number = objects[STHPAC_MERCENARY].anim_index + ARMY_STOP_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = ARMY_STOP;
}
