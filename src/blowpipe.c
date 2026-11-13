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

#define BLOW_WAIT1 1
#define BLOW_WALK 2
#define BLOW_RUN 3
#define BLOW_ATTACK1 4
#define BLOW_ATTACK3 6
#define BLOW_AIM3 8
#define BLOW_DEATH 9
#define BLOW_WAIT2 11

#define SQUARE(x) ((x) * (x))
#define BLOW_BIFF_DAMAGE 100
#define BLOW_BIFF_ENEMY_DAMAGE 5
#define BLOW_WALK_TURN (ONE_DEGREE * 9)
#define BLOW_RUN_TURN (ONE_DEGREE * 6)
#define BLOW_WAIT_TURN (ONE_DEGREE * 2)
#define BLOW_PIPE_RANGE SQUARE(WALL_L * 8)
#define BLOW_CLOSE_RANGE SQUARE(WALL_L / 2)
#define BLOW_WALK_RANGE SQUARE(WALL_L * 2)
#define BLOW_AWARE_DISTANCE WALL_L
#define BLOW_HIT_RANGE (STEP_L * 2)
#define BLOW_SHIFT_CHANCE 0x200
#define BLOW_TOUCH 0x2400
#define BLOW_KNEELING_DIE_ANIM 21
#define BLOW_STANDING_DIE_ANIM 20
#define GUARD 1

int CreatureActive(sint16 item_number);
void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info);
void GetCreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
void CreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn);
int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);
void CreatureJoint(ITEM_INFO *item, sint16 joint, sint16 required);
sint16 CreatureEffect(ITEM_INFO *item, BITE_INFO *bite, sint16 (*generate)(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 yrot, sint16 room_number));
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);
void GetAITarget(CREATURE_INFO *creature);
void AlertAllGuards(sint16 item_number);
sint16 AIGuard(CREATURE_INFO *creature);
int TargetVisible(ITEM_INFO *item, AI_INFO *info);
int Targetable(ITEM_INFO *item, AI_INFO *info);
sint16 CreateItem(void);
void InitialiseItem(sint16 item_num);
void phd_GetVectorAngles(sint32 x, sint32 y, sint32 z, PHD_ANGLE *dest);
void TriggerDartSmoke(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 zv, sint32 hit);

extern BITE_INFO blow_biff_hit;
extern BITE_INFO blow_shoot_hit;

int abs(int x);
#pragma intrinsic(abs)

void BlowpipeControl(sint16 item_number)
{
    ITEM_INFO *item, *enemy;
    CREATURE_INFO *blow;
    sint16 angle, head_x, head_y, torso_x, torso_y, tilt, meta_mood;
    AI_INFO info;
    ROOM_INFO *r;

    r = &room[lara_item->room_number];
    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    blow = (CREATURE_INFO *)item->data;
    head_x = head_y = torso_x = torso_y = angle = tilt = 0;
    if (item->hit_points <= 0) {
        if (item->current_anim_state != BLOW_DEATH) {
            if (item->current_anim_state == BLOW_WAIT1 || item->current_anim_state == BLOW_ATTACK1)
                item->anim_number = objects[item->object_number].anim_index + BLOW_KNEELING_DIE_ANIM;
            else
                item->anim_number = objects[item->object_number].anim_index + BLOW_STANDING_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = BLOW_DEATH;
        }
    } else {
        if (item->ai_bits)
            GetAITarget(blow);
        CreatureAIInfo(item, &info);
        if (info.zone_number == info.enemy_zone)
            meta_mood = 1;
        else
            meta_mood = 0;
        GetCreatureMood(item, &info, meta_mood);
        if (item->hit_status && lara.poisoned >= 0x100 && blow->mood == BORED_MOOD)
            blow->mood = ESCAPE_MOOD;
        CreatureMood(item, &info, 0);
        angle = CreatureTurn(item, blow->mood == BORED_MOOD ? BLOW_WAIT_TURN : blow->maximum_turn);
        if (info.ahead) {
            head_y = info.angle >> 1;
            torso_y = info.angle >> 1;
        }
        if (item->hit_status
            || (blow->enemy == lara_item && (info.distance < BLOW_AWARE_DISTANCE || TargetVisible(item, &info))
                && abs(lara_item->pos.y_pos - item->pos.y_pos) < WALL_L * 2))
            AlertAllGuards(item_number);
        switch (item->current_anim_state) {
        case BLOW_WAIT1:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle >> 1;
            }
            blow->flags &= 0x0fff;
            blow->maximum_turn = BLOW_WAIT_TURN;
            if (item->ai_bits & GUARD) {
                head_y = AIGuard(blow);
                torso_y = 0;
                torso_x = 0;
                blow->maximum_turn = 0;
                if (!(GetRandomControl() & 0xff))
                    item->goal_anim_state = BLOW_WAIT2;
                break;
            } else if (blow->mood == ESCAPE_MOOD) {
                if (lara.target != item && info.ahead && !item->hit_status)
                    item->goal_anim_state = BLOW_WAIT1;
                else
                    item->goal_anim_state = BLOW_RUN;
            } else if (info.bite && info.distance < BLOW_CLOSE_RANGE)
                item->goal_anim_state = BLOW_WAIT2;
            else if (info.bite && info.distance < BLOW_WALK_RANGE)
                item->goal_anim_state = BLOW_WALK;
            else if (Targetable(item, &info) && info.distance < BLOW_PIPE_RANGE)
                item->goal_anim_state = BLOW_ATTACK1;
            else if (blow->mood == BORED_MOOD) {
                if (GetRandomControl() < BLOW_SHIFT_CHANCE)
                    item->goal_anim_state = BLOW_WALK;
                else
                    break;
            } else
                item->goal_anim_state = BLOW_RUN;
            break;
        case BLOW_WAIT2:
            blow->flags &= 0x0fff;
            blow->maximum_turn = BLOW_WAIT_TURN;
            if (item->ai_bits & GUARD) {
                head_y = AIGuard(blow);
                torso_y = 0;
                torso_x = 0;
                blow->maximum_turn = 0;
                if (!(GetRandomControl() & 0xff))
                    item->goal_anim_state = BLOW_WAIT1;
                break;
            } else if (blow->mood == ESCAPE_MOOD) {
                if (lara.target != item && info.ahead && !item->hit_status)
                    item->goal_anim_state = BLOW_WAIT1;
                else
                    item->goal_anim_state = BLOW_RUN;
            } else if (info.bite && info.distance < BLOW_CLOSE_RANGE)
                item->goal_anim_state = BLOW_ATTACK3;
            else if (info.bite && info.distance < BLOW_WALK_RANGE)
                item->goal_anim_state = BLOW_WALK;
            else if (Targetable(item, &info) && info.distance < BLOW_PIPE_RANGE)
                item->goal_anim_state = BLOW_WAIT1;
            else if (blow->mood == BORED_MOOD && GetRandomControl() < BLOW_SHIFT_CHANCE)
                item->goal_anim_state = BLOW_WALK;
            else
                item->goal_anim_state = BLOW_RUN;
            break;
        case BLOW_WALK:
            blow->maximum_turn = BLOW_WALK_TURN;
            if (info.bite && info.distance < BLOW_CLOSE_RANGE)
                item->goal_anim_state = BLOW_WAIT2;
            else if (info.bite && info.distance < BLOW_WALK_RANGE)
                item->goal_anim_state = BLOW_WALK;
            else if (Targetable(item, &info) && info.distance < BLOW_PIPE_RANGE)
                item->goal_anim_state = BLOW_WAIT1;
            else if (blow->mood == ESCAPE_MOOD)
                item->goal_anim_state = BLOW_RUN;
            else if (blow->mood == BORED_MOOD) {
                if (GetRandomControl() > BLOW_SHIFT_CHANCE)
                    item->goal_anim_state = BLOW_WALK;
                else if (GetRandomControl() > BLOW_SHIFT_CHANCE)
                    item->goal_anim_state = BLOW_WAIT2;
                else
                    item->goal_anim_state = BLOW_WAIT1;
            } else if (info.distance > BLOW_WALK_RANGE)
                item->goal_anim_state = BLOW_RUN;
            break;
        case BLOW_RUN:
            blow->flags &= 0x0fff;
            blow->maximum_turn = BLOW_RUN_TURN;
            tilt = angle >> 2;
            if (info.bite && info.distance < BLOW_CLOSE_RANGE)
                item->goal_anim_state = BLOW_WAIT2;
            else if (Targetable(item, &info) && info.distance < BLOW_PIPE_RANGE)
                item->goal_anim_state = BLOW_WAIT1;
            if (item->ai_bits & GUARD)
                item->goal_anim_state = BLOW_WAIT2;
            else if (blow->mood == ESCAPE_MOOD && lara.target != item && info.ahead)
                item->goal_anim_state = BLOW_WAIT2;
            else if (blow->mood == BORED_MOOD)
                item->goal_anim_state = BLOW_WAIT1;
            break;
        case BLOW_AIM3:
            if (!info.bite || info.distance > BLOW_CLOSE_RANGE)
                item->goal_anim_state = BLOW_WAIT2;
            else
                item->goal_anim_state = BLOW_ATTACK3;
            break;
        case BLOW_ATTACK1:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            blow->maximum_turn = 0;
            if (abs(info.angle) < BLOW_WAIT_TURN)
                item->pos.y_rot += info.angle;
            else if (info.angle < 0)
                item->pos.y_rot -= BLOW_WAIT_TURN;
            else
                item->pos.y_rot += BLOW_WAIT_TURN;
            if (item->frame_number == anims[item->anim_number].frame_base + 15) {
                ITEM_INFO *dart;
                PHD_ANGLE angles[2];
                PHD_VECTOR pos1, pos2;
                BITE_INFO *bite;
                sint16 dart_num, lp;

                dart_num = CreateItem();
                if (dart_num != NO_ITEM) {
                    dart = &items[dart_num];
                    dart->object_number = 90;
                    dart->room_number = item->room_number;
                    bite = &blow_shoot_hit;
                    pos1.x = bite->x;
                    pos1.y = bite->y;
                    pos1.z = bite->z;
                    GetJointAbsPosition(item, &pos1, bite->mesh_num);
                    pos2.x = bite->x;
                    pos2.y = bite->y;
                    pos2.z = bite->z << 1;
                    GetJointAbsPosition(item, &pos2, bite->mesh_num);
                    phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);
                    dart->pos.x_pos = pos1.x;
                    dart->pos.y_pos = pos1.y;
                    dart->pos.z_pos = pos1.z;
                    InitialiseItem(dart_num);
                    dart->pos.x_rot = angles[1];
                    dart->pos.y_rot = angles[0];
                    dart->speed = 256;
                    AddActiveItem(dart_num);
                    dart->status = ACTIVE;
                    pos1.x = bite->x;
                    pos1.y = bite->y;
                    pos1.z = bite->z + 96;
                    GetJointAbsPosition(item, &pos1, bite->mesh_num);
                    for (lp = 0; lp < 2; lp++)
                        TriggerDartSmoke(pos1.x, pos1.y, pos1.z, 0, 0, 1);
                }
                item->goal_anim_state = BLOW_WAIT1;
            }
            break;
        case BLOW_ATTACK3:
            enemy = blow->enemy;
            if (enemy == lara_item) {
                if (!(blow->flags & 0xf000) && (item->touch_bits & BLOW_TOUCH)) {
                    lara_item->hit_points -= BLOW_BIFF_DAMAGE;
                    lara_item->hit_status = 1;
                    blow->flags |= 0x1000;
                    SoundEffect(70, &item->pos, 0);
                    CreatureEffect(item, &blow_biff_hit, DoBloodSplat);
                }
            } else if (!(blow->flags & 0xf000) && enemy) {
                if (ABS(enemy->pos.x_pos - item->pos.x_pos) < BLOW_HIT_RANGE
                    && ABS(enemy->pos.y_pos - item->pos.y_pos) < BLOW_HIT_RANGE
                    && ABS(enemy->pos.z_pos - item->pos.z_pos) < BLOW_HIT_RANGE) {
                    enemy->hit_points -= BLOW_BIFF_ENEMY_DAMAGE;
                    enemy->hit_status = 1;
                    blow->flags |= 0x1000;
                    SoundEffect(70, &item->pos, 0);
                }
            }
            break;
        }
    }
    CreatureTilt(item, tilt);
    head_y -= torso_y;
    CreatureJoint(item, 0, torso_y);
    CreatureJoint(item, 1, torso_x);
    CreatureJoint(item, 2, head_y);
    CreatureJoint(item, 3, head_x);
    CreatureAnimation(item_number, angle, 0);
}

#ifdef TR3_EXE
BITE_INFO blow_biff_hit = { 0, 0, -200, 13 };
BITE_INFO blow_shoot_hit = { 8, 40, -248, 13 };
#endif
