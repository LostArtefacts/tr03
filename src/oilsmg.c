#include "game.h"

#define OILSMG_STOP_ANIM 12
#define OILSMG_STOP 1

void InitialiseOilSMG(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    InitialiseCreature(item_number);
    item->anim_number = objects[WHITE_SOLDIER].anim_index + OILSMG_STOP_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = OILSMG_STOP;
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

#define OILSMG_WALK 2
#define OILSMG_RUN 3
#define OILSMG_WAIT 4
#define OILSMG_SHOOT1 5
#define OILSMG_SHOOT2 6
#define OILSMG_DEATH 7
#define OILSMG_AIM1 8
#define OILSMG_AIM2 9
#define OILSMG_AIM3 10
#define OILSMG_SHOOT3 11

#define SQUARE(x) ((x) * (x))
#define OILSMG_SHOT_DAMAGE 28
#define OILSMG_WALK_TURN (ONE_DEGREE * 5)
#define OILSMG_RUN_TURN (ONE_DEGREE * 10)
#define OILSMG_RUN_RANGE SQUARE(WALL_L * 2)
#define OILSMG_SHOOT1_RANGE SQUARE(WALL_L * 3)
#define OILSMG_AWARE_DISTANCE SQUARE(WALL_L)
#define OILSMG_DIE_ANIM 19
#define OILSMG_WALK_STOP_ANIM 17
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

extern BITE_INFO oilsmg_gun;

int abs(int x);
#pragma intrinsic(abs)

void OilSMGControl(sint16 item_number)
{
    ITEM_INFO *item, *real_enemy;
    CREATURE_INFO *oilsmg;
    sint16 angle, torso_y, torso_x, head, tilt, meta_mood;
    sint32 lara_dx, lara_dz;
    AI_INFO info, lara_info;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    oilsmg = (CREATURE_INFO *)item->data;
    torso_y = torso_x = head = angle = tilt = 0;
    if (item->fired_weapon) {
        PHD_VECTOR pos;

        phd_PushMatrix();
        pos.x = oilsmg_gun.x;
        pos.y = oilsmg_gun.y;
        pos.z = oilsmg_gun.z;
        GetJointAbsPosition(item, &pos, oilsmg_gun.mesh_num);
        TriggerDynamic(pos.x, pos.y, pos.z, (item->fired_weapon << 1) + 8, 24, 16, 4);
        phd_PopMatrix();
    }
    if (item->hit_points <= 0) {
        if (item->current_anim_state != OILSMG_DEATH) {
            item->anim_number = objects[item->object_number].anim_index + OILSMG_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = OILSMG_DEATH;
            if (!(GetRandomControl() & 3))
                oilsmg->flags = 1;
            else
                oilsmg->flags = 0;
        } else if (oilsmg->flags
                   && (item->frame_number > anims[item->anim_number].frame_base + 3
                       && item->frame_number < anims[item->anim_number].frame_base + 31)
                   && !(item->frame_number & 3)) {
            CreatureAIInfo(item, &info);
            torso_y = info.angle;
            head = info.angle;
            ShotLara(item, &info, &oilsmg_gun, 0, 0);
            SoundEffect(72, &item->pos, 3 << 13);
        }
    } else {
        if (item->ai_bits)
            GetAITarget(oilsmg);
        else
            oilsmg->enemy = lara_item;
        CreatureAIInfo(item, &info);
        if (oilsmg->enemy == lara_item) {
            lara_info.angle = info.angle;
            lara_info.distance = info.distance;
        } else {
            lara_dz = lara_item->pos.z_pos - item->pos.z_pos;
            lara_dx = lara_item->pos.x_pos - item->pos.x_pos;
            lara_info.angle = phd_atan(lara_dz, lara_dx) - item->pos.y_rot;
            lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
        }
        if (oilsmg->enemy != lara_item)
            meta_mood = 1;
        else
            meta_mood = 0;
        GetCreatureMood(item, &info, meta_mood);
        CreatureMood(item, &info, meta_mood);
        angle = CreatureTurn(item, oilsmg->maximum_turn);
        real_enemy = oilsmg->enemy;
        oilsmg->enemy = lara_item;
        if ((lara_info.distance < OILSMG_AWARE_DISTANCE || item->hit_status || TargetVisible(item, &lara_info))
            && !(item->ai_bits & FOLLOW)) {
            if (!oilsmg->alerted)
                SoundEffect(300, &item->pos, 0);
            AlertAllGuards(item_number);
        }
        oilsmg->enemy = real_enemy;
        switch (item->current_anim_state) {
        case OILSMG_STOP:
            head = lara_info.angle;
            oilsmg->flags = 0;
            oilsmg->maximum_turn = 0;
            if (item->anim_number == objects[item->object_number].anim_index + OILSMG_WALK_STOP_ANIM) {
                if (abs(info.angle) < OILSMG_RUN_TURN)
                    item->pos.y_rot += info.angle;
                else if (info.angle < 0)
                    item->pos.y_rot -= OILSMG_RUN_TURN;
                else
                    item->pos.y_rot += OILSMG_RUN_TURN;
            }
            if (item->ai_bits & GUARD) {
                head = AIGuard(oilsmg);
                if (!(GetRandomControl() & 0xff)) {
                    if (item->current_anim_state == OILSMG_STOP)
                        item->goal_anim_state = OILSMG_WAIT;
                    else
                        item->goal_anim_state = OILSMG_STOP;
                }
                break;
            } else if (item->ai_bits & PATROL1)
                item->goal_anim_state = OILSMG_WALK;
            else if (oilsmg->mood == ESCAPE_MOOD)
                item->goal_anim_state = OILSMG_RUN;
            else if (Targetable(item, &info)) {
                if (info.distance < OILSMG_SHOOT1_RANGE || info.zone_number != info.enemy_zone) {
                    if (GetRandomControl() < 0x4000)
                        item->goal_anim_state = OILSMG_AIM1;
                    else
                        item->goal_anim_state = OILSMG_AIM3;
                } else
                    item->goal_anim_state = OILSMG_WALK;
            } else if (oilsmg->mood == BORED_MOOD
                       || ((item->ai_bits & FOLLOW)
                           && (oilsmg->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
                item->goal_anim_state = OILSMG_STOP;
            else if (oilsmg->mood != BORED_MOOD && info.distance > OILSMG_RUN_RANGE)
                item->goal_anim_state = OILSMG_RUN;
            else
                item->goal_anim_state = OILSMG_WALK;
            break;
        case OILSMG_WAIT:
            head = lara_info.angle;
            oilsmg->flags = 0;
            oilsmg->maximum_turn = 0;
            if (item->ai_bits & GUARD) {
                head = AIGuard(oilsmg);
                if (!(GetRandomControl() & 0xff)) {
                    if (item->current_anim_state == OILSMG_STOP)
                        item->goal_anim_state = OILSMG_WAIT;
                    else
                        item->goal_anim_state = OILSMG_STOP;
                }
                break;
            } else if (Targetable(item, &info))
                item->goal_anim_state = OILSMG_SHOOT1;
            else if (oilsmg->mood != BORED_MOOD || !info.ahead)
                item->goal_anim_state = OILSMG_STOP;
            break;
        case OILSMG_WALK:
            head = lara_info.angle;
            oilsmg->flags = 0;
            oilsmg->maximum_turn = OILSMG_WALK_TURN;
            if (item->ai_bits & PATROL1) {
                item->goal_anim_state = OILSMG_WALK;
                head = 0;
            } else if (oilsmg->mood == ESCAPE_MOOD)
                item->goal_anim_state = OILSMG_RUN;
            else if ((item->ai_bits & GUARD)
                     || ((item->ai_bits & FOLLOW)
                         && (oilsmg->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
                item->goal_anim_state = OILSMG_STOP;
            else if (Targetable(item, &info)) {
                if (info.distance < OILSMG_SHOOT1_RANGE || info.zone_number != info.enemy_zone)
                    item->goal_anim_state = OILSMG_STOP;
                else
                    item->goal_anim_state = OILSMG_AIM2;
            } else if (oilsmg->mood == BORED_MOOD && info.ahead)
                item->goal_anim_state = OILSMG_STOP;
            else if (oilsmg->mood != BORED_MOOD && info.distance > OILSMG_RUN_RANGE)
                item->goal_anim_state = OILSMG_RUN;
            break;
        case OILSMG_RUN:
            if (info.ahead)
                head = info.angle;
            oilsmg->maximum_turn = OILSMG_RUN_TURN;
            tilt = angle / 2;
            if ((item->ai_bits & GUARD)
                || ((item->ai_bits & FOLLOW)
                    && (oilsmg->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
                item->goal_anim_state = OILSMG_WALK;
            else if (oilsmg->mood == ESCAPE_MOOD)
                break;
            else if (Targetable(item, &info))
                item->goal_anim_state = OILSMG_WALK;
            else if (oilsmg->mood == BORED_MOOD
                     || (oilsmg->mood == STALK_MOOD && !(item->ai_bits & FOLLOW)
                         && info.distance < OILSMG_RUN_RANGE))
                item->goal_anim_state = OILSMG_WALK;
            break;
        case OILSMG_AIM1:
        case OILSMG_AIM3:
            oilsmg->flags = 0;
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
                if (Targetable(item, &info))
                    item->goal_anim_state = (item->current_anim_state == OILSMG_AIM1) ? OILSMG_SHOOT1 : OILSMG_SHOOT3;
                else
                    item->goal_anim_state = OILSMG_STOP;
            }
            break;
        case OILSMG_AIM2:
            oilsmg->flags = 0;
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
                if (Targetable(item, &info))
                    item->goal_anim_state = OILSMG_SHOOT2;
                else
                    item->goal_anim_state = OILSMG_WALK;
            }
            break;
        case OILSMG_SHOOT3:
            if (item->goal_anim_state != OILSMG_STOP) {
                if (oilsmg->mood == ESCAPE_MOOD || info.distance > OILSMG_SHOOT1_RANGE || !Targetable(item, &info))
                    item->goal_anim_state = OILSMG_STOP;
            }
        case OILSMG_SHOOT2:
        case OILSMG_SHOOT1:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            if (!oilsmg->flags) {
                ShotLara(item, &info, &oilsmg_gun, torso_y, OILSMG_SHOT_DAMAGE);
                oilsmg->flags = 5;
            } else
                oilsmg->flags--;
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
BITE_INFO oilsmg_gun = { 0, 400, 64, 7 };
#endif
