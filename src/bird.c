#include "game.h"

#define CROW 27
#define VULTURE 29
#define BIRD_START_ANIM 5
#define CROW_START_ANIM 14
#define BIRD_STOP 2
#define BIRD_EAT 7

void InitialiseVulture(sint16 item_number)
{
    ITEM_INFO *item;

    InitialiseCreature(item_number);
    item = &items[item_number];
    if (item->object_number == CROW) {
        item->anim_number = objects[CROW].anim_index + CROW_START_ANIM;
        item->frame_number = anims[item->anim_number].frame_base;
        item->current_anim_state = item->goal_anim_state = BIRD_EAT;
    } else {
        item->anim_number = objects[VULTURE].anim_index + BIRD_START_ANIM;
        item->frame_number = anims[item->anim_number].frame_base;
        item->current_anim_state = item->goal_anim_state = BIRD_STOP;
    }
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

extern BITE_INFO bird_bite, crow_bite;

void VultureControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *bird;
    AI_INFO info;
    sint16 angle = 0;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    bird = (CREATURE_INFO *)item->data;
    if (item->hit_points <= 0) {
        switch (item->current_anim_state) {
        case 4:
            if (item->pos.y_pos > item->floor) {
                item->pos.y_pos = item->floor;
                item->gravity_status = 0;
                item->fallspeed = 0;
                item->goal_anim_state = 5;
            }
            break;
        case 5:
            item->pos.y_pos = item->floor;
            break;
        default:
            if (item->object_number == CROW)
                item->anim_number = objects[CROW].anim_index + 1;
            else
                item->anim_number = objects[VULTURE].anim_index + 8;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = 4;
            item->gravity_status = 1;
            item->speed = 0;
            break;
        }
        item->pos.x_rot = 0;
    } else {
        CreatureAIInfo(item, &info);
        GetCreatureMood(item, &info, 0);
        CreatureMood(item, &info, 0);
        angle = CreatureTurn(item, 546);
        switch (item->current_anim_state) {
        case BIRD_EAT:
            item->pos.y_pos = item->floor;
            if (bird->mood != BORED_MOOD)
                item->goal_anim_state = 1;
            break;
        case BIRD_STOP:
            item->pos.y_pos = item->floor;
            if (bird->mood == BORED_MOOD)
                break;
            else
                item->goal_anim_state = 1;
            break;
        case 1:
            bird->flags = 0;
            if (item->required_anim_state)
                item->goal_anim_state = item->required_anim_state;
            if (bird->mood == BORED_MOOD)
                item->goal_anim_state = BIRD_STOP;
            else if (info.ahead && info.distance < 0x40000)
                item->goal_anim_state = 6;
            else
                item->goal_anim_state = 3;
            break;
        case 3:
            if (bird->mood == BORED_MOOD) {
                item->required_anim_state = BIRD_STOP;
                item->goal_anim_state = 1;
            } else if (info.ahead && info.distance < 0x40000)
                item->goal_anim_state = 6;
            break;
        case 6:
            if (!bird->flags && item->touch_bits) {
                lara_item->hit_points -= 20;
                lara_item->hit_status = 1;
                if (item->object_number == CROW)
                    CreatureEffect(item, &crow_bite, DoBloodSplat);
                else
                    CreatureEffect(item, &bird_bite, DoBloodSplat);
                bird->flags = 1;
            }
            break;
        }
    }
    CreatureAnimation(item_number, angle, 0);
}

#ifdef TR3_EXE
BITE_INFO bird_bite = { 15, 46, 21, 6 };
BITE_INFO crow_bite = { 2, 10, 60, 14 };
#endif
