#include "game.h"

extern sint32 BatsTriggered;
extern sint32 next_spark;

void TriggerBats(sint32 x, sint32 y, sint32 z, sint32 angle);

void DetatchSpark(sint32 num, sint32 type)
{
    sint32 lp;
    SPARKS *sptr;
    FX_INFO *fx;
    ITEM_INFO *item;

    sptr = &sparks[0];
    for (lp = 0; lp < MAX_SPARKS; lp++) {
        if (sptr->On && (sptr->Flags & type) && sptr->FxObj == num) {
            if (type == 64) {
                if (sptr->Flags & 1024) {
                    sptr->On = 0;
                } else {
                    fx = &effects[num];
                    sptr->x += fx->pos.x_pos;
                    sptr->y += fx->pos.y_pos;
                    sptr->z += fx->pos.z_pos;
                    sptr->Flags &= ~64;
                }
            } else if (type == 128) {
                if (sptr->Flags & 1024) {
                    sptr->On = 0;
                } else {
                    item = &items[num];
                    sptr->x += item->pos.x_pos;
                    sptr->y += item->pos.y_pos;
                    sptr->z += item->pos.z_pos;
                    sptr->Flags &= ~128;
                }
            }
        }
        sptr++;
    }
}

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

int abs(int x);
#pragma intrinsic(abs)

void TriggerRicochetSpark(GAME_VECTOR *pos, sint32 angle, sint32 size)
{
    sint32 lp1;
    sint32 tangle;
    SPARKS *sptr;

    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 255;
    sptr->sG = 32 + (GetRandomControl() & 31);
    sptr->sB = 0;
    sptr->dR = 192;
    sptr->dG = 96 + (GetRandomControl() & 63);
    sptr->dB = 0;
    sptr->ColFadeSpeed = 8;
    sptr->FadeToBlack = 8;
    sptr->sLife = sptr->Life = 24;
    sptr->TransType = 2;
    sptr->Dynamic = -1;
    sptr->x = pos->x;
    sptr->y = pos->y;
    sptr->z = pos->z;
    tangle = (angle + ((GetRandomControl() & 2047) - 1024)) & 4095;
    sptr->Zvel = rcossin_tbl[(tangle << 1) + 1] >> 1;
    sptr->Xvel = -rcossin_tbl[tangle << 1] >> 1;
    sptr->Yvel = ((GetRandomControl() & 511) - 384) << 1;
    sptr->Friction = 1;
    sptr->Flags = 2;
    sptr->Scalar = 3;
    sptr->Gravity = (GetRandomControl() & 31) + abs(sptr->Yvel >> 6);
    sptr->Width = sptr->sWidth = (GetRandomControl() & 3) + 4;
    sptr->dWidth = (GetRandomControl() & 1) + 1;
    sptr->Height = sptr->sHeight = (GetRandomControl() & 3) + 4;
    sptr->dHeight = (GetRandomControl() & 1) + 1;
    sptr->MaxYvel = 0;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    lp1 = 128 + (GetRandomControl() & 63);
    sptr->sR = lp1;
    sptr->sG = lp1;
    sptr->sB = lp1;
    sptr->dR = lp1 >> 1;
    sptr->dG = lp1 >> 1;
    sptr->dB = lp1 >> 1;
    sptr->ColFadeSpeed = 8;
    sptr->FadeToBlack = 16;
    sptr->sLife = sptr->Life = 28;
    sptr->TransType = 3;
    sptr->Dynamic = -1;
    sptr->x = pos->x;
    sptr->y = pos->y;
    sptr->z = pos->z;
    tangle = (angle + ((GetRandomControl() & 2047) - 1023)) & 4095;
    sptr->Zvel = rcossin_tbl[(tangle << 1) + 1] >> 1;
    sptr->Xvel = -rcossin_tbl[tangle << 1] >> 1;
    sptr->Yvel = (GetRandomControl() & 511) - 384;
    sptr->Friction = 1 | 32;
    sptr->Flags = 2;
    sptr->Scalar = 3;
    sptr->Gravity = (GetRandomControl() & 7) + 4;
    sptr->Width = sptr->sWidth = (GetRandomControl() & 3) + 4;
    sptr->dWidth = (GetRandomControl() & 1) + 1;
    sptr->Height = sptr->sHeight = (GetRandomControl() & 3) + 4;
    sptr->dHeight = (GetRandomControl() & 1) + 1;
    sptr->MaxYvel = 0;
}

void TriggerGunSmoke(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv, sint32 initial, sint32 weapon, sint32 count)
{
    sint32 size;
    SPARKS *sptr;

    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 0;
    sptr->sG = 0;
    sptr->sB = 0;
    sptr->dR = count << 2;
    sptr->dG = count << 2;
    sptr->dB = count << 2;
    sptr->ColFadeSpeed = 4;
    sptr->FadeToBlack = 32 - (initial << 4);
    sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 40;
    if (weapon == LG_PISTOLS || weapon == LG_MAGNUMS || weapon == LG_UZIS) {
        if (sptr->dR > 64) {
            sptr->dR = 64;
            sptr->dG = 64;
            sptr->dB = 64;
        }
    }
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + (GetRandomControl() & 31) - 16;
    sptr->y = y + (GetRandomControl() & 31) - 16;
    sptr->z = z + (GetRandomControl() & 31) - 16;
    if (initial) {
        sptr->Xvel = ((GetRandomControl() & 1023) - 512) + xv;
        sptr->Yvel = ((GetRandomControl() & 1023) - 512) + yv;
        sptr->Zvel = ((GetRandomControl() & 1023) - 512) + zv;
    } else {
        sptr->Xvel = ((GetRandomControl() & 511) - 256) >> 1;
        sptr->Yvel = ((GetRandomControl() & 511) - 256) >> 1;
        sptr->Zvel = ((GetRandomControl() & 511) - 256) >> 1;
    }
    sptr->Friction = 4;
    if (GetRandomControl() & 1) {
        if (room[lara_item->room_number].flags & 0x20)
            sptr->Flags = 2 | 8 | 16 | 256 | 512;
        else
            sptr->Flags = 2 | 8 | 16 | 512;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 15) - 16;
        else
            sptr->RotAdd = (GetRandomControl() & 15) + 16;
    } else {
        if (room[lara_item->room_number].flags & 0x20)
            sptr->Flags = 2 | 8 | 256 | 512;
        else
            sptr->Flags = 2 | 8 | 512;
    }
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 3;
    sptr->Gravity = -(GetRandomControl() & 1) - 2;
    sptr->MaxYvel = -(GetRandomControl() & 1) - 2;
    size = (GetRandomControl() & 7) + 24 - ((weapon != LG_ROCKET && weapon != LG_GRENADE) ? 12 : 0);
    if (initial) {
        sptr->Width = sptr->sWidth = size >> 1;
        sptr->dWidth = (size << 1) + 8;
    } else {
        sptr->Width = sptr->sWidth = size >> 2;
        sptr->dWidth = size;
    }
    if (initial) {
        sptr->Height = sptr->sHeight = size >> 1;
        sptr->dHeight = (size << 1) + 8;
    } else {
        sptr->Height = sptr->sHeight = size >> 2;
        sptr->dHeight = size;
    }
}

void TriggerExplosionSmokeEnd(sint32 x, sint32 y, sint32 z, sint32 uw)
{
    sint32 size;
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - x;
    dz = lara_item->pos.z_pos - z;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    if (uw) {
        sptr->sR = 0;
        sptr->sG = 0;
        sptr->sB = 0;
        sptr->dR = 192;
        sptr->dG = 192;
        sptr->dB = 208;
    } else {
        sptr->sR = 144;
        sptr->sG = 144;
        sptr->sB = 144;
        sptr->dR = 64;
        sptr->dG = 64;
        sptr->dB = 64;
    }
    sptr->ColFadeSpeed = 8;
    sptr->FadeToBlack = 64;
    sptr->sLife = sptr->Life = (GetRandomControl() & 31) + 96;
    if (uw)
        sptr->TransType = 2;
    else
        sptr->TransType = 3;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + (GetRandomControl() & 31) - 16;
    sptr->y = y + (GetRandomControl() & 31) - 16;
    sptr->z = z + (GetRandomControl() & 31) - 16;
    sptr->Xvel = ((GetRandomControl() & 4095) - 2048) >> 2;
    sptr->Yvel = (GetRandomControl() & 255) - 128;
    sptr->Zvel = ((GetRandomControl() & 4095) - 2048) >> 2;
    if (uw) {
        sptr->Yvel >>= 4;
        sptr->y += 32;
        sptr->Friction = 4 | (1 << 4);
    } else
        sptr->Friction = 6;
    sptr->Flags = 2 | 8 | 16 | 512;
    sptr->RotAng = GetRandomControl() & 4095;
    if (GetRandomControl() & 1)
        sptr->RotAdd = -(GetRandomControl() & 15) - 16;
    else
        sptr->RotAdd = (GetRandomControl() & 15) + 16;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 3;
    if (uw)
        sptr->Gravity = sptr->MaxYvel = 0;
    else {
        sptr->Gravity = -(GetRandomControl() & 3) - 3;
        sptr->MaxYvel = -(GetRandomControl() & 3) - 4;
    }
    size = (GetRandomControl() & 31) + 128;
    sptr->Width = sptr->sWidth = size >> 2;
    sptr->dWidth = size;
    size += (GetRandomControl() & 31) + 32;
    sptr->Height = sptr->sHeight = size >> 3;
    sptr->dHeight = size;
}

void TriggerExplosionSmoke(sint32 x, sint32 y, sint32 z, sint32 uw)
{
    sint32 size;
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - x;
    dz = lara_item->pos.z_pos - z;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 144;
    sptr->sG = 144;
    sptr->sB = 144;
    sptr->dR = 64;
    sptr->dG = 64;
    sptr->dB = 64;
    sptr->ColFadeSpeed = 2;
    sptr->FadeToBlack = 8;
    sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 10;
    sptr->TransType = 3;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + (GetRandomControl() & 511) - 256;
    sptr->y = y + (GetRandomControl() & 511) - 256;
    sptr->z = z + (GetRandomControl() & 511) - 256;
    sptr->Xvel = ((GetRandomControl() & 4095) - 2048) >> 2;
    sptr->Yvel = (GetRandomControl() & 255) - 128;
    sptr->Zvel = ((GetRandomControl() & 4095) - 2048) >> 2;
    if (uw)
        sptr->Friction = 2;
    else
        sptr->Friction = 6;
    sptr->Flags = 2 | 8 | 16 | 512;
    sptr->RotAng = GetRandomControl() & 4095;
    sptr->RotAdd = (GetRandomControl() & 15) + 16;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 1;
    sptr->Gravity = -(GetRandomControl() & 3) - 3;
    sptr->MaxYvel = -(GetRandomControl() & 3) - 4;
    size = (GetRandomControl() & 31) + 128;
    sptr->Width = sptr->sWidth = size >> 2;
    sptr->dWidth = size;
    size += (GetRandomControl() & 31) + 32;
    sptr->Height = sptr->sHeight = size >> 3;
    sptr->dHeight = size;
}

void TriggerFireSmoke(sint32 x, sint32 y, sint32 z, sint32 body_part, sint32 type)
{
    sint32 size;
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - x;
    dz = lara_item->pos.z_pos - z;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 0;
    sptr->sG = 0;
    sptr->sB = 0;
    sptr->dR = 32;
    sptr->dG = 32;
    sptr->dB = 32;
    if (body_part != -1) {
        sptr->ColFadeSpeed = 4 + (GetRandomControl() & 3);
        sptr->FadeToBlack = 12;
        sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 20;
    } else {
        if (type == 255) {
            sptr->ColFadeSpeed = 16 + (GetRandomControl() & 3);
            sptr->FadeToBlack = 8;
            sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 28;
        } else {
            sptr->ColFadeSpeed = 32 + (GetRandomControl() & 7);
            sptr->FadeToBlack = 16;
            sptr->sLife = sptr->Life = (GetRandomControl() & 15) + 57;
        }
    }
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 15) - 8);
    sptr->y = y - (GetRandomControl() & 127) - 256;
    sptr->z = z + ((GetRandomControl() & 15) - 8);
    sptr->Xvel = (GetRandomControl() & 255) - 128;
    sptr->Yvel = -(GetRandomControl() & 15) - 16;
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
    sptr->Gravity = -(GetRandomControl() & 15) - 16;
    sptr->MaxYvel = -(GetRandomControl() & 7) - 8;
    size = (GetRandomControl() & 63) + 64;
    sptr->Width = sptr->sWidth = size >> 2;
    sptr->dWidth = size;
    sptr->Height = sptr->sHeight = size >> 2;
    sptr->dHeight = size;
}

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

void TriggerShotgunSparks(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv)
{
    SPARKS *sptr;

    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 255;
    sptr->sG = 255;
    sptr->sB = 0;
    sptr->dR = 255;
    sptr->dG = (GetRandomControl() & 127) + 64;
    sptr->dB = 0;
    sptr->ColFadeSpeed = 3;
    sptr->FadeToBlack = 5;
    sptr->sLife = sptr->Life = 10;
    sptr->TransType = 2;
    sptr->Dynamic = -1;
    sptr->x = x + (GetRandomControl() & 7) - 3;
    sptr->y = y + (GetRandomControl() & 7) - 3;
    sptr->z = z + (GetRandomControl() & 7) - 3;
    sptr->Xvel = xv + ((GetRandomControl() & 511) - 256);
    sptr->Yvel = yv + ((GetRandomControl() & 511) - 256);
    sptr->Zvel = zv + ((GetRandomControl() & 511) - 256);
    sptr->Friction = 0;
    sptr->Flags = 2;
    sptr->Scalar = 2;
    sptr->Width = sptr->sWidth = (GetRandomControl() & 3) + 4;
    sptr->dWidth = 1;
    sptr->Height = sptr->sHeight = (GetRandomControl() & 3) + 4;
    sptr->dHeight = 1;
    sptr->Gravity = sptr->MaxYvel = 0;
}

void TriggerRocketFlame(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv, sint32 item_no)
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
    sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
    sptr->FadeToBlack = 12;
    sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 28;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 31) - 16);
    sptr->y = y;
    sptr->z = z + ((GetRandomControl() & 31) - 16);
    sptr->Xvel = xv;
    sptr->Yvel = yv;
    sptr->Zvel = zv;
    sptr->Friction = 3 | (3 << 4);
    if (GetRandomControl() & 1) {
        sptr->Flags = 2 | 8 | 16 | 128 | 512;
        sptr->FxObj = item_no;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 15) - 16;
        else
            sptr->RotAdd = (GetRandomControl() & 15) + 16;
    } else {
        sptr->Flags = 2 | 8 | 128 | 512;
        sptr->FxObj = item_no;
    }
    sptr->Gravity = 0;
    sptr->MaxYvel = 0;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 2;
    size = (GetRandomControl() & 7) + 32;
    sptr->Width = sptr->sWidth = size;
    sptr->dWidth = 2;
    sptr->Height = sptr->sHeight = size;
    sptr->dHeight = 2;
}

void TriggerRocketSmoke(sint32 x, sint32 y, sint32 z, sint32 body_part)
{
    sint32 size;
    SPARKS *sptr;

    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 0;
    sptr->sG = 0;
    sptr->sB = 0;
    sptr->dR = 64 + body_part;
    sptr->dG = 64 + body_part;
    sptr->dB = 64 + body_part;
    sptr->ColFadeSpeed = 4 + (GetRandomControl() & 3);
    sptr->FadeToBlack = 12;
    sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 20;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 15) - 8);
    sptr->y = y + ((GetRandomControl() & 15) - 8);
    sptr->z = z + ((GetRandomControl() & 15) - 8);
    sptr->Xvel = (GetRandomControl() & 255) - 128;
    sptr->Yvel = -(GetRandomControl() & 3) - 4;
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
    sptr->Gravity = -(GetRandomControl() & 3) - 4;
    sptr->MaxYvel = -(GetRandomControl() & 3) - 4;
    size = (GetRandomControl() & 7) + 32;
    sptr->Width = sptr->sWidth = size >> 2;
    sptr->dWidth = size;
    sptr->Height = sptr->sHeight = size >> 2;
    sptr->dHeight = size;
}

void TriggerBlood(sint32 x, sint32 y, sint32 z, sint32 angle, sint32 num)
{
    sint32 lp1;
    sint32 tangle, tspeed;
    SPARKS *sptr;

    for (lp1 = 0; lp1 < num; lp1++) {
        sptr = &sparks[GetFreeSpark()];
        sptr->On = 1;
        if (gameflow.language == 2) {
            sptr->sB = 224;
            sptr->sG = 0;
            sptr->sR = 112;
            sptr->dB = 192;
            sptr->dG = 0;
            sptr->dR = 96;
        } else {
            sptr->sR = 224;
            sptr->sG = 0;
            sptr->sB = 32;
            sptr->dR = 192;
            sptr->dG = 0;
            sptr->dB = 24;
        }
        sptr->ColFadeSpeed = 8;
        sptr->FadeToBlack = 8;
        sptr->sLife = sptr->Life = 24;
        sptr->TransType = 1;
        sptr->Dynamic = -1;
        sptr->x = x + ((GetRandomControl() & 31) - 16);
        sptr->y = y + ((GetRandomControl() & 31) - 16);
        sptr->z = z + ((GetRandomControl() & 31) - 16);
        tangle = (angle + ((GetRandomControl() & 31) - 16)) & 4095;
        tspeed = GetRandomControl() & 15;
        sptr->Zvel = (rcossin_tbl[(tangle << 1) + 1] * tspeed) >> 5;
        sptr->Xvel = (-rcossin_tbl[tangle << 1] * tspeed) >> 5;
        sptr->Yvel = -((GetRandomControl() & 255) + 128);
        sptr->Friction = 4;
        sptr->Flags = SP_BLOOD;
        sptr->Scalar = 3;
        sptr->Gravity = (GetRandomControl() & 31) + 31;
        sptr->Width = sptr->sWidth = 2;
        sptr->dWidth = 2 - (GetRandomControl() & 1);
        sptr->Height = sptr->sHeight = 2;
        sptr->dHeight = 2 - (GetRandomControl() & 1);
        sptr->MaxYvel = 0;
        sptr++;
    }
}

void TriggerBloodD(sint32 x, sint32 y, sint32 z, sint32 angle, sint32 num)
{
    sint32 lp1;
    sint32 tangle, tspeed;
    SPARKS *sptr;

    for (lp1 = 0; lp1 < num; lp1++) {
        sptr = &sparks[GetFreeSpark()];
        sptr->On = 1;
        if (gameflow.language == 2) {
            sptr->sB = 224;
            sptr->sG = 0;
            sptr->sR = 112;
            sptr->dB = 192;
            sptr->dG = 0;
            sptr->dR = 96;
        } else {
            sptr->sR = 224;
            sptr->sG = 0;
            sptr->sB = 32;
            sptr->dR = 192;
            sptr->dG = 0;
            sptr->dB = 24;
        }
        sptr->ColFadeSpeed = 8;
        sptr->FadeToBlack = 8;
        sptr->sLife = sptr->Life = 24;
        sptr->TransType = 1;
        sptr->Dynamic = -1;
        sptr->x = x + ((GetRandomDraw() & 31) - 16);
        sptr->y = y + ((GetRandomDraw() & 31) - 16);
        sptr->z = z + ((GetRandomDraw() & 31) - 16);
        tangle = (angle + ((GetRandomDraw() & 31) - 16)) & 4095;
        tspeed = GetRandomDraw() & 15;
        sptr->Zvel = (rcossin_tbl[(tangle << 1) + 1] * tspeed) >> 5;
        sptr->Xvel = (-rcossin_tbl[tangle << 1] * tspeed) >> 5;
        sptr->Yvel = -((GetRandomDraw() & 255) + 128);
        sptr->Friction = 4;
        sptr->Flags = 0;
        sptr->Scalar = 3;
        sptr->Gravity = (GetRandomDraw() & 31) + 31;
        sptr->Width = sptr->sWidth = 2;
        sptr->dWidth = 2 - (GetRandomDraw() & 1);
        sptr->Height = sptr->sHeight = 2;
        sptr->dHeight = 2 - (GetRandomDraw() & 1);
        sptr->MaxYvel = 0;
        sptr++;
    }
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

extern sint32 wibble;

void UpdateBats(void)
{
    sint32 lp;
    BAT_STRUCT *bptr;

    bptr = &bats[0];
    for (lp = 0; lp < MAX_BATS; lp++) {
        if (bptr->flags & 1) {
            if ((lp & 3) == 0 && (GetRandomControl() & 7) == 0) {
                PHD_3DPOS bpos;

                bpos.x_pos = bptr->x;
                bpos.y_pos = bptr->y;
                bpos.z_pos = bptr->z;
                SoundEffect(157, &bpos, 0);
            }

            bptr->x -= (rcossin_tbl[(bptr->angle << 1) + 1] * bptr->speed) >> 14;
            bptr->y -= GetRandomControl() & 3;
            bptr->z += (rcossin_tbl[bptr->angle << 1] * bptr->speed) >> 14;
            bptr->WingYoff += 11;
            bptr->WingYoff &= 63;
            if (bptr->life < 128) {
                bptr->y -= (lp >> 1) + 4;
                if ((GetRandomControl() & 3) == 0) {
                    bptr->angle += (GetRandomControl() & 255) - 128;
                    bptr->angle &= 4095;
                    bptr->speed += GetRandomControl() & 3;
                }
            }

            bptr->speed += 12;
            if (bptr->speed > 300)
                bptr->speed = 300;

            if (bptr->life && (wibble & 4)) {
                bptr->life--;
                if (!bptr->life)
                    bptr->flags = 0;
            }
        }
        bptr++;
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

void TriggerBats(sint32 x, sint32 y, sint32 z, sint32 angle)
{
    sint32 lp;
    BAT_STRUCT *bptr;

    angle -= 0x400;
    angle &= 4095;
    bptr = &bats[0];
    for (lp = 0; lp < MAX_BATS; lp++) {
        bptr->x = x + (GetRandomControl() & 511) - 256;
        bptr->y = y + 256 - (GetRandomControl() & 255);
        bptr->z = z + (GetRandomControl() & 511) - 256;
        bptr->angle = (angle + (GetRandomControl() & 127) - 64) & 4095;
        bptr->speed = (GetRandomControl() & 31) + 64;
        bptr->WingYoff = GetRandomControl() & 63;
        bptr->flags |= 1;
        bptr->life = 144 + (GetRandomControl() & 7);
        bptr++;
    }
}

void TriggerStaticFlame(sint32 x, sint32 y, sint32 z, sint32 size)
{
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - x;
    dz = lara_item->pos.z_pos - z;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;

    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = sptr->dR = 192 + (GetRandomControl() & 63);
    sptr->sG = sptr->dG = 128 + (GetRandomControl() & 63);
    sptr->sB = sptr->dB = 64;
    sptr->ColFadeSpeed = 1;
    sptr->FadeToBlack = 0;
    sptr->sLife = sptr->Life = 2;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 7) - 4);
    sptr->y = y;
    sptr->z = z + ((GetRandomControl() & 7) - 4);
    sptr->Xvel = sptr->Yvel = sptr->Zvel = sptr->Friction = sptr->Gravity = sptr->MaxYvel = 0;
    sptr->Flags = 2 | 8 | 512;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 2;
    sptr->Width = sptr->sWidth = size;
    sptr->dWidth = size;
    sptr->Height = sptr->sHeight = size;
    sptr->dHeight = size;
}

void KillAllCurrentItems(sint16 item_number)
{
    KillEverythingFlag = 1;
}

typedef struct {
    sint16 angle;
    uint8 speed;
    uint8 on;
    sint16 angle_time;
    sint16 speed_time;
    sint16 Xrange;
    sint16 Zrange;
    sint16 Yrange;
} LEADER_INFO;

extern LEADER_INFO lead_info[];
extern sint16 next_item_active;
void KillEffect(sint16 fx_num);

void KillEverything(void)
{
    sint16 item_num, nex, obj_num;

    item_num = next_item_active;
    while (item_num != NO_ITEM) {
        nex = items[item_num].next_active;
        obj_num = items[item_num].object_number;
        if (obj_num != LARA && obj_num != FLARE && (obj_num < 131 || obj_num > 138) && items[item_num].active
            && !(items[item_num].flags & REVERSE))
            KillItem(item_num);
        item_num = nex;
    }

    item_num = next_fx_active;
    while (item_num != NO_ITEM) {
        nex = effects[item_num].next_active;
        if (objects[effects[item_num].object_number].control
            && !(effects[item_num].object_number == 329 && effects[item_num].counter < 0))
            KillEffect(item_num);
        item_num = nex;
    }

    for (item_num = 0; item_num < 8; item_num++)
        lead_info[item_num].on = 0;

    KillEverythingFlag = 0;
}

void TriggerBubble(sint32 x, sint32 y, sint32 z, sint32 size, sint32 sizerange, sint16 itemnum)
{
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - x;
    dz = lara_item->pos.z_pos - z;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;

    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 0;
    sptr->sG = 0;
    sptr->sB = 0;
    sptr->dR = 144;
    sptr->dG = 144;
    sptr->dB = 144;
    sptr->ColFadeSpeed = 4;
    sptr->FadeToBlack = 2;
    sptr->sLife = sptr->Life = 128;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = 0;
    sptr->y = 0;
    sptr->z = 0;
    sptr->Xvel = 0;
    sptr->Yvel = 0;
    sptr->Zvel = 0;
    sptr->Friction = 0;
    sptr->Flags = 2 | 8 | 1024 | 64;
    sptr->FxObj = itemnum;
    sptr->Def = objects[297].mesh_index;
    sptr->Scalar = 0;
    sptr->Gravity = 0;
    sptr->MaxYvel = 0;
    size += GetRandomControl() % sizerange;
    sptr->Width = sptr->sWidth = size;
    sptr->dWidth = size << 3;
    sptr->Height = sptr->sHeight = size;
    sptr->dHeight = size << 3;
}

void CreateBubble(PHD_3DPOS *pos, sint16 room_number, int size, int sizerange);

void TriggerExplosionBubble(sint32 x, sint32 y, sint32 z, sint16 roomnum)
{
    SPARKS *sptr;
    sint32 dx, dz, size;

    dx = lara_item->pos.x_pos - x;
    dz = lara_item->pos.z_pos - z;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;

    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 128;
    sptr->sG = 64;
    sptr->sB = 0;
    sptr->dR = 128;
    sptr->dG = 128;
    sptr->dB = 128;
    sptr->ColFadeSpeed = 8;
    sptr->FadeToBlack = 12;
    sptr->sLife = sptr->Life = 24;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x;
    sptr->y = y;
    sptr->z = z;
    sptr->Xvel = 0;
    sptr->Yvel = 0;
    sptr->Zvel = 0;
    sptr->Friction = 0;
    sptr->Flags = 2 | 8 | 2048;
    sptr->Def = objects[297].mesh_index;
    sptr->Scalar = 3;
    sptr->Gravity = 0;
    sptr->MaxYvel = 0;
    size = (GetRandomControl() & 7) + 63;
    sptr->Width = sptr->sWidth = size >> 1;
    sptr->dWidth = size << 1;
    sptr->Height = sptr->sHeight = size >> 1;
    sptr->dHeight = size << 1;

    for (dx = 0; dx < 7; dx++) {
        PHD_3DPOS pos;

        pos.x_pos = x + (GetRandomControl() & 511) - 256;
        pos.y_pos = y + (GetRandomControl() & 127) - 64;
        pos.z_pos = z + (GetRandomControl() & 511) - 256;
        CreateBubble(&pos, roomnum, 6, 16);
    }
}

void TriggerBreath(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv)
{
    sint32 size;
    SPARKS *sptr;

    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 0;
    sptr->sG = 0;
    sptr->sB = 0;
    sptr->dR = 32;
    sptr->dG = 32;
    sptr->dB = 32;
    sptr->ColFadeSpeed = 4;
    sptr->FadeToBlack = 32;
    sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 37;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 15) - 8);
    sptr->y = y + ((GetRandomControl() & 15) - 8);
    sptr->z = z + ((GetRandomControl() & 15) - 8);
    sptr->Xvel = xv;
    sptr->Yvel = yv;
    sptr->Zvel = zv;
    sptr->Friction = 0;
    if (room[lara_item->room_number].flags & 32)
        sptr->Flags = 2 | 8 | 512 | 256;
    else
        sptr->Flags = 2 | 8 | 512;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 3;
    sptr->Gravity = 0;
    sptr->MaxYvel = 0;
    size = (GetRandomControl() & 7) + 32;
    sptr->Width = sptr->sWidth = size >> 3;
    sptr->dWidth = size;
    sptr->Height = sptr->sHeight = size >> 3;
    sptr->dHeight = size;
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

void ControlColouredLights(sint16 item_number)
{
    ITEM_INFO *item;
    sint32 itemnum;
    uint8 cols[5][3] = { { 31, 0, 0 }, { 0, 31, 0 }, { 0, 0, 31 }, { 31, 24, 0 }, { 28, 28, 31 } };

    item = &items[item_number];
    if (TriggerActive(item)) {
        itemnum = item->object_number - 324;
        TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 24, cols[itemnum][0], cols[itemnum][1], cols[itemnum][2]);
    }
}
