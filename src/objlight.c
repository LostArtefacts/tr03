#include "game.h"

void ControlOnOffLight(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (TriggerActive(item))
        TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 16, 31, 31, 31);
}

void ControlBeaconLight(sint16 item_number)
{
    ITEM_INFO *item;
    sint32 r, g, b;

    item = &items[item_number];
    if (TriggerActive(item))
    {
        item->item_flags[0]++;
        item->item_flags[0] &= 63;
        if (item->item_flags[0] < 3)
        {
            r = g = 31 - (GetRandomControl() & 1);
            b = 31 - (GetRandomControl() & 3);
            TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 16, r, g, b);
        }
    }
}
