#include "game.h"

#define STOP_ANIM 8
#define DOG_STOP 1

void InitialiseDog(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->anim_number = objects[item->object_number].anim_index + STOP_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = DOG_STOP;
}
