#include "game.h"

#define COBRA_DOWN_ANIM 2
#define COBRA_DOWN 3

void InitialiseCobra(sint16 item_number)
{
    ITEM_INFO *item;

    InitialiseCreature(item_number);
    item = &items[item_number];
    item->anim_number = objects[item->object_number].anim_index + COBRA_DOWN_ANIM;
    item->frame_number = anims[item->anim_number].frame_base + 45;
    item->current_anim_state = item->goal_anim_state = COBRA_DOWN;
    item->item_flags[2] = item->hit_points;
    item->hit_points = DONT_TARGET;
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

extern BITE_INFO cobra_hit;
extern sint32 snake_inner_radius, snake_forget_radius, snake_outer_radius;
extern uint32 CurrentLevel;

int abs(int x);
#pragma intrinsic(abs)

void CobraControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *cobra;
    sint16 angle, head, tilt;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;
    if (CurrentLevel < 9) {
        snake_forget_radius = 0x900000;
        snake_outer_radius = 0x240000;
        snake_inner_radius = 0x100000;
    } else {
        snake_forget_radius = 0x640000;
        snake_outer_radius = 0x190000;
        snake_inner_radius = 0x718e4;
    }
    item = &items[item_number];
    cobra = (CREATURE_INFO *)item->data;
    head = angle = tilt = 0;
    if (!cobra)
        return;
    if (item->hit_points <= 0 && item->hit_points != DONT_TARGET) {
        if (item->current_anim_state != 4) {
            item->anim_number = objects[item->object_number].anim_index + 4;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = 4;
        }
    } else {
        CreatureAIInfo(item, &info);
        info.angle += 0xc00;
        cobra->target.x = lara_item->pos.x_pos;
        cobra->target.z = lara_item->pos.z_pos;
        angle = CreatureTurn(item, cobra->maximum_turn);
        if (info.ahead)
            head = info.angle;
        if (abs(info.angle) < 0x71c)
            item->pos.y_rot += info.angle;
        else if (info.angle < 0)
            item->pos.y_rot -= 0x71c;
        else
            item->pos.y_rot += 0x71c;
        switch (item->current_anim_state) {
        case 1:
            cobra->flags = 0;
            if (info.distance > snake_forget_radius)
                item->goal_anim_state = 3;
            else if (lara_item->hit_points > 0 &&
                     ((info.ahead && info.distance < snake_inner_radius) || item->hit_status ||
                      lara_item->speed > 15))
                item->goal_anim_state = 2;
            break;
        case 3:
            cobra->flags = 0;
            if (item->hit_points != DONT_TARGET) {
                item->item_flags[2] = item->hit_points;
                item->hit_points = DONT_TARGET;
            }
            if (info.distance < snake_outer_radius && lara_item->hit_points > 0) {
                item->goal_anim_state = 0;
                item->hit_points = item->item_flags[2];
            }
            break;
        case 2:
            if (cobra->flags != 1 && (item->touch_bits & 0x2000)) {
                cobra->flags = 1;
                lara_item->hit_points -= 80;
                lara_item->hit_status = 1;
                lara.poisoned = 0x100;
                CreatureEffect(item, &cobra_hit, DoBloodSplat);
            }
            break;
        case 0:
            item->hit_points = item->item_flags[2];
            break;
        }
    }
    CreatureAnimation(item_number, angle, 0);
}
