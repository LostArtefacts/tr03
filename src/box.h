#ifndef BOX_H
#define BOX_H

#include "items.h"

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint32 mesh_num;
} BITE_INFO;

void InitialiseCreature(sint16 item_number);
void CreatureTilt(ITEM_INFO *item, sint16 angle);
void DisableBaddieAI(sint16 item_number);
int EnableBaddieAI(sint16 item_number, int Always);

#endif
