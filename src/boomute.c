#include "game.h"

extern BITE_INFO seal_gas_boomute;

sint16 TriggerSealmuteGasThrower_boomute(ITEM_INFO *item, BITE_INFO *bite, sint16 speed);

void BoomuteControl(sint16 item_number)
{
    ITEM_INFO *item;
    uint32 length;

    item = &items[item_number];
    if (item->status == DEACTIVATED)
        return;
    if (item->status == ACTIVE) {
        if (item->frame_number >= anims[item->anim_number].frame_base + 1 && item->frame_number <= anims[item->anim_number].frame_end - 8) {
            length = item->frame_number - anims[item->anim_number].frame_base + 1;
            if (length > 24) {
                length = anims[item->anim_number].frame_end - item->frame_number - 8;
                if (length <= 0)
                    length = 1;
                else if (length > 24)
                    length = (GetRandomControl() & 15) + 8;
            }
            if (!length)
                length = 1;
            TriggerSealmuteGasThrower_boomute(item, &seal_gas_boomute, length);
        }
        AnimateItem(item);
    }
}

sint16 CreateEffect(sint16 room_num);
void phd_GetVectorAngles(sint32 x, sint32 y, sint32 z, PHD_ANGLE *dest);
void TriggerSealmuteGas(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv, sint32 fxnum);

sint16 TriggerSealmuteGasThrower_boomute(ITEM_INFO *item, BITE_INFO *bite, sint16 speed)
{
    FX_INFO *fx;
    PHD_VECTOR pos, pos1;
    sint32 lp, s, r, xv, yv, zv;
    sint16 fx_number;
    PHD_ANGLE angles[2];

    fx_number = CreateEffect(item->room_number);
    if (fx_number != NO_ITEM) {
        fx = &effects[fx_number];
        pos.x = bite->x;
        pos.y = bite->y;
        pos.z = bite->z;
        GetJointAbsPosition(item, &pos, bite->mesh_num);
        pos1.x = bite->x;
        pos1.y = bite->y << 1;
        pos1.z = bite->z << 3;
        GetJointAbsPosition(item, &pos1, bite->mesh_num);
        phd_GetVectorAngles(pos1.x - pos.x, pos1.y - pos.y, pos1.z - pos.z, angles);
        fx->pos.x_pos = pos.x;
        fx->pos.y_pos = pos.y;
        fx->pos.z_pos = pos.z;
        fx->room_number = item->room_number;
        fx->pos.x_rot = angles[1];
        fx->pos.z_rot = 0;
        fx->pos.y_rot = angles[0];
        fx->speed = speed << 2;
        fx->object_number = 307;
        fx->counter = 20;
        fx->flag1 = 1;
        TriggerSealmuteGas(0, 0, 0, 0, 0, 0, fx_number);
        for (lp = 0; lp < 2; lp++) {
            s = GetRandomControl() % (speed << 2) + 32;
            r = (s * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT;
            zv = (r * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT;
            xv = (r * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT;
            yv = -((s * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT);
            TriggerSealmuteGas(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, xv << 5, yv << 5, zv << 5, -1);
        }
        r = ((speed << 1) * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT;
        zv = (r * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT;
        xv = (r * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT;
        yv = -(((speed << 1) * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT);
        TriggerSealmuteGas(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, xv << 5, yv << 5, zv << 5, -2);
    }
    return fx_number;
}

sint32 GetFreeSpark(void);

void TriggerSealmuteGas(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv, sint32 fxnum)
{
    sint32 size;
    SPARKS *sptr;

    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 128 + (GetRandomControl() & 63);
    sptr->sG = 128 + (GetRandomControl() & 63);
    sptr->sB = 32;
    sptr->dR = 32 + (GetRandomControl() & 15);
    sptr->dG = 32 + (GetRandomControl() & 15);
    sptr->dB = 0;
    if (xv || yv || zv) {
        sptr->ColFadeSpeed = 6;
        sptr->FadeToBlack = 2;
        sptr->sLife = sptr->Life = (GetRandomControl() & 1) + 16;
    } else {
        sptr->ColFadeSpeed = 8;
        sptr->FadeToBlack = 16;
        sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 28;
    }
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 31) - 16);
    sptr->y = y;
    sptr->z = z + ((GetRandomControl() & 31) - 16);
    sptr->Xvel = ((GetRandomControl() & 15) - 16) + xv;
    sptr->Yvel = yv;
    sptr->Zvel = ((GetRandomControl() & 15) - 16) + zv;
    sptr->Friction = 0;
    if (GetRandomControl() & 1) {
        if (fxnum >= 0)
            sptr->Flags = 2 | 8 | 16 | 512 | 64;
        else
            sptr->Flags = 2 | 8 | 16 | 512;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 15) - 16;
        else
            sptr->RotAdd = (GetRandomControl() & 15) + 16;
    } else {
        if (fxnum >= 0)
            sptr->Flags = 2 | 8 | 512 | 64;
        else
            sptr->Flags = 2 | 8 | 512;
    }
    sptr->FxObj = fxnum;
    sptr->Gravity = sptr->MaxYvel = 0;
    sptr->Def = objects[296].mesh_index;
    size = (GetRandomControl() & 31) + 48;
    if (xv || yv || zv) {
        sptr->sWidth = sptr->Width = size >> 5;
        sptr->sHeight = sptr->Height = size >> 5;
        if (fxnum == -2)
            sptr->Scalar = 2;
        else
            sptr->Scalar = 3;
    } else {
        sptr->sWidth = sptr->Width = size >> 4;
        sptr->sHeight = sptr->Height = size >> 4;
        sptr->Scalar = 4;
    }
    sptr->dWidth = size >> 1;
    sptr->dHeight = size >> 1;
}
