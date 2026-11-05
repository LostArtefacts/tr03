#include <windows.h>
#include "game.h"

#define NO_AI_PATH -1

extern sint32 closest_ai_path;

void DrawWillBossShield(ITEM_INFO *item);

void S_DrawWillBoss(ITEM_INFO *item)
{
    DrawAnimatingItem(item);
    if (bossdata.explode_count)
        DrawExplosionRings();
    if (bossdata.explode_count && bossdata.explode_count <= 64)
        DrawWillBossShield(item);
}

void InitialiseWillBoss(sint16 item_number)
{
    closest_ai_path = NO_AI_PATH;
    items[item_number].item_flags[1] = 0;
    bossdata.dead = bossdata.dropped_icon = bossdata.explode_count = 0;
}

typedef struct {
    sint16 x;
    sint16 y;
    sint16 z;
    uint8 rsub;
    uint8 gsub;
    uint8 bsub;
    uint8 pad[3];
    sint32 rgb;
} SHIELD_POINTS;

typedef struct {
    sint16 x;
    sint16 z;
    sint32 rgb;
} EXPLOSION_VERTS;

typedef struct {
    sint16 on;
    sint16 life;
    sint16 speed;
    sint16 radius;
    sint16 xrot;
    sint16 zrot;
    sint32 x;
    sint32 y;
    sint32 z;
    EXPLOSION_VERTS verts[16];
} EXPLOSION_RING;

extern sint16 rcossin_tbl[];
extern SHIELD_POINTS WillBossShield[40];
extern EXPLOSION_RING ExpRings[];
extern sint32 death_radii_willboss[5];
extern sint32 death_heights_willboss[5];
extern sint32 dradii_willboss[5];
extern sint32 dheights1_willboss[5];
extern sint32 dheights2_willboss[5];
extern sint32 wibble;

void TriggerPlasmaBall_willboss(PHD_VECTOR *pos, sint16 room_number, sint16 angle, sint16 type);
void TriggerExplosionSparks(sint32 x, sint32 y, sint32 z, sint32 extra_trigs, sint32 dyn, sint32 uw, sint16 roomnum);

void ExplodeWillBoss(ITEM_INFO *item)
{
    SHIELD_POINTS *shptr;
    sint32 lp, lp1, angle, y, r, g, b;
    sint32 x, z;

    if (bossdata.explode_count == 1 || bossdata.explode_count == 15 || bossdata.explode_count == 25 ||
        bossdata.explode_count == 35 || bossdata.explode_count == 45 || bossdata.explode_count == 55) {
        x = item->pos.x_pos + ((GetRandomDraw() & 1023) - 512);
        y = item->pos.y_pos - (GetRandomDraw() & 1023) - 256;
        z = item->pos.z_pos + ((GetRandomDraw() & 1023) - 512);

        ExpRings[bossdata.ring_count].x = x;
        ExpRings[bossdata.ring_count].y = y;
        ExpRings[bossdata.ring_count].z = z;
        ExpRings[bossdata.ring_count].on = 1;
        bossdata.ring_count++;

        for (x = 0; x < 24; x += 3) {
            PHD_VECTOR pos;

            pos.x = 0;
            pos.y = 0;
            pos.z = 0;
            GetJointAbsPosition(item, &pos, x);
            TriggerPlasmaBall_willboss(&pos, item->room_number, GetRandomControl() << 1, 4);
        }

        TriggerExplosionSparks(x, y, z, 3, -2, 2, 0);

        SoundEffect(76, &item->pos, 0x800004);
    }

    for (lp = 0; lp < 5; lp++) {
        if (bossdata.explode_count < 128) {
            death_radii_willboss[lp] = (dradii_willboss[lp] >> 4) + ((dradii_willboss[lp] * bossdata.explode_count) >> 7);
            death_heights_willboss[lp] = dheights2_willboss[lp] + (((dheights1_willboss[lp] - dheights2_willboss[lp]) * bossdata.explode_count) >> 7);
        }
    }

    shptr = &WillBossShield[0];

    for (lp = 0; lp < 5; lp++) {
        y = death_heights_willboss[lp];
        angle = (wibble << 3) & 511;
        for (lp1 = 0; lp1 < 8; lp1++) {
            shptr->x = (rcossin_tbl[angle << 1] * death_radii_willboss[lp]) >> 11;
            shptr->y = y;
            shptr->z = (rcossin_tbl[(angle << 1) + 1] * death_radii_willboss[lp]) >> 11;

            if (lp != 0 && lp != 4 && bossdata.explode_count < 64) {
                g = (GetRandomDraw() & 31) + 224;
                b = (g >> 1) + (GetRandomDraw() & 63);
                r = GetRandomDraw() & 63;
                r = (r * (64 - bossdata.explode_count)) >> 6;
                g = (g * (64 - bossdata.explode_count)) >> 6;
                b = (b * (64 - bossdata.explode_count)) >> 6;
                shptr->rgb = r | (g << 8) | (b << 16);
            } else {
                shptr->rgb = 0;
            }
            angle += 512;
            angle &= 4095;
            shptr++;
        }
    }
}

sint16 CreateEffect(sint16 room_num);

void TriggerPlasmaBall_willboss(PHD_VECTOR *pos, sint16 room_number, sint16 angle, sint16 type)
{
    FX_INFO *fx;
    sint16 fx_number;

    fx_number = CreateEffect(room_number);
    if (fx_number != NO_ITEM) {
        fx = &effects[fx_number];
        fx->pos.x_pos = pos->x;
        fx->pos.y_pos = pos->y;
        fx->pos.z_pos = pos->z;
        fx->pos.x_rot = 0;
        fx->pos.y_rot = angle;
        fx->object_number = 369;
        fx->speed = 16 + type ? (GetRandomControl() & 31) + 16 : 0;
        fx->fallspeed = -(type << 4);
        fx->flag1 = type;
    }
}

extern sint32 wibble;
void phd_GetVectorAngles(sint32 x, sint32 y, sint32 z, PHD_ANGLE *dest);
void TriggerPlasmaBallFlame_willboss(sint16 fx_number, sint32 type, sint32 xv, sint32 yv, sint32 zv);
void KillEffect(sint16 fx_num);
void EffectNewRoom(sint16 fx_num, sint16 room_number);
int ItemNearLara(PHD_3DPOS *pos, sint32 radius);
void LaraBurn(void);
sint32 GetFreeSpark(void);

void ControlWillbossPlasmaBall(sint16 fx_number)
{
    FLOOR_INFO *floor;
    FX_INFO *fx;
    sint32 old_x, old_y, old_z, speed;
    sint32 tx, ty, tz;
    sint32 rnd, r, g, b;
    uint8 radtab[5] = {13, 7, 7, 7, 7};
    sint16 room_number;
    PHD_ANGLE angles[2];

    fx = &effects[fx_number];
    old_x = fx->pos.x_pos;
    old_y = fx->pos.y_pos;
    old_z = fx->pos.z_pos;
    if (fx->flag1 == 0) {
        tx = lara_item->pos.x_pos;
        ty = lara_item->pos.y_pos - 256;
        tz = lara_item->pos.z_pos;
        phd_GetVectorAngles(tx - fx->pos.x_pos, ty - fx->pos.y_pos, tz - fx->pos.z_pos, angles);
        fx->pos.y_rot = angles[0];
        fx->pos.x_rot = angles[1];
        if (fx->speed < 512)
            fx->speed += (fx->speed >> 4) + 4;
        if (wibble & 4)
            TriggerPlasmaBallFlame_willboss(fx_number, fx->speed >> 1, 0, 0, 0);
    } else {
        fx->fallspeed += (fx->flag1 == 1) ? 1 : 2;
        if (!(wibble & 12)) {
            if (fx->speed)
                fx->speed--;
            TriggerPlasmaBallFlame_willboss(fx_number, -1 - fx->flag1, 0, -(GetRandomControl() & 31), 0);
        }
    }
    speed = (fx->speed * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT;
    fx->pos.z_pos += (speed * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT;
    fx->pos.x_pos += (speed * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT;
    fx->pos.y_pos += -((fx->speed * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT) + fx->fallspeed;
    room_number = fx->room_number;
    floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);
    if (fx->pos.y_pos >= GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos) || fx->pos.y_pos < GetCeiling(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos)) {
        PHD_VECTOR pos;

        if (fx->flag1 == 0) {
            pos.x = old_x;
            pos.y = old_y;
            pos.z = old_z;
            r = 2 + (GetRandomControl() & 3);
            for (g = 0; g < r; g++)
                TriggerPlasmaBall_willboss(&pos, fx->room_number, fx->pos.y_rot + 0x6000 + (GetRandomControl() & 0x3fff), 1);
        }
        KillEffect(fx_number);
        return;
    }
    if (ItemNearLara(&fx->pos, 200) && !fx->flag1) {
        PHD_VECTOR pos;

        for (g = 14; g >= 0; g -= 2) {
            pos.x = 0;
            pos.y = 0;
            pos.z = 0;
            GetJointAbsPosition(lara_item, &pos, g);
            TriggerPlasmaBall_willboss(&pos, fx->room_number, GetRandomControl() << 1, 1);
        }
        LaraBurn();
        lara_item->hit_points = -1;
        lara.BurnGreen = 1;
        KillEffect(fx_number);
        return;
    }
    if (room_number != fx->room_number)
        EffectNewRoom(fx_number, lara_item->room_number);
    if (radtab[fx->flag1]) {
        rnd = GetRandomControl();
        g = 31 - ((rnd >> 4) & 3);
        b = 24 - ((rnd >> 6) & 3);
        r = rnd & 7;
        TriggerDynamic(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, radtab[fx->flag1], r, g, b);
    }
}

void TriggerPlasmaBallFlame_willboss(sint16 fx_number, sint32 type, sint32 xv, sint32 yv, sint32 zv)
{
    SPARKS *sptr;
    sint32 size;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - effects[fx_number].pos.x_pos;
    dz = lara_item->pos.z_pos - effects[fx_number].pos.z_pos;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sG = 255;
    sptr->sB = 48 + (GetRandomControl() & 31);
    sptr->sR = 48;
    sptr->dG = 192 + (GetRandomControl() & 63);
    sptr->dB = 128 + (GetRandomControl() & 63);
    sptr->dR = 32;
    sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
    sptr->FadeToBlack = 8;
    sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 24;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = (GetRandomControl() & 15) - 8;
    sptr->y = 0;
    sptr->z = (GetRandomControl() & 15) - 8;
    sptr->Xvel = (xv + (GetRandomControl() & 255) - 128) << 1;
    sptr->Yvel = (GetRandomControl() & 511) - 256;
    sptr->Zvel = (zv + (GetRandomControl() & 255) - 128) << 1;
    sptr->Friction = 5 | (5 << 4);
    if (GetRandomControl() & 1) {
        sptr->Flags = 2 | 8 | 16 | 512 | 64;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 15) - 16;
        else
            sptr->RotAdd = (GetRandomControl() & 15) + 16;
    } else {
        sptr->Flags = 2 | 8 | 512 | 64;
    }
    sptr->FxObj = fx_number;
    sptr->Def = objects[296].mesh_index;
    sptr->Gravity = sptr->MaxYvel = 0;
    if (type >= 0) {
        sptr->Scalar = 3;
        size = type;
    } else {
        if (type < -2)
            sptr->Scalar = 4;
        else
            sptr->Scalar = 2;
        size = (GetRandomControl() & 15) + 16;
        sptr->Xvel = xv + (GetRandomControl() & 255) - 128;
        sptr->Yvel = yv;
        sptr->Zvel = zv + (GetRandomControl() & 255) - 128;
        sptr->Friction = 5;
    }
    sptr->Width = sptr->sWidth = size;
    sptr->Height = sptr->sHeight = size;
    sptr->dWidth = size >> 3;
    sptr->dHeight = size >> 3;
}
