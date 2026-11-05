#include "game.h"

void DrawLondonBossShield(ITEM_INFO *item);
void DrawSummonRings(void);
void DrawKnockBackRings(void);
void UpdateElectricityPoints(void);
void LaraElectricDeath(sint32 copy, ITEM_INFO *item);

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

extern sint16 rcossin_tbl[];
extern sint32 LondonBossRadii[5];
extern sint32 LondonBossHeights[5];
extern SHIELD_POINTS LondonBossShield[40];

void S_DrawLondonBoss(ITEM_INFO *item)
{
    DrawAnimatingItem(item);
    if (bossdata.explode_count)
    {
        DrawLondonBossShield(item);
        DrawExplosionRings();
    }
    else
    {
        DrawSummonRings();
        DrawKnockBackRings();
    }
    if (item->hit_points <= 0 && bossdata.explode_count == 0)
    {
        UpdateElectricityPoints();
        LaraElectricDeath(0, item);
        LaraElectricDeath(1, item);
    }
}

void InitialiseLondonBoss(sint16 item_number)
{
    SHIELD_POINTS *shptr;
    sint32 lp, lp1, angle, y;

    bossdata.explode_count = bossdata.ring_count = bossdata.dead = bossdata.dropped_icon = 0;
    shptr = &LondonBossShield[0];
    for (lp = 0; lp < 5; lp++) {
        y = LondonBossHeights[lp];
        angle = 0;
        for (lp1 = 0; lp1 < 8; lp1++) {
            shptr->x = (rcossin_tbl[angle << 1] * LondonBossRadii[lp]) >> 11;
            shptr->y = y;
            shptr->z = (rcossin_tbl[(angle << 1) + 1] * LondonBossRadii[lp]) >> 11;
            shptr->rgb = 0;
            angle += 512;
            shptr++;
        }
    }
}

sint16 CreateItem(void);
void phd_GetVectorAngles(sint32 x, sint32 y, sint32 z, PHD_ANGLE *dest);
void InitialiseItem(sint16 item_num);

void TriggerLaserBolt(PHD_VECTOR *pos, ITEM_INFO *item, sint32 type, sint16 yang)
{
    sint16 item_number;
    ITEM_INFO *bolt_item;
    PHD_ANGLE angles[2];

    item_number = CreateItem();
    if (item_number != NO_ITEM) {
        bolt_item = &items[item_number];
        bolt_item->object_number = 371;
        bolt_item->room_number = item->room_number;
        bolt_item->pos.x_pos = pos->x;
        bolt_item->pos.y_pos = pos->y;
        bolt_item->pos.z_pos = pos->z;
        InitialiseItem(item_number);
        if (type == 2) {
            bolt_item->pos.y_pos += item->pos.y_pos - 384;
            bolt_item->pos.x_rot = -pos->y << 5;
            bolt_item->pos.y_rot = GetRandomControl() << 1;
        } else {
            phd_GetVectorAngles(lara_item->pos.x_pos - pos->x, lara_item->pos.y_pos - 256 - pos->y,
                                lara_item->pos.z_pos - pos->z, angles);
            bolt_item->pos.x_rot = angles[1];
            bolt_item->pos.y_rot = yang;
            bolt_item->pos.z_rot = 0;
        }
        if (type != 1) {
            bolt_item->speed = 16;
            bolt_item->item_flags[0] = -24;
            bolt_item->item_flags[1] = 4;
            if (type == 2)
                bolt_item->item_flags[2] = 1;
        } else {
            bolt_item->speed = 24;
            bolt_item->item_flags[0] = 31;
            bolt_item->item_flags[1] = 16;
        }
        AddActiveItem(item_number);
    }
}

#define SQUARE(A) ((A) * (A))

void TriggerExplosionSparks(sint32 x, sint32 y, sint32 z, sint32 extra_trigs, sint32 dyn, sint32 uw, sint16 roomnum);
void TriggerPlasmaBall_londboss(ITEM_INFO *item, sint32 type, PHD_VECTOR *pos, sint16 room_number, sint16 angle);
void TriggerDynamic(sint32 x, sint32 y, sint32 z, sint32 falloff, sint32 r, sint32 g, sint32 b);
int ItemNearLara(PHD_3DPOS *pos, sint32 radius);

void ControlLaserBolts(sint16 item_number)
{
    FLOOR_INFO *floor;
    ITEM_INFO *item;
    PHD_VECTOR oldpos;
    sint32 g, b, rad;
    sint16 room_number, oldroom, hitlara;
    sint32 speed;

    item = &items[item_number];
    oldpos.x = item->pos.x_pos;
    oldpos.y = item->pos.y_pos;
    oldpos.z = item->pos.z_pos;
    oldroom = item->room_number;
    speed = (item->speed * phd_cos(item->pos.x_rot)) >> W2V_SHIFT;
    item->pos.z_pos += (speed * phd_cos(item->pos.y_rot)) >> W2V_SHIFT;
    item->pos.x_pos += (speed * phd_sin(item->pos.y_rot)) >> W2V_SHIFT;
    item->pos.y_pos += -((item->speed * phd_sin(item->pos.x_rot)) >> W2V_SHIFT);
    if (item->speed < 384)
        item->speed += (item->speed >> 3) + 2;
    if (item->item_flags[2] && item->speed > (384 >> 1)) {
        item->item_flags[3]++;
        if (item->item_flags[3] >= 16) {
            KillItem(item_number);
            return;
        }
    }

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (item->room_number != room_number)
        ItemNewRoom(item_number, room_number);

    if (!item->item_flags[2]) {
        if (ItemNearLara(&item->pos, 400))
            hitlara = 1;
        else
            hitlara = 0;
        item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
        if (hitlara || item->pos.y_pos >= item->floor
            || item->pos.y_pos <= GetCeiling(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos)) {
            SoundEffect(105, &item->pos, 0);
            if (item->item_flags[0] < 0)
                rad = 2;
            else
                rad = 3;
            TriggerExplosionSparks(oldpos.x, oldpos.y, oldpos.z, rad, -2, 2, item->room_number);
            for (g = 0; g < rad; g++)
                TriggerExplosionSparks(oldpos.x, oldpos.y, oldpos.z, 2, -1, 2, item->room_number);
            rad++;
            for (g = 0; g < rad; g++)
                TriggerPlasmaBall_londboss(item, 1, &oldpos, oldroom, item->pos.y_rot);
            if (hitlara) {
                lara_item->hit_points -= 30 + ((item->item_flags[0] >= 0) << 9);
                lara_item->hit_status = 1;
            } else {
                sint32 dx, dy, dz;

                dx = SQUARE(lara_item->pos.x_pos - item->pos.x_pos);
                dy = SQUARE(lara_item->pos.y_pos - 256 - item->pos.y_pos);
                dz = SQUARE(lara_item->pos.z_pos - item->pos.z_pos);
                dx = phd_sqrt(dx + dy + dz);
                if (dx < 0x400) {
                    lara_item->hit_points -= (0x400 - dx) >> (6 - ((item->item_flags[0] >= 0) << 1));
                    lara_item->hit_status = 1;
                }
            }
            KillItem(item_number);
            return;
        }
    }

    g = 31 - (GetRandomControl() & 7);
    b = g >> 1;
    if (item->item_flags[0] < 0) {
        if (item->item_flags[2]) {
            g = (g * (16 - item->item_flags[3])) >> 4;
            b = (b * (16 - item->item_flags[3])) >> 4;
        }
        rad = -item->item_flags[0];
        if (rad > 10) {
            item->item_flags[0]++;
            item->item_flags[1] += 2;
        }
    } else {
        rad = item->item_flags[0];
        if (rad > 16) {
            item->item_flags[0]--;
            item->item_flags[1] += 4;
        }
    }
    TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, rad, 0, g, b);
}

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

int abs(int x);
#pragma intrinsic(abs)
extern sint32 wibble;
void TriggerPlasmaBallFlame_londboss(sint16 fx_number, sint32 type, sint32 xv, sint32 yv, sint32 zv);
int ItemNearLara(PHD_3DPOS *pos, sint32 radius);
void KillEffect(sint16 fx_num);
void EffectNewRoom(sint16 fx_num, sint16 room_number);

void ControlLondBossPlasmaBall(sint16 fx_number)
{
    FX_INFO *fx;
    FLOOR_INFO *floor;
    sint32 speed;
    sint32 rnd, r, g, b, old_y;
    uint8 radtab[2] = {13, 7};
    sint16 room_number;

    fx = &effects[fx_number];
    old_y = fx->pos.y_pos;
    fx->fallspeed++;
    if (fx->speed > 8)
        fx->speed -= 2;
    if (fx->pos.x_rot > -0x3c00)
        fx->pos.x_rot -= 0x100;
    speed = (fx->speed * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT;
    fx->pos.z_pos += (speed * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT;
    fx->pos.x_pos += (speed * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT;
    fx->pos.y_pos += -((fx->speed * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT) + fx->fallspeed;
    if (!(wibble & 15))
        TriggerPlasmaBallFlame_londboss(fx_number, 0, 0, abs(old_y - fx->pos.y_pos) << 3, 0);
    room_number = fx->room_number;
    floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);
    if (fx->pos.y_pos >= GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos) || fx->pos.y_pos < GetCeiling(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos)) {
        KillEffect(fx_number);
        return;
    }
    if (room[room_number].flags & UNDERWATER) {
        KillEffect(fx_number);
        return;
    }
    if (!fx->flag2 && ItemNearLara(&fx->pos, 200)) {
        lara_item->hit_points -= 25;
        lara_item->hit_status = 1;
        KillEffect(fx_number);
        return;
    }
    if (room_number != fx->room_number)
        EffectNewRoom(fx_number, lara_item->room_number);
    rnd = GetRandomControl();
    g = 31 - ((rnd >> 4) & 3);
    b = 24 - ((rnd >> 6) & 3);
    r = rnd & 7;
    TriggerDynamic(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, radtab[fx->flag1], r, g, b);
}

void TriggerPlasmaBallFlame_londboss(sint16 fx_number, sint32 type, sint32 xv, sint32 yv, sint32 zv)
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
    sptr->Xvel = xv + (GetRandomControl() & 255) - 128;
    sptr->Yvel = yv;
    sptr->Zvel = zv + (GetRandomControl() & 255) - 128;
    sptr->Friction = 5;
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
    sptr->Scalar = 1;
    size = (GetRandomControl() & 31) + 64;
    sptr->Width = sptr->sWidth = size;
    sptr->Height = sptr->sHeight = size;
    sptr->dWidth = size >> 2;
    sptr->dHeight = size >> 2;
    sptr->Gravity = sptr->MaxYvel = 0;
}

void TriggerExplosionSparks(sint32 x, sint32 y, sint32 z, sint32 extra_trigs, sint32 dyn, sint32 uw, sint16 roomnum);
void TriggerPlasmaBall_londboss(ITEM_INFO *item, sint32 type, PHD_VECTOR *pos, sint16 room_number, sint16 angle);

sint32 KnockBackCollision(EXPLOSION_RING *erptr)
{
    int radius, x, z, dist;
    sint16 angle, diff;

    radius = erptr->radius;
    x = lara_item->pos.x_pos - erptr->x;
    z = lara_item->pos.z_pos - erptr->z;
    if (x > 16000 || x < -16000 || z > 16000 || z < -16000)
        dist = 0x7fff;
    else
        dist = x * x + z * z;
    if (dist < radius * radius) {
        lara_item->hit_points -= 200;
        lara_item->hit_status = 1;
        angle = phd_atan(z, x);
        diff = lara_item->pos.y_rot - angle;
        if (ABS(diff) < 0x4000) {
            lara_item->speed = 75;
            lara_item->pos.y_rot = angle;
        } else {
            lara_item->pos.y_rot = (sint16)(angle - 0x8000);
            lara_item->speed = -75;
        }
        lara_item->gravity_status = 1;
        lara_item->fallspeed = -50;
        lara_item->pos.x_rot = lara_item->pos.z_rot = 0;
        lara_item->anim_number = FALLDOWN_A;
        lara_item->frame_number = FALLDOWN_F;
        lara_item->current_anim_state = AS_FORWARDJUMP;
        lara_item->goal_anim_state = AS_FORWARDJUMP;
        TriggerExplosionSparks(lara_item->pos.x_pos, lara_item->pos.y_pos, lara_item->pos.z_pos, 3, -2, 2,
                               lara_item->room_number);
        for (x = 0; x < 3; x++)
            TriggerPlasmaBall_londboss(lara_item, 2, (PHD_VECTOR *)&lara_item->pos, lara_item->room_number,
                                       GetRandomControl() << 1);
        return 1;
    }
    return 0;
}
