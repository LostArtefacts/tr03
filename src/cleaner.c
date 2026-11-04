#include "game.h"

#define CLEANER_TURN 1024
#define CLEANER_SPEED 64

void InitialiseCleaner(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->pos.x_pos &= ~1023;
    item->pos.z_pos &= ~1023;
    item->pos.x_pos |= 512;
    item->pos.z_pos |= 512;
    item->item_flags[0] = CLEANER_TURN;
    item->item_flags[1] = 0;
    item->item_flags[2] = CLEANER_SPEED;
    item->collidable = 1;
    item->data = NULL;
}
