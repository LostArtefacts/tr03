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

void InitialiseRollingBall(sint16 item_number)
{
    ITEM_INFO *item;
    GAME_VECTOR *old;

    item = &items[item_number];
    old = item->data = game_malloc(sizeof(GAME_VECTOR), ROLLINGBALL_STUFF);
    old->x = item->pos.x_pos;
    old->y = item->pos.y_pos;
    old->z = item->pos.z_pos;
    old->room_number = item->room_number;
}

void TrapDoorCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    if (!OnTrapDoor(item, x, z))
        return;
    if (y > item->pos.y_pos)
    {
        if (item->current_anim_state == DOOR_CLOSED && item->pos.y_pos > *height)
        {
            *height = item->pos.y_pos + STEP_L;
            CeilingObject = item;
        }
    }
}
