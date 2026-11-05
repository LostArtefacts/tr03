#include "game.h"

#define SWAT_STOP_ANIM 12
#define SWAT_STOP 1

void InitialiseSwat(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    InitialiseCreature(item_number);
    item->anim_number = objects[item->object_number].anim_index + SWAT_STOP_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = SWAT_STOP;
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

#define SWAT_WALK 2
#define SWAT_RUN 3
#define SWAT_WAIT 4
#define SWAT_SHOOT1 5
#define SWAT_SHOOT2 6
#define SWAT_DEATH 7
#define SWAT_AIM1 8
#define SWAT_AIM2 9
#define SWAT_AIM3 10
#define SWAT_SHOOT3 11

#define SQUARE(x) ((x) * (x))
#define SWAT_SHOT_DAMAGE 28
#define SWAT_WALK_TURN (ONE_DEGREE * 6)
#define SWAT_RUN_TURN (ONE_DEGREE * 9)
#define SWAT_DEATH_SHOT_ANGLE 0x2000
#define SWAT_RUN_RANGE SQUARE(WALL_L * 2)
#define SWAT_SHOOT1_RANGE SQUARE(WALL_L * 3)
#define SWAT_AWARE_DISTANCE SQUARE(WALL_L)
#define SWAT_DIE_ANIM 19
#define SWAT_WALK_STOP_ANIM 17
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
void GetAITarget(CREATURE_INFO *creature);
void AlertAllGuards(sint16 item_number);
sint16 AIGuard(CREATURE_INFO *creature);
int TargetVisible(ITEM_INFO *item, AI_INFO *info);
int Targetable(ITEM_INFO *item, AI_INFO *info);
int ShotLara(ITEM_INFO *item, AI_INFO *info, BITE_INFO *gun, sint16 extra_rotation, int damage);
void phd_PushMatrix(void);
void DoLotsOfBloodD(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num, int number);

extern BITE_INFO swat_gun;

int abs(int x);
#pragma intrinsic(abs)

void SwatControl(sint16 item_number)
{
    ITEM_INFO *item, *real_enemy;
    CREATURE_INFO *swat;
    sint16 angle, torso_y, torso_x, head, tilt, meta_mood;
    sint32 lara_dx, lara_dz;
    AI_INFO info, lara_info;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    swat = (CREATURE_INFO *)item->data;
    torso_y = torso_x = head = angle = tilt = 0;
    if (!swat)
        return;
    if (item->fired_weapon) {
        PHD_VECTOR pos;

        phd_PushMatrix();
        pos.x = swat_gun.x;
        pos.y = swat_gun.y;
        pos.z = swat_gun.z;
        GetJointAbsPosition(item, &pos, swat_gun.mesh_num);
        TriggerDynamic(pos.x, pos.y, pos.z, (item->fired_weapon << 1) + 8, 24, 16, 4);
        phd_PopMatrix();
    }
    if (boxes[item->box_number].overlap_index & 0x4000) {
        DoLotsOfBloodD(item->pos.x_pos, item->pos.y_pos - (GetRandomControl() & 255) - 32, item->pos.z_pos,
                       (GetRandomControl() & 127) + 128, GetRandomControl() << 1, item->room_number, 3);
        item->hit_points -= 20;
    }
    if (item->hit_points <= 0) {
        if (item->current_anim_state != SWAT_DEATH) {
            item->anim_number = objects[item->object_number].anim_index + SWAT_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = SWAT_DEATH;
            if (!(GetRandomControl() & 1))
                swat->flags = 1;
            else
                swat->flags = 0;
        } else if (swat->flags
                   && item->frame_number > anims[item->anim_number].frame_base + 44
                   && item->frame_number < anims[item->anim_number].frame_base + 52
                   && !(item->frame_number & 3)) {
            CreatureAIInfo(item, &info);
            if (Targetable(item, &info)) {
                if (info.angle > -SWAT_DEATH_SHOT_ANGLE && info.angle < SWAT_DEATH_SHOT_ANGLE) {
                    torso_y = info.angle;
                    head = info.angle;
                    ShotLara(item, &info, &swat_gun, torso_y, SWAT_SHOT_DAMAGE * 3);
                    if (item->object_number == 51)
                        SoundEffect(72, &item->pos, 3 << 13);
                    else
                        SoundEffect(137, &item->pos, 3 << 13);
                }
            }
        }
    } else {
        if (item->ai_bits)
            GetAITarget(swat);
        else
            swat->enemy = lara_item;
        CreatureAIInfo(item, &info);
        if (swat->enemy == lara_item) {
            lara_info.angle = info.angle;
            lara_info.distance = info.distance;
        } else {
            lara_dz = lara_item->pos.z_pos - item->pos.z_pos;
            lara_dx = lara_item->pos.x_pos - item->pos.x_pos;
            lara_info.angle = phd_atan(lara_dz, lara_dx) - item->pos.y_rot;
            lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
        }
        if (swat->enemy != lara_item)
            meta_mood = 1;
        else
            meta_mood = 0;
        GetCreatureMood(item, &info, meta_mood);
        CreatureMood(item, &info, meta_mood);
        angle = CreatureTurn(item, swat->maximum_turn);
        real_enemy = swat->enemy;
        swat->enemy = lara_item;
        if (item->hit_status
            || ((lara_info.distance < SWAT_AWARE_DISTANCE || TargetVisible(item, &lara_info))
                && abs(lara_item->pos.y_pos - item->pos.y_pos) < WALL_L * 2)) {
            if (!swat->alerted) {
                if (item->object_number == 63)
                    SoundEffect(300, &item->pos, 0);
                else
                    SoundEffect(299, &item->pos, 0);
            }
            AlertAllGuards(item_number);
        }
        swat->enemy = real_enemy;
        switch (item->current_anim_state) {
        case SWAT_STOP:
            head = lara_info.angle;
            swat->flags = 0;
            swat->maximum_turn = 0;
            if (item->anim_number == objects[item->object_number].anim_index + SWAT_WALK_STOP_ANIM) {
                if (abs(info.angle) < SWAT_RUN_TURN)
                    item->pos.y_rot += info.angle;
                else if (info.angle < 0)
                    item->pos.y_rot -= SWAT_RUN_TURN;
                else
                    item->pos.y_rot += SWAT_RUN_TURN;
            }
            if (item->ai_bits & GUARD) {
                head = AIGuard(swat);
                if (!(GetRandomControl() & 0xff)) {
                    if (item->current_anim_state == SWAT_STOP)
                        item->goal_anim_state = SWAT_WAIT;
                    else
                        item->goal_anim_state = SWAT_STOP;
                }
                break;
            } else if (item->ai_bits & PATROL1) {
                item->goal_anim_state = SWAT_WALK;
                head = 0;
            }
            else if (swat->mood == ESCAPE_MOOD)
                item->goal_anim_state = SWAT_RUN;
            else if (Targetable(item, &info)) {
                if (info.distance < SWAT_SHOOT1_RANGE || info.zone_number != info.enemy_zone) {
                    if (GetRandomControl() < 0x4000)
                        item->goal_anim_state = SWAT_AIM1;
                    else
                        item->goal_anim_state = SWAT_AIM3;
                } else
                    item->goal_anim_state = SWAT_WALK;
            } else if (swat->mood == BORED_MOOD
                       || ((item->ai_bits & FOLLOW)
                           && (swat->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
                item->goal_anim_state = SWAT_STOP;
            else if (swat->mood != BORED_MOOD && info.distance > SWAT_RUN_RANGE)
                item->goal_anim_state = SWAT_RUN;
            else
                item->goal_anim_state = SWAT_WALK;
            break;
        case SWAT_WAIT:
            head = lara_info.angle;
            swat->flags = 0;
            swat->maximum_turn = 0;
            if (item->ai_bits & GUARD) {
                head = AIGuard(swat);
                if (!(GetRandomControl() & 0xff)) {
                    if (item->current_anim_state == SWAT_STOP)
                        item->goal_anim_state = SWAT_WAIT;
                    else
                        item->goal_anim_state = SWAT_STOP;
                }
                break;
            } else if (Targetable(item, &info))
                item->goal_anim_state = SWAT_SHOOT1;
            else if (swat->mood != BORED_MOOD || !info.ahead)
                item->goal_anim_state = SWAT_STOP;
            break;
        case SWAT_WALK:
            head = lara_info.angle;
            swat->flags = 0;
            swat->maximum_turn = SWAT_WALK_TURN;
            if (item->ai_bits & PATROL1) {
                head = 0;
                item->goal_anim_state = SWAT_WALK;
            } else if (swat->mood == ESCAPE_MOOD)
                item->goal_anim_state = SWAT_RUN;
            else if ((item->ai_bits & GUARD)
                     || ((item->ai_bits & FOLLOW)
                         && (swat->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
                item->goal_anim_state = SWAT_STOP;
            else if (Targetable(item, &info)) {
                if (info.distance < SWAT_SHOOT1_RANGE || info.zone_number != info.enemy_zone)
                    item->goal_anim_state = SWAT_STOP;
                else
                    item->goal_anim_state = SWAT_AIM2;
            } else if (swat->mood == BORED_MOOD && info.ahead)
                item->goal_anim_state = SWAT_STOP;
            else if (swat->mood != BORED_MOOD && info.distance > SWAT_RUN_RANGE)
                item->goal_anim_state = SWAT_RUN;
            break;
        case SWAT_RUN:
            if (info.ahead)
                head = info.angle;
            swat->maximum_turn = SWAT_RUN_TURN;
            tilt = angle / 2;
            if ((item->ai_bits & GUARD)
                || ((item->ai_bits & FOLLOW)
                    && (swat->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
                item->goal_anim_state = SWAT_WALK;
            else if (swat->mood == ESCAPE_MOOD)
                break;
            else if (Targetable(item, &info))
                item->goal_anim_state = SWAT_WALK;
            else if (swat->mood == BORED_MOOD
                     || (swat->mood == STALK_MOOD && !(item->ai_bits & FOLLOW)
                         && info.distance < SWAT_RUN_RANGE))
                item->goal_anim_state = SWAT_WALK;
            break;
        case SWAT_AIM1:
        case SWAT_AIM3:
            swat->flags = 0;
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
                if (Targetable(item, &info))
                    item->goal_anim_state = (item->current_anim_state == SWAT_AIM1) ? SWAT_SHOOT1 : SWAT_SHOOT3;
                else
                    item->goal_anim_state = SWAT_STOP;
            }
            break;
        case SWAT_AIM2:
            swat->flags = 0;
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
                if (Targetable(item, &info))
                    item->goal_anim_state = SWAT_SHOOT2;
                else
                    item->goal_anim_state = SWAT_WALK;
            }
            break;
        case SWAT_SHOOT3:
            if (item->goal_anim_state != SWAT_STOP) {
                if (swat->mood == ESCAPE_MOOD || info.distance > SWAT_SHOOT1_RANGE || !Targetable(item, &info))
                    item->goal_anim_state = SWAT_STOP;
            }
        case SWAT_SHOOT2:
        case SWAT_SHOOT1:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            if (!swat->flags) {
                ShotLara(item, &info, &swat_gun, torso_y, SWAT_SHOT_DAMAGE);
                swat->flags = 5;
            } else
                swat->flags--;
            break;
        }
    }
    CreatureTilt(item, tilt);
    CreatureJoint(item, 0, torso_y);
    CreatureJoint(item, 1, torso_x);
    CreatureJoint(item, 2, head);
    CreatureAnimation(item_number, angle, 0);
}
