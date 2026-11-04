#include "game.h"

#define MAX_CD_TRACKS 128

void TriggerNormalCDTrack(sint16 value, sint16 flags, sint16 type);

int TriggerActive(ITEM_INFO *item)
{
    int ok;

    ok = (item->flags & REVERSE) ? 0 : 1;
    if ((item->flags & CODE_BITS) != CODE_BITS)
        return !ok;
    if (!item->timer)
        return ok;
    if (item->timer == -1)
        return !ok;
    item->timer--;
    if (!item->timer)
        item->timer = -1;
    return ok;
}

void TriggerCDTrack(sint16 value, sint16 flags, sint16 type)
{
    if (value <= 1 || value >= MAX_CD_TRACKS)
        return;
    TriggerNormalCDTrack(value, flags, type);
}
