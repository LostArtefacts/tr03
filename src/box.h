#ifndef BOX_H
#define BOX_H

#include "items.h"

#define NUM_SLOTS 5
#define NUM_NONLOT_SLOTS 12

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint32 mesh_num;
} BITE_INFO;

typedef struct box_node {
    sint16 exit_box;
    uint16 search_number;
    sint16 next_expansion;
    sint16 box_number;
} BOX_NODE;

typedef struct lot_info {
    BOX_NODE *node;
    sint16 head;
    sint16 tail;
    uint16 search_number;
    uint16 block_mask;
    sint16 step;
    sint16 drop;
    sint16 fly;
    sint16 zone_count;
    sint16 target_box;
    sint16 required_box;
    PHD_VECTOR target;
} LOT_INFO;

typedef enum {
    BORED_MOOD,
    ATTACK_MOOD,
    ESCAPE_MOOD,
    STALK_MOOD
} MOOD_TYPE;

typedef struct creature_info {
    sint16 joint_rotation[4];
    sint16 maximum_turn;
    sint16 flags;
    uint16 alerted:1;
    uint16 head_left:1;
    uint16 head_right:1;
    uint16 reached_goal:1;
    uint16 hurt_by_lara:1;
    uint16 patrol2:1;
    MOOD_TYPE mood;
    sint16 item_num;
    PHD_VECTOR target;
    ITEM_INFO *enemy;
    LOT_INFO LOT;
} CREATURE_INFO;

extern CREATURE_INFO *baddie_slots;
extern CREATURE_INFO *non_lot_slots;

void InitialiseCreature(sint16 item_number);
void CreatureTilt(ITEM_INFO *item, sint16 angle);
void DisableBaddieAI(sint16 item_number);
int EnableBaddieAI(sint16 item_number, int Always);

#endif
