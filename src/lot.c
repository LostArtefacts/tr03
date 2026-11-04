#include "game.h"

extern int number_boxes;
extern int slots_used, nonlot_slots_used;

#define CREATURE_DATA 33
#define CREATURE_LOT 34

void InitialiseLOTarray(void)
{
    int i;
    CREATURE_INFO *creature;

    baddie_slots = (CREATURE_INFO *)game_malloc(NUM_SLOTS * sizeof(CREATURE_INFO), CREATURE_DATA);
    creature = baddie_slots;
    for (i = 0; i < NUM_SLOTS; i++, creature++)
    {
        creature->item_num = NO_ITEM;
        creature->LOT.node = (BOX_NODE *)game_malloc(sizeof(BOX_NODE) * number_boxes, CREATURE_LOT);
    }
    slots_used = 0;
    non_lot_slots = (CREATURE_INFO *)game_malloc(NUM_NONLOT_SLOTS * sizeof(CREATURE_INFO), CREATURE_DATA);
    creature = non_lot_slots;
    for (i = 0; i < NUM_NONLOT_SLOTS; i++, creature++)
        creature->item_num = NO_ITEM;
    nonlot_slots_used = 0;
}

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
