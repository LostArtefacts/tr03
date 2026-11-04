#include "game.h"

#define DOOR_CLOSED 0
#define TRAP_SET 0
#define TRAP_ACTIVATE 1
#define STATUE_STOP 1
#define ROLLINGBALL_STUFF 38

extern ITEM_INFO *CeilingObject;

int OnTrapDoor(ITEM_INFO *item, sint32 x, sint32 z);

void InitialiseBlade(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->anim_number = objects[BLADE].anim_index + 2;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = STATUE_STOP;
}
