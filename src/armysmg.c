#include "game.h"

#define ARMY_STOP_ANIM 12
#define ARMY_STOP 1

void InitialiseArmySMG(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    InitialiseCreature(item_number);
    item->anim_number = objects[STHPAC_MERCENARY].anim_index + ARMY_STOP_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = ARMY_STOP;
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

#define ARMY_WALK 2
#define ARMY_RUN 3
#define ARMY_WAIT 4
#define ARMY_SHOOT1 5
#define ARMY_SHOOT2 6
#define ARMY_DEATH 7
#define ARMY_AIM1 8
#define ARMY_AIM2 9
#define ARMY_AIM3 10
#define ARMY_SHOOT3 11

#define SQUARE(x) ((x) * (x))
#define ARMY_SHOT_DAMAGE 28
#define ARMY_WALK_TURN (ONE_DEGREE * 5)
#define ARMY_RUN_TURN (ONE_DEGREE * 10)
#define ARMY_RUN_RANGE SQUARE(WALL_L * 2)
#define ARMY_SHOOT1_RANGE SQUARE(WALL_L * 3)
#define ARMY_DIE_ANIM 19
#define ARMY_WALK_STOP_ANIM 17
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
int Targetable(ITEM_INFO *item, AI_INFO *info);
int ShotLara(ITEM_INFO *item, AI_INFO *info, BITE_INFO *gun, sint16 extra_rotation, int damage);
void phd_PushMatrix(void);

extern BITE_INFO army_gun;

int abs(int x);
#pragma intrinsic(abs)

void ArmySMGControl(sint16 item_number)
{
    ITEM_INFO *item, *target, *real_enemy;
    CREATURE_INFO *army, *cinfo;
    sint16 angle, torso_y, torso_x, head, tilt, meta_mood, slot;
    sint32 lara_dx, lara_dz, x, z, distance, best_distance;
    AI_INFO info, lara_info;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    army = (CREATURE_INFO *)item->data;
    torso_y = torso_x = head = angle = tilt = 0;
    if (!army)
        return;
    if (item->fired_weapon) {
        PHD_VECTOR pos;

        phd_PushMatrix();
        pos.x = army_gun.x;
        pos.y = army_gun.y;
        pos.z = army_gun.z;
        GetJointAbsPosition(item, &pos, army_gun.mesh_num);
        TriggerDynamic(pos.x, pos.y, pos.z, (item->fired_weapon << 1) + 8, 24, 16, 4);
        phd_PopMatrix();
    }
    if (item->hit_points <= 0) {
        if (item->current_anim_state != ARMY_DEATH) {
            item->anim_number = objects[item->object_number].anim_index + ARMY_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = ARMY_DEATH;
            if (!(GetRandomControl() & 3))
                army->flags = 1;
            else
                army->flags = 0;
        }
    } else {
        if (item->ai_bits)
            GetAITarget(army);
        else if (army->hurt_by_lara)
            army->enemy = lara_item;
        else {
            army->enemy = NULL;
            best_distance = 0x7fffffff;
            cinfo = baddie_slots;
            for (slot = 0; slot < NUM_SLOTS; slot++, cinfo++) {
                if (cinfo->item_num == NO_ITEM || cinfo->item_num == item_number)
                    continue;
                target = &items[cinfo->item_num];
                if (target->object_number == LARA || target->object_number == STHPAC_MERCENARY
                    || (target == lara_item && !army->hurt_by_lara))
                    continue;
                x = target->pos.x_pos - item->pos.x_pos;
                z = target->pos.z_pos - item->pos.z_pos;
                distance = x * x + z * z;
                if (distance < best_distance) {
                    army->enemy = target;
                    best_distance = distance;
                }
            }
        }
        CreatureAIInfo(item, &info);
        if (army->enemy == lara_item) {
            lara_info.angle = info.angle;
            lara_info.distance = info.distance;
        } else {
            lara_dz = lara_item->pos.z_pos - item->pos.z_pos;
            lara_dx = lara_item->pos.x_pos - item->pos.x_pos;
            lara_info.angle = phd_atan(lara_dz, lara_dx) - item->pos.y_rot;
            lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
        }
        if (!army->hurt_by_lara && army->enemy == lara_item)
            army->enemy = NULL;
        if (army->enemy != lara_item)
            meta_mood = 1;
        else
            meta_mood = 0;
        GetCreatureMood(item, &info, meta_mood);
        CreatureMood(item, &info, meta_mood);
        angle = CreatureTurn(item, army->maximum_turn);
        real_enemy = army->enemy;
        if (item->hit_status) {
            if (!army->alerted)
                SoundEffect(300, &item->pos, 0);
            AlertAllGuards(item_number);
        }
        switch (item->current_anim_state) {
        case ARMY_STOP:
            head = lara_info.angle;
            army->flags = 0;
            army->maximum_turn = 0;
            if (item->anim_number == objects[item->object_number].anim_index + ARMY_WALK_STOP_ANIM) {
                if (abs(info.angle) < ARMY_RUN_TURN)
                    item->pos.y_rot += info.angle;
                else if (info.angle < 0)
                    item->pos.y_rot -= ARMY_RUN_TURN;
                else
                    item->pos.y_rot += ARMY_RUN_TURN;
            }
            if (item->ai_bits & GUARD) {
                head = AIGuard(army);
                if (!(GetRandomControl() & 0xff)) {
                    if (item->current_anim_state == ARMY_STOP)
                        item->goal_anim_state = ARMY_WAIT;
                    else
                        item->goal_anim_state = ARMY_STOP;
                }
                break;
            } else if (item->ai_bits & PATROL1) {
                item->goal_anim_state = ARMY_WALK;
                head = 0;
            }
            else if (army->mood == ESCAPE_MOOD)
                item->goal_anim_state = ARMY_RUN;
            else if (Targetable(item, &info)) {
                if (info.distance < ARMY_SHOOT1_RANGE || info.zone_number != info.enemy_zone) {
                    if (GetRandomControl() < 0x4000)
                        item->goal_anim_state = ARMY_AIM1;
                    else
                        item->goal_anim_state = ARMY_AIM3;
                } else
                    item->goal_anim_state = ARMY_WALK;
            } else if ((!army->alerted && army->mood == BORED_MOOD)
                       || ((item->ai_bits & FOLLOW)
                           && (army->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
                item->goal_anim_state = ARMY_STOP;
            else if (army->mood != BORED_MOOD && info.distance > ARMY_RUN_RANGE)
                item->goal_anim_state = ARMY_RUN;
            else
                item->goal_anim_state = ARMY_WALK;
            break;
        case ARMY_WAIT:
            head = lara_info.angle;
            army->flags = 0;
            army->maximum_turn = 0;
            if (item->ai_bits & GUARD) {
                head = AIGuard(army);
                if (!(GetRandomControl() & 0xff))
                    item->goal_anim_state = ARMY_STOP;
                break;
            } else if (Targetable(item, &info))
                item->goal_anim_state = ARMY_SHOOT1;
            else if (army->mood != BORED_MOOD || !info.ahead)
                item->goal_anim_state = ARMY_STOP;
            break;
        case ARMY_WALK:
            head = lara_info.angle;
            army->flags = 0;
            army->maximum_turn = ARMY_WALK_TURN;
            if (item->ai_bits & PATROL1)
                item->goal_anim_state = ARMY_WALK;
            else if (army->mood == ESCAPE_MOOD)
                item->goal_anim_state = ARMY_RUN;
            else if ((item->ai_bits & GUARD)
                     || ((item->ai_bits & FOLLOW)
                         && (army->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
                item->goal_anim_state = ARMY_STOP;
            else if (Targetable(item, &info)) {
                if (info.distance < ARMY_SHOOT1_RANGE || info.zone_number != info.enemy_zone)
                    item->goal_anim_state = ARMY_STOP;
                else
                    item->goal_anim_state = ARMY_AIM2;
            } else if (army->mood == BORED_MOOD && info.ahead)
                item->goal_anim_state = ARMY_STOP;
            else if (army->mood != BORED_MOOD && info.distance > ARMY_RUN_RANGE)
                item->goal_anim_state = ARMY_RUN;
            break;
        case ARMY_RUN:
            if (info.ahead)
                head = info.angle;
            army->maximum_turn = ARMY_RUN_TURN;
            tilt = angle / 2;
            if ((item->ai_bits & GUARD)
                || ((item->ai_bits & FOLLOW)
                    && (army->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
                item->goal_anim_state = ARMY_WALK;
            else if (army->mood == ESCAPE_MOOD)
                break;
            else if (Targetable(item, &info))
                item->goal_anim_state = ARMY_WALK;
            else if (army->mood == BORED_MOOD
                     || (army->mood == STALK_MOOD && !(item->ai_bits & FOLLOW)
                         && info.distance < ARMY_RUN_RANGE))
                item->goal_anim_state = ARMY_WALK;
            break;
        case ARMY_AIM1:
        case ARMY_AIM3:
            army->flags = 0;
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
                if (Targetable(item, &info))
                    item->goal_anim_state = (item->current_anim_state == ARMY_AIM1) ? ARMY_SHOOT1 : ARMY_SHOOT3;
                else
                    item->goal_anim_state = ARMY_STOP;
            }
            break;
        case ARMY_AIM2:
            army->flags = 0;
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
                if (Targetable(item, &info))
                    item->goal_anim_state = ARMY_SHOOT2;
                else
                    item->goal_anim_state = ARMY_WALK;
            }
            break;
        case ARMY_SHOOT3:
            if (item->goal_anim_state != ARMY_STOP) {
                if (army->mood == ESCAPE_MOOD || info.distance > ARMY_SHOOT1_RANGE || !Targetable(item, &info))
                    item->goal_anim_state = ARMY_STOP;
            }
        case ARMY_SHOOT2:
        case ARMY_SHOOT1:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            if (!army->flags) {
                ShotLara(item, &info, &army_gun, torso_y, ARMY_SHOT_DAMAGE);
                army->flags = 5;
            } else
                army->flags--;
            break;
        }
    }
    CreatureTilt(item, tilt);
    CreatureJoint(item, 0, torso_y);
    CreatureJoint(item, 1, torso_x);
    CreatureJoint(item, 2, head);
    CreatureAnimation(item_number, angle, 0);
}

#ifdef TR3_EXE
BITE_INFO army_gun = { 0, 300, 64, 7 };
#endif
