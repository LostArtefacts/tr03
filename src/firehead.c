#include "game.h"

void ControlRotateyThing(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (TriggerActive(item))
        AnimateItem(item);
    else
        KillItem(item_number);
}
