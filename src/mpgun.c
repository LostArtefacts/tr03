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

#define MPGUN_WAIT 1
#define MPGUN_WALK 2
#define MPGUN_RUN 3
#define MPGUN_AIM1 4
#define MPGUN_SHOOT1 5
#define MPGUN_AIM2 6
#define MPGUN_SHOOT2 7
#define MPGUN_SHOOT3A 8
#define MPGUN_SHOOT3B 9
#define MPGUN_SHOOT4A 10
#define MPGUN_AIM3 11
#define MPGUN_AIM4 12
#define MPGUN_DEATH 13
#define MPGUN_SHOOT4B 14
#define MPGUN_DUCK 15
#define MPGUN_DUCKED 16
#define MPGUN_DUCKAIM 17
#define MPGUN_DUCKSHOT 18
#define MPGUN_DUCKWALK 19
#define MPGUN_STAND 20

#define SQUARE(x) ((x) * (x))
#define MPGUN_SHOT_DAMAGE 32
#define MPGUN_WALK_TURN (ONE_DEGREE * 6)
#define MPGUN_RUN_TURN (ONE_DEGREE * 10)
#define MPGUN_WALK_RANGE SQUARE(WALL_L * 3 / 2)
#define MPGUN_FEELER_DISTANCE WALL_L
#define MPGUN_DIE_ANIM 14
#define MPGUN_AIM1_ANIM 12
#define MPGUN_WT1_SHT1_ANIM 1
#define MPGUN_WLK_SHT4A_ANIM 18
#define MPGUN_WLK_SHT4B_ANIM 19
#define MPGUN_WALK_WAIT_ANIM 17
#define MPGUN_RUN_WAIT1_ANIM 27
#define MPGUN_RUN_WAIT2_ANIM 28
#define MPGUN_DEATH_SHOT_ANGLE 0x2000
#define MPGUN_AWARE_DISTANCE SQUARE(WALL_L)
#define MP2 61
#define BOB 62
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

extern BITE_INFO mpgun_gun;

int abs(int x);
#pragma intrinsic(abs)

void MPGunControl(sint16 item_number)
{
    ITEM_INFO *item, *target, *real_enemy;
    CREATURE_INFO *secgun, *cinfo;
    sint16 angle, torso_y, torso_x, head, tilt, meta_mood, slot, room_number, near_cover;
    sint32 lara_dx, lara_dz, x, z, distance, y, height;
    int random;
    AI_INFO info, lara_info;
    FLOOR_INFO *floor;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    secgun = (CREATURE_INFO *)item->data;
    torso_y = torso_x = head = angle = tilt = 0;
    if (item->fired_weapon) {
        PHD_VECTOR pos;

        phd_PushMatrix();
        pos.x = mpgun_gun.x;
        pos.y = mpgun_gun.y;
        pos.z = mpgun_gun.z;
        GetJointAbsPosition(item, &pos, mpgun_gun.mesh_num);
        TriggerDynamic(pos.x, pos.y, pos.z, (item->fired_weapon << 1) + 4, 24, 16, 4);
        phd_PopMatrix();
    }
    if (boxes[item->box_number].overlap_index & 0x4000) {
        DoLotsOfBloodD(item->pos.x_pos, item->pos.y_pos - (GetRandomControl() & 255) - 32, item->pos.z_pos,
                       (GetRandomControl() & 127) + 128, GetRandomControl() << 1, item->room_number, 3);
        item->hit_points -= 20;
    }
    if (item->hit_points <= 0) {
        item->hit_points = 0;
        if (item->current_anim_state != MPGUN_DEATH) {
            item->anim_number = objects[item->object_number].anim_index + MPGUN_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = MPGUN_DEATH;
        } else if (!(GetRandomControl() & 3) && item->frame_number == anims[item->anim_number].frame_base + 1) {
            CreatureAIInfo(item, &info);
            if (Targetable(item, &info)) {
                if (info.angle > -MPGUN_DEATH_SHOT_ANGLE && info.angle < MPGUN_DEATH_SHOT_ANGLE) {
                    torso_y = info.angle;
                    head = info.angle;
                    ShotLara(item, &info, &mpgun_gun, torso_y, MPGUN_SHOT_DAMAGE);
                    SoundEffect(72, &item->pos, 3 << 13);
                }
            }
        }
    } else {
        if (item->ai_bits)
            GetAITarget(secgun);
        else {
            secgun->enemy = lara_item;
            lara_dz = lara_item->pos.z_pos - item->pos.z_pos;
            lara_dx = lara_item->pos.x_pos - item->pos.x_pos;
            lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
            cinfo = baddie_slots;
            for (slot = 0; slot < NUM_SLOTS; slot++, cinfo++) {
                if (cinfo->item_num == NO_ITEM || cinfo->item_num == item_number)
                    continue;
                target = &items[cinfo->item_num];
                if (target->object_number != LARA && target->object_number != BOB)
                    continue;
                x = target->pos.x_pos - item->pos.x_pos;
                z = target->pos.z_pos - item->pos.z_pos;
                distance = x * x + z * z;
                if (distance < lara_info.distance)
                    secgun->enemy = target;
            }
        }
        CreatureAIInfo(item, &info);
        if (secgun->enemy == lara_item) {
            lara_info.angle = info.angle;
            lara_info.distance = info.distance;
        } else {
            lara_dz = lara_item->pos.z_pos - item->pos.z_pos;
            lara_dx = lara_item->pos.x_pos - item->pos.x_pos;
            lara_info.angle = phd_atan(lara_dz, lara_dx) - item->pos.y_rot;
            lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
        }
        if (secgun->enemy != lara_item)
            meta_mood = 1;
        else
            meta_mood = 0;
        GetCreatureMood(item, &info, meta_mood);
        CreatureMood(item, &info, meta_mood);
        angle = CreatureTurn(item, secgun->maximum_turn);
        room_number = item->room_number;
        x = item->pos.x_pos + (MPGUN_FEELER_DISTANCE * phd_sin(item->pos.y_rot + lara_info.angle) >> W2V_SHIFT);
        y = item->pos.y_pos;
        z = item->pos.z_pos + (MPGUN_FEELER_DISTANCE * phd_cos(item->pos.y_rot + lara_info.angle) >> W2V_SHIFT);
        floor = GetFloor(x, y, z, &room_number);
        height = GetHeight(floor, x, y, z);
        near_cover = (item->pos.y_pos > height + STEP_L * 3 / 2 && item->pos.y_pos < height + STEP_L * 9 / 2
                      && lara_info.distance > MPGUN_AWARE_DISTANCE);
        real_enemy = secgun->enemy;
        secgun->enemy = lara_item;
        if (lara_info.distance < MPGUN_AWARE_DISTANCE || item->hit_status || TargetVisible(item, &lara_info)) {
            if (!secgun->alerted)
                SoundEffect(300, &item->pos, 0);
            AlertAllGuards(item_number);
        }
        secgun->enemy = real_enemy;
        switch (item->current_anim_state) {
        case MPGUN_WAIT:
            head = lara_info.angle;
            secgun->maximum_turn = 0;
            if (item->anim_number == objects[item->object_number].anim_index + MPGUN_WALK_WAIT_ANIM
                || item->anim_number == objects[item->object_number].anim_index + MPGUN_RUN_WAIT1_ANIM
                || item->anim_number == objects[item->object_number].anim_index + MPGUN_RUN_WAIT2_ANIM) {
                if (abs(info.angle) < MPGUN_RUN_TURN)
                    item->pos.y_rot += info.angle;
                else if (info.angle < 0)
                    item->pos.y_rot -= MPGUN_RUN_TURN;
                else
                    item->pos.y_rot += MPGUN_RUN_TURN;
            }
            if (item->ai_bits & GUARD) {
                head = AIGuard(secgun);
                item->goal_anim_state = MPGUN_WAIT;
                break;
            } else if (item->ai_bits & PATROL1) {
                item->goal_anim_state = MPGUN_WALK;
                head = 0;
            } else if (near_cover && (lara.target == item || item->hit_status))
                item->goal_anim_state = MPGUN_DUCK;
            else if (item->required_anim_state == MPGUN_DUCK)
                item->goal_anim_state = MPGUN_DUCK;
            else if (secgun->mood == ESCAPE_MOOD)
                item->goal_anim_state = MPGUN_RUN;
            else if (Targetable(item, &info)) {
                random = GetRandomControl();
                if (random < 0x2000)
                    item->goal_anim_state = MPGUN_SHOOT1;
                else if (random < 0x4000)
                    item->goal_anim_state = MPGUN_SHOOT2;
                else
                    item->goal_anim_state = MPGUN_AIM3;
            } else if (secgun->mood == BORED_MOOD
                       || ((item->ai_bits & FOLLOW)
                           && (secgun->reached_goal || lara_info.distance > SQUARE(WALL_L * 2)))) {
                if (info.ahead)
                    item->goal_anim_state = MPGUN_WAIT;
                else
                    item->goal_anim_state = MPGUN_WALK;
            } else
                item->goal_anim_state = MPGUN_RUN;
            break;
        case MPGUN_WALK:
            head = lara_info.angle;
            secgun->maximum_turn = MPGUN_WALK_TURN;
            if (item->ai_bits & PATROL1) {
                item->goal_anim_state = MPGUN_WALK;
                head = 0;
            } else if (near_cover && (lara.target == item || item->hit_status)) {
                item->required_anim_state = MPGUN_DUCK;
                item->goal_anim_state = MPGUN_WAIT;
            } else if (secgun->mood == ESCAPE_MOOD)
                item->goal_anim_state = MPGUN_RUN;
            else if (Targetable(item, &info)) {
                if (info.distance > MPGUN_WALK_RANGE && info.zone_number == info.enemy_zone)
                    item->goal_anim_state = MPGUN_AIM4;
                else
                    item->goal_anim_state = MPGUN_WAIT;
            } else if (secgun->mood == BORED_MOOD) {
                if (info.ahead)
                    item->goal_anim_state = MPGUN_WALK;
                else
                    item->goal_anim_state = MPGUN_WAIT;
            } else
                item->goal_anim_state = MPGUN_RUN;
            break;
        case MPGUN_RUN:
            if (info.ahead)
                head = info.angle;
            secgun->maximum_turn = MPGUN_RUN_TURN;
            tilt = angle / 2;
            if (item->ai_bits & GUARD)
                item->goal_anim_state = MPGUN_WAIT;
            else if (near_cover && (lara.target == item || item->hit_status)) {
                item->required_anim_state = MPGUN_DUCK;
                item->goal_anim_state = MPGUN_WAIT;
            } else if (secgun->mood == ESCAPE_MOOD)
                break;
            else if (Targetable(item, &info)
                     || ((item->ai_bits & FOLLOW)
                         && (secgun->reached_goal || lara_info.distance > SQUARE(WALL_L * 2))))
                item->goal_anim_state = MPGUN_WAIT;
            else if (secgun->mood == BORED_MOOD)
                item->goal_anim_state = MPGUN_WALK;
            break;
        case MPGUN_AIM1:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            if (item->anim_number == objects[MP2].anim_index + MPGUN_AIM1_ANIM
                || (item->anim_number == objects[MP2].anim_index + MPGUN_WT1_SHT1_ANIM
                    && item->frame_number == anims[item->anim_number].frame_base + 10)) {
                if (!ShotLara(item, &info, &mpgun_gun, torso_y, MPGUN_SHOT_DAMAGE))
                    item->required_anim_state = MPGUN_WAIT;
            } else if (item->hit_status && !(GetRandomControl() & 3) && near_cover) {
                item->required_anim_state = MPGUN_DUCK;
                item->goal_anim_state = MPGUN_WAIT;
            }
            break;
        case MPGUN_SHOOT1:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            if (item->required_anim_state == MPGUN_WAIT)
                item->goal_anim_state = MPGUN_WAIT;
            break;
        case MPGUN_SHOOT2:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            if (item->frame_number == anims[item->anim_number].frame_base) {
                if (!ShotLara(item, &info, &mpgun_gun, torso_y, MPGUN_SHOT_DAMAGE))
                    item->goal_anim_state = MPGUN_WAIT;
            } else if (item->hit_status && !(GetRandomControl() & 3) && near_cover) {
                item->required_anim_state = MPGUN_DUCK;
                item->goal_anim_state = MPGUN_WAIT;
            }
            break;
        case MPGUN_SHOOT3A:
        case MPGUN_SHOOT3B:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            if (item->frame_number == anims[item->anim_number].frame_base
                || item->frame_number == anims[item->anim_number].frame_base + 11) {
                if (!ShotLara(item, &info, &mpgun_gun, torso_y, MPGUN_SHOT_DAMAGE))
                    item->goal_anim_state = MPGUN_WAIT;
            } else if (item->hit_status && !(GetRandomControl() & 3) && near_cover) {
                item->required_anim_state = MPGUN_DUCK;
                item->goal_anim_state = MPGUN_WAIT;
            }
            break;
        case MPGUN_AIM4:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            if ((item->anim_number == objects[MP2].anim_index + MPGUN_WLK_SHT4A_ANIM
                 && item->frame_number == anims[item->anim_number].frame_base + 17)
                || (item->anim_number == objects[MP2].anim_index + MPGUN_WLK_SHT4B_ANIM
                    && item->frame_number == anims[item->anim_number].frame_base + 6)) {
                if (!ShotLara(item, &info, &mpgun_gun, torso_y, MPGUN_SHOT_DAMAGE))
                    item->required_anim_state = MPGUN_WALK;
            } else if (item->hit_status && !(GetRandomControl() & 3) && near_cover) {
                item->required_anim_state = MPGUN_DUCK;
                item->goal_anim_state = MPGUN_WAIT;
            }
            if (info.distance < MPGUN_WALK_RANGE)
                item->required_anim_state = MPGUN_WALK;
            break;
        case MPGUN_SHOOT4A:
        case MPGUN_SHOOT4B:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            if (item->required_anim_state == MPGUN_WALK)
                item->goal_anim_state = MPGUN_WALK;
            if (item->frame_number == anims[item->anim_number].frame_base + 16) {
                if (!ShotLara(item, &info, &mpgun_gun, torso_y, MPGUN_SHOT_DAMAGE))
                    item->goal_anim_state = MPGUN_WALK;
            }
            if (info.distance < MPGUN_WALK_RANGE)
                item->goal_anim_state = MPGUN_WALK;
            break;
        case MPGUN_DUCKED:
            if (info.ahead)
                head = info.angle;
            secgun->maximum_turn = 0;
            if (Targetable(item, &info))
                item->goal_anim_state = MPGUN_DUCKAIM;
            else if (item->hit_status || !near_cover || (info.ahead && !(GetRandomControl() & 0x1f)))
                item->goal_anim_state = MPGUN_STAND;
            else
                item->goal_anim_state = MPGUN_DUCKWALK;
            break;
        case MPGUN_DUCKAIM:
            secgun->maximum_turn = ONE_DEGREE;
            if (info.ahead)
                torso_y = info.angle;
            if (Targetable(item, &info))
                item->goal_anim_state = MPGUN_DUCKSHOT;
            else
                item->goal_anim_state = MPGUN_DUCKED;
            break;
        case MPGUN_DUCKSHOT:
            if (info.ahead)
                torso_y = info.angle;
            if (item->frame_number == anims[item->anim_number].frame_base) {
                if (!ShotLara(item, &info, &mpgun_gun, torso_y, MPGUN_SHOT_DAMAGE) || !(GetRandomControl() & 7))
                    item->goal_anim_state = MPGUN_DUCKED;
            }
            break;
        case MPGUN_DUCKWALK:
            if (info.ahead)
                head = info.angle;
            secgun->maximum_turn = MPGUN_WALK_TURN;
            if (Targetable(item, &info) || item->hit_status || !near_cover
                || (info.ahead && !(GetRandomControl() & 0x1f)))
                item->goal_anim_state = MPGUN_DUCKED;
            break;
        }
    }
    CreatureTilt(item, tilt);
    CreatureJoint(item, 0, torso_y);
    CreatureJoint(item, 1, torso_x);
    CreatureJoint(item, 2, head);
    CreatureAnimation(item_number, angle, 0);
}
