#include "game.h"

#define STRUT_WAIT 1
#define STRUT_FALL 2

extern char strut_fall;

void InitialiseArea51Struts(sint16 item_number)
{
    strut_fall = 0;
}

void ControlArea51Struts(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (!strut_fall)
        item->current_anim_state = STRUT_WAIT;
    else if (item->goal_anim_state != STRUT_FALL && item->current_anim_state != STRUT_FALL)
        item->goal_anim_state = STRUT_FALL;
    AnimateItem(item);
}
