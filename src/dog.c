#include "game.h"

#define STOP_ANIM 8
#define DOG_STOP 1

void InitialiseDog(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->anim_number = objects[item->object_number].anim_index + STOP_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = DOG_STOP;
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

int CreatureActive(sint16 item_number);
void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info);
void GetCreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
void CreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn);
int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);
void CreatureFloat(sint16 item_number);
void CreatureJoint(ITEM_INFO *item, sint16 joint, sint16 required);
void CreatureUnderwater(ITEM_INFO *item, sint32 depth);
sint16 CreatureEffect(ITEM_INFO *item, BITE_INFO *bite, sint16 (*generate)(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 yrot, sint16 room_number));
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);

extern BITE_INFO croc_bite;
void CreatureTilt(ITEM_INFO *item, sint16 angle);

void GetAITarget(CREATURE_INFO *creature);
void AlertAllGuards(sint16 item_number);
sint16 AIGuard(CREATURE_INFO *creature);

extern BITE_INFO dog_bite;
extern char dog_death_anims[];

void DogControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *dog;
    sint16 angle, head, tilt, x_head;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    dog = (CREATURE_INFO *)item->data;
    head = x_head = angle = tilt = 0;
    if (item->hit_points <= 0) {
        if (item->current_anim_state != 11) {
            item->anim_number = objects[item->object_number].anim_index + dog_death_anims[GetRandomControl() & 3];
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = 11;
        }
    } else {
        AI_INFO info, lara_info;
        sint16 random, frame;
        sint32 lara_dx, lara_dz;

        if (item->ai_bits)
            GetAITarget(dog);
        else
            dog->enemy = lara_item;
        CreatureAIInfo(item, &info);
        if (dog->enemy == lara_item) {
            lara_info.angle = info.angle;
            lara_info.distance = info.distance;
        } else {
            lara_dz = lara_item->pos.z_pos - item->pos.z_pos;
            lara_dx = lara_item->pos.x_pos - item->pos.x_pos;
            lara_info.angle = phd_atan(lara_dz, lara_dx) - item->pos.y_rot;
            lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
        }
        if (info.ahead) {
            head = info.angle;
            x_head = info.x_angle;
        }
        GetCreatureMood(item, &info, 1);
        CreatureMood(item, &info, 1);
        if (dog->mood == BORED_MOOD)
            dog->maximum_turn >>= 1;
        angle = CreatureTurn(item, dog->maximum_turn);
        if (dog->hurt_by_lara || (lara_info.distance < 0x900000 && !(item->ai_bits & 8))) {
            AlertAllGuards(item_number);
            item->ai_bits &= ~8;
        }
        random = (sint16)GetRandomControl();
        frame = item->frame_number - anims[item->anim_number].frame_base;
        switch (item->current_anim_state) {
        case 8:
        case 0:
            head = 0;
            x_head = 0;
            if (dog->mood == BORED_MOOD || item->ai_bits == 8) {
                dog->maximum_turn = 0;
                dog->flags++;
                if (dog->flags > 300 && random < 0x80)
                    item->goal_anim_state = 1;
            } else
                item->goal_anim_state = 1;
            break;
        case 9:
            if (item->required_anim_state) {
                item->goal_anim_state = item->required_anim_state;
                break;
            }
        case 1:
            dog->maximum_turn = 0;
            if (item->ai_bits & 1) {
                head = AIGuard(dog);
                if (!(GetRandomControl() & 0xff)) {
                    if (item->current_anim_state == 1)
                        item->goal_anim_state = 9;
                    else
                        item->goal_anim_state = 1;
                }
                break;
            } else if (item->current_anim_state == 9 && random < 0x80) {
                item->goal_anim_state = 1;
                break;
            } else if (item->ai_bits & 4) {
                if (item->current_anim_state == 1)
                    item->goal_anim_state = 2;
                else
                    item->goal_anim_state = 1;
            } else if (dog->mood == ESCAPE_MOOD) {
                if (lara.target != item && info.ahead && !item->hit_status)
                    item->goal_anim_state = 1;
                else {
                    item->required_anim_state = 3;
                    item->goal_anim_state = 9;
                }
            } else if (dog->mood == BORED_MOOD) {
                dog->flags = 0;
                dog->maximum_turn = ONE_DEGREE;
                if (random < 0x100 && (item->ai_bits & 8) && item->current_anim_state == 1) {
                    item->goal_anim_state = 8;
                    dog->flags = 0;
                } else if (random < 0x1000) {
                    if (item->current_anim_state == 1)
                        item->goal_anim_state = 2;
                    else
                        item->goal_anim_state = 1;
                } else if (!(random & 0x1f))
                    item->goal_anim_state = 7;
            } else {
                if (item->current_anim_state == 1) {
                    item->required_anim_state = 3;
                    item->goal_anim_state = 9;
                } else
                    item->required_anim_state = 3;
            }
            break;
        case 2:
            dog->maximum_turn = 3 * ONE_DEGREE;
            if (item->ai_bits & 4)
                item->goal_anim_state = 2;
            else if (dog->mood == BORED_MOOD && random < 0x100)
                item->goal_anim_state = 1;
            else
                item->goal_anim_state = 5;
            break;
        case 3:
            dog->maximum_turn = 6 * ONE_DEGREE;
            if (dog->mood == ESCAPE_MOOD) {
                if (lara.target != item && info.ahead)
                    item->goal_anim_state = 9;
                break;
            } else if (dog->mood == BORED_MOOD)
                item->goal_anim_state = 9;
            else if (info.bite && info.distance < 0x100000)
                item->goal_anim_state = 6;
            else if (info.distance < 0x240000) {
                item->required_anim_state = 5;
                item->goal_anim_state = 9;
            }
            break;
        case 5:
            dog->maximum_turn = 3 * ONE_DEGREE;
            if (dog->mood == BORED_MOOD)
                item->goal_anim_state = 9;
            else if (dog->mood == ESCAPE_MOOD)
                item->goal_anim_state = 3;
            else if (info.bite && info.distance < 0x2c4e4) {
                item->goal_anim_state = 12;
                item->required_anim_state = 5;
            } else if (info.distance > 0x240000 || item->hit_status)
                item->goal_anim_state = 3;
            break;
        case 6:
            if (info.bite && (item->touch_bits & 0x6648) && frame >= 4 && frame <= 14) {
                CreatureEffect(item, &dog_bite, DoBloodSplat);
                lara_item->hit_points -= 50;
                lara_item->hit_status = 1;
            }
            item->goal_anim_state = 3;
            break;
        case 12:
            if (info.bite && (item->touch_bits & 0x48) && ((frame >= 9 && frame <= 12) || (frame >= 22 && frame <= 25))) {
                CreatureEffect(item, &dog_bite, DoBloodSplat);
                lara_item->hit_points -= 12;
                lara_item->hit_status = 1;
            }
            break;
        case 7:
            head = 0;
            x_head = 0;
            break;
        }
    }
    CreatureTilt(item, tilt);
    CreatureJoint(item, 0, head);
    CreatureJoint(item, 1, x_head);
    CreatureAnimation(item_number, angle, 0);
}

extern BITE_INFO tiger_bite;

void TigerControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *tiger;
    sint16 angle, head, tilt, random;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    tiger = (CREATURE_INFO *)item->data;
    head = angle = tilt = 0;
    if (item->hit_points <= 0) {
        if (item->current_anim_state != 9) {
            item->anim_number = objects[28].anim_index + 11;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = 9;
        }
    } else {
        CreatureAIInfo(item, &info);
        if (info.ahead)
            head = info.angle;
        GetCreatureMood(item, &info, 1);
        if (tiger->alerted && info.zone_number != info.enemy_zone)
            tiger->mood = ESCAPE_MOOD;
        CreatureMood(item, &info, 1);
        angle = CreatureTurn(item, tiger->maximum_turn);
        switch (item->current_anim_state) {
        case 1:
            tiger->maximum_turn = 0;
            tiger->flags = 0;
            if (tiger->mood == ESCAPE_MOOD) {
                if (lara.target != item && info.ahead)
                    item->goal_anim_state = 1;
                else
                    item->goal_anim_state = 3;
            } else if (tiger->mood == BORED_MOOD) {
                random = (sint16)GetRandomControl();
                if (random < 0x60)
                    item->goal_anim_state = 5;
                item->goal_anim_state = 2;
            } else if (info.bite && info.distance < 116281)
                item->goal_anim_state = 6;
            else if (info.bite && info.distance < 0x100000) {
                tiger->maximum_turn = 546;
                item->goal_anim_state = 8;
            } else if (item->required_anim_state)
                item->goal_anim_state = item->required_anim_state;
            else if (tiger->mood != ATTACK_MOOD && GetRandomControl() < 0x60)
                item->goal_anim_state = 5;
            else
                item->goal_anim_state = 3;
            break;
        case 2:
            tiger->maximum_turn = 546;
            if (tiger->mood == ESCAPE_MOOD || tiger->mood == ATTACK_MOOD)
                item->goal_anim_state = 3;
            else if (GetRandomControl() < 0x60) {
                item->goal_anim_state = 1;
                item->required_anim_state = 5;
            }
            break;
        case 3:
            tiger->maximum_turn = 1092;
            if (tiger->mood == BORED_MOOD)
                item->goal_anim_state = 1;
            else if (tiger->flags && info.ahead)
                item->goal_anim_state = 1;
            else if (info.bite && info.distance < 0x240000) {
                if (lara_item->speed == 0)
                    item->goal_anim_state = 1;
                else
                    item->goal_anim_state = 7;
            } else if (tiger->mood != ATTACK_MOOD && GetRandomControl() < 0x60) {
                item->required_anim_state = 5;
                item->goal_anim_state = 1;
            } else if (tiger->mood == ESCAPE_MOOD && lara.target != item && info.ahead)
                item->goal_anim_state = 1;
            tiger->flags = 0;
            break;
        case 6:
        case 7:
        case 8:
            if (!tiger->flags && (item->touch_bits & 0x7fdc000)) {
                lara_item->hit_status = 1;
                lara_item->hit_points -= 90;
                CreatureEffect(item, &tiger_bite, DoBloodSplat);
                tiger->flags = 1;
            }
            break;
        }
    }
    CreatureTilt(item, tilt);
    CreatureJoint(item, 0, head);
    CreatureAnimation(item_number, angle, tilt);
}

#ifdef TR3_EXE
BITE_INFO dog_bite = { 0, 0, 100, 3 };
char dog_death_anims[4] = {
    20, 21, 22, 20,
};
#endif
