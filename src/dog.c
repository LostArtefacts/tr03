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
