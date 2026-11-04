#include "game.h"

#define AUTOGUN_STILL_ANIM 1
#define AUTOGUN_STILL 1

void InitialiseAutogun(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->anim_number = objects[ROBOT_SENTRY_GUN].anim_index + AUTOGUN_STILL_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = AUTOGUN_STILL;
    item->item_flags[0] = 0;
}
