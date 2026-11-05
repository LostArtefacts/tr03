#include "game.h"

#define CIVVY_STOP_ANIM 6
#define CIVVY_STOP 1

sint32 GetFreeSpark(void);

void TriggerFenceSparks(sint32 x, sint32 y, sint32 z, sint32 kill)
{
    SPARKS *sptr;

    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sB = (GetRandomControl() & 63) + 192;
    sptr->sR = sptr->sB;
    sptr->sG = sptr->sB;
    sptr->dB = (GetRandomControl() & 63) + 192;
    sptr->dR = sptr->sB >> 2;
    sptr->dG = sptr->sB >> 1;
    sptr->ColFadeSpeed = 8;
    sptr->FadeToBlack = 16;
    sptr->sLife = sptr->Life = 32 + (GetRandomControl() & 7);
    sptr->TransType = 2;
    sptr->Dynamic = -1;
    sptr->x = x;
    sptr->y = y;
    sptr->z = z;
    sptr->Xvel = ((GetRandomControl() & 255) - 128) << 1;
    sptr->Yvel = (GetRandomControl() & 15) - 8 - (kill << 5);
    sptr->Zvel = ((GetRandomControl() & 255) - 128) << 1;
    sptr->Friction = 4;
    sptr->Flags = 2;
    sptr->Scalar = 1 + kill;
    sptr->Width = sptr->sWidth = (GetRandomControl() & 3) + 4;
    sptr->dWidth = sptr->sWidth = 1;
    sptr->Height = sptr->sHeight = sptr->Width;
    sptr->dHeight = sptr->dWidth;
    sptr->Gravity = 16 + (GetRandomControl() & 15);
    sptr->MaxYvel = 0;
}

void InitialiseCivvy(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    InitialiseCreature(item_number);
    item->anim_number = objects[CIVVIE].anim_index + CIVVY_STOP_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = CIVVY_STOP;
}
