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

#define CIVVY_WALK 2
#define CIVVY_PUNCH2 3
#define CIVVY_AIM2 4
#define CIVVY_WAIT 5
#define CIVVY_AIM1 6
#define CIVVY_AIM0 7
#define CIVVY_PUNCH1 8
#define CIVVY_PUNCH0 9
#define CIVVY_RUN 10
#define CIVVY_DEATH 11
#define CIVVY_CLIMB3 12
#define CIVVY_CLIMB1 13
#define CIVVY_CLIMB2 14
#define CIVVY_FALL3 15

#define SQUARE(x) ((x) * (x))
#define CIVVY_HIT_DAMAGE 40
#define CIVVY_SWIPE_DAMAGE 50
#define CIVVY_WALK_TURN (ONE_DEGREE * 5)
#define CIVVY_RUN_TURN (ONE_DEGREE * 6)
#define CIVVY_ATTACK0_RANGE SQUARE(WALL_L / 3)
#define CIVVY_ATTACK1_RANGE SQUARE(WALL_L * 2 / 3)
#define CIVVY_ATTACK2_RANGE SQUARE(WALL_L)
#define CIVVY_WALK_RANGE SQUARE(WALL_L)
#define CIVVY_WAIT_CHANCE 0x100
#define CIVVY_DIE_ANIM 26
#define CIVVY_CLIMB1_ANIM 28
#define CIVVY_CLIMB2_ANIM 29
#define CIVVY_CLIMB3_ANIM 27
#define CIVVY_FALL3_ANIM 30
#define CIVVY_TOUCH 0x2400
#define CIVVY_VAULT_SHIFT 260
#define CIVVY_AWARE_DISTANCE SQUARE(WALL_L)
#define CIVVY_ESCAPE_RANGE SQUARE(WALL_L * 3)
#define GUARD 1
#define PATROL1 4
#define FOLLOW 16

int CreatureActive(sint16 item_number);
void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info);
void GetCreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
void CreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn);
int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);
void CreatureJoint(ITEM_INFO *item, sint16 joint, sint16 required);
int CreatureVault(sint16 item_number, sint16 angle, int vault, int shift);
sint16 CreatureEffect(ITEM_INFO *item, BITE_INFO *bite, sint16 (*generate)(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 yrot, sint16 room_number));
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);
void GetAITarget(CREATURE_INFO *creature);
void AlertAllGuards(sint16 item_number);
sint16 AIGuard(CREATURE_INFO *creature);
int TargetVisible(ITEM_INFO *item, AI_INFO *info);
void DoLotsOfBloodD(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num, int number);

extern BITE_INFO civvy_hit;

int abs(int x);
#pragma intrinsic(abs)

void CivvyControl(sint16 item_number)
{
    ITEM_INFO *item, *real_enemy;
    CREATURE_INFO *civvy;
    sint16 angle, torso_y, torso_x, head, tilt;
    sint32 lara_dx, lara_dz;
    AI_INFO info, lara_info;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    civvy = (CREATURE_INFO *)item->data;
    torso_y = torso_x = head = angle = tilt = 0;
    if (boxes[item->box_number].overlap_index & 0x4000) {
        DoLotsOfBloodD(item->pos.x_pos, item->pos.y_pos - (GetRandomControl() & 255) - 32, item->pos.z_pos,
                       (GetRandomControl() & 127) + 128, GetRandomControl() << 1, item->room_number, 3);
        item->hit_points -= 20;
    }
    if (item->hit_points <= 0) {
        if (item->current_anim_state != CIVVY_DEATH) {
            item->anim_number = objects[CIVVIE].anim_index + CIVVY_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = CIVVY_DEATH;
            civvy->LOT.step = STEP_L;
        }
    } else {
        if (item->ai_bits)
            GetAITarget(civvy);
        else
            civvy->enemy = lara_item;
        CreatureAIInfo(item, &info);
        if (civvy->enemy == lara_item) {
            lara_info.angle = info.angle;
            lara_info.distance = info.distance;
        } else {
            lara_dz = lara_item->pos.z_pos - item->pos.z_pos;
            lara_dx = lara_item->pos.x_pos - item->pos.x_pos;
            lara_info.angle = phd_atan(lara_dz, lara_dx) - item->pos.y_rot;
            lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
        }
        GetCreatureMood(item, &info, 1);
        if (civvy->enemy == lara_item && info.distance > CIVVY_ESCAPE_RANGE && info.enemy_facing < 0x3000
            && info.enemy_facing > -0x3000)
            civvy->mood = ESCAPE_MOOD;
        CreatureMood(item, &info, 1);
        angle = CreatureTurn(item, civvy->maximum_turn);
        real_enemy = civvy->enemy;
        civvy->enemy = lara_item;
        if ((lara_info.distance < CIVVY_AWARE_DISTANCE || item->hit_status || TargetVisible(item, &lara_info))
            && !(item->ai_bits & FOLLOW)) {
            if (!civvy->alerted)
                SoundEffect(300, &item->pos, 0);
            AlertAllGuards(item_number);
        }
        civvy->enemy = real_enemy;
        switch (item->current_anim_state) {
        case CIVVY_WAIT:
            if (civvy->alerted || item->goal_anim_state == CIVVY_RUN) {
                item->goal_anim_state = CIVVY_STOP;
                break;
            }
        case CIVVY_STOP:
            civvy->flags = 0;
            civvy->maximum_turn = 0;
            head = lara_info.angle;
            if (item->ai_bits & GUARD) {
                head = AIGuard(civvy);
                if (!(GetRandomControl() & 0xff)) {
                    if (item->current_anim_state == CIVVY_STOP)
                        item->goal_anim_state = CIVVY_WAIT;
                    else
                        item->goal_anim_state = CIVVY_STOP;
                }
                break;
            } else if (item->ai_bits & PATROL1)
                item->goal_anim_state = CIVVY_WALK;
            else if (civvy->mood == ESCAPE_MOOD) {
                if (lara.target != item && info.ahead)
                    item->goal_anim_state = CIVVY_STOP;
                else
                    item->goal_anim_state = CIVVY_RUN;
            } else if (civvy->mood == BORED_MOOD
                       || ((item->ai_bits & FOLLOW)
                           && (civvy->reached_goal || lara_info.distance > SQUARE(WALL_L * 2)))) {
                if (item->required_anim_state)
                    item->goal_anim_state = item->required_anim_state;
                else if (info.ahead)
                    item->goal_anim_state = CIVVY_STOP;
                else
                    item->goal_anim_state = CIVVY_RUN;
            } else if (info.bite && info.distance < CIVVY_ATTACK0_RANGE)
                item->goal_anim_state = CIVVY_AIM0;
            else if (info.bite && info.distance < CIVVY_ATTACK1_RANGE)
                item->goal_anim_state = CIVVY_AIM1;
            else if (info.bite && info.distance < CIVVY_WALK_RANGE)
                item->goal_anim_state = CIVVY_WALK;
            else
                item->goal_anim_state = CIVVY_RUN;
            break;
        case CIVVY_WALK:
            head = lara_info.angle;
            civvy->maximum_turn = CIVVY_WALK_TURN;
            if (item->ai_bits & PATROL1) {
                item->goal_anim_state = CIVVY_WALK;
                head = 0;
            } else if (civvy->mood == ESCAPE_MOOD)
                item->goal_anim_state = CIVVY_RUN;
            else if (civvy->mood == BORED_MOOD) {
                if (GetRandomControl() < CIVVY_WAIT_CHANCE) {
                    item->required_anim_state = CIVVY_WAIT;
                    item->goal_anim_state = CIVVY_STOP;
                }
            } else if (info.bite && info.distance < CIVVY_ATTACK0_RANGE)
                item->goal_anim_state = CIVVY_STOP;
            else if (info.bite && info.distance < CIVVY_ATTACK2_RANGE)
                item->goal_anim_state = CIVVY_AIM2;
            else
                item->goal_anim_state = CIVVY_RUN;
            break;
        case CIVVY_RUN:
            if (info.ahead)
                head = info.angle;
            civvy->maximum_turn = CIVVY_RUN_TURN;
            tilt = angle / 2;
            if (item->ai_bits & GUARD)
                item->goal_anim_state = CIVVY_WAIT;
            else if (civvy->mood == ESCAPE_MOOD) {
                if (lara.target != item && info.ahead)
                    item->goal_anim_state = CIVVY_STOP;
                break;
            } else if ((item->ai_bits & FOLLOW) && (civvy->reached_goal || lara_info.distance > SQUARE(WALL_L * 2)))
                item->goal_anim_state = CIVVY_STOP;
            else if (civvy->mood == BORED_MOOD)
                item->goal_anim_state = CIVVY_WALK;
            else if (info.ahead && info.distance < CIVVY_WALK_RANGE)
                item->goal_anim_state = CIVVY_WALK;
            break;
        case CIVVY_AIM0:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            civvy->maximum_turn = CIVVY_WALK_TURN;
            civvy->flags = 0;
            if (info.bite && info.distance < CIVVY_ATTACK0_RANGE)
                item->goal_anim_state = CIVVY_PUNCH0;
            else
                item->goal_anim_state = CIVVY_STOP;
            break;
        case CIVVY_AIM1:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            civvy->maximum_turn = CIVVY_WALK_TURN;
            civvy->flags = 0;
            if (info.ahead && info.distance < CIVVY_ATTACK1_RANGE)
                item->goal_anim_state = CIVVY_PUNCH1;
            else
                item->goal_anim_state = CIVVY_STOP;
            break;
        case CIVVY_AIM2:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            civvy->maximum_turn = CIVVY_WALK_TURN;
            civvy->flags = 0;
            if (info.bite && info.distance < CIVVY_ATTACK2_RANGE)
                item->goal_anim_state = CIVVY_PUNCH2;
            else
                item->goal_anim_state = CIVVY_WALK;
            break;
        case CIVVY_PUNCH0:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            civvy->maximum_turn = CIVVY_WALK_TURN;
            if (!civvy->flags && (item->touch_bits & CIVVY_TOUCH)) {
                lara_item->hit_points -= CIVVY_HIT_DAMAGE;
                lara_item->hit_status = 1;
                CreatureEffect(item, &civvy_hit, DoBloodSplat);
                SoundEffect(70, &item->pos, 0);
                civvy->flags = 1;
            }
            break;
        case CIVVY_PUNCH1:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            civvy->maximum_turn = CIVVY_WALK_TURN;
            if (!civvy->flags && (item->touch_bits & CIVVY_TOUCH)) {
                lara_item->hit_points -= CIVVY_HIT_DAMAGE;
                lara_item->hit_status = 1;
                CreatureEffect(item, &civvy_hit, DoBloodSplat);
                SoundEffect(70, &item->pos, 0);
                civvy->flags = 1;
            }
            if (info.ahead && info.distance > CIVVY_ATTACK1_RANGE && info.distance < CIVVY_ATTACK2_RANGE)
                item->goal_anim_state = CIVVY_PUNCH2;
            break;
        case CIVVY_PUNCH2:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            civvy->maximum_turn = CIVVY_WALK_TURN;
            if (civvy->flags != 2 && (item->touch_bits & CIVVY_TOUCH)) {
                lara_item->hit_points -= CIVVY_SWIPE_DAMAGE;
                lara_item->hit_status = 1;
                CreatureEffect(item, &civvy_hit, DoBloodSplat);
                SoundEffect(70, &item->pos, 0);
                civvy->flags = 2;
            }
            break;
        }
    }
    CreatureTilt(item, tilt);
    CreatureJoint(item, 0, torso_y);
    CreatureJoint(item, 1, torso_x);
    CreatureJoint(item, 2, head);
    if (item->current_anim_state < CIVVY_DEATH) {
        switch (CreatureVault(item_number, angle, 2, CIVVY_VAULT_SHIFT)) {
        case 2:
            civvy->maximum_turn = 0;
            item->anim_number = objects[CIVVIE].anim_index + CIVVY_CLIMB1_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = CIVVY_CLIMB1;
            break;
        case 3:
            civvy->maximum_turn = 0;
            item->anim_number = objects[CIVVIE].anim_index + CIVVY_CLIMB2_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = CIVVY_CLIMB2;
            break;
        case 4:
            civvy->maximum_turn = 0;
            item->anim_number = objects[CIVVIE].anim_index + CIVVY_CLIMB3_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = CIVVY_CLIMB3;
            break;
        case -4:
            civvy->maximum_turn = 0;
            item->anim_number = objects[CIVVIE].anim_index + CIVVY_FALL3_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = CIVVY_FALL3;
            break;
        }
    } else {
        civvy->maximum_turn = 0;
        CreatureAnimation(item_number, angle, 0);
    }
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
