#include "game.h"

extern sint32 level_items;
extern PHD_VECTOR TrigDynamics[3];

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
extern sint32 TribeBossRadii[5];
extern sint32 TribeBossHeights[5];
extern SHIELD_POINTS TribeBossShield[40];
extern char TribeBossShieldOn;
extern char lizard_man_active;
extern char shield_active;

void UpdateElectricityPoints(void);
void TriggerTribeBossHeadElectricity(ITEM_INFO *item, sint32 copy);
void DrawTribeBossShield(ITEM_INFO *item);

void S_DrawTribeBoss(ITEM_INFO *item)
{
    DrawAnimatingItem(item);
    UpdateElectricityPoints();
    TrigDynamics[0].x = 0;
    TrigDynamics[1].x = 0;
    TrigDynamics[2].x = 0;
    if (!bossdata.explode_count)
    {
        TriggerTribeBossHeadElectricity(item, 0);
        TriggerTribeBossHeadElectricity(item, 1);
    }
    else
        DrawExplosionRings();
    DrawTribeBossShield(item);
}

void TribeBossDie(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->collidable = 0;
    item->hit_points = DONT_TARGET;
    KillItem(item_number);
    DisableBaddieAI(item_number);
    item->flags |= ONESHOT;
}

sint16 FindLizardManItemNumber(sint16 room_number);

void InitialiseTribeBoss(sint16 item_number)
{
    SHIELD_POINTS *shptr;
    sint32 lp, lp1, angle, y;

    bossdata.LizmanItem = FindLizardManItemNumber(items[item_number].room_number);
    bossdata.LizmanRoom = items[bossdata.LizmanItem].room_number;
    for (lp = 0; lp < 3; lp++)
        TrigDynamics[lp].x = 0;
    bossdata.attack_count = bossdata.attack_flag = bossdata.death_count = bossdata.attack_head_count =
        lizard_man_active = TribeBossShieldOn = shield_active = bossdata.explode_count =
        bossdata.ring_count = bossdata.dead = bossdata.dropped_icon = 0;
    shptr = &TribeBossShield[0];
    for (lp = 0; lp < 5; lp++) {
        y = TribeBossHeights[lp];
        angle = 0;
        for (lp1 = 0; lp1 < 8; lp1++) {
            shptr->x = (rcossin_tbl[angle << 1] * TribeBossRadii[lp]) >> 11;
            shptr->y = y;
            shptr->z = (rcossin_tbl[(angle << 1) + 1] * TribeBossRadii[lp]) >> 11;
            shptr->rgb = 0;
            angle += 512;
            shptr++;
        }
    }
}

void CreatureJoint(ITEM_INFO *item, sint16 joint, sint16 required);

void RotateHeadXAngle(ITEM_INFO *item)
{
    PHD_VECTOR pos1, pos2;
    sint32 x, y, z;
    sint16 angle;

    pos1.x = pos1.y = pos1.z = 0;
    GetJointAbsPosition(lara_item, &pos1, 0);
    pos2.x = pos2.y = pos2.z = 0;
    GetJointAbsPosition(item, &pos2, 0);
    x = abs(pos2.x - pos1.x);
    y = pos2.y - pos1.y;
    z = abs(pos2.z - pos1.z);
    x = phd_sqrt(x * x + z * z);
    angle = phd_atan(x, y);
    if (abs(angle) < 0x2000)
        CreatureJoint(item, 2, angle);
    else
        CreatureJoint(item, 2, 0);
}

extern sint32 lizman_summon_coords[][4];

int EnableBaddieAI(sint16 item_number, int Always);
void RemoveDrawnItem(sint16 item_num);

void TriggerLizardMan(void)
{
    ROOM_INFO *r;
    ITEM_INFO *item;
    sint16 room_number;

    item = &items[bossdata.LizmanItem];
    item->object_number = LIZARD_MAN;
    item->room_number = bossdata.LizmanRoom;
    item->pos.x_pos = lizman_summon_coords[bossdata.attack_type - 1][0];
    item->pos.y_pos = lizman_summon_coords[bossdata.attack_type - 1][1];
    item->pos.z_pos = lizman_summon_coords[bossdata.attack_type - 1][2];
    item->anim_number = objects[item->object_number].anim_index;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = anims[item->anim_number].current_anim_state;
    item->required_anim_state = 0;
    item->pos.y_rot = (bossdata.attack_type == 1) ? 0x8000 : 0;
    item->pos.x_rot = item->pos.z_rot = 0;
    item->flags = item->timer = 0;
    item->data = NULL;
    item->mesh_bits = 0xffffffff;
    item->hit_points = objects[item->object_number].hit_points;
    item->collidable = 1;
    item->active = 0;
    item->status = ACTIVE;
    AddActiveItem(bossdata.LizmanItem);
    EnableBaddieAI(bossdata.LizmanItem, 1);
    room_number = item->room_number;
    GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (item->room_number != room_number)
        ItemNewRoom(bossdata.LizmanItem, room_number);
    lizard_man_active = 1;
    RemoveDrawnItem(bossdata.LizmanItem);
    r = &room[item->room_number];
    item->next_item = r->item_number;
    r->item_number = bossdata.LizmanItem;
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
extern sint32 death_radii_triboss[5];
extern sint32 death_heights_triboss[5];
extern sint32 dradii_triboss[5];
extern sint32 dheights1_triboss[5];
extern sint32 dheights2_triboss[5];
extern sint32 wibble;

void TriggerExplosionSparks(sint32 x, sint32 y, sint32 z, sint32 extra_trigs, sint32 dyn, sint32 uw, sint16 roomnum);

void ExplodeTribeBoss(ITEM_INFO *item)
{
    SHIELD_POINTS *shptr;
    sint32 lp, lp1, angle, y, r, g, b;

    TribeBossShieldOn = 0;

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
            death_radii_triboss[lp] = (dradii_triboss[lp] >> 4) + ((dradii_triboss[lp] * bossdata.explode_count) >> 7);
            death_heights_triboss[lp] = dheights2_triboss[lp] + (((dheights1_triboss[lp] - dheights2_triboss[lp]) * bossdata.explode_count) >> 7);
        }
    }

    shptr = &TribeBossShield[0];

    for (lp = 0; lp < 5; lp++) {
        y = death_heights_triboss[lp];
        angle = (wibble << 3) & 511;
        for (lp1 = 0; lp1 < 8; lp1++) {
            shptr->x = (rcossin_tbl[angle << 1] * death_radii_triboss[lp]) >> 11;
            shptr->y = y;
            shptr->z = (rcossin_tbl[(angle << 1) + 1] * death_radii_triboss[lp]) >> 11;

            if (lp != 0 && lp != 4 && bossdata.explode_count < 64) {
                r = GetRandomDraw() & 31;
                b = (GetRandomDraw() & 63) + 224;
                g = (b >> 2) + (GetRandomDraw() & 63);
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

void TriggerSummonSmoke(sint32 x, sint32 y, sint32 z)
{
    SPARKS *sptr;
    sint32 size;

    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 16;
    sptr->sG = 64;
    sptr->sB = 0;
    sptr->dR = 8;
    sptr->dG = 32;
    sptr->dB = 0;
    sptr->ColFadeSpeed = 16 + (GetRandomControl() & 7);
    sptr->FadeToBlack = 64;
    sptr->sLife = sptr->Life = (GetRandomControl() & 15) + 96;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 127) - 64);
    sptr->y = y - (GetRandomControl() & 31);
    sptr->z = z + ((GetRandomControl() & 127) - 64);
    sptr->Xvel = (GetRandomControl() & 255) - 128;
    sptr->Yvel = -(GetRandomControl() & 15) - 16;
    sptr->Zvel = (GetRandomControl() & 255) - 128;
    sptr->Friction = 0;
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
    sptr->Width = sptr->sWidth = size >> 1;
    sptr->dWidth = size;
    size += (GetRandomControl() & 31) + 32;
    sptr->Height = sptr->sHeight = size >> 1;
    sptr->dHeight = size;
}

sint16 FindLizardManItemNumber(sint16 room_number)
{
    ITEM_INFO *target;
    sint16 target_number;

    for (target_number = 0; target_number < level_items; target_number++)
    {
        target = &items[target_number];
        if (target->object_number == LIZARD_MAN && target->room_number == room_number)
            return target_number;
    }
    return -1;
}

#define SQUARE(A) ((A) * (A))

void FindClosestShieldPoint(sint32 x, sint32 y, sint32 z, ITEM_INFO *item)
{
    SHIELD_POINTS *shptr;
    GAME_VECTOR pos;
    sint32 affected[5] = { 0, -1, 1, -8, 8 };
    sint32 closestdist, closestvert, r, g, b, rgb, coladd;
    sint32 lp;

    shptr = &TribeBossShield[0];
    closestdist = 0x7fffffff;
    closestvert = 0;
    for (lp = 0; lp < 40; lp++) {
        sint32 dx, dy, dz;

        if (lp >= 16 && lp <= 23) {
            dx = (shptr->x + item->pos.x_pos) - x;
            dy = (shptr->y + item->pos.y_pos) - y;
            dz = (shptr->z + item->pos.z_pos) - z;
            dx = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);
            if (dx < closestdist) {
                closestdist = dx;
                closestvert = lp;
            }
        }
        shptr++;
    }

    switch (lara.gun_type) {
    case LG_PISTOLS:
        coladd = 144;
        break;
    case LG_MAGNUMS:
        coladd = 200;
        break;
    case LG_UZIS:
        coladd = 144;
        break;
    case LG_SHOTGUN:
        coladd = 192;
        break;
    case LG_HARPOON:
        coladd = 200;
        break;
    case LG_ROCKET:
        coladd = 224;
        break;
    case LG_GRENADE:
        coladd = 224;
        break;
    case LG_M16:
        coladd = 192;
        break;
    default:
        coladd = 0;
        break;
    }

    for (lp = 0; lp < 5; lp++) {
        sint32 vert;

        vert = closestvert + affected[lp];
        if ((vert & 7) == 7 && affected[lp] == -1)
            vert += 8;
        if ((vert & 7) == 0 && affected[lp] == 1)
            vert -= 8;
        shptr = &TribeBossShield[vert];
        rgb = shptr->rgb;
        r = rgb & 255;
        g = (rgb >> 8) & 255;
        b = (rgb >> 16) & 255;
        if (lp == 0) {
            if (coladd >= 200)
                r = coladd;
            else {
                r += coladd >> 2;
                if (r > coladd)
                    r = coladd;
            }
        } else {
            if (coladd >= 200)
                r = coladd >> 1;
            else {
                r += coladd >> 3;
                if (r > coladd >> 1)
                    r = coladd >> 1;
            }
        }
        if (lp == 0) {
            if (coladd >= 200)
                g = coladd;
            else {
                g += coladd >> 2;
                if (g > coladd)
                    g = coladd;
            }
        } else {
            if (coladd >= 200)
                g = coladd >> 1;
            else {
                g += coladd >> 3;
                if (g > coladd >> 1)
                    g = coladd >> 1;
            }
        }
        if (lp == 0) {
            if (coladd >= 200)
                b = coladd;
            else {
                b += coladd >> 2;
                if (b > coladd)
                    b = coladd;
            }
        } else {
            if (coladd >= 200)
                b = coladd >> 1;
            else {
                b += coladd >> 3;
                if (b > coladd >> 1)
                    b = coladd >> 1;
            }
        }
        shptr->rsub = (GetRandomControl() & 7) + 8;
        shptr->gsub = (GetRandomControl() & 7) + 8;
        shptr->bsub = (GetRandomControl() & 7) + 8;
        if (lara.gun_type == LG_ROCKET || lara.gun_type == LG_GRENADE) {
            shptr->rsub >>= 1;
            shptr->gsub >>= 1;
            shptr->bsub >>= 1;
        }
        shptr->rgb = r | (g << 8) | (b << 16);
    }

    pos.x = x;
    pos.y = y;
    pos.z = z;
    for (lp = 0; lp < 7; lp++)
        TriggerElectricSparks_triboss(&pos, 1);
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

#define ATTACK_HEAD 0
#define ATTACK_HAND1 1
#define ATTACK_HAND2 2
#define TRIBEBOSS_WAIT 0
#define TRIBEBOSS_ATTACK_HEAD 1
#define TRIBEBOSS_ATTACK_HAND 2
#define TRIBEBOSS_DEATH 3
#define TRIBEBOSS_TURN (ONE_DEGREE * 3)
#define TRIBEBOSS_DIE_ANIM 3
#define MAX_HEAD_ATTACKS 4
#define SQUARE(x) ((x) * (x))

int CreatureActive(sint16 item_number);
void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info);
sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn);
int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);
void CreatureJoint(ITEM_INFO *item, sint16 joint, sint16 required);
void BossDropIcon(sint16 item_number);

extern ITEM_INFO *TribeBossItem;
extern BITE_INFO tribeboss_hit[];
extern char turned;

void TribeBossControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *tribeboss;
    sint16 angle, head, tilt, lp, dynsize;
    uint16 ty;
    AI_INFO info;
    sint16 tyrot;

    for (lp = 0; lp < 3; lp++) {
        if (TrigDynamics[lp].x != 0) {
            if (lp == 0)
                TriggerDynamic(TrigDynamics[0].x, TrigDynamics[0].y, TrigDynamics[0].z, (GetRandomControl() & 3) + 8, 0,
                               (GetRandomControl() & 7) + 8, (GetRandomControl() & 7) + 16);
            else if (lp == 1) {
                if (bossdata.attack_type == ATTACK_HEAD)
                    TriggerDynamic(TrigDynamics[1].x, TrigDynamics[1].y, TrigDynamics[1].z, (GetRandomControl() & 7) + 8,
                                   0, (GetRandomControl() & 7) + 8, (GetRandomControl() & 7) + 16);
                else
                    TriggerDynamic(TrigDynamics[1].x, TrigDynamics[1].y, TrigDynamics[1].z, (GetRandomControl() & 7) + 8,
                                   0, (GetRandomControl() & 7) + 16, (GetRandomControl() & 7) + 8);
            } else if (bossdata.attack_count) {
                dynsize = (128 - bossdata.attack_count) >> 1;
                if (dynsize > 31)
                    dynsize = 31;
                if (dynsize > 0) {
                    if (bossdata.attack_type == ATTACK_HEAD)
                        TriggerDynamic(TrigDynamics[2].x, TrigDynamics[2].y, TrigDynamics[2].z, dynsize, dynsize >> 2,
                                       (GetRandomControl() & 7) + 16, (GetRandomControl() & 7) + 24);
                    else
                        TriggerDynamic(TrigDynamics[2].x, TrigDynamics[2].y, TrigDynamics[2].z, dynsize, dynsize >> 2,
                                       (GetRandomControl() & 7) + 24, (GetRandomControl() & 7) + 16);
                }
            }
        }
    }
    if (!CreatureActive(item_number))
        return;
    TribeBossItem = item = &items[item_number];
    tribeboss = (CREATURE_INFO *)item->data;
    head = angle = tilt = 0;
    if (item->hit_points <= 0) {
        if (item->current_anim_state != TRIBEBOSS_DEATH) {
            item->anim_number = objects[item->object_number].anim_index + TRIBEBOSS_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = TRIBEBOSS_DEATH;
            bossdata.death_count = 1;
        }
        if (item->frame_number - anims[item->anim_number].frame_base > 119) {
            item->frame_number = anims[item->anim_number].frame_base + 120;
            item->mesh_bits = 0;
            bossdata.death_count = -1;
            if (bossdata.explode_count == 0) {
                bossdata.ring_count = 0;
                SoundEffect(106, &item->pos, 0);
                for (lp = 0; lp < 6; lp++) {
                    ExpRings[lp].on = 0;
                    ExpRings[lp].life = 32;
                    ExpRings[lp].radius = 512;
                    ExpRings[lp].speed = 128 + (lp << 5);
                    ExpRings[lp].xrot = ((GetRandomControl() & 511) - 256) & 4095;
                    ExpRings[lp].zrot = ((GetRandomControl() & 511) - 256) & 4095;
                }
                if (bossdata.dropped_icon == 0) {
                    BossDropIcon(item_number);
                    bossdata.dropped_icon = 1;
                }
            }
            if (bossdata.explode_count < 256)
                bossdata.explode_count++;
            if (bossdata.explode_count > 128 && bossdata.ring_count == 6 && ExpRings[5].life == 0) {
                TribeBossDie(item_number);
                bossdata.dead = 1;
            } else
                ExplodeTribeBoss(item);
            return;
        }
        item->pos.z_rot = (GetRandomControl() % bossdata.death_count) - (bossdata.death_count >> 1);
        if (bossdata.death_count < 2048)
            bossdata.death_count += 32;
    } else {
        CreatureAIInfo(item, &info);
        if (item->hit_status)
            SoundEffect(361, &item->pos, 0);
        tyrot = item->pos.y_rot;
        if (!bossdata.attack_flag) {
            sint32 dx, dz;

            dx = item->pos.x_pos - lara_item->pos.x_pos;
            dz = item->pos.z_pos - lara_item->pos.z_pos;
            if (SQUARE(dx) + SQUARE(dz) < 2048 * 2048)
                bossdata.attack_flag = 1;
        }
        tribeboss->target.x = lara_item->pos.x_pos;
        tribeboss->target.z = lara_item->pos.z_pos;
        if (lizard_man_active && item->current_anim_state == TRIBEBOSS_WAIT) {
            ty = (uint16)item->pos.y_rot;
            if (abs(0xc000 - ty) > ONE_DEGREE)
                item->pos.y_rot += (0xc000 - ty) >> 3;
            else
                item->pos.y_rot = 0xc000;
        } else
            angle = CreatureTurn(item, tribeboss->maximum_turn);
        RotateHeadXAngle(item);
        if (info.ahead)
            head = info.angle;
        switch (item->current_anim_state) {
        case TRIBEBOSS_WAIT:
            bossdata.attack_count = 0;
            if (item->goal_anim_state != TRIBEBOSS_ATTACK_HEAD && item->goal_anim_state != TRIBEBOSS_ATTACK_HAND)
                TribeBossShieldOn = 1;
            if (lizard_man_active)
                CreatureJoint(item, 1, head);
            else
                CreatureJoint(item, 1, 0);
            if (bossdata.attack_flag && !lizard_man_active)
                tribeboss->maximum_turn = TRIBEBOSS_TURN;
            else
                tribeboss->maximum_turn = 0;
            if (item->goal_anim_state != TRIBEBOSS_ATTACK_HEAD && info.angle > -128 && info.angle < 128
                && lara_item->hit_points > 0 && bossdata.attack_head_count < MAX_HEAD_ATTACKS && !lizard_man_active
                && !shield_active) {
                PHD_VECTOR pos;

                pos.x = pos.y = pos.z = 0;
                GetJointAbsPosition(lara_item, &pos, 0);
                bossdata.BeamTarget.x = pos.x;
                bossdata.BeamTarget.y = pos.y;
                bossdata.BeamTarget.z = pos.z;
                item->goal_anim_state = TRIBEBOSS_ATTACK_HEAD;
                tribeboss->maximum_turn = 0;
                TribeBossShieldOn = 0;
                bossdata.attack_head_count++;
                break;
            } else if (item->goal_anim_state != TRIBEBOSS_ATTACK_HAND && bossdata.attack_head_count >= MAX_HEAD_ATTACKS
                       && lara_item->hit_points > 0) {
                tribeboss->maximum_turn = 0;
                if (bossdata.attack_type == ATTACK_HEAD) {
                    sint32 dxz1, dxz2;

                    dxz1 = SQUARE(lara_item->pos.x_pos - lizman_summon_coords[0][0]);
                    dxz1 += SQUARE(lara_item->pos.z_pos - lizman_summon_coords[0][2]);
                    dxz2 = SQUARE(lara_item->pos.x_pos - lizman_summon_coords[1][0]);
                    dxz2 += SQUARE(lara_item->pos.z_pos - lizman_summon_coords[1][2]);
                    if (dxz1 > dxz2)
                        bossdata.attack_type = ATTACK_HAND1;
                    else
                        bossdata.attack_type = ATTACK_HAND2;
                }
                ty = (uint16)item->pos.y_rot;
                if (abs(lizman_summon_coords[bossdata.attack_type - 1][3] - ty) < ONE_DEGREE) {
                    item->pos.y_rot = lizman_summon_coords[bossdata.attack_type - 1][3];
                    if (!shield_active) {
                        item->goal_anim_state = TRIBEBOSS_ATTACK_HAND;
                        bossdata.BeamTarget.x = lizman_summon_coords[bossdata.attack_type - 1][0];
                        bossdata.BeamTarget.y = lizman_summon_coords[bossdata.attack_type - 1][1];
                        bossdata.BeamTarget.z = lizman_summon_coords[bossdata.attack_type - 1][2];
                        tribeboss->maximum_turn = 0;
                        bossdata.attack_head_count = 0;
                        TribeBossShieldOn = 0;
                        break;
                    }
                } else
                    item->pos.y_rot += (lizman_summon_coords[bossdata.attack_type - 1][3] - ty) >> 4;
            }
            break;
        case TRIBEBOSS_ATTACK_HEAD:
            tribeboss->maximum_turn = 0;
            bossdata.attack_count += 3;
            bossdata.attack_type = ATTACK_HEAD;
            CreatureJoint(item, 1, 0);
            break;
        case TRIBEBOSS_ATTACK_HAND:
            tribeboss->maximum_turn = 0;
            if (bossdata.attack_count < 64)
                bossdata.attack_count += 2;
            else
                bossdata.attack_count += 3;
            CreatureJoint(item, 1, 0);
            break;
        default:
            break;
        }
    }
    if (bossdata.attack_count && bossdata.attack_type == ATTACK_HEAD) {
        if (bossdata.attack_count < 64)
            tribeboss_hit[5].z = 136 + (bossdata.attack_count << 2);
    }
    tribeboss->joint_rotation[0] += ONE_DEGREE * 7;
    CreatureAnimation(item_number, angle, 0);
    if (tyrot != item->pos.y_rot && turned == 0) {
        turned = 1;
        SoundEffect(362, &item->pos, 4 | 0x800000);
    } else if (tyrot == item->pos.y_rot)
        turned = 0;
}
