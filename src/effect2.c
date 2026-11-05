#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
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

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint16 InnerXZoff;
    sint16 InnerXZsize;
    sint16 InnerYsize;
    sint16 InnerXZvel;
    sint16 InnerYvel;
    sint16 InnerGravity;
    sint16 InnerFriction;
    sint16 MiddleXZoff;
    sint16 MiddleXZsize;
    sint16 MiddleYsize;
    sint16 MiddleXZvel;
    sint16 MiddleYvel;
    sint16 MiddleGravity;
    sint16 MiddleFriction;
    sint16 OuterXZoff;
    sint16 OuterXZsize;
    sint16 OuterXZvel;
    sint16 OuterFriction;
} SPLASH_SETUP;

extern sint16 SplashRings[8][2];

void SetupSplash(SPLASH_SETUP *spl)
{
    SPLASH_STRUCT *sptr;
    SPLASH_VERTS *svptr;
    sint32 lp, vlp;

    sptr = &splashes[0];
    for (lp = 0; lp < MAX_SPLASHES; lp++) {
        if (!(sptr->flags & SPL_ON)) {
            sptr->flags = SPL_ON | SPL_TRANS;
            if (spl->OuterFriction == -9) {
                sptr->flags |= 0x40;
                spl->OuterFriction = 9;
            }
            sptr->x = spl->x;
            sptr->y = spl->y;
            sptr->z = spl->z;
            sptr->life = 63;
            svptr = &sptr->sv[0];
            for (vlp = 0; vlp < 8; vlp++) {
                svptr->wx = (spl->InnerXZoff * SplashRings[vlp][0]) << 1;
                svptr->wz = (spl->InnerXZoff * SplashRings[vlp][1]) << 1;
                svptr->wy = 0;
                svptr->xv = (spl->InnerXZvel * SplashRings[vlp][0]) / 12;
                svptr->zv = (spl->InnerXZvel * SplashRings[vlp][1]) / 12;
                svptr->yv = 0;
                svptr->oxv = svptr->xv >> 3;
                svptr->ozv = svptr->zv >> 3;
                svptr->friction = spl->InnerFriction - 2;
                svptr->gravity = 0;
                svptr++;
            }
            for (vlp = 0; vlp < 8; vlp++) {
                svptr->wx = ((spl->InnerXZoff + spl->InnerXZsize) * SplashRings[vlp][0]) << 1;
                svptr->wz = ((spl->InnerXZoff + spl->InnerXZsize) * SplashRings[vlp][1]) << 1;
                svptr->wy = spl->InnerYsize;
                svptr->xv = (spl->InnerXZvel * SplashRings[vlp][0]) >> 3;
                svptr->zv = (spl->InnerXZvel * SplashRings[vlp][1]) >> 3;
                svptr->yv = spl->InnerYvel;
                svptr->oxv = svptr->xv >> 3;
                svptr->ozv = svptr->zv >> 3;
                svptr->friction = spl->InnerFriction;
                svptr->gravity = spl->InnerGravity;
                svptr++;
            }
            for (vlp = 0; vlp < 8; vlp++) {
                svptr->wx = (spl->MiddleXZoff * SplashRings[vlp][0]) << 1;
                svptr->wz = (spl->MiddleXZoff * SplashRings[vlp][1]) << 1;
                svptr->wy = 0;
                svptr->xv = (spl->MiddleXZvel * SplashRings[vlp][0]) / 12;
                svptr->zv = (spl->MiddleXZvel * SplashRings[vlp][1]) / 12;
                svptr->yv = 0;
                svptr->oxv = svptr->xv >> 3;
                svptr->ozv = svptr->zv >> 3;
                svptr->friction = spl->MiddleFriction - 2;
                svptr->gravity = 0;
                svptr++;
            }
            for (vlp = 0; vlp < 8; vlp++) {
                svptr->wx = ((spl->MiddleXZoff + spl->MiddleXZsize) * SplashRings[vlp][0]) << 1;
                svptr->wz = ((spl->MiddleXZoff + spl->MiddleXZsize) * SplashRings[vlp][1]) << 1;
                svptr->wy = spl->MiddleYsize;
                svptr->xv = (spl->MiddleXZvel * SplashRings[vlp][0]) >> 3;
                svptr->zv = (spl->MiddleXZvel * SplashRings[vlp][1]) >> 3;
                svptr->yv = spl->MiddleYvel;
                svptr->oxv = svptr->xv >> 3;
                svptr->ozv = svptr->zv >> 3;
                svptr->friction = spl->MiddleFriction;
                svptr->gravity = spl->MiddleGravity;
                svptr++;
            }
            for (vlp = 0; vlp < 8; vlp++) {
                svptr->wx = (spl->OuterXZoff * SplashRings[vlp][0]) << 1;
                svptr->wz = (spl->OuterXZoff * SplashRings[vlp][1]) << 1;
                svptr->wy = 0;
                svptr->xv = (spl->OuterXZvel * SplashRings[vlp][0]) / 12;
                svptr->zv = (spl->OuterXZvel * SplashRings[vlp][1]) / 12;
                svptr->yv = 0;
                svptr->oxv = svptr->xv >> 3;
                svptr->ozv = svptr->zv >> 3;
                svptr->friction = spl->OuterFriction - 2;
                svptr->gravity = 0;
                svptr++;
            }
            for (vlp = 0; vlp < 8; vlp++) {
                svptr->wx = ((spl->OuterXZoff + spl->OuterXZsize) * SplashRings[vlp][0]) << 1;
                svptr->wz = ((spl->OuterXZoff + spl->OuterXZsize) * SplashRings[vlp][1]) << 1;
                svptr->wy = 0;
                svptr->xv = (spl->OuterXZvel * SplashRings[vlp][0]) >> 3;
                svptr->zv = (spl->OuterXZvel * SplashRings[vlp][1]) >> 3;
                svptr->yv = 0;
                svptr->oxv = svptr->xv >> 3;
                svptr->ozv = svptr->zv >> 3;
                svptr->friction = spl->OuterFriction;
                svptr->gravity = 0;
                svptr++;
            }
            break;
        }
        sptr++;
    }
    SoundEffect(33, (PHD_3DPOS *)&spl->x, 0);
}

void UpdateSplashes(void)
{
    RIPPLE_STRUCT *rptr;
    SPLASH_STRUCT *sptr;
    SPLASH_VERTS *svptr;
    sint32 lp, vlp, oyv, declife;

    sptr = &splashes[0];
    for (lp = 0; lp < MAX_SPLASHES; lp++) {
        if (sptr->flags & SPL_ON) {
            svptr = &sptr->sv[0];
            declife = 0;
            for (vlp = 0; vlp < 48; vlp++) {
                svptr->wx += svptr->xv >> 2;
                svptr->wy += svptr->yv >> 6;
                svptr->wz += svptr->zv >> 2;
                svptr->xv -= svptr->xv >> svptr->friction;
                svptr->zv -= svptr->zv >> svptr->friction;
                if (svptr->oxv < 0 && svptr->xv > svptr->oxv)
                    svptr->xv = svptr->oxv;
                else if (svptr->oxv > 0 && svptr->xv < svptr->oxv)
                    svptr->xv = svptr->oxv;
                else if (svptr->ozv < 0 && svptr->zv > svptr->ozv)
                    svptr->zv = svptr->ozv;
                else if (svptr->ozv > 0 && svptr->zv < svptr->ozv)
                    svptr->zv = svptr->ozv;
                oyv = svptr->yv;
                svptr->yv += svptr->gravity << 3;
                if (svptr->yv > 0x10000)
                    svptr->yv = 0x10000;
                if (svptr->wy > 0) {
                    if (vlp < 16)
                        sptr->flags |= 4;
                    else if (vlp < 32)
                        sptr->flags |= 8;
                    svptr->wy = 0;
                    declife = 1;
                }
                svptr++;
            }
            if (declife) {
                sptr->life--;
                if (sptr->life == 0)
                    sptr->flags = 0;
            }
        }
        sptr++;
    }
    rptr = &ripples[0];
    for (lp = 0; lp < MAX_RIPPLES; lp++) {
        if (rptr->flags & SPL_ON) {
            if (rptr->size < 254)
                rptr->size += 2;
            if (!rptr->init) {
                rptr->life -= 2;
                if (rptr->life > 250)
                    rptr->flags = 0;
            } else if (rptr->init < rptr->life) {
                rptr->init += 4;
                if (rptr->init >= rptr->life)
                    rptr->init = 0;
            }
        }
        rptr++;
    }
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

void TriggerFireFlame(sint32 x, sint32 y, sint32 z, sint32 body_part, sint32 type)
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
    if (type != 2) {
        if (type == 254) {
            sptr->sG = 255;
            sptr->sB = 48 + (GetRandomControl() & 31);
            sptr->sR = 48;
            sptr->dG = 192 + (GetRandomControl() & 63);
            sptr->dB = 128 + (GetRandomControl() & 63);
            sptr->dR = 32;
        } else {
            sptr->sR = 255;
            sptr->sG = 48 + (GetRandomControl() & 31);
            sptr->sB = 48;
        }
    } else {
        sptr->sR = 48 + (GetRandomControl() & 31);
        sptr->sG = sptr->sR;
        sptr->sB = 192 + (GetRandomControl() & 63);
    }
    if (type != 254) {
        sptr->dR = 192 + (GetRandomControl() & 63);
        sptr->dG = 128 + (GetRandomControl() & 63);
        sptr->dB = 32;
    }
    if (body_part != -1) {
        sptr->ColFadeSpeed = 8 + (GetRandomControl() & 3);
        sptr->FadeToBlack = 16;
        sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 28;
    } else {
        if (type != 2 && type != 255 && type != 254) {
            sptr->ColFadeSpeed = 20 + (GetRandomControl() & 3);
            sptr->FadeToBlack = 8;
            sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 40;
        } else {
            sptr->ColFadeSpeed = 5 + (GetRandomControl() & 3);
            sptr->FadeToBlack = 6;
            sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 16 + ((type >= 254) ? 8 : 0);
        }
    }
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    if (body_part != -1) {
        sptr->x = (GetRandomControl() & 31) - 16;
        sptr->y = 0;
        sptr->z = (GetRandomControl() & 31) - 16;
    } else {
        if (type == 0 || type == 1) {
            sptr->x = x + ((GetRandomControl() & 31) - 16);
            sptr->y = y;
            sptr->z = z + ((GetRandomControl() & 31) - 16);
        } else if (type >= 254) {
            sptr->x = x + ((GetRandomControl() & 63) - 32);
            sptr->y = y;
            sptr->z = z + ((GetRandomControl() & 63) - 32);
        } else {
            sptr->x = x + ((GetRandomControl() & 15) - 8);
            sptr->y = y;
            sptr->z = z + ((GetRandomControl() & 15) - 8);
        }
    }
    if (type != 2) {
        sptr->Xvel = (GetRandomControl() & 255) - 128;
        sptr->Yvel = -(GetRandomControl() & 15) - 16;
        sptr->Zvel = (GetRandomControl() & 255) - 128;
        if (type == 1)
            sptr->Friction = 3 | (3 << 4);
        else
            sptr->Friction = 5;
    } else {
        sptr->Xvel = (GetRandomControl() & 31) - 16;
        sptr->Yvel = -(GetRandomControl() & 511) - 1024;
        sptr->Zvel = (GetRandomControl() & 31) - 16;
        sptr->Friction = 4 | (4 << 4);
    }
    if (GetRandomControl() & 1) {
        if (body_part != -1) {
            sptr->Flags = 2 | 8 | 16 | 64 | 512;
            sptr->FxObj = body_part;
            sptr->Gravity = -(GetRandomControl() & 63) - 32;
            sptr->MaxYvel = -(GetRandomControl() & 7) - 24;
        } else {
            sptr->Gravity = -(GetRandomControl() & 31) - 16;
            sptr->MaxYvel = -(GetRandomControl() & 7) - 16;
            sptr->Flags = 2 | 8 | 16 | 512;
        }
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 15) - 16;
        else
            sptr->RotAdd = (GetRandomControl() & 15) + 16;
    } else {
        if (body_part != -1) {
            sptr->Flags = 2 | 8 | 64 | 512;
            sptr->FxObj = body_part;
            sptr->Gravity = -(GetRandomControl() & 63) - 32;
            sptr->MaxYvel = -(GetRandomControl() & 7) - 24;
        } else {
            sptr->Flags = 2 | 8 | 512;
            sptr->Gravity = -(GetRandomControl() & 31) - 16;
            sptr->MaxYvel = -(GetRandomControl() & 7) - 16;
        }
    }
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 2;
    if (type == 0)
        size = (GetRandomControl() & 31) + 128;
    else if (type == 1)
        size = (GetRandomControl() & 31) + 64;
    else if (type >= 254)
        size = (GetRandomControl() & 15) + 48;
    else {
        sptr->Gravity = sptr->MaxYvel = 0;
        size = (GetRandomControl() & 31) + 32;
    }
    sptr->Width = sptr->sWidth = size;
    sptr->Height = sptr->sHeight = size;
    if (type != 2) {
        sptr->dWidth = size >> 4;
        sptr->dHeight = size >> 4;
    } else {
        sptr->dWidth = size >> 2;
        sptr->dHeight = size >> 2;
    }
}

void TriggerFlareSparks(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv, sint32 smoke, sint32 FromLara)
{
    SPARKS *sptr;
    SPARKS *sptr1;
    sint32 size;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - x;
    dz = lara_item->pos.z_pos - z;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 255;
    sptr->sG = 255;
    sptr->sB = 255;
    sptr->dR = 255;
    sptr->dG = (GetRandomDraw() & 127) + 64;
    sptr->dB = 192 - sptr->dG;
    sptr->ColFadeSpeed = 3;
    sptr->FadeToBlack = 5;
    sptr->sLife = sptr->Life = 10;
    sptr->TransType = 2;
    sptr->Dynamic = -1;
    sptr->x = x + (GetRandomDraw() & 7) - 3;
    sptr->y = y + (GetRandomDraw() & 7) - 3;
    sptr->z = z + (GetRandomDraw() & 7) - 3;
    sptr->Xvel = xv + ((GetRandomDraw() & 255) - 128);
    sptr->Yvel = yv + ((GetRandomDraw() & 255) - 128);
    sptr->Zvel = zv + ((GetRandomDraw() & 255) - 128);
    sptr->Friction = 2 | (2 << 4);
    sptr->Scalar = 1;
    sptr->Width = sptr->sWidth = (GetRandomDraw() & 3) + 4;
    sptr->dWidth = (GetRandomDraw() & 1) + 1;
    sptr->Height = sptr->sHeight = (GetRandomDraw() & 3) + 4;
    sptr->dHeight = (GetRandomDraw() & 1) + 1;
    sptr->Gravity = sptr->MaxYvel = 0;
    sptr->Flags = 2;
    if (smoke) {
        sptr1 = &sparks[GetFreeSpark()];
        sptr1->On = 1;
        sptr1->sR = sptr->dR >> 1;
        sptr1->sG = sptr->dG >> 1;
        sptr1->sB = sptr->dB >> 1;
        sptr1->dR = 32;
        sptr1->dG = 32;
        sptr1->dB = 32;
        sptr1->ColFadeSpeed = 8 + (GetRandomDraw() & 3);
        sptr1->FadeToBlack = 4;
        sptr1->sLife = sptr1->Life = (GetRandomDraw() & 7) + 13;
        sptr1->TransType = 2;
        sptr1->extras = 0;
        sptr1->Dynamic = -1;
        sptr1->x = x + (xv >> 5);
        sptr1->y = y + (yv >> 5);
        sptr1->z = z + (zv >> 5);
        sptr1->Xvel = xv + ((GetRandomDraw() & 63) - 32);
        sptr1->Yvel = yv;
        sptr1->Zvel = zv + ((GetRandomDraw() & 63) - 32);
        sptr1->Friction = 4;
        if (GetRandomDraw() & 1) {
            sptr1->Flags = 2 | 8 | 16 | 512;
            sptr1->RotAng = GetRandomDraw() & 4095;
            if (GetRandomDraw() & 1)
                sptr1->RotAdd = -(GetRandomDraw() & 15) - 16;
            else
                sptr1->RotAdd = (GetRandomDraw() & 15) + 16;
        } else
            sptr1->Flags = 2 | 8 | 512;
        sptr1->Def = objects[296].mesh_index;
        sptr1->Scalar = 2;
        sptr1->Gravity = -(GetRandomDraw() & 3) - 8;
        sptr1->MaxYvel = -(GetRandomDraw() & 3) - 4;
        size = 24 + (GetRandomDraw() & 15);
        sptr1->Width = sptr1->sWidth = size >> 3;
        sptr1->dWidth = size;
        sptr1->Height = sptr1->sHeight = size >> 3;
        sptr1->dHeight = size;
    }
}

typedef struct {
    uint8 On;
    uint8 Falloff;
    uint8 R;
    uint8 G;
    uint8 B;
    uint8 Flags;
    uint8 Pad[2];
} SP_DYNAMIC;

extern SP_DYNAMIC spark_dynamics[32];

void TriggerExplosionSmokeEnd(sint32 x, sint32 y, sint32 z, sint32 uw);
void TriggerExplosionSmoke(sint32 x, sint32 y, sint32 z, sint32 uw);

void TriggerExplosionSparks(sint32 x, sint32 y, sint32 z, sint32 extra_trigs, sint32 dyn, sint32 uw, sint16 roomnum)
{
    sint32 lp1, fric;
    SPARKS *sptr;
    SP_DYNAMIC *sdptr;
    uint8 extra_tab[6] = {0, 4, 7, 10};
    sint32 dx, dz;
    sint32 tr, tg, tb;

    dx = lara_item->pos.x_pos - x;
    dz = lara_item->pos.z_pos - z;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    if (uw == 1) {
        sptr->sR = 255;
        sptr->sG = 128 + (GetRandomControl() & 63);
        sptr->sB = 32;
        sptr->dR = 192;
        sptr->dG = 64 + (GetRandomControl() & 31);
        sptr->dB = 0;
    } else {
        sptr->sR = 255;
        sptr->sG = 32 + (GetRandomControl() & 15);
        sptr->sB = 0;
        sptr->dR = 192 + (GetRandomControl() & 63);
        sptr->dG = 128 + (GetRandomControl() & 63);
        sptr->dB = 32;
    }
    if (uw == 1) {
        sptr->ColFadeSpeed = 7;
        sptr->FadeToBlack = 8;
        sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 16;
        sptr->TransType = 2;
        sptr->RoomNumber = roomnum;
    } else {
        sptr->ColFadeSpeed = 8;
        sptr->FadeToBlack = 16;
        sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 24;
        sptr->TransType = 2;
    }
    fric = extra_tab[extra_trigs] + (GetRandomControl() & 7) - 4;
    sptr->extras = extra_trigs | (fric << 3);
    sptr->Dynamic = dyn;
    if (dyn == -2) {
        sdptr = &spark_dynamics[0];
        for (lp1 = 0; lp1 < 32; lp1++) {
            if (!sdptr->On) {
                sdptr->On = 1;
                sdptr->Falloff = 4;
                if (uw == 1)
                    sdptr->Flags = 2;
                else
                    sdptr->Flags = 1;
                sptr->Dynamic = lp1;
                break;
            }
            sdptr++;
        }
        if (lp1 == 32)
            sptr->Dynamic = -1;
    }
    sptr->x = x + (GetRandomControl() & 31) - 16;
    sptr->y = y + (GetRandomControl() & 31) - 16;
    sptr->z = z + (GetRandomControl() & 31) - 16;
    sptr->Xvel = (GetRandomControl() & 4095) - 2048;
    sptr->Yvel = (GetRandomControl() & 4095) - 2048;
    sptr->Zvel = (GetRandomControl() & 4095) - 2048;
    if (dyn == -2 && uw != 1) {
        sptr->x = x + (GetRandomControl() & 511) - 256;
        sptr->y = y + (GetRandomControl() & 511) - 256;
        sptr->z = z + (GetRandomControl() & 511) - 256;
    } else {
        sptr->x = x + (GetRandomControl() & 31) - 16;
        sptr->y = y + (GetRandomControl() & 31) - 16;
        sptr->z = z + (GetRandomControl() & 31) - 16;
    }
    if (uw == 1)
        sptr->Friction = 1 | (1 << 4);
    else
        sptr->Friction = 3 | (3 << 4);
    if (GetRandomControl() & 1) {
        if (uw == 1)
            sptr->Flags = 2 | 8 | 16 | 512 | 2048;
        else
            sptr->Flags = 2 | 8 | 16 | 512;
        sptr->RotAng = GetRandomControl() & 4095;
        sptr->RotAdd = (GetRandomControl() & 255) - 128;
    } else {
        if (uw == 1)
            sptr->Flags = 2 | 8 | 512 | 2048;
        else
            sptr->Flags = 2 | 8 | 512;
    }
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 3;
    sptr->Gravity = 0;
    fric = (GetRandomControl() & 15) + 40;
    sptr->Width = sptr->sWidth = fric;
    sptr->dWidth = fric << 1;
    fric += (GetRandomControl() & 7) + 8;
    sptr->Height = sptr->sHeight = fric;
    sptr->dHeight = fric << 1;
    sptr->dHeight = fric << 1;
    sptr->MaxYvel = 0;
    if (uw != 2) {
        if (extra_trigs == 0)
            TriggerExplosionSmokeEnd(x, y, z, uw);
        else
            TriggerExplosionSmoke(x, y, z, uw);
    } else {
        tr = sptr->sR;
        tg = sptr->sG;
        tb = sptr->sB;
        sptr->sR = tb;
        sptr->sG = tr;
        sptr->sB = tg;
        tr = sptr->dR;
        tg = sptr->dG;
        tb = sptr->dB;
        sptr->dR = tb;
        sptr->dG = tr;
        sptr->dB = tg;
        sptr->Flags |= 0x2000;
    }
}

extern sint32 SmokeWindX, SmokeWindZ;

void TriggerExplosionBubble(sint32 x, sint32 y, sint32 z, sint16 roomnum);

void UpdateSparks(void)
{
    sint32 lp, lp1;
    sint32 divmult;
    sint32 random, expflag;
    SPARKS *sptr;

    sptr = &sparks[0];
    for (lp = 0; lp < MAX_SPARKS; lp++) {
        if (sptr->On == 0) {
            sptr++;
            continue;
        }
        if (!(sptr->Flags & 0x400) || ((sptr->Flags & 0x400) && sptr->Life > 16))
            sptr->Life--;
        if (sptr->Life == 0) {
            if (sptr->Dynamic != -1) {
                spark_dynamics[sptr->Dynamic].On = 0;
                sptr->Dynamic = -1;
            }
            sptr->On = 0;
            sptr++;
            continue;
        }
        if (sptr->sLife - sptr->Life < sptr->ColFadeSpeed) {
            divmult = (65536 * (sptr->sLife - sptr->Life)) / sptr->ColFadeSpeed;
            sptr->R = sptr->sR + (((sptr->dR - sptr->sR) * divmult) >> 16);
            sptr->G = sptr->sG + (((sptr->dG - sptr->sG) * divmult) >> 16);
            sptr->B = sptr->sB + (((sptr->dB - sptr->sB) * divmult) >> 16);
        } else if (sptr->Life < sptr->FadeToBlack) {
            divmult = 65536 - ((65536 * (sptr->FadeToBlack - sptr->Life)) / sptr->FadeToBlack);
            sptr->R = (sptr->dR * divmult) >> 16;
            sptr->G = (sptr->dG * divmult) >> 16;
            sptr->B = (sptr->dB * divmult) >> 16;
        } else {
            sptr->R = sptr->dR;
            sptr->G = sptr->dG;
            sptr->B = sptr->dB;
        }
        if (sptr->Life == sptr->FadeToBlack && (sptr->Flags & 0x800)) {
            sptr->dWidth >>= 2;
            sptr->dHeight >>= 2;
        }
        if (sptr->Flags & 16) {
            sptr->RotAng += sptr->RotAdd;
            sptr->RotAng &= 4095;
        }
        if (sptr->Flags & 512) {
            if (sptr->R < 16 && sptr->G < 16 && sptr->B < 16)
                sptr->Def = objects[296].mesh_index + 3;
            else if (sptr->R < 64 && sptr->G < 64 && sptr->B < 64)
                sptr->Def = objects[296].mesh_index + 2;
            else if (sptr->R < 96 && sptr->G < 96 && sptr->B < 96)
                sptr->Def = objects[296].mesh_index + 1;
            else
                sptr->Def = objects[296].mesh_index;
        }
        if (sptr->sLife - sptr->Life == (sptr->extras >> 3) && (sptr->extras & 7)) {
            if (sptr->Flags & 0x800)
                expflag = 1;
            else if (sptr->Flags & 0x2000)
                expflag = 2;
            else
                expflag = 0;
            for (lp1 = 0; lp1 < (sptr->extras & 7); lp1++) {
                TriggerExplosionSparks(sptr->x, sptr->y, sptr->z, (sptr->extras & 7) - 1, sptr->Dynamic, expflag,
                                       sptr->RoomNumber);
                sptr->Dynamic = -1;
            }
            if (sptr->Flags & 0x800)
                TriggerExplosionBubble(sptr->x, sptr->y, sptr->z, sptr->RoomNumber);
            sptr->extras = 0;
        }
        divmult = (65536 * (sptr->sLife - sptr->Life)) / sptr->sLife;
        sptr->Yvel += sptr->Gravity;
        if (sptr->MaxYvel) {
            if ((sptr->Yvel < 0 && sptr->Yvel < (sptr->MaxYvel << 5)) ||
                (sptr->Yvel > 0 && sptr->Yvel > (sptr->MaxYvel << 5)))
                sptr->Yvel = sptr->MaxYvel << 5;
        }
        if (sptr->Friction & 15) {
            sptr->Xvel -= sptr->Xvel >> (sptr->Friction & 15);
            sptr->Zvel -= sptr->Zvel >> (sptr->Friction & 15);
        }
        if ((sptr->Friction >> 4) & 15)
            sptr->Yvel -= sptr->Yvel >> ((sptr->Friction >> 4) & 15);
        sptr->x += sptr->Xvel >> 5;
        sptr->y += sptr->Yvel >> 5;
        sptr->z += sptr->Zvel >> 5;
        if (sptr->Flags & 256) {
            sptr->x += SmokeWindX >> 1;
            sptr->z += SmokeWindZ >> 1;
        }
        sptr->Width = sptr->sWidth + (((sptr->dWidth - sptr->sWidth) * divmult) >> 16);
        sptr->Height = sptr->sHeight + (((sptr->dHeight - sptr->sHeight) * divmult) >> 16);
        sptr++;
    }
    sptr = &sparks[0];
    for (lp = 0; lp < MAX_SPARKS; lp++) {
        if (sptr->On && sptr->Dynamic != -1) {
            sint32 r, g, b, falloff;
            sint32 x, y, z;
            SP_DYNAMIC *sdptr;

            sdptr = &spark_dynamics[sptr->Dynamic];
            if (sdptr->Flags & 3) {
                random = GetRandomControl();
                x = sptr->x + ((random & 0xf) << 4);
                y = sptr->y + (random & 0xf0);
                z = sptr->z + ((random & 0xf00) >> 4);
                falloff = sptr->sLife - sptr->Life - 1;
                if (falloff < 2) {
                    if (sdptr->Falloff < 28)
                        sdptr->Falloff += 6;
                    r = 31 - (random & 3) - falloff;
                    g = 31 - (random & 3) - (falloff << 1);
                    b = 31 - (random & 3) - (falloff << 3);
                } else if (falloff < 4) {
                    if (sdptr->Falloff < 28)
                        sdptr->Falloff += 6;
                    r = 31 - (random & 3) - falloff;
                    g = 16 - falloff;
                    b = 16 - (falloff << 2);
                    if (b < 0)
                        b = 0;
                } else {
                    if (sdptr->Falloff > 0)
                        sdptr->Falloff--;
                    r = 28 + (random & 3);
                    g = 16 + ((random >> 4) & 3);
                    b = (random >> 8) & 7;
                }
                if (sptr->Flags & 0x2000)
                    TriggerDynamic(x, y, z, (sdptr->Falloff > 31) ? 31 : sdptr->Falloff, b, r, g);
                else
                    TriggerDynamic(x, y, z, (sdptr->Falloff > 31) ? 31 : sdptr->Falloff, r, g, b);
            }
        }
        sptr++;
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

void TriggerSideFlame(sint32 x, sint32 y, sint32 z, sint32 angle, sint32 speed, sint32 pilot)
{
    sint32 size, zv, xv;
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - x;
    dz = lara_item->pos.z_pos - z;
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
    sptr->FadeToBlack = 8;
    sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 28;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 31) - 16);
    sptr->y = y + ((GetRandomControl() & 31) - 16);
    sptr->z = z + ((GetRandomControl() & 31) - 16);
    if (!pilot) {
        speed <<= 8;
        speed += GetRandomControl() & 511;
    } else {
        speed <<= 7;
        speed += GetRandomControl() & 31;
    }
    zv = (rcossin_tbl[angle + 1] * speed) >> 11;
    xv = (rcossin_tbl[angle] * speed) >> 11;
    sptr->Xvel = xv + ((GetRandomControl() & 127) - 64);
    sptr->Yvel = -(GetRandomControl() & 7) - 6;
    sptr->Zvel = zv + ((GetRandomControl() & 127) - 64);
    sptr->Friction = 4;
    sptr->Flags = 2 | 8 | 512;
    sptr->Gravity = -(GetRandomControl() & 15) - 8;
    sptr->MaxYvel = -(GetRandomControl() & 7) - 8;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 3;
    size = (GetRandomControl() & 31) + 128;
    if (pilot)
        size >>= 2;
    sptr->Width = sptr->sWidth = size >> 1;
    sptr->Height = sptr->sHeight = size >> 1;
    sptr->dWidth = size;
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

void GetLaraHandAbsPosition(PHD_VECTOR *vec, sint32 lr);

void TriggerGunShell(sint16 leftright, sint32 shelltype, sint32 weapon)
{
    FX_INFO *fx;
    PHD_VECTOR pos;
    sint16 fx_number;

    if (leftright == 0) {
        switch (weapon) {
        case LG_PISTOLS:
            pos.x = -12;
            pos.y = 48;
            pos.z = 40;
            break;
        case LG_MAGNUMS:
            pos.x = -16;
            pos.y = 40;
            pos.z = 56;
            break;
        case LG_UZIS:
            pos.x = -16;
            pos.y = 35;
            pos.z = 48;
            break;
        }
        GetLaraHandAbsPosition(&pos, 0);
    } else {
        switch (weapon) {
        case LG_PISTOLS:
            pos.x = 8;
            pos.y = 48;
            pos.z = 40;
            break;
        case LG_MAGNUMS:
            pos.x = 16;
            pos.y = 40;
            pos.z = 56;
            break;
        case LG_UZIS:
            pos.x = 8;
            pos.y = 35;
            pos.z = 48;
            break;
        case LG_M16:
            pos.x = 16;
            pos.y = 2;
            pos.z = 64;
            break;
        case LG_SHOTGUN:
            pos.x = 16;
            pos.y = 114;
            pos.z = 32;
            break;
        }
        GetLaraHandAbsPosition(&pos, 1);
    }
    if (weapon == LG_SHOTGUN)
        TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, weapon, 24);
    else
        TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 0, weapon, 16);
    fx_number = CreateEffect(lara_item->room_number);
    if (fx_number != NO_ITEM) {
        fx = &effects[fx_number];
        fx->pos.x_pos = pos.x;
        fx->pos.y_pos = pos.y;
        fx->pos.z_pos = pos.z;
        fx->room_number = lara_item->room_number;
        fx->pos.y_rot = 0;
        fx->pos.x_rot = 0;
        fx->pos.z_rot = GetRandomControl();
        fx->speed = 16 + (GetRandomControl() & 31);
        fx->fallspeed = -48 - (GetRandomControl() & 7);
        fx->object_number = shelltype;
        fx->frame_number = objects[fx->object_number].mesh_index;
        fx->shade = 0x4210;
        fx->counter = (GetRandomControl() & 1) + 1;
        if (leftright == 0) {
            fx->flag1 = lara.left_arm.y_rot + lara_item->pos.y_rot - 0x4800 + (GetRandomControl() & 0xfff);
        } else {
            if (weapon == LG_SHOTGUN) {
                fx->flag1 = lara.left_arm.y_rot + lara_item->pos.y_rot + lara.torso_y_rot + 0x2800 -
                            (GetRandomControl() & 0xfff);
                fx->pos.y_rot += lara.left_arm.y_rot + lara_item->pos.y_rot + lara.torso_y_rot;
                if (fx->speed < 24)
                    fx->speed += 24;
            } else if (weapon == LG_M16) {
                fx->flag1 = lara.left_arm.y_rot + lara_item->pos.y_rot + lara.torso_y_rot + 0x4800 -
                            (GetRandomControl() & 0xfff);
                fx->pos.y_rot += lara.left_arm.y_rot + lara_item->pos.y_rot + lara.torso_y_rot;
            } else
                fx->flag1 = lara.left_arm.y_rot + lara_item->pos.y_rot + 0x4800 - (GetRandomControl() & 0xfff);
        }
    }
}

void EffectNewRoom(sint16 fx_num, sint16 room_number);
void KillEffect(sint16 fx_num);

void ControlGunShell(sint16 fx_number)
{
    FX_INFO *fx;
    FLOOR_INFO *floor;
    sint32 height, ceiling, ox, oy, oz;
    sint16 room_number;

    fx = &effects[fx_number];
    ox = fx->pos.x_pos;
    oy = fx->pos.y_pos;
    oz = fx->pos.z_pos;
    fx->pos.y_rot += 182 * fx->speed;
    fx->pos.x_rot += 182 * ((fx->speed >> 1) + 7);
    fx->pos.z_rot += 182 * 23;
    fx->pos.z_pos += (fx->speed * phd_cos(fx->flag1)) >> (W2V_SHIFT + 1);
    fx->pos.x_pos += (fx->speed * phd_sin(fx->flag1)) >> (W2V_SHIFT + 1);
    fx->fallspeed += 6;
    fx->pos.y_pos += fx->fallspeed;
    room_number = fx->room_number;
    floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);
    if (room_number != fx->room_number)
        EffectNewRoom(fx_number, room_number);
    if (room[room_number].flags & UNDERWATER) {
        SetupRipple(fx->pos.x_pos, room[room_number].maxceiling, fx->pos.z_pos, -8 - (GetRandomControl() & 3), 1);
        KillEffect(fx_number);
        return;
    }
    ceiling = GetCeiling(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);
    if (fx->pos.y_pos < ceiling) {
        SoundEffect(49, &fx->pos, 0);
        fx->counter--;
        fx->speed -= 4;
        if (fx->counter < 0 || fx->speed < 8) {
            KillEffect(fx_number);
            return;
        }
        fx->fallspeed = -fx->fallspeed;
        fx->pos.y_pos = ceiling;
    }
    height = GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);
    if (fx->pos.y_pos >= height) {
        SoundEffect(49, &fx->pos, 0);
        fx->speed -= 8;
        fx->counter--;
        if (fx->counter < 0 || fx->speed < 8) {
            KillEffect(fx_number);
            return;
        }
        if (oy > height) {
            fx->flag1 += -0x8000;
            fx->pos.x_pos = ox;
            fx->pos.z_pos = oz;
        } else
            fx->fallspeed = -fx->fallspeed >> 1;
        fx->pos.y_pos = oy;
    }
    room_number = fx->room_number;
    floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);
    if (room_number != fx->room_number)
        EffectNewRoom(fx_number, room_number);
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

void TriggerWaterfallMist(sint32 x, sint32 y, sint32 z, sint32 angle)
{
    sint32 xsize, zsize, tangle, dist, lp;
    sint32 dist_tab[4] = {512 + 64, 171 + 32, -171 - 32, -512 - 64};
    SPARKS *sptr;

    for (lp = 0; lp < 4; lp++) {
        sptr = &sparks[GetFreeSpark()];
        sptr->On = 1;
        sptr->sR = 128;
        sptr->sG = 128;
        sptr->sB = 128;
        sptr->dR = 192;
        sptr->dG = 192;
        sptr->dB = 192;
        sptr->ColFadeSpeed = 2;
        sptr->FadeToBlack = 4;
        sptr->sLife = sptr->Life = 6 + (GetRandomControl() & 3);
        sptr->TransType = 2;
        sptr->extras = 0;
        sptr->Dynamic = -1;
        dist = dist_tab[lp] + (GetRandomControl() & 31) - 16;
        tangle = (angle + 1024) & 4095;
        xsize = (rcossin_tbl[tangle << 1] * dist) >> 12;
        zsize = (rcossin_tbl[(tangle << 1) + 1] * dist) >> 12;
        sptr->x = x + xsize + ((GetRandomControl() & 15) - 8);
        sptr->y = y + ((GetRandomControl() & 15) - 8);
        sptr->z = z + zsize + ((GetRandomControl() & 15) - 8);
        sptr->Xvel = rcossin_tbl[angle << 1] >> 12;
        sptr->Zvel = rcossin_tbl[(angle << 1) + 1] >> 12;
        sptr->Yvel = 0;
        sptr->Friction = 3;
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
        sptr->Scalar = 6;
        sptr->Gravity = 0;
        sptr->MaxYvel = 0;
        xsize = (GetRandomControl() & 7) + 12;
        sptr->Width = sptr->sWidth = xsize >> 1;
        sptr->dWidth = xsize;
        sptr->Height = sptr->sHeight = xsize >> 1;
        sptr->dHeight = xsize;
    }
}

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

void TriggerDartSmoke(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 zv, sint32 hit)
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
    sptr->sR = 16;
    sptr->sG = 8;
    sptr->sB = 4;
    sptr->dR = 64;
    sptr->dG = 48;
    sptr->dB = 32;
    sptr->ColFadeSpeed = 8;
    sptr->FadeToBlack = 4;
    sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 32;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 31) - 16);
    sptr->y = y + ((GetRandomControl() & 31) - 16);
    sptr->z = z + ((GetRandomControl() & 31) - 16);
    if (hit) {
        sptr->Xvel = -xv + ((GetRandomControl() & 255) - 128);
        sptr->Yvel = -(GetRandomControl() & 3) - 4;
        sptr->Zvel = -zv + ((GetRandomControl() & 255) - 128);
        sptr->Friction = 3;
    } else {
        if (xv)
            sptr->Xvel = -xv;
        else
            sptr->Xvel = (GetRandomControl() & 255) - 128;
        sptr->Yvel = -(GetRandomControl() & 3) - 4;
        if (zv)
            sptr->Zvel = -zv;
        else
            sptr->Zvel = (GetRandomControl() & 255) - 128;
        sptr->Friction = 3;
    }
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
    sptr->Scalar = 1;
    size = (GetRandomControl() & 63) + 72;
    if (hit) {
        size >>= 1;
        sptr->Width = sptr->sWidth = size >> 2;
        sptr->Height = sptr->sHeight = size >> 2;
        sptr->Gravity = sptr->MaxYvel = 0;
    } else {
        sptr->Width = sptr->sWidth = size >> 4;
        sptr->Height = sptr->sHeight = size >> 4;
        sptr->Gravity = -(GetRandomControl() & 3) - 4;
        sptr->MaxYvel = -(GetRandomControl() & 3) - 4;
    }
    sptr->dWidth = size;
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

void ControlSmokeEmitter(sint16 item_number)
{
    ITEM_INFO *item;
    SPARKS *sptr;
    sint32 size;
    sint32 dx, dz;

    item = &items[item_number];
    if (!TriggerActive(item) || (wibble & 12))
        return;
    if (item->object_number == 336 && (wibble & 31))
        return;
    dx = lara_item->pos.x_pos - item->pos.x_pos;
    dz = lara_item->pos.z_pos - item->pos.z_pos;
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
    sptr->ColFadeSpeed = 16 + (GetRandomControl() & 7);
    sptr->FadeToBlack = 64;
    sptr->sLife = sptr->Life = (GetRandomControl() & 15) + 96;
    if (item->object_number == 335)
        sptr->TransType = 3;
    else
        sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = item->pos.x_pos + ((GetRandomControl() & 15) - 8);
    sptr->y = item->pos.y_pos + ((GetRandomControl() & 15) - 8);
    sptr->z = item->pos.z_pos + ((GetRandomControl() & 15) - 8);
    sptr->Xvel = (GetRandomControl() & 255) - 128;
    sptr->Yvel = -(GetRandomControl() & 15) - 16;
    sptr->Zvel = (GetRandomControl() & 255) - 128;
    sptr->Friction = 4;
    if (GetRandomControl() & 1) {
        sptr->Flags = 2 | 8 | 16 | 512 | 256;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 7) - 4;
        else
            sptr->RotAdd = (GetRandomControl() & 7) + 4;
    } else
        sptr->Flags = 2 | 8 | 512 | 256;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 3;
    sptr->Gravity = -(GetRandomControl() & 7) - 8;
    sptr->MaxYvel = -(GetRandomControl() & 7) - 4;
    size = (GetRandomControl() & 31) + 128;
    sptr->Width = sptr->sWidth = size >> 2;
    sptr->dWidth = size;
    size += (GetRandomControl() & 31) + 32;
    sptr->Height = sptr->sHeight = size >> 2;
    sptr->dHeight = size;
    if (item->object_number == 336) {
        sptr->Gravity >>= 1;
        sptr->MaxYvel >>= 1;
        sptr->Yvel >>= 1;
        sptr->dR = 24;
        sptr->dG = 24;
        sptr->dB = 24;
    }
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
