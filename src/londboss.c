#include <windows.h>
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

extern EXPLOSION_RING ExpRings[];
extern sint32 death_radii_londboss[5];
extern sint32 death_heights_londboss[5];
extern sint32 dradii_londboss[5];
extern sint32 dheights1_londboss[5];
extern sint32 dheights2_londboss[5];
extern sint32 wibble;

void TriggerExplosionSparks(sint32 x, sint32 y, sint32 z, sint32 extra_trigs, sint32 dyn, sint32 uw, sint16 roomnum);

void ExplodeLondonBoss(ITEM_INFO *item)
{
    SHIELD_POINTS *shptr;
    sint32 lp, lp1, angle, y, r, g, b;

    if (bossdata.explode_count == 1 || bossdata.explode_count == 15 || bossdata.explode_count == 25 ||
        bossdata.explode_count == 35 || bossdata.explode_count == 45 || bossdata.explode_count == 55) {
        sint32 x, y, z;

        x = item->pos.x_pos + ((GetRandomDraw() & 1023) - 512);
        y = item->pos.y_pos - (GetRandomDraw() & 1023) - 256;
        z = item->pos.z_pos + ((GetRandomDraw() & 1023) - 512);

        ExpRings[bossdata.ring_count].x = x;
        ExpRings[bossdata.ring_count].y = y;
        ExpRings[bossdata.ring_count].z = z;
        ExpRings[bossdata.ring_count].on = 1;
        bossdata.ring_count++;

        TriggerExplosionSparks(x, y, z, 3, -2, 2, 0);
        for (lp = 0; lp < 2; lp++) {
            TriggerExplosionSparks(x, y, z, 3, -1, 2, 0);
        }

        SoundEffect(76, &item->pos, 0x800004);
    }

    for (lp = 0; lp < 5; lp++) {
        if (bossdata.explode_count < 128) {
            death_radii_londboss[lp] = (dradii_londboss[lp] >> 4) + ((dradii_londboss[lp] * bossdata.explode_count) >> 7);
            death_heights_londboss[lp] = dheights2_londboss[lp] + (((dheights1_londboss[lp] - dheights2_londboss[lp]) * bossdata.explode_count) >> 7);
        }
    }

    shptr = &LondonBossShield[0];

    for (lp = 0; lp < 5; lp++) {
        y = death_heights_londboss[lp];
        angle = (wibble << 3) & 511;
        for (lp1 = 0; lp1 < 8; lp1++) {
            shptr->x = (rcossin_tbl[angle << 1] * death_radii_londboss[lp]) >> 11;
            shptr->y = y;
            shptr->z = (rcossin_tbl[(angle << 1) + 1] * death_radii_londboss[lp]) >> 11;

            if (lp != 0 && lp != 4 && bossdata.explode_count < 64) {
                r = (GetRandomDraw() & 31) + 224;
                g = (r >> 2) + (GetRandomDraw() & 63);
                b = GetRandomDraw() & 63;
                r = (r * (64 - bossdata.explode_count)) >> 6;
                g = (g * (64 - bossdata.explode_count)) >> 6;
                b = (b * (64 - bossdata.explode_count)) >> 6;
                shptr->rgb = b | (r << 8) | (g << 16);
            } else {
                shptr->rgb = 0;
            }
            angle += 512;
            angle &= 4095;
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

#define GUARD 1
#define AMBUSH 2
#define PATROL1 4
#define FOLLOW 16
#define VIOLENT 1

#define KNOCKBACK 2
#define NORMAL_BOLT 0
#define LARGE_BOLT 1
#define SUMMON_BOLT 2
#define RIGHT_PRONG 0
#define ICONPOS 1
#define LEFT_PRONG 2

#define LONDONBOSS_STAND 1
#define LONDONBOSS_WALK 2
#define LONDONBOSS_RUN 3
#define LONDONBOSS_SUMMON 4
#define LONDONBOSS_BIGZAP 5
#define LONDONBOSS_DEATH 6
#define LONDONBOSS_LAUGH 7
#define LONDONBOSS_LILZAP 8
#define LONDONBOSS_VAULT2 9
#define LONDONBOSS_VAULT3 10
#define LONDONBOSS_VAULT4 11
#define LONDONBOSS_GODOWN 12

#define LONDONBOSS_VAULT2_ANIM 9
#define LONDONBOSS_VAULT3_ANIM 18
#define LONDONBOSS_VAULT4_ANIM 15
#define LONDONBOSS_GODOWN_ANIM 21
#define LONDONBOSS_STND2SUM_ANIM 1
#define LONDONBOSS_SUMMON_ANIM 2
#define LONDONBOSS_DIE_ANIM 17
#define LONDONBOSS_VAULT_SHIFT 96
#define LONDONBOSS_AWARE_DISTANCE SQUARE(WALL_L)
#define LONDONBOSS_WALK_TURN (ONE_DEGREE * 4)
#define LONDONBOSS_RUN_TURN (ONE_DEGREE * 7)
#define LONDONBOSS_WALK_RANGE SQUARE(WALL_L)
#define LONDONBOSS_LAUGH_CHANCE 0x100
#define LONDONBOSS_FINAL_HEIGHT -11776
#define BIGZAP_TIMER 600

typedef struct {
    sint16 zone_number;
    sint16 enemy_zone;
    sint32 distance;
    sint32 ahead;
    sint32 bite;
    sint16 angle;
    sint16 x_angle;
    sint16 enemy_facing;
} AI_INFO;

extern BITE_INFO londonboss_points[3];
extern EXPLOSION_RING KBRings[];

int CreatureActive(sint16 item_number);
void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info);
void GetCreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
void CreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn);
int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);
void CreatureTilt(ITEM_INFO *item, sint16 angle);
void CreatureJoint(ITEM_INFO *item, sint16 joint, sint16 required);
int CreatureVault(sint16 item_number, sint16 angle, int vault, int shift);
void GetAITarget(CREATURE_INFO *creature);
void AlertAllGuards(sint16 item_number);
sint16 AIGuard(CREATURE_INFO *creature);
int TargetVisible(ITEM_INFO *item, AI_INFO *info);
void BossDropIcon(sint16 item_number);
void LondonBossDie(sint16 item_number);
void TriggerDynamic(sint32 x, sint32 y, sint32 z, sint32 falloff, sint32 r, sint32 g, sint32 b);
void StopSoundEffect(int sample_index);
int SoundEffect(int nSFX, PHD_3DPOS *pPos, int flags);

void LondonBossControl(sint16 item_number)
{
    ITEM_INFO *item, *enemy;
    CREATURE_INFO *londonboss;
    sint16 angle, head, tilt, lp, torso_y, torso_x;
    sint32 lara_dx, lara_dz, lara_dy;
    AI_INFO info, lara_info;
    PHD_VECTOR pos1, trident[3];

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    londonboss = (CREATURE_INFO *)item->data;
    head = angle = tilt = torso_x = torso_y = 0;

    if (item->item_flags[2]) {
        if (item->item_flags[2] == 1)
            item->hit_points = 0;
        if (item->item_flags[2] >= 12)
            TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, (GetRandomControl() & 3) + 8, 0,
                (GetRandomControl() & 7) + 8, (GetRandomControl() & 7) + 16);
        else
            TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos,
                25 - (item->item_flags[2] << 1) + (GetRandomControl() & 1),
                (16 - item->item_flags[2]) + (GetRandomControl() & 7), 32 - item->item_flags[2], 31);
    }

    for (lp = 0; lp < 3; lp++) {
        trident[lp].x = londonboss_points[lp].x;
        trident[lp].y = londonboss_points[lp].y;
        trident[lp].z = londonboss_points[lp].z;
        GetJointAbsPosition(item, &trident[lp], londonboss_points[lp].mesh_num);
    }

    if (item->hit_points <= 0) {
        if (item->current_anim_state != LONDONBOSS_DEATH) {
            item->anim_number = objects[item->object_number].anim_index + LONDONBOSS_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = LONDONBOSS_DEATH;
        }
        if (anims[item->anim_number].frame_end - item->frame_number == 1) {
            item->frame_number = anims[item->anim_number].frame_end - 1;
            item->mesh_bits = 0;
            if (bossdata.explode_count == 0) {
                bossdata.ring_count = 0;
                for (lp = 0; lp < 6; lp++) {
                    ExpRings[lp].on = 0;
                    ExpRings[lp].life = 32;
                    ExpRings[lp].radius = 512;
                    ExpRings[lp].speed = 128 + (lp << 5);
                    ExpRings[lp].xrot = ((GetRandomControl() & 511) - 256) << 4;
                    ExpRings[lp].zrot = ((GetRandomControl() & 511) - 256) << 4;
                }
                if (bossdata.dropped_icon == 0) {
                    BossDropIcon(item_number);
                    bossdata.dropped_icon = 1;
                }
            }
            if (bossdata.explode_count < 256)
                bossdata.explode_count++;
            if (bossdata.explode_count > 128 && bossdata.ring_count == 6 && ExpRings[5].life == 0) {
                LondonBossDie(item_number);
                bossdata.dead = 1;
            } else
                ExplodeLondonBoss(item);
            return;
        }
    } else {
        if (item->ai_bits)
            GetAITarget(londonboss);
        CreatureAIInfo(item, &info);

        if (londonboss->enemy == lara_item) {
            lara_info.angle = info.angle;
            lara_info.distance = info.distance;
            lara_info.x_angle = info.x_angle;
        } else {
            lara_dz = lara_item->pos.z_pos - item->pos.z_pos;
            lara_dx = lara_item->pos.x_pos - item->pos.x_pos;
            lara_dy = item->pos.y_pos - lara_item->pos.y_pos;
            lara_info.angle = phd_atan(lara_dz, lara_dx) - item->pos.y_rot;
            lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
            if (abs(lara_dx) > abs(lara_dz))
                lara_info.x_angle = phd_atan(abs(lara_dx) + (abs(lara_dz) >> 1), lara_dy);
            else
                lara_info.x_angle = phd_atan(abs(lara_dz) + (abs(lara_dx) >> 1), lara_dy);
        }

        GetCreatureMood(item, &info, VIOLENT);
        CreatureMood(item, &info, VIOLENT);
        angle = CreatureTurn(item, londonboss->maximum_turn);

        enemy = londonboss->enemy;
        londonboss->enemy = lara_item;
        if (item->hit_status || lara_info.distance < LONDONBOSS_AWARE_DISTANCE || TargetVisible(item, &lara_info)
            || lara_item->pos.y_pos < item->pos.y_pos)
            AlertAllGuards(item_number);
        londonboss->enemy = enemy;

        if (lara_item->pos.y_pos < item->pos.y_pos)
            londonboss->hurt_by_lara = 1;
        if (item->timer > 0)
            item->timer--;
        item->hit_points = 300;

        switch (item->current_anim_state) {
        case LONDONBOSS_LAUGH:
            if (abs(lara_info.angle) < LONDONBOSS_WALK_TURN)
                item->pos.y_rot += lara_info.angle;
            else if (lara_info.angle < 0)
                item->pos.y_rot -= LONDONBOSS_WALK_TURN;
            else
                item->pos.y_rot += LONDONBOSS_WALK_TURN;
            if (londonboss->alerted) {
                item->goal_anim_state = LONDONBOSS_STAND;
                break;
            }

        case LONDONBOSS_STAND:
            londonboss->flags = 0;
            londonboss->maximum_turn = 0;
            if (londonboss->reached_goal) {
                londonboss->reached_goal = 0;
                item->ai_bits |= AMBUSH;
                item->item_flags[3] += 0x2000;
            }
            head = lara_info.angle;
            if (item->ai_bits & GUARD) {
                if ((head < -0x3000 || head > 0x3000) && item->pos.y_pos > LONDONBOSS_FINAL_HEIGHT) {
                    item->goal_anim_state = LONDONBOSS_WALK;
                    londonboss->maximum_turn = LONDONBOSS_WALK_TURN;
                }
                break;
                head = AIGuard(londonboss);
                if (!(GetRandomControl() & 0xf))
                    item->goal_anim_state = LONDONBOSS_LAUGH;
                break;
            } else if ((item->pos.y_pos <= LONDONBOSS_FINAL_HEIGHT || item->pos.y_pos < lara_item->pos.y_pos)
                && !(GetRandomControl() & 0xf) && !bossdata.charged && item->timer)
                item->goal_anim_state = LONDONBOSS_LAUGH;
            else if (londonboss->reached_goal || lara_item->pos.y_pos > item->pos.y_pos
                || item->pos.y_pos <= LONDONBOSS_FINAL_HEIGHT) {
                if (bossdata.charged)
                    item->goal_anim_state = LONDONBOSS_BIGZAP;
                else if (!item->timer)
                    item->goal_anim_state = LONDONBOSS_SUMMON;
                else
                    item->goal_anim_state = LONDONBOSS_LILZAP;
            } else if (item->ai_bits & PATROL1)
                item->goal_anim_state = LONDONBOSS_WALK;
            else if (londonboss->mood == ESCAPE_MOOD || item->pos.y_pos > lara_item->pos.y_pos)
                item->goal_anim_state = LONDONBOSS_RUN;
            else if (londonboss->mood == BORED_MOOD
                || ((item->ai_bits & FOLLOW) && (londonboss->reached_goal || lara_info.distance > SQUARE(WALL_L * 2)))) {
                if (item->required_anim_state)
                    item->goal_anim_state = item->required_anim_state;
                else if (info.ahead)
                    item->goal_anim_state = LONDONBOSS_STAND;
                else
                    item->goal_anim_state = LONDONBOSS_RUN;
            } else if (info.bite && info.distance < LONDONBOSS_WALK_RANGE)
                item->goal_anim_state = LONDONBOSS_WALK;
            else
                item->goal_anim_state = LONDONBOSS_RUN;
            break;

        case LONDONBOSS_WALK:
            head = lara_info.angle;
            londonboss->flags = 0;
            londonboss->maximum_turn = LONDONBOSS_WALK_TURN;
            if (item->ai_bits & GUARD || (londonboss->reached_goal && !(item->ai_bits & FOLLOW)))
                item->goal_anim_state = LONDONBOSS_STAND;
            else if (item->ai_bits & PATROL1) {
                item->goal_anim_state = LONDONBOSS_WALK;
                head = 0;
            } else if (londonboss->mood == ESCAPE_MOOD)
                item->goal_anim_state = LONDONBOSS_RUN;
            else if (londonboss->mood == BORED_MOOD) {
                if (GetRandomControl() < LONDONBOSS_LAUGH_CHANCE) {
                    item->required_anim_state = LONDONBOSS_LAUGH;
                    item->goal_anim_state = LONDONBOSS_STAND;
                }
            } else if (info.distance > LONDONBOSS_WALK_RANGE)
                item->goal_anim_state = LONDONBOSS_RUN;
            break;

        case LONDONBOSS_RUN:
            if (info.ahead)
                head = info.angle;
            londonboss->maximum_turn = LONDONBOSS_RUN_TURN;
            tilt = angle / 2;
            if (item->ai_bits & GUARD || (londonboss->reached_goal && !(item->ai_bits & FOLLOW)))
                item->goal_anim_state = LONDONBOSS_STAND;
            else if (londonboss->mood == ESCAPE_MOOD)
                break;
            else if ((item->ai_bits & FOLLOW) && (londonboss->reached_goal || lara_info.distance > SQUARE(WALL_L * 2)))
                item->goal_anim_state = LONDONBOSS_STAND;
            else if (londonboss->mood == BORED_MOOD)
                item->goal_anim_state = LONDONBOSS_WALK;
            else if (info.ahead && info.distance < LONDONBOSS_WALK_RANGE)
                item->goal_anim_state = LONDONBOSS_WALK;
            break;

        case LONDONBOSS_SUMMON:
            head = lara_info.angle;
            if (londonboss->reached_goal) {
                londonboss->reached_goal = 0;
                item->ai_bits = AMBUSH;
                item->item_flags[3] += 0x2000;
            }
            if (item->anim_number == objects[item->object_number].anim_index + LONDONBOSS_STND2SUM_ANIM) {
                if (item->frame_number == anims[item->anim_number].frame_base) {
                    bossdata.hp_counter = item->hit_points;
                    item->timer = BIGZAP_TIMER;
                } else if (item->hit_status && item->goal_anim_state != LONDONBOSS_STAND) {
                    StopSoundEffect(352);
                    SoundEffect(353, &item->pos, 0);
                    SoundEffect(355, &item->pos, 0);
                    item->goal_anim_state = LONDONBOSS_STAND;
                }
            } else if (item->anim_number == objects[item->object_number].anim_index + LONDONBOSS_SUMMON_ANIM
                && item->frame_number == anims[item->anim_number].frame_end)
                bossdata.charged = 1;
            if (abs(lara_info.angle) < LONDONBOSS_WALK_TURN)
                item->pos.y_rot += lara_info.angle;
            else if (lara_info.angle < 0)
                item->pos.y_rot -= LONDONBOSS_WALK_TURN;
            else
                item->pos.y_rot += LONDONBOSS_WALK_TURN;
            if ((wibble & 7) == 0) {
                pos1.x = item->pos.x_pos;
                pos1.y = (GetRandomControl() & 511) - 256;
                pos1.z = item->pos.z_pos;
                TriggerLaserBolt(&pos1, item, SUMMON_BOLT, 0);
                for (lp = 0; lp < 6; lp++) {
                    sint32 rnd;

                    if (!ExpRings[lp].on) {
                        ExpRings[lp].on = 1;
                        ExpRings[lp].life = 64;
                        ExpRings[lp].speed = (GetRandomControl() & 15) + 16;
                        ExpRings[lp].x = item->pos.x_pos;
                        ExpRings[lp].y = item->pos.y_pos + 128 - (rnd = (GetRandomControl() & 1023));
                        ExpRings[lp].z = item->pos.z_pos;
                        ExpRings[lp].xrot = ((GetRandomControl() & 511) - 256) << 4;
                        ExpRings[lp].zrot = ((GetRandomControl() & 511) - 256) << 4;
                        ExpRings[lp].radius = (1024 + (1024 - (abs(rnd - 512))));
                        break;
                    }
                }
            }
            londonboss->maximum_turn = 0;
            break;

        case LONDONBOSS_BIGZAP:
            if (londonboss->reached_goal) {
                londonboss->reached_goal = 0;
                item->ai_bits = AMBUSH;
                item->item_flags[3] += 0x2000;
            }
            bossdata.charged = 0;
            if (abs(lara_info.angle) < LONDONBOSS_WALK_TURN)
                item->pos.y_rot += lara_info.angle;
            else if (lara_info.angle < 0)
                item->pos.y_rot -= LONDONBOSS_WALK_TURN;
            else
                item->pos.y_rot += LONDONBOSS_WALK_TURN;
            torso_y = lara_info.angle;
            torso_x = lara_info.x_angle;
            londonboss->maximum_turn = 0;
            if (item->frame_number == anims[item->anim_number].frame_base + 36) {
                TriggerLaserBolt(&trident[RIGHT_PRONG], item, NORMAL_BOLT, item->pos.y_rot + 0x200);
                TriggerLaserBolt(&trident[ICONPOS], item, LARGE_BOLT, item->pos.y_rot);
                TriggerLaserBolt(&trident[LEFT_PRONG], item, NORMAL_BOLT, item->pos.y_rot - 0x200);
            }
            break;

        case LONDONBOSS_LILZAP:
            if (londonboss->reached_goal) {
                londonboss->reached_goal = 0;
                item->ai_bits = AMBUSH;
                item->item_flags[3] += 0x2000;
            }
            if (abs(lara_info.angle) < LONDONBOSS_WALK_TURN)
                item->pos.y_rot += lara_info.angle;
            else if (lara_info.angle < 0)
                item->pos.y_rot -= LONDONBOSS_WALK_TURN;
            else
                item->pos.y_rot += LONDONBOSS_WALK_TURN;
            torso_y = lara_info.angle;
            torso_x = lara_info.x_angle;
            londonboss->maximum_turn = 0;
            if (item->frame_number == anims[item->anim_number].frame_base + 14) {
                TriggerLaserBolt(&trident[RIGHT_PRONG], item, NORMAL_BOLT, item->pos.y_rot + 0x200);
                TriggerLaserBolt(&trident[LEFT_PRONG], item, NORMAL_BOLT, item->pos.y_rot - 0x200);
            }
            break;
        }
    }

    CreatureTilt(item, tilt);
    CreatureJoint(item, 0, torso_y);
    CreatureJoint(item, 1, torso_x);
    CreatureJoint(item, 2, head);

    if ((item->current_anim_state < LONDONBOSS_VAULT2 || item->current_anim_state > LONDONBOSS_GODOWN)
        && item->current_anim_state != LONDONBOSS_DEATH) {
        switch (CreatureVault(item_number, angle, 2, LONDONBOSS_VAULT_SHIFT)) {
        case 2:
            londonboss->maximum_turn = 0;
            item->anim_number = objects[57].anim_index + LONDONBOSS_VAULT2_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = LONDONBOSS_VAULT2;
            break;
        case 3:
            londonboss->maximum_turn = 0;
            item->anim_number = objects[57].anim_index + LONDONBOSS_VAULT3_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = LONDONBOSS_VAULT3;
            break;
        case 4:
            londonboss->maximum_turn = 0;
            item->anim_number = objects[57].anim_index + LONDONBOSS_VAULT4_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = LONDONBOSS_VAULT4;
            break;
        case -4:
            londonboss->maximum_turn = 0;
            item->anim_number = objects[57].anim_index + LONDONBOSS_GODOWN_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = LONDONBOSS_GODOWN;
            break;
        }
    } else {
        londonboss->maximum_turn = 0;
        CreatureAnimation(item_number, angle, 0);
    }

    {
        sint32 g, b;

        g = abs(rcossin_tbl[item->item_flags[1] << 7] >> 7);
        g += (GetRandomControl() & 7);
        if (g > 31)
            g = 31;
        b = g >> 1;
        TriggerDynamic(trident[ICONPOS].x, trident[ICONPOS].y, trident[ICONPOS].z, 10, 0, g >> 1, b >> 1);
        item->item_flags[1]++;
        item->item_flags[1] &= 63;
    }

    if (item->hit_points > 0 && item->item_flags[0] != KNOCKBACK && lara_item->hit_points > 0) {
        sint32 dx, dy, dz;

        dx = lara_item->pos.x_pos - item->pos.x_pos;
        dy = lara_item->pos.y_pos - 256 - item->pos.y_pos;
        dz = lara_item->pos.z_pos - item->pos.z_pos;
        if (dx > 8000 || dx < -8000 || dy > 8000 || dy < -8000 || dz > 8000 || dz < -8000)
            dx = 0xfff;
        else {
            dx = SQUARE(dx);
            dy = SQUARE(dy);
            dz = SQUARE(dz);
            dx = phd_sqrt(dx + dy + dz);
        }
        if (dx < 0xb00) {
            item->item_flags[0] = KNOCKBACK;
            for (lp = 0; lp < 3; lp++) {
                KBRings[lp].on = 1;
                KBRings[lp].life = 32;
                KBRings[lp].speed = 64 + ((lp == 1) << 4);
                KBRings[lp].x = item->pos.x_pos;
                KBRings[lp].y = item->pos.y_pos - 384 - 128 + (lp << 7);
                KBRings[lp].z = item->pos.z_pos;
                KBRings[lp].xrot = KBRings[lp].zrot = 0;
                KBRings[lp].radius = 512 + ((lp == 1) << 8);
            }
        }
    } else if (KBRings[0].on == 0 && KBRings[1].on == 0 && KBRings[2].on == 0)
        item->item_flags[0] = 0;
}

void TriggerPlasmaBall_londboss(ITEM_INFO *item, sint32 type, PHD_VECTOR *pos, sint16 room_number, sint16 angle)
{
    FX_INFO *fx;
    PHD_VECTOR p;
    sint16 fx_number;
    sint16 speed;
    sint16 angles[2];

    p.x = pos->x;
    p.y = pos->y;
    p.z = pos->z;
    speed = (GetRandomControl() & 0x1f) + 64;
    angles[0] = GetRandomControl() + 0x4000 + angle;
    fx_number = CreateEffect(room_number);
    if (fx_number != NO_ITEM) {
        fx = &effects[fx_number];
        fx->speed = speed;
        fx->pos.y_rot = angles[0];
        fx->pos.x_pos = p.x;
        fx->pos.y_pos = p.y;
        fx->pos.z_pos = p.z;
        fx->pos.x_rot = 0x2000;
        fx->object_number = 372;
        fx->fallspeed = 0;
        fx->flag1 = 1;
        if (type == 2)
            fx->flag2 = 1;
        else
            fx->flag2 = 0;
    }
}

#ifdef TR3_EXE
void LondonBossDie(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->collidable = 0;
    item->hit_points = DONT_TARGET;
    KillItem(item_number);
    DisableBaddieAI(item_number);
    item->flags |= ONESHOT;
}
#endif

#ifdef TR3_EXE
BITE_INFO londonboss_points[3] = {
    { 16, 56, 356, 10 },
    { -28, 48, 304, 10 },
    { -72, 48, 356, 10 },
};
#endif
