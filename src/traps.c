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
#define LV_OFFICE 12
#define LV_AREA51 15

extern ITEM_INFO *CeilingObject;
extern int OnObject;
extern int height_type;
extern int clonked;
extern uint32 CurrentLevel;

void LaraRapidsDrown(void);
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

void SpringBoardControl(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (item->current_anim_state == 0 && lara_item->pos.y_pos == item->pos.y_pos && (lara_item->pos.x_pos >> WALL_SHIFT) == (item->pos.x_pos >> WALL_SHIFT) && (lara_item->pos.z_pos >> WALL_SHIFT) == (item->pos.z_pos >> WALL_SHIFT)) {
        if (lara_item->hit_points <= 0)
            return;
        if (lara_item->current_anim_state == AS_BACK || lara_item->current_anim_state == AS_FASTBACK)
            lara_item->speed = -lara_item->speed;
        lara_item->fallspeed = -240;
        lara_item->gravity_status = 1;
        lara_item->anim_number = FALLDOWN_A;
        lara_item->frame_number = FALLDOWN_F;
        lara_item->current_anim_state = AS_FORWARDJUMP;
        lara_item->goal_anim_state = AS_FORWARDJUMP;
        item->goal_anim_state = 1;
    }
    AnimateItem(item);
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

int OnTrapDoor(ITEM_INFO *item, sint32 x, sint32 z)
{
    int tx, tz;

    x >>= WALL_SHIFT;
    z >>= WALL_SHIFT;
    tx = item->pos.x_pos >> WALL_SHIFT;
    tz = item->pos.z_pos >> WALL_SHIFT;
    if (item->pos.y_rot == 0 && x == tx && (z == tz || z == tz + 1))
        return 1;
    else if (item->pos.y_rot == -0x8000 && x == tx && (z == tz || z == tz - 1))
        return 1;
    else if (item->pos.y_rot == 0x4000 && z == tz && (x == tx || x == tx + 1))
        return 1;
    else if (item->pos.y_rot == -0x4000 && z == tz && (x == tx || x == tx - 1))
        return 1;
    return 0;
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

void KillEffect(sint16 fx_num);

extern BITE_INFO teeth1a, teeth1b, teeth2a, teeth2b, teeth3a, teeth3b;

void BaddieBiteEffect(ITEM_INFO *item, BITE_INFO *bite);

void TeethTrap(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (TriggerActive(item)) {
        item->goal_anim_state = 1;
        if (item->touch_bits && item->current_anim_state == 1) {
            lara_item->hit_points -= 400;
            lara_item->hit_status = 1;
            BaddieBiteEffect(item, &teeth1a);
            BaddieBiteEffect(item, &teeth1b);
            BaddieBiteEffect(item, &teeth2a);
            BaddieBiteEffect(item, &teeth2b);
            BaddieBiteEffect(item, &teeth3a);
            BaddieBiteEffect(item, &teeth3b);
        }
    } else {
        item->goal_anim_state = 0;
    }
    AnimateItem(item);
}

void FallingCeiling(sint16 item_number)
{
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    sint16 room_number;

    item = &items[item_number];
    if (item->current_anim_state == TRAP_SET) {
        item->goal_anim_state = TRAP_ACTIVATE;
        item->gravity_status = 1;
    } else if (item->current_anim_state == TRAP_ACTIVATE) {
        if (item->touch_bits) {
            lara_item->hit_points -= 300;
            lara_item->hit_status = 1;
        }
    }
    AnimateItem(item);
    if (item->status == DEACTIVATED) {
        RemoveActiveItem(item_number);
        return;
    }
    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    if (room_number != item->room_number)
        ItemNewRoom(item_number, room_number);
    if (item->current_anim_state == TRAP_ACTIVATE && item->pos.y_pos >= item->floor) {
        item->goal_anim_state = 2;
        item->pos.y_pos = item->floor;
        item->fallspeed = 0;
        item->gravity_status = 0;
    }
}

void SideFlameEmitterControl(sint16 item_number)
{
    ITEM_INFO *item;
    FX_INFO *fx;
    sint16 fx_number;

    item = &items[item_number];
    if (TriggerActive(item)) {
        if (item->data == NULL) {
            fx_number = CreateEffect(item->room_number);
            if (fx_number != NO_ITEM) {
                fx = &effects[fx_number];
                fx->pos.x_pos = item->pos.x_pos;
                fx->pos.y_pos = item->pos.y_pos;
                fx->pos.z_pos = item->pos.z_pos;
                fx->pos.y_rot = item->pos.y_rot;
                fx->frame_number = 3;
                fx->object_number = FLAME;
                fx->counter = fx->flag1 = fx->flag2 = 0;
            }
            item->data = (void *)(fx_number + 1);
        }
    } else if (item->data) {
        KillEffect((sint16)((int)item->data - 1));
        item->data = NULL;
    }
}

void FlameEmitterControl(sint16 item_number)
{
    ITEM_INFO *item;
    FX_INFO *fx;
    sint16 fx_number;

    item = &items[item_number];
    if (TriggerActive(item)) {
        if (item->data == NULL) {
            fx_number = CreateEffect(item->room_number);
            if (fx_number != NO_ITEM) {
                fx = &effects[fx_number];
                fx->pos.x_pos = item->pos.x_pos;
                fx->pos.y_pos = item->pos.y_pos;
                fx->pos.z_pos = item->pos.z_pos;
                fx->pos.y_rot = item->pos.y_rot;
                fx->frame_number = 0;
                fx->object_number = FLAME;
                fx->counter = 0;
            }
            item->data = (void *)(fx_number + 1);
        }
    } else if (item->data) {
        KillEffect((sint16)((int)item->data - 1));
        item->data = NULL;
    }
}

void FlameEmitter2Control(sint16 item_number)
{
    ITEM_INFO *item;
    FX_INFO *fx;
    sint16 fx_number;

    item = &items[item_number];
    if (TriggerActive(item)) {
        if (item->data == NULL) {
            fx_number = CreateEffect(item->room_number);
            if (fx_number != NO_ITEM) {
                fx = &effects[fx_number];
                fx->pos.x_pos = item->pos.x_pos;
                fx->pos.y_pos = item->pos.y_pos;
                fx->pos.z_pos = item->pos.z_pos;
                fx->pos.y_rot = item->pos.y_rot;
                fx->frame_number = SMALL_FIRE;
                fx->object_number = FLAME;
                fx->counter = 0;
            }
            item->data = (void *)(fx_number + 1);
        }
    } else if (item->data) {
        KillEffect((sint16)((int)item->data - 1));
        item->data = NULL;
    }
}

void FlameEmitter3Control(sint16 item_number)
{
    ITEM_INFO *item;
    FX_INFO *fx;
    sint16 fx_number;

    item = &items[item_number];
    if (TriggerActive(item)) {
        if (item->data == NULL) {
            fx_number = CreateEffect(item->room_number);
            if (fx_number != NO_ITEM) {
                fx = &effects[fx_number];
                fx->pos.x_pos = item->pos.x_pos;
                fx->pos.y_pos = item->pos.y_pos;
                fx->pos.z_pos = item->pos.z_pos;
                fx->frame_number = 2;
                fx->object_number = FLAME;
                fx->counter = fx->flag1 = 0;
                fx->flag2 = GetRandomControl() & 63;
            }
            item->data = (void *)(fx_number + 1);
        }
    } else if (item->data) {
        KillEffect((sint16)((int)item->data - 1));
        item->data = NULL;
    }
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

void LavaBurn(ITEM_INFO *item)
{
    sint16 room_number;
    FLOOR_INFO *floor;

    if (item->hit_points < 0 || lara.water_status == LARA_CHEAT)
        return;
    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, 32000, item->pos.z_pos, &room_number);
    if (item->floor != GetHeight(floor, item->pos.x_pos, 32000, item->pos.z_pos))
        return;
    if (CurrentLevel == LV_RAPIDS)
    {
        LaraRapidsDrown();
    }
    else
    {
        item->hit_status = 1;
        item->hit_points = -1;
        if (CurrentLevel == LV_AREA51 || CurrentLevel == LV_OFFICE)
            lara.electric = 1;
        else
            LaraBurn();
    }
}
