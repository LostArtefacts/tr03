#include "game.h"

#define STRUT_WAIT 1
#define STRUT_FALL 2

extern char strut_fall;

sint32 GetFreeSpark(void);

void Trigger51RocketSmoke(sint32 x, sint32 y, sint32 z, sint32 yv, sint32 fire)
{
    sint32 size;
    SPARKS *sptr;

    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    if (fire) {
        sptr->sR = 48 + (GetRandomControl() & 31);
        sptr->sG = sptr->sR;
        sptr->sB = 192 + (GetRandomControl() & 63);
        sptr->dR = 192 + (GetRandomControl() & 63);
        sptr->dG = 128 + (GetRandomControl() & 63);
        sptr->dB = 32;
    } else {
        sptr->sR = 0;
        sptr->sG = 0;
        sptr->sB = 0;
        sptr->dR = 32 + (yv >> 5);
        sptr->dG = 32 + (yv >> 5);
        sptr->dB = 32 + (yv >> 5);
    }
    sptr->ColFadeSpeed = 16 - (fire ? (yv >> 9) : 0);
    sptr->FadeToBlack = fire ? 0 : 16;
    sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 60 - (fire ? (yv >> 8) : 0);
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 15) - 8);
    sptr->y = y;
    sptr->z = z + ((GetRandomControl() & 15) - 8);
    sptr->Xvel = (GetRandomControl() & 255) - 128;
    sptr->Yvel = yv + (GetRandomControl() & 15);
    sptr->Zvel = (GetRandomControl() & 255) - 128;
    sptr->Friction = 4;
    if (GetRandomControl() & 1) {
        sptr->Flags = 2 | 8 | 16 | 512;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 15) - 16;
        else
            sptr->RotAdd = (GetRandomControl() & 15) + 16;
    } else
        sptr->Flags = 2 | 8 | 512;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 3;
    sptr->Gravity = sptr->MaxYvel = 0;
    size = (GetRandomControl() & 63) + 64 + (yv >> 5);
    if (size > 255)
        size = 255;
    sptr->Width = sptr->sWidth = size >> 2;
    sptr->dWidth = size;
    sptr->Height = sptr->sHeight = size >> 2;
    sptr->dHeight = size;
}

void Trigger51BlastFire(sint32 x, sint32 y, sint32 z, sint32 smoke, sint32 end)
{
    sint32 size;
    SPARKS *sptr;

    if (end >= 0)
        sptr = &sparks[end];
    else
        sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    if (smoke) {
        sptr->sR = 0;
        sptr->sG = 0;
        sptr->sB = 0;
        sptr->dR = 64;
        sptr->dG = 64;
        sptr->dB = 64;
    } else {
        sptr->sR = 128 + (GetRandomControl() & 31);
        sptr->sG = 64 + (GetRandomControl() & 31);
        sptr->sB = 32;
        sptr->dR = 224 + (GetRandomControl() & 31);
        sptr->dG = 160 + (GetRandomControl() & 31);
        sptr->dB = 32;
    }
    sptr->ColFadeSpeed = 16;
    if (end) {
        sptr->FadeToBlack = 32 + (GetRandomControl() & 31);
        sptr->sLife = sptr->Life = 72 + (end >> 1);
    } else {
        sptr->FadeToBlack = 8 + (smoke ? 24 : 0);
        sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 32 + (smoke ? 32 : 0);
    }
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 15) - 8);
    sptr->y = y;
    sptr->z = z + ((GetRandomControl() & 15) - 8);
    sptr->Xvel = (GetRandomControl() & 255) - 128;
    sptr->Yvel = -(GetRandomControl() & 7);
    sptr->Zvel = (GetRandomControl() & 255) - 128;
    sptr->Friction = 4;
    sptr->Flags = 2 | 8 | 16 | 512;
    sptr->RotAng = GetRandomControl() & 4095;
    if (GetRandomControl() & 1)
        sptr->RotAdd = -(GetRandomControl() & 15) - 16;
    else
        sptr->RotAdd = (GetRandomControl() & 15) + 16;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 4;
    sptr->Gravity = sptr->MaxYvel = 0;
    size = (GetRandomControl() & 63) + 64;
    if (end)
        sptr->Width = sptr->sWidth = sptr->dWidth = sptr->Height = sptr->sHeight = sptr->dHeight = size;
    else {
        sptr->Width = sptr->sWidth = size >> 1;
        sptr->dWidth = size;
        sptr->Height = sptr->sHeight = size >> 1;
        sptr->dHeight = size;
    }
}

void InitialiseArea51Struts(sint16 item_number)
{
    strut_fall = 0;
}

void ControlArea51Struts(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (!strut_fall)
        item->current_anim_state = STRUT_WAIT;
    else if (item->goal_anim_state != STRUT_FALL && item->current_anim_state != STRUT_FALL)
        item->goal_anim_state = STRUT_FALL;
    AnimateItem(item);
}
