#include "game.h"

#define DOOR_CLOSED 0
#define DOOR_OPEN 1
#define WALL 0
#define TRAP_SET 0
#define TRAP_ACTIVATE 1
#define STATUE_STOP 1
#define ROLLINGBALL_STUFF 38
#define SMALL_FIRE 1
#define STATUE_CUT 2
#define HOOK_DAMAGE 50
#define BLADE_DAMAGE 100
#define SFX_ALWAYS 2
#define LV_SHORE 5
#define LV_RAPIDS 7

extern ITEM_INFO *CeilingObject;
extern int OnObject;
extern int height_type;
extern int clonked;
extern uint32 CurrentLevel;

void DoLotsOfBlood(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num, int number);

int OnTrapDoor(ITEM_INFO *item, sint32 x, sint32 z);

void HookControl(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (item->touch_bits && !clonked)
    {
        lara_item->hit_points -= HOOK_DAMAGE;
        lara_item->hit_status = 1;
        DoLotsOfBlood(lara_item->pos.x_pos, lara_item->pos.y_pos - WALL_L / 2, lara_item->pos.z_pos, lara_item->speed, lara_item->pos.y_rot, lara_item->room_number, 3);
    }
    else
        clonked = 0;
    AnimateItem(item);
}

void InitialiseBlade(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->anim_number = objects[BLADE].anim_index + 2;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = STATUE_STOP;
}

void BladeControl(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (TriggerActive(item) && item->current_anim_state == STATUE_STOP)
        item->goal_anim_state = STATUE_CUT;
    else
        item->goal_anim_state = STATUE_STOP;
    if ((item->touch_bits & 2) && item->current_anim_state == STATUE_CUT)
    {
        lara_item->hit_status = 1;
        lara_item->hit_points -= BLADE_DAMAGE;
        DoLotsOfBlood(lara_item->pos.x_pos, item->pos.y_pos - STEP_L, lara_item->pos.z_pos, lara_item->speed, lara_item->pos.y_rot, lara_item->room_number, 2);
    }
    AnimateItem(item);
}

void InitialiseKillerStatue(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->anim_number = objects[item->object_number].anim_index + 3;
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

void SpikeControl(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (!TriggerActive(item))
        return;
    if (CurrentLevel == LV_SHORE || CurrentLevel == LV_RAPIDS)
    {
        if (item->frame_number == anims[item->anim_number].frame_base)
        {
            if (CurrentLevel == LV_SHORE)
                SoundEffect(259, &item->pos, SFX_ALWAYS);
            else
                SoundEffect(34, &item->pos, SFX_ALWAYS);
        }
        AnimateItem(item);
    }
}

void TrapDoorControl(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (TriggerActive(item))
    {
        if (item->current_anim_state == DOOR_CLOSED)
            item->goal_anim_state = DOOR_OPEN;
    }
    else
    {
        if (item->current_anim_state == DOOR_OPEN)
            item->goal_anim_state = DOOR_CLOSED;
    }
    AnimateItem(item);
}

void TrapDoorFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    if (!OnTrapDoor(item, x, z))
        return;
    if (y <= item->pos.y_pos)
    {
        if (item->current_anim_state == DOOR_CLOSED && item->pos.y_pos < *height)
        {
            *height = item->pos.y_pos;
            OnObject = 1;
            height_type = WALL;
            if (item == lara_item)
                lara_item->item_flags[0] = 1;
        }
    }
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

void FallingBlockFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    sint32 origin;

    if (item->object_number == FALLING_PLANK)
        origin = WALL_L;
    else
        origin = STEP_L * 2;
    if (y <= item->pos.y_pos - origin)
        if (item->current_anim_state == TRAP_SET || item->current_anim_state == TRAP_ACTIVATE)
        {
            *height = item->pos.y_pos - origin;
            OnObject = 1;
        }
}

void FallingBlockCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    sint32 origin;

    if (item->object_number == FALLING_PLANK)
        origin = WALL_L;
    else
        origin = STEP_L * 2;
    if (y > item->pos.y_pos - origin)
        if (item->current_anim_state == TRAP_SET || item->current_anim_state == TRAP_ACTIVATE)
            *height = item->pos.y_pos + STEP_L - origin;
}

void LaraBurn(void)
{
    sint16 fx_number;
    FX_INFO *fx;

    if (lara.burn)
        return;

    fx_number = CreateEffect(lara_item->room_number);
    if (fx_number != NO_ITEM)
    {
        fx = &effects[fx_number];
        fx->frame_number = SMALL_FIRE;
        fx->object_number = FLAME;
        fx->counter = -1;
        lara.burn = 1;
    }
}
