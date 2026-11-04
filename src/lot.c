#include "game.h"

typedef struct {
    sint16 exit_box;
    uint16 search_number;
    sint16 next_expansion;
    sint16 box_number;
} BOX_NODE;

typedef struct {
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

extern int number_boxes;

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
