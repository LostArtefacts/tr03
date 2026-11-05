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

extern BITE_INFO mouse_bite;

void MouseControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *mouse;
    sint16 head, angle, random;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    mouse = (CREATURE_INFO *)item->data;
    head = angle = 0;
    if (item->hit_points <= 0) {
        if (item->current_anim_state != 6) {
            item->anim_number = objects[23].anim_index + 9;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = 6;
        }
    } else {
        CreatureAIInfo(item, &info);
        if (info.ahead)
            head = info.angle;
        GetCreatureMood(item, &info, 0);
        CreatureMood(item, &info, 0);
        angle = CreatureTurn(item, 1092);
        switch (item->current_anim_state) {
        case 4:
            if (mouse->mood == BORED_MOOD || mouse->mood == STALK_MOOD) {
                random = (sint16)GetRandomControl();
                if (random < 0x500)
                    item->required_anim_state = 3;
                else if (random > 0xa00)
                    item->required_anim_state = 1;
            } else if (info.distance < 116281)
                item->required_anim_state = 5;
            else
                item->required_anim_state = 1;
            if (item->required_anim_state)
                item->goal_anim_state = 2;
            break;
        case 2:
            mouse->maximum_turn = 0;
            if (item->required_anim_state)
                item->goal_anim_state = item->required_anim_state;
            break;
        case 1:
            mouse->maximum_turn = 1092;
            if (mouse->mood == BORED_MOOD || mouse->mood == STALK_MOOD) {
                random = (sint16)GetRandomControl();
                if (random < 0x500) {
                    item->required_anim_state = 3;
                    item->goal_anim_state = 2;
                } else if (random < 0xa00)
                    item->goal_anim_state = 2;
            } else if (info.ahead && info.distance < 116281)
                item->goal_anim_state = 2;
            break;
        case 5:
            if (!item->required_anim_state && (item->touch_bits & 0x7f)) {
                CreatureEffect(item, &mouse_bite, DoBloodSplat);
                lara_item->hit_points -= 20;
                lara_item->hit_status = 1;
                item->required_anim_state = 2;
            }
            break;
        case 3:
            if (GetRandomControl() < 0x500)
                item->goal_anim_state = 2;
            break;
        }
    }
    CreatureJoint(item, 0, head);
    CreatureAnimation(item_number, angle, 0);
}
