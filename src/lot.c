#include "game.h"

extern int number_boxes;
extern int slots_used, nonlot_slots_used;

void DisableBaddieAI(sint16 item_num)
{
    ITEM_INFO *item;
    CREATURE_INFO *creature;

    item = &items[item_num];
    if (item_num == lara.item_number)
    {
        creature = lara.creature;
        lara.creature = NULL;
    }
    else
    {
        creature = (CREATURE_INFO *)item->data;
        item->data = NULL;
    }
    if (creature)
    {
        creature->item_num = NO_ITEM;
        if (objects[item->object_number].non_lot)
            nonlot_slots_used--;
        else
            slots_used--;
    }
}

void ClearLOT(LOT_INFO *LOT)
{
    int i;
    BOX_NODE *node;

    LOT->head = LOT->tail = NO_BOX;
    LOT->search_number = 0;
    LOT->target_box = NO_BOX;
    LOT->required_box = NO_BOX;
    for (i = 0, node = LOT->node; i < number_boxes; i++, node++)
    {
        node->exit_box = node->next_expansion = NO_BOX;
        node->search_number = 0;
    }
}
