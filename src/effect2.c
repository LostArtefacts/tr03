#include "game.h"

extern sint32 BatsTriggered;

void TriggerBats(sint32 x, sint32 y, sint32 z, sint32 angle);

void InitialiseSparks(void)
{
    sint32 lp;
    SPARKS *sptr;

    sptr = &sparks[0];
    for (lp = 0; lp < MAX_SPARKS; lp++)
    {
        sptr->On = 0;
        sptr->Dynamic = -1;
        sptr++;
    }
    for (lp = 0; lp < MAX_SPLASHES; lp++)
        splashes[lp].flags = 0;
    for (lp = 0; lp < MAX_RIPPLES; lp++)
        ripples[lp].flags = 0;
    for (lp = 0; lp < MAX_BATS; lp++)
        bats[lp].flags = 0;
}

void TriggerUnderwaterBlood(sint32 x, sint32 y, sint32 z, sint32 size)
{
    RIPPLE_STRUCT *rptr;
    sint32 lp;

    rptr = &ripples[0];
    for (lp = 0; lp < MAX_RIPPLES; lp++)
    {
        if (!(rptr->flags & SPL_ON))
        {
            rptr->flags = SPL_ON | SPL_TRANS | SPL_MORETRANS | SPL_BLOOD;
            rptr->life = 240 + (GetRandomControl() & 7);
            rptr->init = 1;
            rptr->size = size;
            rptr->x = x + (GetRandomControl() & 63) - 32;
            rptr->y = y;
            rptr->z = z + (GetRandomControl() & 63) - 32;
            break;
        }
        rptr++;
    }
}

void BatEmitterControl(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (item->active)
    {
        TriggerBats(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->pos.y_rot >> 4);
        KillItem(item_number);
        BatsTriggered = 1;
    }
}

void KillAllCurrentItems(sint16 item_number)
{
    KillEverythingFlag = 1;
}

void TriggerDynamic(sint32 x, sint32 y, sint32 z, sint32 falloff, sint32 r, sint32 g, sint32 b)
{
    DYNAMIC *dlptr;

    if (number_dynamics == MAX_DYNAMICS)
        return;

    dlptr = &dynamics[number_dynamics];
    dlptr->on = 1;
    dlptr->x = x;
    dlptr->y = y;
    dlptr->z = z;
    dlptr->falloff = falloff << 8;
    dlptr->r = r;
    dlptr->g = g;
    dlptr->b = b;
    number_dynamics++;
}

void ClearDynamics(void)
{
    sint32 lp;

    number_dynamics = 0;
    for (lp = 0; lp < MAX_DYNAMICS; lp++)
        dynamics[lp].on = 0;
}
