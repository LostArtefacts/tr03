#include "game.h"

void LaserSplitterToggle(ITEM_INFO *item);

void LaserControl(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    LaserSplitterToggle(item);
}
