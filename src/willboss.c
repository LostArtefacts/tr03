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

sint32 GetFreeSpark(void);

void TriggerPlasma_willboss(sint16 item_number, sint32 node, sint32 size)
{
    ITEM_INFO *item;
    SPARKS *sptr;
    sint32 dx, dz;

    item = &items[item_number];
    dx = lara_item->pos.x_pos - item->pos.x_pos;
    dz = lara_item->pos.z_pos - item->pos.z_pos;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sG = 255;
    sptr->sB = (GetRandomControl() & 31) + 48;
    sptr->sR = 48;
    sptr->dG = (GetRandomControl() & 63) + 192;
    sptr->dB = (GetRandomControl() & 63) + 128;
    sptr->dR = 32;
    sptr->ColFadeSpeed = (GetRandomControl() & 3) + 12;
    sptr->FadeToBlack = 8;
    sptr->Life = (GetRandomControl() & 7) + 24;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->sLife = sptr->Life;
    sptr->Dynamic = -1;
    sptr->x = (GetRandomControl() & 15) - 8;
    sptr->y = 0;
    sptr->z = (GetRandomControl() & 15) - 8;
    sptr->Xvel = (GetRandomControl() & 31) - 16;
    sptr->Yvel = (GetRandomControl() & 7) + 8;
    sptr->Zvel = (GetRandomControl() & 31) - 16;
    sptr->Friction = 3;
    if (GetRandomControl() & 1) {
        sptr->Flags = 0x129a;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -16 - (GetRandomControl() & 15);
        else
            sptr->RotAdd = (GetRandomControl() & 15) + 16;
    } else {
        sptr->Flags = 0x128a;
    }
    sptr->Gravity = (GetRandomControl() & 7) + 8;
    sptr->MaxYvel = (GetRandomControl() & 7) + 16;
    sptr->FxObj = (uint8)item_number;
    sptr->NodeNumber = (uint8)node;
    sptr->Def = (uint8)objects[296].mesh_index;
    sptr->Scalar = 1;
    size += GetRandomControl() & 15;
    sptr->Width = sptr->sWidth = (uint8)size;
    sptr->Height = sptr->sHeight = (uint8)size;
    sptr->dWidth = (uint8)(size >> 2);
    sptr->dHeight = (uint8)(size >> 2);
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

#define WILLBOSS_STOP 0
#define WILLBOSS_WALK 1
#define WILLBOSS_LUNGE 2
#define WILLBOSS_BIGKILL 3
#define WILLBOSS_STUNNED 4
#define WILLBOSS_KNOCKOUT 5
#define WILLBOSS_GETUP 6
#define WILLBOSS_WALKATAK1 7
#define WILLBOSS_WALKATAK2 8
#define WILLBOSS_180 9
#define WILLBOSS_SHOOT 10

#define WILLBOSS_TURN (ONE_DEGREE * 5)
#define WILLBOSS_ATTACK_TURN (ONE_DEGREE * 2)
#define WILLBOSS_TOUCH 0x900000
#define WILLBOSS_BITE_DAMAGE 220
#define WILLBOSS_TOUCH_DAMAGE 10
#define WILLBOSS_ATTACK_RANGE SQUARE(WALL_L * 3 / 2)
#define WILLBOSS_LUNGE_RANGE SQUARE(WALL_L * 2)
#define WILLBOSS_FIRE_RANGE SQUARE(WALL_L * 4)
#define WILLBOSS_KILL_ANIM 6
#define WILLBOSS_HP_AFTER_KO 200
#define WILLBOSS_KO_TIME 280
#define WILLBOSS_PATH_DISTANCE 1024
#define WILLBOSS_STUN_ANIM 7
#define SQUARE(x) ((x) * (x))

extern PHD_3DPOS ai_path[16];
extern PHD_3DPOS ai_junction[4];
extern sint32 junction_index[4];
extern sint32 lara_ai_path;
extern sint32 lara_junction;
extern sint32 direction;
extern sint32 desired_direction;
extern uint8 puzzle_complete;
extern BITE_INFO willboss_bite_left;
extern BITE_INFO willboss_bite_right;

int CreatureActive(sint16 item_number);
void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info);
sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn);
int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);
void CreatureKill(ITEM_INFO *item, int kill_anim, int kill_state, int lara_kill_state);
sint16 CreatureEffect(ITEM_INFO *item, BITE_INFO *bite, sint16 (*generate)(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 yrot, sint16 room_number));
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);
int Inv_RequestItem(int item_num);
void TonyBossDie(sint16 item_number);
void ExplodeWillBoss(ITEM_INFO *item);
void TriggerPlasma_willboss(sint16 item_number, sint32 node, sint32 size);
void TriggerPlasmaBall_willboss(PHD_VECTOR *pos, sint16 room_number, sint16 angle, sint16 type);

int abs(int x);
#pragma intrinsic(abs)

void WillBossControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *willboss;
    sint16 angle, head, tilt, i, linknum, pathnum, juncnum, old_closest, pathdiff, lara_alive, in_fire_zone;
    AI_INFO info;
    sint32 distance, best_distance, best_junc_distance, will2junc_distance, x, z;

    if (!CreatureActive(item_number))
        return;

    item = &items[item_number];
    willboss = (CREATURE_INFO *)item->data;
    head = angle = tilt = 0;
    lara_alive = (lara_item->hit_points > 0);

    if (closest_ai_path == NO_AI_PATH) {
        for (linknum = room[item->room_number].item_number, pathnum = 0, juncnum = 0; linknum != NO_ITEM;
             linknum = items[linknum].next_item) {
            if (items[linknum].object_number == 80 && pathnum < 16) {
                ai_path[pathnum] = items[linknum].pos;
                pathnum++;
            } else if (items[linknum].object_number == 81 && juncnum < 4) {
                ai_junction[juncnum] = items[linknum].pos;
                juncnum++;
            }
        }

        closest_ai_path = -1;
        best_distance = 0x7fffffff;
        for (i = 0; i < 16; i++) {
            x = (ai_path[i].x_pos - item->pos.x_pos) >> 6;
            z = (ai_path[i].z_pos - item->pos.z_pos) >> 6;
            distance = x * x + z * z;
            if (distance < best_distance) {
                closest_ai_path = i;
                best_distance = distance;
            }
        }

        lara_ai_path = -1;
        best_distance = 0x7fffffff;
        for (i = 0; i < 16; i++) {
            x = (ai_path[i].x_pos - lara_item->pos.x_pos) >> 6;
            z = (ai_path[i].z_pos - lara_item->pos.z_pos) >> 6;
            distance = x * x + z * z;
            if (distance < best_distance) {
                lara_ai_path = i;
                best_distance = distance;
            }
        }

        for (juncnum = 0; juncnum < 4; juncnum++) {
            pathnum = -1;
            best_distance = 0x7fffffff;
            for (i = 0; i < 16; i++) {
                x = abs((ai_path[i].x_pos - ai_junction[juncnum].x_pos) >> 6);
                z = abs((ai_path[i].z_pos - ai_junction[juncnum].z_pos) >> 6);
                if (x > z)
                    distance = x + (z >> 1);
                else
                    distance = x + (z >> 1);
                if (distance < best_distance) {
                    pathnum = i;
                    best_distance = distance;
                }
            }
            junction_index[juncnum] = pathnum;
        }
    }

    best_distance = 0x7fffffff;
    old_closest = closest_ai_path;
    for (i = old_closest - 1; i < old_closest + 2; i++) {
        if (i < 0)
            pathnum = i + 16;
        else if (i > 15)
            pathnum = i - 16;
        else
            pathnum = i;

        x = (ai_path[pathnum].x_pos - item->pos.x_pos) >> 6;
        z = (ai_path[pathnum].z_pos - item->pos.z_pos) >> 6;
        distance = x * x + z * z;
        if (distance < best_distance) {
            closest_ai_path = pathnum;
            best_distance = distance;
        }
    }

    best_distance = 0x7fffffff;
    old_closest = lara_ai_path;
    for (i = old_closest - 1; i < old_closest + 2; i++) {
        if (i < 0)
            pathnum = i + 16;
        else if (i > 15)
            pathnum = i - 16;
        else
            pathnum = i;

        x = (ai_path[pathnum].x_pos - lara_item->pos.x_pos) >> 6;
        z = (ai_path[pathnum].z_pos - lara_item->pos.z_pos) >> 6;
        distance = x * x + z * z;
        if (distance < best_distance) {
            lara_ai_path = pathnum;
            best_distance = distance;
        }
    }

    best_junc_distance = 0x7fffffff;
    for (i = 0; i < 4; i++) {
        x = (ai_junction[i].x_pos - lara_item->pos.x_pos) >> 6;
        z = (ai_junction[i].z_pos - lara_item->pos.z_pos) >> 6;
        distance = x * x + z * z;
        if (distance < best_junc_distance) {
            lara_junction = i;
            best_junc_distance = distance;
        }
    }

    in_fire_zone = (best_junc_distance < best_distance || item->pos.y_pos > lara_item->pos.y_pos + WALL_L * 2);
    x = ai_junction[lara_junction].x_pos - item->pos.x_pos;
    z = ai_junction[lara_junction].z_pos - item->pos.z_pos;
    will2junc_distance = x * x + z * z;

    if (item->hit_points <= 0) {
        puzzle_complete = Inv_RequestItem(240);
        puzzle_complete += Inv_RequestItem(241);
        puzzle_complete += Inv_RequestItem(242);
        puzzle_complete += Inv_RequestItem(243);

        if (puzzle_complete != 4 || item->item_flags[1] == 0) {
            willboss->maximum_turn = 0;
            switch (item->current_anim_state) {
            case WILLBOSS_STOP:
                item->goal_anim_state = WILLBOSS_STUNNED;
                break;
            case WILLBOSS_STUNNED:
                bossdata.death_count = WILLBOSS_KO_TIME;
                break;
            case WILLBOSS_KNOCKOUT:
                if (bossdata.death_count-- < 0)
                    item->goal_anim_state = WILLBOSS_GETUP;
                break;
            case WILLBOSS_GETUP:
                item->hit_points = WILLBOSS_HP_AFTER_KO;
                if (puzzle_complete == 4)
                    item->item_flags[1] = 1;
                willboss->maximum_turn = WILLBOSS_ATTACK_TURN;
                break;
            default:
                item->goal_anim_state = WILLBOSS_STOP;
            }
        } else {
            if (item->current_anim_state != WILLBOSS_STUNNED) {
                item->anim_number = objects[item->object_number].anim_index + WILLBOSS_STUN_ANIM;
                item->frame_number = anims[item->anim_number].frame_base;
                item->current_anim_state = WILLBOSS_STUNNED;
            } else if (item->frame_number >= anims[item->anim_number].frame_end - 2) {
                sint32 lp;

                item->frame_number = anims[item->anim_number].frame_end - 2;
                item->mesh_bits = 0;
                if (bossdata.explode_count == 0) {
                    bossdata.ring_count = 0;
                    for (lp = 0; lp < 6; lp++) {
                        ExpRings[lp].on = 0;
                        ExpRings[lp].life = 32;
                        ExpRings[lp].radius = 512;
                        ExpRings[lp].speed = 128 + (lp << 5);
                        ExpRings[lp].xrot = ((GetRandomControl() & 511) - 256) & 4095;
                        ExpRings[lp].zrot = ((GetRandomControl() & 511) - 256) & 4095;
                    }
                }

                if (bossdata.explode_count < 256)
                    bossdata.explode_count++;

                if (bossdata.explode_count > 128 && bossdata.ring_count == 6 && ExpRings[5].life == 0) {
                    TonyBossDie(item_number);
                    bossdata.dead = 1;
                } else {
                    ExplodeWillBoss(item);
                }
                return;
            }
        }
    } else {
        CreatureAIInfo(item, &info);

        if (item->touch_bits)
            lara_item->hit_points -= WILLBOSS_TOUCH_DAMAGE;

        pathdiff = lara_ai_path - closest_ai_path;

        if (direction == -1 && ((pathdiff < 0 && pathdiff > -6) || pathdiff > 10))
            desired_direction = 1;
        else if (direction == 1 && ((pathdiff > 0 && pathdiff < 6) || pathdiff < -10))
            desired_direction = -1;

        willboss->target.x = ai_path[closest_ai_path].x_pos +
                             (direction * WILLBOSS_PATH_DISTANCE * phd_sin(ai_path[closest_ai_path].y_rot) >> W2V_SHIFT);
        willboss->target.z = ai_path[closest_ai_path].z_pos +
                             (direction * WILLBOSS_PATH_DISTANCE * phd_cos(ai_path[closest_ai_path].y_rot) >> W2V_SHIFT);

        switch (item->current_anim_state) {
        case WILLBOSS_STOP:
            willboss->maximum_turn = 0;
            willboss->flags = 0;
            if (direction != desired_direction)
                item->goal_anim_state = WILLBOSS_180;
            else if (in_fire_zone && info.ahead && will2junc_distance < WILLBOSS_FIRE_RANGE &&
                     lara_item->hit_points > 0)
                item->goal_anim_state = WILLBOSS_SHOOT;
            else if (info.bite && info.distance < WILLBOSS_LUNGE_RANGE)
                item->goal_anim_state = WILLBOSS_LUNGE;
            else
                item->goal_anim_state = WILLBOSS_WALK;
            break;

        case WILLBOSS_WALK:
            willboss->maximum_turn = WILLBOSS_TURN;
            willboss->flags = 0;
            if (direction != desired_direction)
                item->goal_anim_state = WILLBOSS_STOP;
            else if (in_fire_zone && info.ahead && will2junc_distance < WILLBOSS_FIRE_RANGE)
                item->goal_anim_state = WILLBOSS_STOP;
            else if (info.bite && info.distance < WILLBOSS_ATTACK_RANGE) {
                if ((GetRandomControl() & 3) == 1)
                    item->goal_anim_state = WILLBOSS_STOP;
                else if (item->frame_number < anims[item->anim_number].frame_base + 30)
                    item->goal_anim_state = WILLBOSS_WALKATAK2;
                else
                    item->goal_anim_state = WILLBOSS_WALKATAK1;
            }
            break;

        case WILLBOSS_180:
            willboss->maximum_turn = 0;
            willboss->flags = 0;
            if (item->frame_number == anims[item->anim_number].frame_base + 51) {
                item->pos.y_rot += 0x8000;
                direction = -direction;
            }
            break;

        case WILLBOSS_LUNGE:
            willboss->target.x = lara_item->pos.x_pos;
            willboss->target.z = lara_item->pos.z_pos;
            willboss->maximum_turn = WILLBOSS_ATTACK_TURN;
            if (!willboss->flags && (item->touch_bits & WILLBOSS_TOUCH)) {
                lara_item->hit_status = 1;
                lara_item->hit_points -= WILLBOSS_BITE_DAMAGE * 2;
                CreatureEffect(item, &willboss_bite_left, DoBloodSplat);
                CreatureEffect(item, &willboss_bite_right, DoBloodSplat);
                willboss->flags = 1;
            }
            break;

        case WILLBOSS_WALKATAK1:
        case WILLBOSS_WALKATAK2:
            if (!willboss->flags && (item->touch_bits & WILLBOSS_TOUCH)) {
                lara_item->hit_status = 1;
                lara_item->hit_points -= WILLBOSS_BITE_DAMAGE;
                CreatureEffect(item, &willboss_bite_left, DoBloodSplat);
                CreatureEffect(item, &willboss_bite_right, DoBloodSplat);
                willboss->flags = 1;
            }
            if (in_fire_zone && info.bite && will2junc_distance < WILLBOSS_FIRE_RANGE)
                item->goal_anim_state = WILLBOSS_WALK;
            else if (info.bite && info.distance < WILLBOSS_ATTACK_RANGE) {
                if (item->current_anim_state == WILLBOSS_WALKATAK1)
                    item->goal_anim_state = WILLBOSS_WALKATAK2;
                else
                    item->goal_anim_state = WILLBOSS_WALKATAK1;
            } else
                item->goal_anim_state = WILLBOSS_WALK;
            break;

        case WILLBOSS_BIGKILL:
            switch (item->frame_number - anims[item->anim_number].frame_base) {
            case 0:
            case 43:
            case 95:
            case 105:
                CreatureEffect(item, &willboss_bite_left, DoBloodSplat);
                break;
            case 61:
            case 91:
            case 101:
                CreatureEffect(item, &willboss_bite_right, DoBloodSplat);
                break;
            }
            break;

        case WILLBOSS_SHOOT:
            willboss->target.x = lara_item->pos.x_pos;
            willboss->target.z = lara_item->pos.z_pos;
            willboss->maximum_turn = WILLBOSS_ATTACK_TURN;
            if (item->frame_number - anims[item->anim_number].frame_base == 40 && lara_item->hit_points > 0) {
                PHD_VECTOR pos;

                pos.x = -64;
                pos.y = 410;
                pos.z = 0;
                GetJointAbsPosition(item, &pos, 20);
                TriggerPlasmaBall_willboss(&pos, item->room_number, item->pos.y_rot - 0x1000, 0);
                pos.x = 64;
                pos.y = 410;
                pos.z = 0;
                GetJointAbsPosition(item, &pos, 23);
                TriggerPlasmaBall_willboss(&pos, item->room_number, item->pos.y_rot + 0x1000, 0);
            }

            {
                PHD_VECTOR pos;
                sint32 bright, r, g, b, rnd;

                bright = item->frame_number - anims[item->anim_number].frame_base;
                if (bright > 16) {
                    bright = anims[item->anim_number].frame_end - item->frame_number;
                    if (bright > 16)
                        bright = 16;
                }

                rnd = GetRandomControl();
                g = 31 - ((rnd >> 4) & 3);
                b = 24 - ((rnd >> 6) & 3);
                r = rnd & 7;
                r = (r * bright) >> 4;
                g = (g * bright) >> 4;
                b = (b * bright) >> 4;

                pos.x = pos.y = pos.z = 0;
                GetJointAbsPosition(item, &pos, 17);
                TriggerDynamic(pos.x, pos.y, pos.z, 12, r, g, b);
                TriggerPlasma_willboss(item_number, 7, bright << 2);
                TriggerPlasma_willboss(item_number, 8, bright << 2);
            }
            break;
        }

        if (lara_alive && lara_item->hit_points <= 0) {
            CreatureKill(item, WILLBOSS_KILL_ANIM, WILLBOSS_BIGKILL, 2);
            willboss->maximum_turn = 0;
            return;
        }
    }

    angle = CreatureTurn(item, willboss->maximum_turn);
    CreatureAnimation(item_number, angle, 0);
}
