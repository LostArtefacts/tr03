#include "game.h"

extern sint32 BatsTriggered;
extern sint32 next_spark;

void TriggerBats(sint32 x, sint32 y, sint32 z, sint32 angle);

sint32 GetFreeSpark(void)
{
    sint32 lp, eldest, free;
    SPARKS *sptr;

    sptr = &sparks[next_spark];
    for (free = next_spark, lp = 0; lp < MAX_SPARKS; lp++)
    {
        if (sptr->On == 0)
        {
            next_spark = (free + 1) & (MAX_SPARKS - 1);
            return free;
        }
        if (free == MAX_SPARKS - 1)
        {
            sptr = &sparks[0];
            free = 0;
        }
        else
        {
            free++;
            sptr++;
        }
    }
    eldest = 0xfff;
    free = 0;
    sptr = &sparks[0];
    for (lp = 0; lp < MAX_SPARKS; lp++)
    {
        if (sptr->Life < eldest && sptr->Dynamic == -1)
        {
            if (!(sptr->Flags & SP_BLOOD) || (lp & 1))
            {
                free = lp;
                eldest = sptr->Life;
            }
        }
        sptr++;
    }
    next_spark = (free + 1) & (MAX_SPARKS - 1);
    return free;
}

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

RIPPLE_STRUCT *SetupRipple(sint32 x, sint32 y, sint32 z, sint32 size, sint32 moretrans)
{
    RIPPLE_STRUCT *rptr;
    sint32 lp;

    rptr = &ripples[0];
    for (lp = 0; lp < MAX_RIPPLES; lp++)
    {
        if (!(rptr->flags & SPL_ON))
        {
            if (size < 0)
            {
                if (moretrans)
                    rptr->flags = SPL_ON | SPL_TRANS | SPL_MORETRANS;
                else
                    rptr->flags = SPL_ON | SPL_TRANS;
                size = -size;
            }
            else
                rptr->flags = SPL_ON;
            rptr->life = 48 + (GetRandomControl() & 15);
            rptr->init = 1;
            rptr->size = size;
            rptr->x = x + (GetRandomControl() & 127) - 64;
            rptr->y = y;
            rptr->z = z + (GetRandomControl() & 127) - 64;
            break;
        }
        rptr++;
    }
    return rptr;
}

extern sint16 rcossin_tbl[];

int LOS(GAME_VECTOR *start, GAME_VECTOR *target);

void TriggerAlertLight(sint32 x, sint32 y, sint32 z, sint32 r, sint32 g, sint32 b, sint32 angle, sint16 room_no)
{
    sint32 sin, cos;
    GAME_VECTOR src, target;

    src.x = x;
    src.y = y;
    src.z = z;
    GetFloor(x, y, z, &room_no);
    src.room_number = room_no;
    sin = ((sint16)rcossin_tbl[angle << 1]) << 1;
    cos = ((sint16)rcossin_tbl[(angle << 1) + 1]) << 1;
    target.x = x + sin;
    target.y = y;
    target.z = z + cos;
    if (!LOS(&src, &target))
        TriggerDynamic(target.x, target.y, target.z, 8, r, g, b);
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

void TriggerUnderwaterBloodD(sint32 x, sint32 y, sint32 z, sint32 size)
{
    RIPPLE_STRUCT *rptr;
    sint32 lp;

    rptr = &ripples[0];
    for (lp = 0; lp < MAX_RIPPLES; lp++)
    {
        if (!(rptr->flags & SPL_ON))
        {
            rptr->flags = SPL_ON | SPL_TRANS | SPL_MORETRANS | SPL_BLOOD;
            rptr->life = 240 + (GetRandomDraw() & 7);
            rptr->init = 1;
            rptr->size = size;
            rptr->x = x + (GetRandomDraw() & 63) - 32;
            rptr->y = y;
            rptr->z = z + (GetRandomDraw() & 63) - 32;
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
