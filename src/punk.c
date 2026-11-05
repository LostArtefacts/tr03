#include "game.h"

#define PUNK_STOP_ANIM 6
#define PUNK_STOP 1

void InitialisePunk(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    InitialiseCreature(item_number);
    item->anim_number = objects[PUNK1].anim_index + PUNK_STOP_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = PUNK_STOP;
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

#define PUNK_WALK 2
#define PUNK_PUNCH2 3
#define PUNK_AIM2 4
#define PUNK_WAIT 5
#define PUNK_AIM1 6
#define PUNK_AIM0 7
#define PUNK_PUNCH1 8
#define PUNK_PUNCH0 9
#define PUNK_RUN 10
#define PUNK_DEATH 11
#define PUNK_CLIMB3 12
#define PUNK_CLIMB1 13
#define PUNK_CLIMB2 14
#define PUNK_FALL3 15

#define SQUARE(x) ((x) * (x))
#define PUNK_HIT_DAMAGE 80
#define PUNK_SWIPE_DAMAGE 100
#define PUNK_WALK_TURN (ONE_DEGREE * 5)
#define PUNK_RUN_TURN (ONE_DEGREE * 6)
#define PUNK_ATTACK0_RANGE SQUARE(WALL_L / 2)
#define PUNK_ATTACK1_RANGE SQUARE(WALL_L)
#define PUNK_ATTACK2_RANGE SQUARE(WALL_L * 5 / 4)
#define PUNK_WALK_RANGE SQUARE(WALL_L)
#define PUNK_WAIT_CHANCE 0x100
#define PUNK_DIE_ANIM 26
#define PUNK_CLIMB1_ANIM 28
#define PUNK_CLIMB2_ANIM 29
#define PUNK_CLIMB3_ANIM 27
#define PUNK_FALL3_ANIM 30
#define PUNK_TOUCH 0x2400
#define PUNK_VAULT_SHIFT 260
#define PUNK_AWARE_DISTANCE SQUARE(WALL_L)
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
void LaraBurn(void);
void TriggerPunkFlame(sint16 item_number);

extern uint32 CurrentLevel;
extern BITE_INFO punk_hit;

int abs(int x);
#pragma intrinsic(abs)

void PunkControl(sint16 item_number)
{
    ITEM_INFO *item, *real_enemy;
    CREATURE_INFO *punk;
    PHD_VECTOR pos1;
    sint16 angle, torso_y, torso_x, head, tilt;
    sint32 lara_dx, lara_dz, rnd;
    AI_INFO info, lara_info;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    punk = (CREATURE_INFO *)item->data;
    torso_y = torso_x = head = angle = tilt = 0;
    if (item->item_flags[2]) {
        TriggerPunkFlame(item_number);
        rnd = GetRandomControl();
        pos1.x = punk_hit.x + (rnd & 15) - 8;
        pos1.y = punk_hit.y + ((rnd >> 4) & 15) - 8;
        pos1.z = punk_hit.z + ((rnd >> 8) & 15) - 8;
        GetJointAbsPosition(&items[item_number], &pos1, punk_hit.mesh_num);
        TriggerDynamic(pos1.x, pos1.y, pos1.z, 13, 31 - ((rnd >> 4) & 3), 24 - ((rnd >> 6) & 3), rnd & 7);
    }
    if (item->hit_points <= 0) {
        if (item->current_anim_state != PUNK_DEATH) {
            item->anim_number = objects[PUNK1].anim_index + PUNK_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = PUNK_DEATH;
            punk->LOT.step = STEP_L;
        }
    } else {
        if (item->ai_bits)
            GetAITarget(punk);
        else
            punk->enemy = lara_item;
        CreatureAIInfo(item, &info);
        if (punk->enemy == lara_item) {
            lara_info.angle = info.angle;
            lara_info.distance = info.distance;
        } else {
            lara_dz = lara_item->pos.z_pos - item->pos.z_pos;
            lara_dx = lara_item->pos.x_pos - item->pos.x_pos;
            lara_info.angle = phd_atan(lara_dz, lara_dx) - item->pos.y_rot;
            lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
        }
        if (!punk->alerted && punk->enemy == lara_item)
            punk->enemy = NULL;
        GetCreatureMood(item, &info, 1);
        CreatureMood(item, &info, 1);
        angle = CreatureTurn(item, punk->maximum_turn);
        real_enemy = punk->enemy;
        punk->enemy = lara_item;
        if (item->hit_status
            || ((lara_info.distance < PUNK_AWARE_DISTANCE || TargetVisible(item, &lara_info))
                && abs(lara_item->pos.y_pos - item->pos.y_pos) < STEP_L * 5 && CurrentLevel != 11
                && !(item->ai_bits & FOLLOW))) {
            if (!punk->alerted)
                SoundEffect(299, &item->pos, 0);
            AlertAllGuards(item_number);
        }
        punk->enemy = real_enemy;
        switch (item->current_anim_state) {
        case PUNK_WAIT:
            if (punk->alerted || item->goal_anim_state == PUNK_RUN) {
                item->goal_anim_state = PUNK_STOP;
                break;
            }
        case PUNK_STOP:
            punk->flags = 0;
            punk->maximum_turn = 0;
            head = lara_info.angle;
            if (item->ai_bits & GUARD) {
                head = AIGuard(punk);
                if (!(GetRandomControl() & 0xff)) {
                    if (item->current_anim_state == PUNK_STOP)
                        item->goal_anim_state = PUNK_WAIT;
                    else
                        item->goal_anim_state = PUNK_STOP;
                }
                break;
            } else if (item->ai_bits & PATROL1)
                item->goal_anim_state = PUNK_WALK;
            else if (punk->mood == ESCAPE_MOOD) {
                if (lara.target != item && info.ahead && !item->hit_status)
                    item->goal_anim_state = PUNK_STOP;
                else
                    item->goal_anim_state = PUNK_RUN;
            } else if (punk->mood == BORED_MOOD
                       || ((item->ai_bits & FOLLOW)
                           && (punk->reached_goal || lara_info.distance > SQUARE(WALL_L * 2)))) {
                if (item->required_anim_state)
                    item->goal_anim_state = item->required_anim_state;
                else if (info.ahead)
                    item->goal_anim_state = PUNK_STOP;
                else
                    item->goal_anim_state = PUNK_RUN;
            } else if (info.bite && info.distance < PUNK_ATTACK0_RANGE)
                item->goal_anim_state = PUNK_AIM0;
            else if (info.bite && info.distance < PUNK_ATTACK1_RANGE)
                item->goal_anim_state = PUNK_AIM1;
            else if (info.bite && info.distance < PUNK_WALK_RANGE)
                item->goal_anim_state = PUNK_WALK;
            else
                item->goal_anim_state = PUNK_RUN;
            break;
        case PUNK_WALK:
            head = lara_info.angle;
            punk->maximum_turn = PUNK_WALK_TURN;
            if (item->ai_bits & PATROL1) {
                item->goal_anim_state = PUNK_WALK;
                head = 0;
            } else if (punk->mood == ESCAPE_MOOD)
                item->goal_anim_state = PUNK_RUN;
            else if (punk->mood == BORED_MOOD) {
                if (GetRandomControl() < PUNK_WAIT_CHANCE) {
                    item->required_anim_state = PUNK_WAIT;
                    item->goal_anim_state = PUNK_STOP;
                }
            } else if (info.bite && info.distance < PUNK_ATTACK0_RANGE)
                item->goal_anim_state = PUNK_STOP;
            else if (info.bite && info.distance < PUNK_ATTACK2_RANGE)
                item->goal_anim_state = PUNK_AIM2;
            else
                item->goal_anim_state = PUNK_RUN;
            break;
        case PUNK_RUN:
            if (info.ahead)
                head = info.angle;
            punk->maximum_turn = PUNK_RUN_TURN;
            tilt = angle / 2;
            if (item->ai_bits & GUARD)
                item->goal_anim_state = PUNK_WAIT;
            else if (punk->mood == ESCAPE_MOOD) {
                if (lara.target != item && info.ahead)
                    item->goal_anim_state = PUNK_STOP;
                break;
            } else if ((item->ai_bits & FOLLOW) && (punk->reached_goal || lara_info.distance > SQUARE(WALL_L * 2)))
                item->goal_anim_state = PUNK_STOP;
            else if (punk->mood == BORED_MOOD)
                item->goal_anim_state = PUNK_WALK;
            else if (info.ahead && info.distance < PUNK_WALK_RANGE)
                item->goal_anim_state = PUNK_WALK;
            break;
        case PUNK_AIM0:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            punk->maximum_turn = PUNK_WALK_TURN;
            punk->flags = 0;
            if (info.bite && info.distance < PUNK_ATTACK0_RANGE)
                item->goal_anim_state = PUNK_PUNCH0;
            else
                item->goal_anim_state = PUNK_STOP;
            break;
        case PUNK_AIM1:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            punk->maximum_turn = PUNK_WALK_TURN;
            punk->flags = 0;
            if (info.ahead && info.distance < PUNK_ATTACK1_RANGE)
                item->goal_anim_state = PUNK_PUNCH1;
            else
                item->goal_anim_state = PUNK_STOP;
            break;
        case PUNK_AIM2:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            punk->maximum_turn = PUNK_WALK_TURN;
            punk->flags = 0;
            if (info.bite && info.distance < PUNK_ATTACK2_RANGE)
                item->goal_anim_state = PUNK_PUNCH2;
            else
                item->goal_anim_state = PUNK_WALK;
            break;
        case PUNK_PUNCH0:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            punk->maximum_turn = PUNK_WALK_TURN;
            if (!punk->flags && (item->touch_bits & PUNK_TOUCH)) {
                lara_item->hit_points -= PUNK_HIT_DAMAGE;
                lara_item->hit_status = 1;
                CreatureEffect(item, &punk_hit, DoBloodSplat);
                SoundEffect(70, &item->pos, 0);
                if (item->item_flags[2] == 1)
                    LaraBurn();
                else if (item->item_flags[2])
                    item->item_flags[2]--;
                punk->flags = 1;
            }
            break;
        case PUNK_PUNCH1:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            punk->maximum_turn = PUNK_WALK_TURN;
            if (!punk->flags && (item->touch_bits & PUNK_TOUCH)) {
                lara_item->hit_points -= PUNK_HIT_DAMAGE;
                lara_item->hit_status = 1;
                CreatureEffect(item, &punk_hit, DoBloodSplat);
                SoundEffect(70, &item->pos, 0);
                if (item->item_flags[2] == 1)
                    LaraBurn();
                else if (item->item_flags[2])
                    item->item_flags[2]--;
                punk->flags = 1;
            }
            if (info.ahead && info.distance > PUNK_ATTACK1_RANGE && info.distance < PUNK_ATTACK2_RANGE)
                item->goal_anim_state = PUNK_PUNCH2;
            break;
        case PUNK_PUNCH2:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            punk->maximum_turn = PUNK_WALK_TURN;
            if (punk->flags != 2 && (item->touch_bits & PUNK_TOUCH)) {
                lara_item->hit_points -= PUNK_SWIPE_DAMAGE;
                lara_item->hit_status = 1;
                CreatureEffect(item, &punk_hit, DoBloodSplat);
                SoundEffect(70, &item->pos, 0);
                if (item->item_flags[2] == 1)
                    LaraBurn();
                else if (item->item_flags[2])
                    item->item_flags[2]--;
                punk->flags = 2;
            }
            break;
        }
    }
    CreatureTilt(item, tilt);
    CreatureJoint(item, 0, torso_y);
    CreatureJoint(item, 1, torso_x);
    CreatureJoint(item, 2, head);
    if (item->current_anim_state < PUNK_DEATH) {
        switch (CreatureVault(item_number, angle, 2, PUNK_VAULT_SHIFT)) {
        case 2:
            punk->maximum_turn = 0;
            item->anim_number = objects[PUNK1].anim_index + PUNK_CLIMB1_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = PUNK_CLIMB1;
            break;
        case 3:
            punk->maximum_turn = 0;
            item->anim_number = objects[PUNK1].anim_index + PUNK_CLIMB2_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = PUNK_CLIMB2;
            break;
        case 4:
            punk->maximum_turn = 0;
            item->anim_number = objects[PUNK1].anim_index + PUNK_CLIMB3_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = PUNK_CLIMB3;
            break;
        case -4:
            punk->maximum_turn = 0;
            item->anim_number = objects[PUNK1].anim_index + PUNK_FALL3_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = PUNK_FALL3;
            break;
        }
    } else {
        punk->maximum_turn = 0;
        CreatureAnimation(item_number, angle, 0);
    }
}

sint32 GetFreeSpark(void);

void TriggerPunkFlame(sint16 item_number)
{
    sint32 size;
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - items[item_number].pos.x_pos;
    dz = lara_item->pos.z_pos - items[item_number].pos.z_pos;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 255;
    sptr->sG = 48 + (GetRandomControl() & 31);
    sptr->sB = 48;
    sptr->dR = 192 + (GetRandomControl() & 63);
    sptr->dG = 128 + (GetRandomControl() & 63);
    sptr->dB = 32;
    sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
    sptr->FadeToBlack = 8;
    sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 24;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = (GetRandomControl() & 15) - 8;
    sptr->y = 0;
    sptr->z = (GetRandomControl() & 15) - 8;
    sptr->Xvel = (GetRandomControl() & 255) - 128;
    sptr->Yvel = -(GetRandomControl() & 15) - 16;
    sptr->Zvel = (GetRandomControl() & 255) - 128;
    sptr->Friction = 5;
    if (GetRandomControl() & 1) {
        sptr->Gravity = -(GetRandomControl() & 31) - 16;
        sptr->MaxYvel = -(GetRandomControl() & 7) - 16;
        sptr->Flags = 2 | 8 | 16 | 128 | 512 | 4096;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 15) - 16;
        else
            sptr->RotAdd = (GetRandomControl() & 15) + 16;
    } else {
        sptr->Flags = 2 | 8 | 128 | 512 | 4096;
        sptr->Gravity = -(GetRandomControl() & 31) - 16;
        sptr->MaxYvel = -(GetRandomControl() & 7) - 16;
    }
    sptr->FxObj = item_number;
    sptr->NodeNumber = 2;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 1;
    size = (GetRandomControl() & 31) + 64;
    sptr->Width = sptr->sWidth = size;
    sptr->Height = sptr->sHeight = size;
    sptr->dWidth = size >> 2;
    sptr->dHeight = size >> 2;
}
