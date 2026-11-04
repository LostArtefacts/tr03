#include "game.h"

#define OILSMG_STOP_ANIM 12
#define OILSMG_STOP 1

void InitialiseOilSMG(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    InitialiseCreature(item_number);
    item->anim_number = objects[WHITE_SOLDIER].anim_index + OILSMG_STOP_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = OILSMG_STOP;
}
