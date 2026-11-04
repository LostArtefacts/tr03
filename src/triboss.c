#include "game.h"

extern sint32 level_items;

void TribeBossDie(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->collidable = 0;
    item->hit_points = DONT_TARGET;
    KillItem(item_number);
    DisableBaddieAI(item_number);
    item->flags |= ONESHOT;
}

sint16 FindLizardManItemNumber(sint16 room_number)
{
    ITEM_INFO *target;
    sint16 target_number;

    for (target_number = 0; target_number < level_items; target_number++)
    {
        target = &items[target_number];
        if (target->object_number == LIZARD_MAN && target->room_number == room_number)
            return target_number;
    }
    return -1;
}
