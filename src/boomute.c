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
