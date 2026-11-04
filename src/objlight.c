#include "game.h"

void TriggerDynamic(sint32 x, sint32 y, sint32 z, int falloff, int r, int g, int b);

void ControlOnOffLight(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (TriggerActive(item))
        TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 16, 31, 31, 31);
}
