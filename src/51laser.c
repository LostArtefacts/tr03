#include "game.h"

void InitialiseArea51Laser(sint16 item_number)
{
    ITEM_INFO *item;
    sint32 dx, dz;

    item = &items[item_number];
    dx = item->pos.x_pos + ((2560 * phd_sin(item->pos.y_rot)) >> W2V_SHIFT);
    dz = item->pos.z_pos + ((2560 * phd_cos(item->pos.y_rot)) >> W2V_SHIFT);
    item->item_flags[0] = ((item->pos.x_pos >> 9) & 255) | (((item->pos.z_pos >> 9) & 255) << 8);
    item->item_flags[1] = ((dx >> 9) & 255) | (((dz >> 9) & 255) << 8);
    item->item_flags[2] = 1;
    item->item_flags[3] = 0;
    item->shadeB = 0;
}
