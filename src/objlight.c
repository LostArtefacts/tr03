#include "game.h"
#include <stdlib.h>

extern sint16 rcossin_tbl[];

void ControlPulseLight(sint16 item_number)
{
    ITEM_INFO *item;
    sint32 angle, sin;

    item = &items[item_number];
    if (TriggerActive(item))
    {
        item->item_flags[0] += ONE_DEGREE << 2;
        angle = (item->item_flags[0] >> 4) & 4095;
        sin = rcossin_tbl[angle << 1] >> 7;
        sin = abs(sin);
        if (sin > 31)
            sin = 31;
        else if (sin < 8)
        {
            sin = 8;
            item->item_flags[0] += 2048;
        }
        TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, sin, 31, 12, 0);
    }
}

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
