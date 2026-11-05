#include "game.h"

sint32 GetFreeSpark(void);

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
