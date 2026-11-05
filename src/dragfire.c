#include "game.h"

sint32 GetFreeSpark(void);
void KillEffect(sint16 fx_num);
void EffectNewRoom(sint16 fx_num, sint16 room_number);
int ItemNearLara(PHD_3DPOS *pos, sint32 distance);
void LaraBurn(void);
void TriggerFlamethrowerHitFlame(sint32 x, sint32 y, sint32 z);
void TriggerFlamethrowerSmoke(sint32 x, sint32 y, sint32 z, sint32 uw);

void ControlFlameThrower(sint16 fx_number)
{
    FX_INFO *fx;
    FLOOR_INFO *floor;
    sint16 room_number;
    sint32 speed;

    fx = &effects[fx_number];
    fx->counter--;
    if (!fx->counter) {
        KillEffect(fx_number);
        return;
    }
    speed = (fx->speed * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT;
    fx->pos.z_pos += (speed * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT;
    fx->pos.x_pos += (speed * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT;
    fx->pos.y_pos += -((fx->speed * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT);
    room_number = fx->room_number;
    floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);
    if (room[room_number].flags & UNDERWATER && fx->flag1 == 0) {
        if (GetRandomControl() & 1)
            TriggerFlamethrowerSmoke(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, 1);
        KillEffect(fx_number);
        return;
    }
    if (fx->pos.y_pos >= GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos) ||
        fx->pos.y_pos <= GetCeiling(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos)) {
        if (fx->flag1 == 0) {
            TriggerFlamethrowerHitFlame(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);
            TriggerDynamic(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, 24, 31, 24, GetRandomControl() & 7);
        }
        KillEffect(fx_number);
        return;
    }
    if (room_number != fx->room_number)
        EffectNewRoom(fx_number, room_number);
    if (ItemNearLara(&fx->pos, 350)) {
        if (fx->flag1 == 0) {
            lara_item->hit_points -= 3;
            lara_item->hit_status = 1;
            LaraBurn();
        } else
            lara.poisoned += 4;
        return;
    }
}
