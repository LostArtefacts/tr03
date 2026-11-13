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

extern BITE_INFO raptor_bite;

void RaptorControl(sint16 item_number)
{
    ITEM_INFO *item, *enemy, *best, *target;
    CREATURE_INFO *raptor, *cinfo;
    sint16 head, neck, angle, tilt;
    int slot, distance, best_distance, x, y, z;
    AI_INFO info;

    best = NULL;
    best_distance = 0x7fffffff;
    item = &items[item_number];
    if (item->status == INVISIBLE) {
        if (!EnableBaddieAI(item_number, 0))
            return;
        item->status = ACTIVE;
    }
    raptor = (CREATURE_INFO *)item->data;
    head = neck = angle = tilt = 0;
    if (item->hit_points <= 0) {
        if (item->current_anim_state != 5) {
            if (GetRandomControl() > 0x4000)
                item->anim_number = objects[item->object_number].anim_index + 9;
            else
                item->anim_number = objects[item->object_number].anim_index + 10;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = 5;
        }
    } else {
        if (raptor->enemy == NULL || !(GetRandomControl() & 0x7f)) {
            cinfo = baddie_slots;
            for (slot = 0; slot < 5; slot++, cinfo++) {
                if (cinfo->item_num == NO_ITEM || cinfo->item_num == item_number)
                    continue;
                target = &items[cinfo->item_num];
                x = (target->pos.x_pos - item->pos.x_pos) >> 6;
                y = (target->pos.y_pos - item->pos.y_pos) >> 6;
                z = (target->pos.z_pos - item->pos.z_pos) >> 6;
                distance = x * x + y * y + z * z;
                if (distance < best_distance && item->hit_points > 0) {
                    best = target;
                    best_distance = distance;
                }
            }
            if (best && (best->object_number != 288 || (GetRandomControl() < 0x400 && best_distance < 0x400000)))
                raptor->enemy = best;
            x = (lara_item->pos.x_pos - item->pos.x_pos) >> 6;
            y = (lara_item->pos.y_pos - item->pos.y_pos) >> 6;
            z = (lara_item->pos.z_pos - item->pos.z_pos) >> 6;
            distance = x * x + y * y + z * z;
            if (distance <= best_distance)
                raptor->enemy = lara_item;
        }
        if (item->ai_bits)
            GetAITarget(raptor);
        CreatureAIInfo(item, &info);
        if (info.ahead)
            head = info.angle;
        GetCreatureMood(item, &info, 1);
        CreatureMood(item, &info, 1);
        if (raptor->mood == BORED_MOOD)
            raptor->maximum_turn >>= 1;
        angle = CreatureTurn(item, raptor->maximum_turn);
        neck = -(angle * 6);
        switch (item->current_anim_state) {
        case 1:
            raptor->maximum_turn = 0;
            raptor->flags &= ~1;
            if (item->required_anim_state)
                item->goal_anim_state = item->required_anim_state;
            else if (raptor->flags & 2) {
                raptor->flags &= ~2;
                item->goal_anim_state = 6;
            } else if ((item->touch_bits & 0xff7c00) || (info.distance < 0x538d1 && info.bite))
                item->goal_anim_state = 8;
            else if (info.bite && info.distance < 0x240000)
                item->goal_anim_state = 4;
            else if (raptor->mood == ESCAPE_MOOD && lara.target != item && info.ahead && !item->hit_status)
                item->goal_anim_state = 1;
            else if (raptor->mood == BORED_MOOD)
                item->goal_anim_state = 2;
            else
                item->goal_anim_state = 3;
            break;
        case 2:
            raptor->maximum_turn = 2 * ONE_DEGREE;
            raptor->flags &= ~1;
            if (raptor->mood != BORED_MOOD)
                item->goal_anim_state = 1;
            else if (info.ahead && GetRandomControl() < 0x80) {
                item->required_anim_state = 6;
                item->goal_anim_state = 1;
                raptor->flags &= ~2;
            }
            break;
        case 3:
            tilt = angle;
            raptor->maximum_turn = 4 * ONE_DEGREE;
            raptor->flags &= ~1;
            enemy = raptor->enemy;
            if (item->touch_bits & 0xff7c00)
                item->goal_anim_state = 1;
            else if (raptor->flags & 2) {
                item->required_anim_state = 6;
                item->goal_anim_state = 1;
                raptor->flags &= ~2;
            } else if (info.bite && info.distance < 0x240000) {
                if (item->goal_anim_state == 3) {
                    if (GetRandomControl() < 0x2000)
                        item->goal_anim_state = 1;
                    else
                        item->goal_anim_state = 7;
                }
            } else if (info.ahead && raptor->mood != ESCAPE_MOOD && GetRandomControl() < 0x80 && enemy->object_number != 354) {
                item->required_anim_state = 6;
                item->goal_anim_state = 1;
            } else if (raptor->mood == BORED_MOOD || (raptor->mood == ESCAPE_MOOD && lara.target != item && info.ahead))
                item->goal_anim_state = 1;
            break;
        case 4:
            tilt = angle;
            raptor->maximum_turn = 2 * ONE_DEGREE;
            enemy = raptor->enemy;
            if (enemy == lara_item) {
                if (!(raptor->flags & 1) && (item->touch_bits & 0xff7c00)) {
                    raptor->flags |= 1;
                    CreatureEffect(item, &raptor_bite, DoBloodSplat);
                    if (lara_item->hit_points <= 0)
                        raptor->flags |= 2;
                    lara_item->hit_points -= 100;
                    lara_item->hit_status = 1;
                    item->required_anim_state = 1;
                }
            } else {
                if (!(raptor->flags & 1) && enemy) {
                    if (ABS(enemy->pos.x_pos - item->pos.x_pos) < 512 && ABS(enemy->pos.y_pos - item->pos.y_pos) < 512 && ABS(enemy->pos.z_pos - item->pos.z_pos) < 512) {
                        enemy->hit_points -= 100 >> 2;
                        enemy->hit_status = 1;
                        if (enemy->hit_points <= 0)
                            raptor->flags |= 2;
                        raptor->flags |= 1;
                        CreatureEffect(item, &raptor_bite, DoBloodSplat);
                    }
                }
            }
            break;
        case 8:
            tilt = angle;
            raptor->maximum_turn = 2 * ONE_DEGREE;
            enemy = raptor->enemy;
            if (enemy == lara_item) {
                if (!(raptor->flags & 1) && (item->touch_bits & 0xff7c00)) {
                    raptor->flags |= 1;
                    CreatureEffect(item, &raptor_bite, DoBloodSplat);
                    if (lara_item->hit_points <= 0)
                        raptor->flags |= 2;
                    lara_item->hit_points -= 100;
                    lara_item->hit_status = 1;
                    item->required_anim_state = 1;
                }
            } else {
                if (!(raptor->flags & 1) && enemy) {
                    if (ABS(enemy->pos.x_pos - item->pos.x_pos) < 512 && ABS(enemy->pos.y_pos - item->pos.y_pos) < 512 && ABS(enemy->pos.z_pos - item->pos.z_pos) < 512) {
                        enemy->hit_points -= 100 >> 2;
                        enemy->hit_status = 1;
                        if (enemy->hit_points <= 0)
                            raptor->flags |= 2;
                        raptor->flags |= 1;
                        CreatureEffect(item, &raptor_bite, DoBloodSplat);
                    }
                }
            }
            break;
        case 7:
            tilt = angle;
            raptor->maximum_turn = 2 * ONE_DEGREE;
            enemy = raptor->enemy;
            if (enemy == lara_item) {
                if (!(raptor->flags & 1) && (item->touch_bits & 0xff7c00)) {
                    raptor->flags |= 1;
                    CreatureEffect(item, &raptor_bite, DoBloodSplat);
                    lara_item->hit_points -= 100;
                    lara_item->hit_status = 1;
                    if (lara_item->hit_points <= 0)
                        raptor->flags |= 2;
                    item->required_anim_state = 3;
                }
            } else {
                if (!(raptor->flags & 1) && enemy) {
                    if (ABS(enemy->pos.x_pos - item->pos.x_pos) < 512 && ABS(enemy->pos.y_pos - item->pos.y_pos) < 512 && ABS(enemy->pos.z_pos - item->pos.z_pos) < 512) {
                        enemy->hit_points -= 100 >> 2;
                        enemy->hit_status = 1;
                        if (enemy->hit_points <= 0)
                            raptor->flags |= 2;
                        raptor->flags |= 1;
                        CreatureEffect(item, &raptor_bite, DoBloodSplat);
                    }
                }
            }
            break;
        }
    }
    CreatureTilt(item, tilt);
    CreatureJoint(item, 0, head >> 1);
    CreatureJoint(item, 1, head >> 1);
    CreatureJoint(item, 2, neck);
    CreatureJoint(item, 3, neck);
    CreatureAnimation(item_number, angle, tilt);
}

#ifdef TR3_EXE
BITE_INFO raptor_bite = { 0, 66, 318, 22 };
#endif
