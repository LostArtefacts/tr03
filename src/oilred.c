#include "game.h"

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

#define OILRED_WAIT 1
#define OILRED_WALK 2
#define OILRED_RUN 3
#define OILRED_AIM1 4
#define OILRED_SHOOT1 5
#define OILRED_AIM2 6
#define OILRED_SHOOT2 7
#define OILRED_SHOOT3A 8
#define OILRED_SHOOT3B 9
#define OILRED_SHOOT4A 10
#define OILRED_AIM3 11
#define OILRED_AIM4 12
#define OILRED_DEATH 13
#define OILRED_SHOOT4B 14
#define OILRED_DUCK 15
#define OILRED_DUCKED 16
#define OILRED_DUCKAIM 17
#define OILRED_DUCKSHOT 18
#define OILRED_DUCKWALK 19
#define OILRED_STAND 20

#define SQUARE(x) ((x) * (x))
#define OILRED_SHOT_DAMAGE 35
#define OILRED_WALK_TURN (ONE_DEGREE * 6)
#define OILRED_RUN_TURN (ONE_DEGREE * 10)
#define OILRED_WALK_RANGE SQUARE(WALL_L * 2)
#define OILRED_FEELER_DISTANCE WALL_L
#define OILRED_DIE_ANIM 14
#define OILRED_AIM1_ANIM 12
#define OILRED_WT1_SHT1_ANIM 1
#define OILRED_WLK_SHT4A_ANIM 18
#define OILRED_WLK_SHT4B_ANIM 19
#define OILRED_WALK_WAIT_ANIM 17
#define OILRED_RUN_WAIT1_ANIM 27
#define OILRED_RUN_WAIT2_ANIM 28
#define OILRED_DEATH_SHOT_ANGLE 0x2000
#define OILRED_AWARE_DISTANCE SQUARE(WALL_L)
#define OILRED 39
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

extern BITE_INFO oilred_gun;

int abs(int x);
#pragma intrinsic(abs)

void OilRedControl(sint16 item_number)
{
    ITEM_INFO *item, *real_enemy;
    CREATURE_INFO *sec;
    sint16 angle, torso_y, torso_x, head, tilt, room_number, near_cover, meta_mood;
    sint32 lara_dx, lara_dz, height, x, y, z;
    int random;
    AI_INFO info, lara_info;
    FLOOR_INFO *floor;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    sec = (CREATURE_INFO *)item->data;
    torso_y = torso_x = head = angle = tilt = 0;
    if (item->fired_weapon) {
        PHD_VECTOR pos;

        phd_PushMatrix();
        pos.x = oilred_gun.x;
        pos.y = oilred_gun.y;
        pos.z = oilred_gun.z;
        GetJointAbsPosition(item, &pos, oilred_gun.mesh_num);
        TriggerDynamic(pos.x, pos.y, pos.z, (item->fired_weapon << 1) + 4, 24, 16, 4);
        phd_PopMatrix();
    }
    if (item->hit_points <= 0) {
        item->hit_points = 0;
        if (item->current_anim_state != OILRED_DEATH) {
            item->anim_number = objects[OILRED].anim_index + OILRED_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = OILRED_DEATH;
        } else if (item->frame_number == anims[item->anim_number].frame_base + 47) {
            CreatureAIInfo(item, &info);
            if (Targetable(item, &info)) {
                if (info.angle > -OILRED_DEATH_SHOT_ANGLE && info.angle < OILRED_DEATH_SHOT_ANGLE) {
                    torso_y = info.angle;
                    head = info.angle;
                    ShotLara(item, &info, &oilred_gun, torso_y, OILRED_SHOT_DAMAGE * 3);
                    SoundEffect(72, &item->pos, 3 << 13);
                }
            }
        }
    } else {
        if (item->ai_bits)
            GetAITarget(sec);
        else
            sec->enemy = lara_item;
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
        if (sec->enemy != lara_item)
            meta_mood = 1;
        else
            meta_mood = 0;
        GetCreatureMood(item, &info, meta_mood);
        CreatureMood(item, &info, meta_mood);
        angle = CreatureTurn(item, sec->maximum_turn);
        room_number = item->room_number;
        x = item->pos.x_pos + (OILRED_FEELER_DISTANCE * phd_sin(item->pos.y_rot + lara_info.angle) >> W2V_SHIFT);
        y = item->pos.y_pos;
        z = item->pos.z_pos + (OILRED_FEELER_DISTANCE * phd_cos(item->pos.y_rot + lara_info.angle) >> W2V_SHIFT);
        floor = GetFloor(x, y, z, &room_number);
        height = GetHeight(floor, x, y, z);
        near_cover = (item->pos.y_pos > height + STEP_L * 3 / 2 && item->pos.y_pos < height + STEP_L * 9 / 2
                      && lara_info.distance > OILRED_AWARE_DISTANCE);
        real_enemy = sec->enemy;
        sec->enemy = lara_item;
        if ((lara_info.distance < OILRED_AWARE_DISTANCE || item->hit_status || TargetVisible(item, &lara_info))
            && !(item->ai_bits & FOLLOW)) {
            if (!sec->alerted)
                SoundEffect(300, &item->pos, 0);
            AlertAllGuards(item_number);
        }
        sec->enemy = real_enemy;
        switch (item->current_anim_state) {
        case OILRED_WAIT:
            head = lara_info.angle;
            if (item->anim_number == objects[OILRED].anim_index + OILRED_WALK_WAIT_ANIM
                || item->anim_number == objects[OILRED].anim_index + OILRED_RUN_WAIT1_ANIM
                || item->anim_number == objects[OILRED].anim_index + OILRED_RUN_WAIT2_ANIM) {
                if (abs(info.angle) < OILRED_RUN_TURN)
                    item->pos.y_rot += info.angle;
                else if (info.angle < 0)
                    item->pos.y_rot -= OILRED_RUN_TURN;
                else
                    item->pos.y_rot += OILRED_RUN_TURN;
            }
            sec->maximum_turn = 0;
            if (item->ai_bits & GUARD) {
                head = AIGuard(sec);
                item->goal_anim_state = OILRED_WAIT;
                break;
            } else if (item->ai_bits & PATROL1)
                item->goal_anim_state = OILRED_WALK;
            else if (near_cover && (lara.target == item || item->hit_status))
                item->goal_anim_state = OILRED_DUCK;
            else if (item->required_anim_state == OILRED_DUCK)
                item->goal_anim_state = OILRED_DUCK;
            else if (sec->mood == ESCAPE_MOOD)
                item->goal_anim_state = OILRED_RUN;
            else if (Targetable(item, &info)) {
                if (info.distance > OILRED_WALK_RANGE)
                    item->goal_anim_state = OILRED_WALK;
                else {
                    random = GetRandomControl();
                    if (random < 0x2000)
                        item->goal_anim_state = OILRED_SHOOT1;
                    else if (random < 0x4000)
                        item->goal_anim_state = OILRED_SHOOT2;
                    else
                        item->goal_anim_state = OILRED_AIM3;
                }
            } else if (sec->mood == BORED_MOOD
                       || ((item->ai_bits & FOLLOW)
                           && (sec->reached_goal || lara_info.distance > SQUARE(WALL_L * 2)))) {
                if (info.ahead)
                    item->goal_anim_state = OILRED_WAIT;
                else
                    item->goal_anim_state = OILRED_WALK;
            } else
                item->goal_anim_state = OILRED_RUN;
            break;
        case OILRED_WALK:
            head = lara_info.angle;
            sec->maximum_turn = OILRED_WALK_TURN;
            if (item->ai_bits & PATROL1) {
                item->goal_anim_state = OILRED_WALK;
                head = 0;
            } else if (near_cover && (lara.target == item || item->hit_status)) {
                item->required_anim_state = OILRED_DUCK;
                item->goal_anim_state = OILRED_WAIT;
            } else if (sec->mood == ESCAPE_MOOD)
                item->goal_anim_state = OILRED_RUN;
            else if (Targetable(item, &info)) {
                if (info.distance > OILRED_WALK_RANGE && info.zone_number == info.enemy_zone)
                    item->goal_anim_state = OILRED_AIM4;
                else
                    item->goal_anim_state = OILRED_WAIT;
            } else if (sec->mood == BORED_MOOD) {
                if (info.ahead)
                    item->goal_anim_state = OILRED_WALK;
                else
                    item->goal_anim_state = OILRED_WAIT;
            } else
                item->goal_anim_state = OILRED_RUN;
            break;
        case OILRED_RUN:
            if (info.ahead)
                head = info.angle;
            sec->maximum_turn = OILRED_RUN_TURN;
            tilt = angle / 2;
            if (item->ai_bits & GUARD)
                item->goal_anim_state = OILRED_WAIT;
            else if (near_cover && (lara.target == item || item->hit_status)) {
                item->required_anim_state = OILRED_DUCK;
                item->goal_anim_state = OILRED_WAIT;
            } else if (sec->mood == ESCAPE_MOOD)
                break;
            else if (Targetable(item, &info)
                     || ((item->ai_bits & FOLLOW)
                         && (sec->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
                item->goal_anim_state = OILRED_WAIT;
            else if (sec->mood == BORED_MOOD)
                item->goal_anim_state = OILRED_WALK;
            break;
        case OILRED_AIM1:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            if (item->anim_number == objects[OILRED].anim_index + OILRED_AIM1_ANIM
                || (item->anim_number == objects[OILRED].anim_index + OILRED_WT1_SHT1_ANIM
                    && item->frame_number == anims[item->anim_number].frame_base + 10)) {
                if (!ShotLara(item, &info, &oilred_gun, torso_y, OILRED_SHOT_DAMAGE))
                    item->required_anim_state = OILRED_WAIT;
            } else if (item->hit_status && !(GetRandomControl() & 3) && near_cover) {
                item->required_anim_state = OILRED_DUCK;
                item->goal_anim_state = OILRED_WAIT;
            }
            break;
        case OILRED_SHOOT1:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            if (item->required_anim_state == OILRED_WAIT)
                item->goal_anim_state = OILRED_WAIT;
            break;
        case OILRED_SHOOT2:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            if (item->frame_number == anims[item->anim_number].frame_base) {
                if (!ShotLara(item, &info, &oilred_gun, torso_y, OILRED_SHOT_DAMAGE))
                    item->goal_anim_state = OILRED_WAIT;
            } else if (item->hit_status && !(GetRandomControl() & 3) && near_cover) {
                item->required_anim_state = OILRED_DUCK;
                item->goal_anim_state = OILRED_WAIT;
            }
            break;
        case OILRED_SHOOT3A:
        case OILRED_SHOOT3B:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            if (item->frame_number == anims[item->anim_number].frame_base
                || item->frame_number == anims[item->anim_number].frame_base + 11) {
                if (!ShotLara(item, &info, &oilred_gun, torso_y, OILRED_SHOT_DAMAGE))
                    item->goal_anim_state = OILRED_WAIT;
            } else if (item->hit_status && !(GetRandomControl() & 3) && near_cover) {
                item->required_anim_state = OILRED_DUCK;
                item->goal_anim_state = OILRED_WAIT;
            }
            break;
        case OILRED_AIM4:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            if ((item->anim_number == objects[OILRED].anim_index + OILRED_WLK_SHT4A_ANIM
                 && item->frame_number == anims[item->anim_number].frame_base + 17)
                || (item->anim_number == objects[OILRED].anim_index + OILRED_WLK_SHT4B_ANIM
                    && item->frame_number == anims[item->anim_number].frame_base + 6)) {
                if (!ShotLara(item, &info, &oilred_gun, torso_y, OILRED_SHOT_DAMAGE))
                    item->required_anim_state = OILRED_WALK;
            } else if (item->hit_status && !(GetRandomControl() & 3) && near_cover) {
                item->required_anim_state = OILRED_DUCK;
                item->goal_anim_state = OILRED_WAIT;
            }
            if (info.distance < OILRED_WALK_RANGE)
                item->required_anim_state = OILRED_WALK;
            break;
        case OILRED_SHOOT4A:
        case OILRED_SHOOT4B:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            if (item->required_anim_state == OILRED_WALK)
                item->goal_anim_state = OILRED_WALK;
            if (item->frame_number == anims[item->anim_number].frame_base + 16) {
                if (!ShotLara(item, &info, &oilred_gun, torso_y, OILRED_SHOT_DAMAGE))
                    item->goal_anim_state = OILRED_WALK;
            }
            if (info.distance < OILRED_WALK_RANGE)
                item->goal_anim_state = OILRED_WALK;
            break;
        case OILRED_DUCKED:
            if (info.ahead)
                head = info.angle;
            sec->maximum_turn = 0;
            if (Targetable(item, &info))
                item->goal_anim_state = OILRED_DUCKAIM;
            else if (item->hit_status || !near_cover || (info.ahead && !(GetRandomControl() & 0x1f)))
                item->goal_anim_state = OILRED_STAND;
            else
                item->goal_anim_state = OILRED_DUCKWALK;
            break;
        case OILRED_DUCKAIM:
            sec->maximum_turn = ONE_DEGREE;
            if (info.ahead)
                torso_y = info.angle;
            if (Targetable(item, &info))
                item->goal_anim_state = OILRED_DUCKSHOT;
            else
                item->goal_anim_state = OILRED_DUCKED;
            break;
        case OILRED_DUCKSHOT:
            if (info.ahead)
                torso_y = info.angle;
            if (item->frame_number == anims[item->anim_number].frame_base) {
                if (!ShotLara(item, &info, &oilred_gun, torso_y, OILRED_SHOT_DAMAGE) || !(GetRandomControl() & 7))
                    item->goal_anim_state = OILRED_DUCKED;
            }
            break;
        case OILRED_DUCKWALK:
            if (info.ahead)
                head = info.angle;
            sec->maximum_turn = OILRED_WALK_TURN;
            if (Targetable(item, &info) || item->hit_status || !near_cover
                || (info.ahead && !(GetRandomControl() & 0x1f)))
                item->goal_anim_state = OILRED_DUCKED;
            break;
        case OILRED_STAND:
            if (abs(info.angle) < OILRED_WALK_TURN)
                item->pos.y_rot += info.angle;
            else if (info.angle < 0)
                item->pos.y_rot -= OILRED_WALK_TURN;
            else
                item->pos.y_rot += OILRED_WALK_TURN;
        }
    }
    if (torso_y > 0x2000)
        torso_y = 0x2000;
    else if (torso_y < -0x2000)
        torso_y = -0x2000;
    CreatureTilt(item, tilt);
    CreatureJoint(item, 0, torso_y);
    CreatureJoint(item, 1, torso_x);
    CreatureJoint(item, 2, head);
    CreatureAnimation(item_number, angle, 0);
}
