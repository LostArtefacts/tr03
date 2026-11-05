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

extern uint32 CurrentLevel;

void ControlElectricFence(sint16 item_number)
{
    ITEM_INFO *item;
    sint32 x, z, xsize, zsize;
    sint32 dx, dz, tx, ty, tz, xand, zand;

    item = &items[item_number];
    if (!TriggerActive(item))
        return;

    dx = lara_item->pos.x_pos - item->pos.x_pos;
    dz = lara_item->pos.z_pos - item->pos.z_pos;
    if (dx < -0x5000 || dx > 0x5000 || dz < -0x5000 || dz > 0x5000)
        return;

    switch (item->pos.y_rot) {
    case 0:
        x = item->pos.x_pos + 512;
        z = item->pos.z_pos + 512;
        tx = x - 1024 + 32;
        tz = z - 256;
        xand = 2047;
        zand = 0;
        xsize = 1024 + 32;
        zsize = 128;
        break;
    case 16384:
        x = item->pos.x_pos + 512;
        z = item->pos.z_pos - 512;
        tx = x - 256;
        tz = z - 1024 + 32;
        xand = 0;
        zand = 2047;
        xsize = 128;
        zsize = 1024 + 32;
        break;
    case -32768:
        x = item->pos.x_pos - 512;
        z = item->pos.z_pos - 512;
        tx = x - 1024 + 32;
        tz = z + 256;
        xand = 2047;
        zand = 0;
        xsize = 1024 + 32;
        zsize = 128;
        break;
    case -16384:
        x = item->pos.x_pos - 512;
        z = item->pos.z_pos + 512;
        tx = x + 256;
        tz = z - 1024 + 32;
        xand = 0;
        zand = 2047;
        xsize = 128;
        zsize = 1024 + 32;
        break;
    default:
        x = z = xsize = zsize = tx = tz = xand = zand = 0;
        break;
    }

    if ((GetRandomControl() & 63) == 0) {
        sint32 lp, cnt;

        cnt = (GetRandomControl() & 3) + 3;
        if (xand)
            tx += GetRandomControl() & xand;
        else
            tz += GetRandomControl() & zand;
        if (CurrentLevel != 12)
            ty = item->pos.y_pos - (GetRandomControl() & 2047) - (GetRandomControl() & 1023);
        else
            ty = item->pos.y_pos - (GetRandomControl() & 0x1f);
        for (lp = 0; lp < cnt; lp++) {
            TriggerFenceSparks(tx, ty, tz, 0);
            if (xand)
                tx += ((GetRandomControl() & xand) & 7) - 4;
            else
                tz += ((GetRandomControl() & zand) & 7) - 4;
            ty += (GetRandomControl() & 7) - 4;
        }
    }

    if (lara.electric || CurrentLevel == 12
        || lara_item->pos.x_pos < x - xsize || lara_item->pos.x_pos > x + xsize
        || lara_item->pos.z_pos < z - zsize || lara_item->pos.z_pos > z + zsize
        || lara_item->pos.y_pos > item->pos.y_pos + 32 || lara_item->pos.y_pos < item->pos.y_pos - 3072)
        return;

    {
        sint32 lp, cnt, lp2, cnt2, sx, sz;

        sx = tx;
        sz = tz;
        cnt = (GetRandomControl() & 15) + 3;
        for (lp = 0; lp < cnt; lp++) {
            if (xand)
                tx = lara_item->pos.x_pos + (GetRandomControl() & 511) - 256;
            else
                tz = lara_item->pos.z_pos + (GetRandomControl() & 511) - 256;
            ty = lara_item->pos.y_pos - (GetRandomControl() % 768);
            cnt2 = (GetRandomControl() & 3) + 6;
            for (lp2 = 0; lp2 < cnt2; lp2++) {
                TriggerFenceSparks(tx, ty, tz, 1);
                if (xand)
                    tx += ((GetRandomControl() & xand) & 7) - 4;
                else
                    tz += ((GetRandomControl() & zand) & 7) - 4;
                ty += (GetRandomControl() & 7) - 4;
            }
            tx = sx;
            tz = sz;
        }
    }
    lara.electric = 1;
    lara_item->hit_points = 0;
}
