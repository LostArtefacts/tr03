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
void CreatureFloat(sint16 item_number);
void CreatureJoint(ITEM_INFO *item, sint16 joint, sint16 required);
void CreatureUnderwater(ITEM_INFO *item, sint32 depth);
sint16 CreatureEffect(ITEM_INFO *item, BITE_INFO *bite, sint16 (*generate)(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 yrot, sint16 room_number));
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);

extern BITE_INFO croc_bite;

void CrocControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *croc;
    sint16 head, angle;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    croc = (CREATURE_INFO *)item->data;
    head = angle = 0;
    if (item->hit_points <= 0) {
        if (item->current_anim_state != 3) {
            item->anim_number = objects[32].anim_index + 4;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = 3;
            item->hit_points = DONT_TARGET;
        }
        CreatureFloat(item_number);
        return;
    }
    CreatureAIInfo(item, &info);
    if (info.ahead)
        head = info.angle;
    GetCreatureMood(item, &info, 1);
    if (!(room[lara_item->room_number].flags & UNDERWATER) && lara.skidoo == NO_ITEM)
        croc->mood = BORED_MOOD;
    CreatureMood(item, &info, 1);
    CreatureTurn(item, 546);
    switch (item->current_anim_state) {
    case 1:
        if (info.bite && item->touch_bits)
            item->goal_anim_state = 2;
        break;
    case 2:
        if (item->frame_number == anims[item->anim_number].frame_base)
            item->required_anim_state = 0;
        if (info.bite && item->touch_bits) {
            if (!item->required_anim_state) {
                CreatureEffect(item, &croc_bite, DoBloodSplat);
                lara_item->hit_points -= 100;
                lara_item->hit_status = 1;
                item->required_anim_state = 1;
            }
        } else
            item->goal_anim_state = 1;
        break;
    }
    CreatureJoint(item, 0, head);
    CreatureUnderwater(item, 256);
    CreatureAnimation(item_number, angle, 0);
}

#ifdef TR3_EXE
BITE_INFO croc_bite = { 5, -21, 467, 9 };
#endif
