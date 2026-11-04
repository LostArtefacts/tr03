#include "game.h"

extern int number_boxes;
extern int slots_used, nonlot_slots_used;

#define CREATURE_DATA 33
#define CREATURE_LOT 34
#define BLOCKED 0x4000
#define BLOCKABLE 0x8000
#define ZONE(A) (((A) >> (WALL_SHIFT - 2)) - 1)

extern sint16 *ground_zone[4][2];

int EnableNonLotAI(sint16 item_number, int Always);
void InitialiseSlot(sint16 item_number, int slot);
void InitialiseNonLotAI(sint16 item_number, int slot);
void CreateZone(ITEM_INFO *item);
void ClearLOT(LOT_INFO *LOT);

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

int EnableBaddieAI(sint16 item_number, int Always)
{
    int x, y, z;
    int worstslot, slot;
    int worstdist, dist;
    ITEM_INFO *item;
    CREATURE_INFO *creature;

    if (lara.item_number == item_number) {
        if (lara.creature)
            return 1;
    } else if (items[item_number].data) {
        return 1;
    } else if (objects[items[item_number].object_number].non_lot) {
        return EnableNonLotAI(item_number, Always);
    }

    if (slots_used < NUM_SLOTS) {
        creature = baddie_slots;
        for (slot = 0; slot < NUM_SLOTS; slot++, creature++) {
            if (creature->item_num == NO_ITEM) {
                InitialiseSlot(item_number, slot);
                return 1;
            }
        }
    }

    if (!Always) {
        item = &items[item_number];
        x = (item->pos.x_pos - camera.pos.x) >> 8;
        y = (item->pos.y_pos - camera.pos.y) >> 8;
        z = (item->pos.z_pos - camera.pos.z) >> 8;
        worstdist = x * x + y * y + z * z;
    } else {
        worstdist = 0;
    }

    worstslot = -1;
    creature = baddie_slots;
    for (slot = 0; slot < NUM_SLOTS; slot++, creature++) {
        item = &items[creature->item_num];
        x = (item->pos.x_pos - camera.pos.x) >> 8;
        y = (item->pos.y_pos - camera.pos.y) >> 8;
        z = (item->pos.z_pos - camera.pos.z) >> 8;
        dist = x * x + y * y + z * z;
        if (dist > worstdist) {
            worstdist = dist;
            worstslot = slot;
        }
    }

    if (worstslot >= 0) {
        items[baddie_slots[worstslot].item_num].status = INVISIBLE;
        DisableBaddieAI(baddie_slots[worstslot].item_num);
        InitialiseSlot(item_number, worstslot);
        return 1;
    }
    return 0;
}

void InitialiseSlot(sint16 item_number, int slot)
{
    CREATURE_INFO *creature;
    ITEM_INFO *item;
    int i;

    creature = &baddie_slots[slot];
    item = &items[item_number];
    if (item_number == lara.item_number)
        lara.creature = creature;
    else
        item->data = (void *)creature;

    creature->item_num = item_number;
    creature->mood = BORED_MOOD;
    for (i = 0; i < 4; i++)
        creature->joint_rotation[i] = 0;
    creature->maximum_turn = ONE_DEGREE;
    creature->flags = 0;
    creature->enemy = NULL;
    creature->alerted = 0;
    creature->head_left = 0;
    creature->head_right = 0;
    creature->reached_goal = 0;
    creature->hurt_by_lara = 0;
    creature->patrol2 = 0;
    creature->LOT.step = STEP_L;
    creature->LOT.drop = -STEP_L * 2;
    creature->LOT.block_mask = BLOCKED;
    creature->LOT.fly = NO_FLYING;

    switch (item->object_number) {
    case 0:
        creature->LOT.step = WALL_L * 20;
        creature->LOT.drop = -WALL_L * 20;
        creature->LOT.fly = STEP_L;
        break;

    case 25:
    case 26:
    case 27:
    case 29:
    case 32:
    case 44:
        creature->LOT.step = WALL_L * 20;
        creature->LOT.drop = -WALL_L * 20;
        creature->LOT.fly = STEP_L / 16;
        if (item->object_number == 25)
            creature->LOT.block_mask = BLOCKABLE;
        break;

    case 35:
    case 49:
    case 53:
    case 60:
    case 62:
    case 65:
    case 71:
        creature->LOT.step = WALL_L;
        creature->LOT.drop = -WALL_L;
        break;

    case 70:
    case 287:
        creature->LOT.block_mask = BLOCKABLE;
        break;

    case 57:
        creature->LOT.step = WALL_L;
        creature->LOT.drop = -STEP_L * 3;
        break;
    }

    ClearLOT(&creature->LOT);
    if (item_number != lara.item_number)
        CreateZone(item);
    slots_used++;
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
