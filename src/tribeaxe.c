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

extern BITE_INFO tribeaxe_hit;
extern uint8 tribeaxe_hitframes[13][3];

void TribeAxeControl(sint16 item_number)
{
    ITEM_INFO *item, *enemy;
    CREATURE_INFO *tribeaxe;
    sint16 angle, head, tilt;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    tribeaxe = (CREATURE_INFO *)item->data;
    head = angle = tilt = 0;
    if (item->hit_points <= 0) {
        if (item->current_anim_state != 9) {
            if (item->current_anim_state == 1 || item->current_anim_state == 7)
                item->anim_number = objects[item->object_number].anim_index + 21;
            else
                item->anim_number = objects[item->object_number].anim_index + 20;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = 9;
        }
    } else {
        CreatureAIInfo(item, &info);
        GetCreatureMood(item, &info, 1);
        if (tribeaxe->enemy == lara_item && tribeaxe->hurt_by_lara && info.distance > 0x900000 && info.enemy_facing < 0x3000 && info.enemy_facing > -0x3000)
            tribeaxe->mood = ESCAPE_MOOD;
        CreatureMood(item, &info, 1);
        angle = CreatureTurn(item, tribeaxe->maximum_turn);
        if (info.ahead)
            head = info.angle;
        switch (item->current_anim_state) {
        case 1:
            tribeaxe->maximum_turn = 4 * ONE_DEGREE;
            tribeaxe->flags = 0;
            if (tribeaxe->mood == BORED_MOOD) {
                tribeaxe->maximum_turn = 0;
                if (GetRandomControl() < 0x100)
                    item->goal_anim_state = 2;
            } else if (tribeaxe->mood == ESCAPE_MOOD) {
                if (lara.target != item && info.ahead && !item->hit_status)
                    item->goal_anim_state = 1;
                else
                    item->goal_anim_state = 3;
            } else if (item->item_flags[0]) {
                item->item_flags[0] = 0;
                item->goal_anim_state = 11;
            } else if (info.ahead && info.distance < 0x718e4)
                item->goal_anim_state = 7;
            else if (info.ahead && info.distance < 0x100000) {
                if (GetRandomControl() < 0x4000)
                    item->goal_anim_state = 2;
                else
                    item->goal_anim_state = 7;
            } else if (info.ahead && info.distance < 0x400000)
                item->goal_anim_state = 2;
            else
                item->goal_anim_state = 3;
            break;
        case 11:
            tribeaxe->maximum_turn = 4 * ONE_DEGREE;
            tribeaxe->flags = 0;
            if (tribeaxe->mood == BORED_MOOD) {
                tribeaxe->maximum_turn = 0;
                if (GetRandomControl() < 0x100)
                    item->goal_anim_state = 2;
            } else if (tribeaxe->mood == ESCAPE_MOOD) {
                if (lara.target != item && info.ahead && !item->hit_status)
                    item->goal_anim_state = 1;
                else
                    item->goal_anim_state = 3;
            } else if (info.ahead && info.distance < 0x718e4) {
                if (GetRandomControl() < 0x800)
                    item->goal_anim_state = 5;
                else
                    item->goal_anim_state = 8;
            } else if (info.distance < 0x400000)
                item->goal_anim_state = 2;
            else
                item->goal_anim_state = 3;
            break;
        case 2:
            tribeaxe->flags = 0;
            tribeaxe->maximum_turn = 9 * ONE_DEGREE;
            tilt = angle >> 3;
            if (tribeaxe->mood == BORED_MOOD) {
                tribeaxe->maximum_turn >>= 2;
                if (GetRandomControl() < 0x100) {
                    if (GetRandomControl() < 0x2000)
                        item->goal_anim_state = 1;
                    else
                        item->goal_anim_state = 11;
                }
            } else if (tribeaxe->mood == ESCAPE_MOOD)
                item->goal_anim_state = 3;
            else if (info.ahead && info.distance < 0x718e4) {
                if (GetRandomControl() < 0x2000)
                    item->goal_anim_state = 1;
                else
                    item->goal_anim_state = 11;
            } else if (info.distance > 0x400000)
                item->goal_anim_state = 3;
            break;
        case 3:
            tribeaxe->flags = 0;
            tribeaxe->maximum_turn = 6 * ONE_DEGREE;
            tilt = angle >> 2;
            if (tribeaxe->mood == BORED_MOOD) {
                tribeaxe->maximum_turn >>= 2;
                if (GetRandomControl() < 0x100) {
                    if (GetRandomControl() < 0x4000)
                        item->goal_anim_state = 1;
                    else
                        item->goal_anim_state = 11;
                }
            } else if (tribeaxe->mood == ESCAPE_MOOD && lara.target != item && info.ahead)
                item->goal_anim_state = 11;
            else if (info.bite || info.distance < 0x400000) {
                if (GetRandomControl() < 0x4000)
                    item->goal_anim_state = 12;
                else if (GetRandomControl() < 0x2000)
                    item->goal_anim_state = 10;
                else
                    item->goal_anim_state = 2;
            }
            break;
        case 8:
            tribeaxe->maximum_turn = 4 * ONE_DEGREE;
            if (info.bite || info.distance < 0x718e4)
                item->goal_anim_state = 6;
            else
                item->goal_anim_state = 11;
            break;
        case 5:
        case 6:
        case 7:
        case 10:
        case 12:
            item->item_flags[0] = 1;
            tribeaxe->maximum_turn = 4 * ONE_DEGREE;
            enemy = tribeaxe->enemy;
            tribeaxe->flags = item->frame_number - anims[item->anim_number].frame_base;
            if (enemy == lara_item) {
                if ((item->touch_bits & 0x2000) && tribeaxe->flags >= tribeaxe_hitframes[item->current_anim_state][0] && tribeaxe->flags <= tribeaxe_hitframes[item->current_anim_state][1]) {
                    sint32 lp;

                    lara_item->hit_points -= tribeaxe_hitframes[item->current_anim_state][2];
                    lara_item->hit_status = 1;
                    for (lp = 0; lp < tribeaxe_hitframes[item->current_anim_state][2]; lp += 8)
                        CreatureEffect(item, &tribeaxe_hit, DoBloodSplat);
                    SoundEffect(70, &item->pos, 0);
                }
            } else {
                if (enemy) {
                    if (ABS(enemy->pos.x_pos - item->pos.x_pos) < 512 && ABS(enemy->pos.y_pos - item->pos.y_pos) < 512 && ABS(enemy->pos.z_pos - item->pos.z_pos) < 512 && tribeaxe->flags >= tribeaxe_hitframes[item->current_anim_state][0] && tribeaxe->flags <= tribeaxe_hitframes[item->current_anim_state][1]) {
                        enemy->hit_points -= 2;
                        enemy->hit_status = 1;
                        CreatureEffect(item, &tribeaxe_hit, DoBloodSplat);
                        SoundEffect(70, &item->pos, 0);
                    }
                }
            }
            break;
        }
    }
    CreatureTilt(item, tilt);
    CreatureJoint(item, 0, head >> 1);
    CreatureJoint(item, 1, head >> 1);
    CreatureAnimation(item_number, angle, 0);
}
