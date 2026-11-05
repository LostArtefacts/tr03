#include "game.h"

sint32 GetFreeSpark(void);

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
