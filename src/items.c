#include "game.h"

extern sint16 body_bag;

void InitialiseFXArray(void)
{
    int i;
    FX_INFO *fx;

    next_fx_active = NO_ITEM;
    next_fx_free = 0;
    fx = effects;
    for (i = 1; i < NUM_EFFECTS; i++, fx++)
        fx->next_fx = i;
    fx->next_fx = NO_ITEM;
}

void ClearBodyBag(void)
{
    sint16 item_number;
    ITEM_INFO *item;

    if (body_bag == NO_ITEM)
        return;

    item_number = body_bag;
    while (item_number != NO_ITEM)
    {
        item = &items[item_number];
        KillItem(item_number);
        item_number = item->next_active;
        item->next_active = NO_ITEM;
    }
    body_bag = NO_ITEM;
}
