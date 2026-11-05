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

int CreatureActive(sint16 item_number);
void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info);
void GetCreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
void CreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn);
int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);
void CreatureJoint(ITEM_INFO *item, sint16 joint, sint16 required);
void CreatureTilt(ITEM_INFO *item, sint16 angle);
sint16 CreatureEffect(ITEM_INFO *item, BITE_INFO *bite, sint16 (*generate)(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 yrot, sint16 room_number));
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);
void GetAITarget(CREATURE_INFO *creature);
void AlertAllGuards(sint16 item_number);
sint16 AIGuard(CREATURE_INFO *creature);
int TargetVisible(ITEM_INFO *item, AI_INFO *info);
int Targetable(ITEM_INFO *item, AI_INFO *info);

extern BITE_INFO hybrid_bite_left;
extern BITE_INFO hybrid_bite_right;

void HybridControl(sint16 item_number)
{
    ITEM_INFO *item, *real_enemy;
    CREATURE_INFO *hybrid;
    sint16 angle, head, torso_y, torso_x, tilt;
    sint32 lara_dx, lara_dz;
    AI_INFO info, lara_info;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    hybrid = (CREATURE_INFO *)item->data;
    head = torso_y = torso_x = angle = tilt = 0;
    if (item->hit_points <= 0) {
        if (item->current_anim_state != 11) {
            item->anim_number = objects[item->object_number].anim_index + 18;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = 11;
        }
    } else {
        if (item->ai_bits)
            GetAITarget(hybrid);
        CreatureAIInfo(item, &info);
        if (hybrid->enemy == lara_item) {
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
        angle = CreatureTurn(item, hybrid->maximum_turn);
        real_enemy = hybrid->enemy;
        hybrid->enemy = lara_item;
        if (lara_info.distance < 0x100000 || item->hit_status || TargetVisible(item, &lara_info))
            AlertAllGuards(item_number);
        hybrid->enemy = real_enemy;
        switch (item->current_anim_state) {
        case 1:
            hybrid->maximum_turn = 0;
            hybrid->flags = 0;
            head = info.angle;
            if (item->ai_bits & 1) {
                head = AIGuard(hybrid);
                item->goal_anim_state = 1;
                break;
            } else if (item->ai_bits & 4) {
                item->goal_anim_state = 2;
                head = 0;
            } else if (hybrid->mood == ESCAPE_MOOD) {
                if (lara.target != item && info.ahead)
                    item->goal_anim_state = 1;
                else
                    item->goal_anim_state = 3;
            } else if (info.angle < 0x2000 && info.angle > -0x2000 && info.distance > 0x100000)
                item->goal_anim_state = 4;
            else if (info.bite && info.distance < 0x100000) {
                torso_y = info.angle;
                torso_x = info.x_angle;
                if (info.angle < 0)
                    item->goal_anim_state = 7;
                else
                    item->goal_anim_state = 8;
            } else if (item->required_anim_state)
                item->goal_anim_state = item->required_anim_state;
            else if (info.distance < 0x400000)
                item->goal_anim_state = 2;
            else
                item->goal_anim_state = 3;
            break;
        case 2:
            hybrid->maximum_turn = 3 * ONE_DEGREE;
            if (info.ahead)
                head = info.angle;
            if (item->ai_bits & 4) {
                item->goal_anim_state = 2;
                head = 0;
            } else if (info.bite && info.distance < 0x1c6e39) {
                hybrid->maximum_turn = 3 * ONE_DEGREE;
                item->goal_anim_state = 10;
            } else if (hybrid->mood == ESCAPE_MOOD || hybrid->mood == ATTACK_MOOD)
                item->goal_anim_state = 3;
            break;
        case 3:
            hybrid->maximum_turn = 6 * ONE_DEGREE;
            if (info.ahead)
                head = info.angle;
            if (item->ai_bits & 1)
                item->goal_anim_state = 1;
            else if (hybrid->mood == BORED_MOOD || (hybrid->mood == ESCAPE_MOOD && lara.target != item && info.ahead))
                item->goal_anim_state = 1;
            else if (hybrid->flags && info.ahead)
                item->goal_anim_state = 1;
            else if (info.bite && info.distance < 0x400000) {
                if (lara_item->speed == 0)
                    item->goal_anim_state = 1;
                else
                    item->goal_anim_state = 9;
            } else if (info.distance < 0x400000)
                item->goal_anim_state = 2;
            hybrid->flags = 0;
            break;
        case 10:
        case 9:
        case 7:
            hybrid->maximum_turn = 3 * ONE_DEGREE;
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            if (!hybrid->flags && (item->touch_bits & 0x80)) {
                lara_item->hit_status = 1;
                lara_item->hit_points -= 100;
                CreatureEffect(item, &hybrid_bite_left, DoBloodSplat);
                hybrid->flags = 1;
            }
            if (!(info.bite && info.distance < 0x100000))
                item->goal_anim_state = 1;
            if (item->frame_number == anims[item->anim_number].frame_end)
                hybrid->flags = 0;
            break;
        case 8:
            hybrid->maximum_turn = 3 * ONE_DEGREE;
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            if (!hybrid->flags && (item->touch_bits & 0x800)) {
                lara_item->hit_status = 1;
                lara_item->hit_points -= 80;
                CreatureEffect(item, &hybrid_bite_right, DoBloodSplat);
                hybrid->flags = 1;
            }
            if (!(info.bite && info.distance < 0x100000))
                item->goal_anim_state = 1;
            if (item->frame_number == anims[item->anim_number].frame_end)
                hybrid->flags = 0;
            break;
        case 4:
            hybrid->maximum_turn = 3 * ONE_DEGREE;
            break;
        case 5:
        case 6:
            hybrid->maximum_turn = 0;
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            if (item->touch_bits & 0x80) {
                lara_item->hit_status = 1;
                lara_item->hit_points -= 20;
                CreatureEffect(item, &hybrid_bite_left, DoBloodSplat);
            }
            break;
        }
    }
    CreatureTilt(item, tilt);
    CreatureJoint(item, 0, torso_x);
    CreatureJoint(item, 1, torso_y);
    CreatureJoint(item, 2, head);
    CreatureAnimation(item_number, angle, tilt);
}
