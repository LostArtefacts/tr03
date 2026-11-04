#include "game.h"

extern sint16 body_bag;
extern sint16 next_item_free, next_item_active;
extern sint32 level_items;

void InitialiseItemArray(int numitems)
{
    int i;
    ITEM_INFO *item;

    body_bag = NO_ITEM;
    next_item_active = NO_ITEM;
    next_item_free = level_items;
    item = &items[level_items];
    for (i = level_items + 1; i < numitems; i++, item++)
    {
        item->next_item = i;
        item->active = 0;
        item->il.init = 0;
    }
    item->next_item = NO_ITEM;
}

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
