#include "game.h"

#define SHIVA_START_ANIM 14
#define SHIVA_START 4

void InitialiseShiva(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (item->object_number == SHIVA)
    {
        item->anim_number = objects[SHIVA].anim_index + SHIVA_START_ANIM;
        item->frame_number = anims[item->anim_number].frame_base;
        item->current_anim_state = item->goal_anim_state = SHIVA_START;
    }
    item->status = NOT_ACTIVE;
    item->mesh_bits = 0;
}
