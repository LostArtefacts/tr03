#include "game.h"

#define BATON_STOP_ANIM 6
#define BATON_STOP 1

void InitialiseBaton(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    InitialiseCreature(item_number);
    item->anim_number = objects[MP1].anim_index + BATON_STOP_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = BATON_STOP;
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

#define BATON_WALK 2
#define BATON_PUNCH2 3
#define BATON_AIM2 4
#define BATON_WAIT 5
#define BATON_AIM1 6
#define BATON_AIM0 7
#define BATON_PUNCH1 8
#define BATON_PUNCH0 9
#define BATON_RUN 10
#define BATON_DEATH 11
#define BATON_KICK 12
#define BATON_CLIMB3 13
#define BATON_CLIMB1 14
#define BATON_CLIMB2 15
#define BATON_FALL3 16

#define SQUARE(x) ((x) * (x))
#define BATON_HIT_DAMAGE 80
#define BATON_SWIPE_DAMAGE 100
#define BATON_KICK_DAMAGE 150
#define BATON_KICK_RANGE SQUARE(WALL_L * 3 / 2)
#define BATON_KICK_TOUCH 0x60
#define BATON_WALK_TURN (ONE_DEGREE * 6)
#define BATON_RUN_TURN (ONE_DEGREE * 7)
#define BATON_ATTACK0_RANGE SQUARE(WALL_L / 2)
#define BATON_ATTACK1_RANGE SQUARE(WALL_L)
#define BATON_ATTACK2_RANGE SQUARE(WALL_L * 5 / 4)
#define BATON_WALK_RANGE SQUARE(WALL_L)
#define BATON_WAIT_CHANCE 0x100
#define BATON_DIE_ANIM 26
#define BATON_CLIMB1_ANIM 28
#define BATON_CLIMB2_ANIM 29
#define BATON_CLIMB3_ANIM 27
#define BATON_FALL3_ANIM 30
#define BATON_TOUCH 0x2400
#define BATON_VAULT_SHIFT 260
#define BATON_AWARE_DISTANCE SQUARE(WALL_L)
#define GUARD 1
#define PATROL1 4
#define FOLLOW 16
#define BATON_HIT_RADIUS STEP_L

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

extern BITE_INFO baton_hit;
extern BITE_INFO baton_kick;

int abs(int x);
#pragma intrinsic(abs)

void BatonControl(sint16 item_number)
{
    ITEM_INFO *item, *enemy, *target;
    CREATURE_INFO *sec, *cinfo;
    sint16 angle, torso_y, torso_x, head, tilt, slot;
    sint32 lara_dx, lara_dz, x, z, distance, best_distance;
    AI_INFO info, lara_info;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    sec = (CREATURE_INFO *)item->data;
    torso_y = torso_x = head = angle = tilt = 0;
    if (boxes[item->box_number].overlap_index & 0x4000) {
        DoLotsOfBloodD(item->pos.x_pos, item->pos.y_pos - (GetRandomControl() & 255) - 32, item->pos.z_pos,
                       (GetRandomControl() & 127) + 128, GetRandomControl() << 1, item->room_number, 3);
        item->hit_points -= 20;
    }
    if (item->hit_points <= 0) {
        if (item->current_anim_state != BATON_DEATH) {
            item->anim_number = objects[MP1].anim_index + BATON_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = BATON_DEATH;
            sec->LOT.step = STEP_L;
        }
    } else {
        if (item->ai_bits)
            GetAITarget(sec);
        else {
            sec->enemy = lara_item;
            lara_dz = lara_item->pos.z_pos - item->pos.z_pos;
            lara_dx = lara_item->pos.x_pos - item->pos.x_pos;
            lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
            best_distance = 0x7fffffff;
            cinfo = baddie_slots;
            for (slot = 0; slot < NUM_SLOTS; slot++, cinfo++) {
                if (cinfo->item_num == NO_ITEM || cinfo->item_num == item_number)
                    continue;
                target = &items[cinfo->item_num];
                if (target->object_number != LARA && target->object_number != BOB)
                    continue;
                x = target->pos.x_pos - item->pos.x_pos;
                z = target->pos.z_pos - item->pos.z_pos;
                if (z > 32000 || z < -32000 || x > 32000 || x < -32000)
                    continue;
                distance = x * x + z * z;
                if (distance < best_distance && distance < lara_info.distance) {
                    sec->enemy = target;
                    best_distance = distance;
                }
            }
        }
        CreatureAIInfo(item, &info);
        if (sec->enemy == lara_item) {
            lara_info.angle = info.angle;
            lara_info.distance = info.distance;
        } else {
            lara_dz = lara_item->pos.z_pos - item->pos.z_pos;
            lara_dx = lara_item->pos.x_pos - item->pos.x_pos;
            lara_info.angle = phd_atan(lara_dz, lara_dx) - item->pos.y_rot;
            lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
        }
        GetCreatureMood(item, &info, 1);
        CreatureMood(item, &info, 1);
        angle = CreatureTurn(item, sec->maximum_turn);
        enemy = sec->enemy;
        sec->enemy = lara_item;
        if (item->hit_status
            || ((lara_info.distance < BATON_AWARE_DISTANCE || TargetVisible(item, &lara_info))
                && abs(lara_item->pos.y_pos - item->pos.y_pos) < WALL_L)) {
            if (!sec->alerted)
                SoundEffect(300, &item->pos, 0);
            AlertAllGuards(item_number);
        }
        sec->enemy = enemy;
        switch (item->current_anim_state) {
        case BATON_WAIT:
            if (sec->alerted || item->goal_anim_state == BATON_RUN) {
                item->goal_anim_state = BATON_STOP;
                break;
            }
        case BATON_STOP:
            sec->flags = 0;
            sec->maximum_turn = 0;
            head = lara_info.angle;
            if (item->ai_bits & GUARD) {
                head = AIGuard(sec);
                if (!(GetRandomControl() & 0xff)) {
                    if (item->current_anim_state == BATON_STOP)
                        item->goal_anim_state = BATON_WAIT;
                    else
                        item->goal_anim_state = BATON_STOP;
                }
                break;
            } else if (item->ai_bits & PATROL1)
                item->goal_anim_state = BATON_WALK;
            else if (sec->mood == ESCAPE_MOOD) {
                if (lara.target != item && info.ahead && !item->hit_status)
                    item->goal_anim_state = BATON_STOP;
                else
                    item->goal_anim_state = BATON_RUN;
            } else if (sec->mood == BORED_MOOD
                       || ((item->ai_bits & FOLLOW)
                           && (sec->reached_goal || lara_info.distance > SQUARE(WALL_L * 2)))) {
                if (item->required_anim_state)
                    item->goal_anim_state = item->required_anim_state;
                else if (info.ahead)
                    item->goal_anim_state = BATON_STOP;
                else
                    item->goal_anim_state = BATON_RUN;
            } else if (info.bite && info.distance < BATON_ATTACK0_RANGE)
                item->goal_anim_state = BATON_AIM0;
            else if (info.bite && info.distance < BATON_ATTACK1_RANGE)
                item->goal_anim_state = BATON_AIM1;
            else if (info.bite && info.distance < BATON_WALK_RANGE)
                item->goal_anim_state = BATON_WALK;
            else
                item->goal_anim_state = BATON_RUN;
            break;
        case BATON_WALK:
            head = lara_info.angle;
            sec->flags = 0;
            sec->maximum_turn = BATON_WALK_TURN;
            if (item->ai_bits & PATROL1) {
                item->goal_anim_state = BATON_WALK;
                head = 0;
            } else if (sec->mood == ESCAPE_MOOD)
                item->goal_anim_state = BATON_RUN;
            else if (sec->mood == BORED_MOOD) {
                if (GetRandomControl() < BATON_WAIT_CHANCE) {
                    item->required_anim_state = BATON_WAIT;
                    item->goal_anim_state = BATON_STOP;
                }
            } else if (info.bite && info.distance < BATON_KICK_RANGE && info.x_angle < 0)
                item->goal_anim_state = BATON_KICK;
            else if (info.bite && info.distance < BATON_ATTACK0_RANGE)
                item->goal_anim_state = BATON_STOP;
            else if (info.bite && info.distance < BATON_ATTACK2_RANGE)
                item->goal_anim_state = BATON_AIM2;
            else
                item->goal_anim_state = BATON_RUN;
            break;
        case BATON_RUN:
            if (info.ahead)
                head = info.angle;
            sec->maximum_turn = BATON_RUN_TURN;
            tilt = angle / 2;
            if (item->ai_bits & GUARD)
                item->goal_anim_state = BATON_WAIT;
            else if (sec->mood == ESCAPE_MOOD) {
                if (lara.target != item && info.ahead)
                    item->goal_anim_state = BATON_STOP;
                break;
            } else if ((item->ai_bits & FOLLOW) && (sec->reached_goal || lara_info.distance > SQUARE(WALL_L * 2)))
                item->goal_anim_state = BATON_STOP;
            else if (sec->mood == BORED_MOOD)
                item->goal_anim_state = BATON_WALK;
            else if (info.ahead && info.distance < BATON_WALK_RANGE)
                item->goal_anim_state = BATON_WALK;
            break;
        case BATON_AIM0:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            sec->maximum_turn = BATON_WALK_TURN;
            sec->flags = 0;
            if (info.bite && info.distance < BATON_ATTACK0_RANGE)
                item->goal_anim_state = BATON_PUNCH0;
            else
                item->goal_anim_state = BATON_STOP;
            break;
        case BATON_AIM1:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            sec->maximum_turn = BATON_WALK_TURN;
            sec->flags = 0;
            if (info.ahead && info.distance < BATON_ATTACK1_RANGE)
                item->goal_anim_state = BATON_PUNCH1;
            else
                item->goal_anim_state = BATON_STOP;
            break;
        case BATON_AIM2:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            sec->maximum_turn = BATON_WALK_TURN;
            sec->flags = 0;
            if (info.bite && info.distance < BATON_ATTACK2_RANGE)
                item->goal_anim_state = BATON_PUNCH2;
            else
                item->goal_anim_state = BATON_WALK;
            break;
        case BATON_PUNCH0:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            sec->maximum_turn = BATON_WALK_TURN;
            if (enemy == lara_item) {
                if (!sec->flags && (item->touch_bits & BATON_TOUCH)) {
                    lara_item->hit_points -= BATON_HIT_DAMAGE;
                    lara_item->hit_status = 1;
                    CreatureEffect(item, &baton_hit, DoBloodSplat);
                    SoundEffect(70, &item->pos, 0);
                    sec->flags = 1;
                }
            } else if (!sec->flags && enemy) {
                if (ABS(enemy->pos.x_pos - item->pos.x_pos) < BATON_HIT_RADIUS
                    && ABS(enemy->pos.y_pos - item->pos.y_pos) <= BATON_HIT_RADIUS
                    && ABS(enemy->pos.z_pos - item->pos.z_pos) < BATON_HIT_RADIUS) {
                    enemy->hit_points -= BATON_HIT_DAMAGE >> 4;
                    enemy->hit_status = 1;
                    sec->flags = 1;
                    CreatureEffect(item, &baton_hit, DoBloodSplat);
                    SoundEffect(70, &item->pos, 0);
                }
            }
            break;
        case BATON_PUNCH1:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            sec->maximum_turn = BATON_WALK_TURN;
            if (enemy == lara_item) {
                if (!sec->flags && (item->touch_bits & BATON_TOUCH)) {
                    lara_item->hit_points -= BATON_HIT_DAMAGE;
                    lara_item->hit_status = 1;
                    CreatureEffect(item, &baton_hit, DoBloodSplat);
                    SoundEffect(70, &item->pos, 0);
                    sec->flags = 1;
                }
            } else if (!sec->flags && enemy) {
                if (ABS(enemy->pos.x_pos - item->pos.x_pos) < BATON_HIT_RADIUS
                    && ABS(enemy->pos.y_pos - item->pos.y_pos) <= BATON_HIT_RADIUS
                    && ABS(enemy->pos.z_pos - item->pos.z_pos) < BATON_HIT_RADIUS) {
                    enemy->hit_points -= BATON_HIT_DAMAGE >> 4;
                    enemy->hit_status = 1;
                    sec->flags = 1;
                    CreatureEffect(item, &baton_hit, DoBloodSplat);
                    SoundEffect(70, &item->pos, 0);
                }
            }
            if (info.ahead && info.distance > BATON_ATTACK1_RANGE && info.distance < BATON_ATTACK2_RANGE)
                item->goal_anim_state = BATON_PUNCH2;
            break;
        case BATON_PUNCH2:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            sec->maximum_turn = BATON_WALK_TURN;
            if (enemy == lara_item) {
                if (sec->flags != 2 && (item->touch_bits & BATON_TOUCH)) {
                    lara_item->hit_points -= BATON_SWIPE_DAMAGE;
                    lara_item->hit_status = 1;
                    CreatureEffect(item, &baton_hit, DoBloodSplat);
                    sec->flags = 2;
                    SoundEffect(70, &item->pos, 0);
                }
            } else if (sec->flags != 2 && enemy) {
                if (ABS(enemy->pos.x_pos - item->pos.x_pos) < BATON_HIT_RADIUS
                    && ABS(enemy->pos.y_pos - item->pos.y_pos) <= BATON_HIT_RADIUS
                    && ABS(enemy->pos.z_pos - item->pos.z_pos) < BATON_HIT_RADIUS) {
                    enemy->hit_points -= BATON_SWIPE_DAMAGE >> 4;
                    enemy->hit_status = 1;
                    sec->flags = 2;
                    CreatureEffect(item, &baton_hit, DoBloodSplat);
                    SoundEffect(70, &item->pos, 0);
                }
            }
            break;
        case BATON_KICK:
            if (info.ahead)
                torso_y = info.angle;
            sec->maximum_turn = BATON_WALK_TURN;
            if (enemy == lara_item) {
                if (sec->flags != 1 && (item->touch_bits & BATON_KICK_TOUCH)
                    && item->frame_number > anims[item->anim_number].frame_base + 8) {
                    lara_item->hit_points -= BATON_KICK_DAMAGE;
                    lara_item->hit_status = 1;
                    CreatureEffect(item, &baton_kick, DoBloodSplat);
                    SoundEffect(70, &item->pos, 0);
                    sec->flags = 1;
                }
            } else if (!sec->flags != 1 && enemy && item->frame_number > anims[item->anim_number].frame_base + 8) {
                if (ABS(enemy->pos.x_pos - item->pos.x_pos) < BATON_HIT_RADIUS
                    && ABS(enemy->pos.y_pos - item->pos.y_pos) <= BATON_HIT_RADIUS
                    && ABS(enemy->pos.z_pos - item->pos.z_pos) < BATON_HIT_RADIUS) {
                    enemy->hit_points -= BATON_KICK_DAMAGE >> 4;
                    enemy->hit_status = 1;
                    sec->flags = 1;
                    CreatureEffect(item, &baton_kick, DoBloodSplat);
                    SoundEffect(70, &item->pos, 0);
                }
            }
            break;
        }
    }
    CreatureTilt(item, tilt);
    CreatureJoint(item, 0, torso_y);
    CreatureJoint(item, 1, torso_x);
    CreatureJoint(item, 2, head);
    if (item->current_anim_state < BATON_DEATH) {
        switch (CreatureVault(item_number, angle, 2, BATON_VAULT_SHIFT)) {
        case 2:
            sec->maximum_turn = 0;
            item->anim_number = objects[MP1].anim_index + BATON_CLIMB1_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = BATON_CLIMB1;
            break;
        case 3:
            sec->maximum_turn = 0;
            item->anim_number = objects[MP1].anim_index + BATON_CLIMB2_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = BATON_CLIMB2;
            break;
        case 4:
            sec->maximum_turn = 0;
            item->anim_number = objects[MP1].anim_index + BATON_CLIMB3_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = BATON_CLIMB3;
            break;
        case -4:
            sec->maximum_turn = 0;
            item->anim_number = objects[MP1].anim_index + BATON_FALL3_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = BATON_FALL3;
            break;
        }
    } else {
        sec->maximum_turn = 0;
        CreatureAnimation(item_number, angle, 0);
    }
}
