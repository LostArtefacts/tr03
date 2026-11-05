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

void TriggerFlamethrowerHitFlame(sint32 x, sint32 y, sint32 z)
{
    sint32 size;
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - x;
    dz = lara_item->pos.z_pos - z;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 255;
    sptr->sG = 48 + (GetRandomControl() & 31);
    sptr->sB = 48;
    sptr->dR = 192 + (GetRandomControl() & 63);
    sptr->dG = 128 + (GetRandomControl() & 63);
    sptr->dB = 32;
    sptr->ColFadeSpeed = 8 + (GetRandomControl() & 3);
    sptr->FadeToBlack = 8;
    sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 20;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 31) - 16);
    sptr->y = y;
    sptr->z = z + ((GetRandomControl() & 31) - 16);
    sptr->Xvel = (GetRandomControl() & 255) - 128;
    sptr->Yvel = -(GetRandomControl() & 15) - 16;
    sptr->Zvel = (GetRandomControl() & 255) - 128;
    sptr->Friction = 5;
    if (GetRandomControl() & 1) {
        sptr->Gravity = -(GetRandomControl() & 31) - 16;
        sptr->MaxYvel = -(GetRandomControl() & 7) - 16;
        sptr->Flags = 2 | 8 | 16 | 512;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 15) - 16;
        else
            sptr->RotAdd = (GetRandomControl() & 15) + 16;
    } else {
        sptr->Flags = 2 | 8 | 512;
        sptr->Gravity = -(GetRandomControl() & 31) - 16;
        sptr->MaxYvel = -(GetRandomControl() & 7) - 16;
    }
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 2;
    size = (GetRandomControl() & 31) + 128;
    sptr->Width = sptr->sWidth = size;
    sptr->Height = sptr->sHeight = size;
    sptr->dWidth = size >> 4;
    sptr->dHeight = size >> 4;
}
