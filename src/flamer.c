#include "game.h"

sint32 GetFreeSpark(void);

void TriggerFlamethrowerFlame(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv, sint32 fxnum);
void phd_GetVectorAngles(sint32 x, sint32 y, sint32 z, PHD_ANGLE *dest);

sint16 TriggerFlameThrower(ITEM_INFO *item, BITE_INFO *bite, sint16 speed)
{
    PHD_VECTOR pos1, pos2;
    FX_INFO *fx;
    sint16 fx_number;
    PHD_ANGLE angles[2];
    sint32 xv, yv, zv, vel, spd, lp;

    fx_number = CreateEffect(item->room_number);
    if (fx_number != NO_ITEM) {
        fx = &effects[fx_number];
        pos1.x = bite->x;
        pos1.y = bite->y;
        pos1.z = bite->z;
        GetJointAbsPosition(item, &pos1, bite->mesh_num);
        pos2.x = bite->x;
        pos2.y = bite->y << 1;
        pos2.z = bite->z;
        GetJointAbsPosition(item, &pos2, bite->mesh_num);
        phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);
        fx->pos.x_pos = pos1.x;
        fx->pos.y_pos = pos1.y;
        fx->pos.z_pos = pos1.z;
        fx->room_number = item->room_number;
        fx->pos.x_rot = angles[1];
        fx->pos.z_rot = 0;
        fx->pos.y_rot = angles[0];
        fx->speed = speed << 2;
        fx->object_number = 307;
        fx->counter = 20;
        fx->flag1 = 0;
        TriggerFlamethrowerFlame(0, 0, 0, 0, 0, 0, fx_number);
        for (lp = 0; lp < 2; lp++) {
            spd = (GetRandomControl() % (speed << 2)) + 32;
            vel = (spd * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT;
            zv = (vel * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT;
            xv = (vel * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT;
            yv = -((spd * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT);
            TriggerFlamethrowerFlame(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, xv << 5, yv << 5, zv << 5, -1);
        }
        vel = ((speed << 1) * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT;
        zv = (vel * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT;
        xv = (vel * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT;
        yv = -(((speed << 1) * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT);
        TriggerFlamethrowerFlame(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, xv << 5, yv << 5, zv << 5, -2);
    }
    return fx_number;
}

void TriggerFlamethrowerFlame(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv, sint32 fxnum)
{
    sint32 size;
    SPARKS *sptr;

    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 48 + (GetRandomControl() & 31);
    sptr->sG = sptr->sR;
    sptr->sB = 192 + (GetRandomControl() & 63);
    sptr->dR = 192 + (GetRandomControl() & 63);
    sptr->dG = 128 + (GetRandomControl() & 63);
    sptr->dB = 32;
    if (xv || yv || zv) {
        sptr->ColFadeSpeed = 6;
        sptr->FadeToBlack = 2;
        sptr->sLife = sptr->Life = (GetRandomControl() & 1) + 12;
    } else {
        sptr->ColFadeSpeed = 8;
        sptr->FadeToBlack = 16;
        sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 20;
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
    size = (GetRandomControl() & 31) + 64;
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

void TriggerPilotFlame(sint32 itemnum)
{
    sint32 size;
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - items[itemnum].pos.x_pos;
    dz = lara_item->pos.z_pos - items[itemnum].pos.z_pos;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 48 + (GetRandomControl() & 31);
    sptr->sG = sptr->sR;
    sptr->sB = 192 + (GetRandomControl() & 63);
    sptr->dR = 192 + (GetRandomControl() & 63);
    sptr->dG = 128 + (GetRandomControl() & 63);
    sptr->dB = 32;
    sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
    sptr->FadeToBlack = 4;
    sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 20;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = (GetRandomControl() & 31) - 16;
    sptr->y = (GetRandomControl() & 31) - 16;
    sptr->z = (GetRandomControl() & 31) - 16;
    sptr->Xvel = (GetRandomControl() & 31) - 16;
    sptr->Yvel = -(GetRandomControl() & 3);
    sptr->Zvel = (GetRandomControl() & 31) - 16;
    sptr->Flags = 2 | 8 | 512 | 128 | 4096;
    sptr->FxObj = itemnum;
    sptr->NodeNumber = 0;
    sptr->Friction = 4;
    sptr->Gravity = -(GetRandomControl() & 3) - 2;
    sptr->MaxYvel = -(GetRandomControl() & 3) - 4;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 0;
    size = (GetRandomControl() & 7) + 32;
    sptr->Width = sptr->sWidth = size >> 1;
    sptr->Height = sptr->sHeight = size >> 1;
    sptr->dWidth = size;
    sptr->dHeight = size;
}
