#include "game.h"

#define CLEANER_TURN 1024
#define CLEANER_SPEED 64

void InitialiseCleaner(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->pos.x_pos &= ~1023;
    item->pos.z_pos &= ~1023;
    item->pos.x_pos |= 512;
    item->pos.z_pos |= 512;
    item->item_flags[0] = CLEANER_TURN;
    item->item_flags[1] = 0;
    item->item_flags[2] = CLEANER_SPEED;
    item->collidable = 1;
    item->data = NULL;
}

sint32 GetFreeSpark(void);

void TriggerElectricSparks_cleaner(PHD_VECTOR *pos, sint16 item_number, sint16 node)
{
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - pos->x;
    dz = lara_item->pos.z_pos - pos->z;
    if (dx < -0x5000 || dx > 0x5000 || dz < -0x5000 || dz > 0x5000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sB = (GetRandomControl() & 63) + 192;
    sptr->sR = sptr->sB;
    sptr->sG = sptr->sB;
    sptr->dB = (GetRandomControl() & 63) + 192;
    sptr->dR = sptr->sB >> 2;
    sptr->dG = sptr->sB >> 1;
    sptr->ColFadeSpeed = 8;
    sptr->FadeToBlack = 8;
    sptr->Life = (GetRandomControl() & 7) + 20;
    sptr->TransType = 2;
    sptr->Dynamic = -1;
    sptr->sLife = sptr->Life;
    sptr->x = (GetRandomControl() & 31) - 16;
    sptr->y = (GetRandomControl() & 31) - 16;
    sptr->z = (GetRandomControl() & 31) - 16;
    sptr->Xvel = ((GetRandomControl() & 255) << 2) - 512;
    sptr->Yvel = (GetRandomControl() & 7) - 4;
    sptr->Zvel = ((GetRandomControl() & 255) << 2) - 512;
    sptr->Friction = 4;
    sptr->Flags = 0x1082;
    sptr->FxObj = (uint8)item_number;
    sptr->NodeNumber = (uint8)node;
    sptr->Scalar = 1;
    sptr->Width = sptr->sWidth = (GetRandomControl() & 3) + 4;
    sptr->dWidth = sptr->Width >> 1;
    sptr->Height = sptr->sHeight = sptr->Width;
    sptr->dHeight = sptr->Height >> 1;
    sptr->Gravity = (GetRandomControl() & 3) + 4;
    sptr->MaxYvel = 0;
}
