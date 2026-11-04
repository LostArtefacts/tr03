#include "game.h"

void AnimateItem(ITEM_INFO *item);

void ControlLaraExtra(sint16 item_num)
{
    AnimateItem(&items[item_num]);
}
