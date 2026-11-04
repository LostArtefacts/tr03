#ifndef BOX_H
#define BOX_H

#include "items.h"

void InitialiseCreature(sint16 item_number);
void CreatureTilt(ITEM_INFO *item, sint16 angle);
void DisableBaddieAI(sint16 item_number);

#endif
