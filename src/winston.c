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

void CreatureDie(sint16 item_number, int explode);

void WinstonControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *winston, *cinfo;
    sint16 angle, slot;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    winston = (CREATURE_INFO *)item->data;
    CreatureAIInfo(item, &info);
    GetCreatureMood(item, &info, 1);
    CreatureMood(item, &info, 1);
    angle = CreatureTurn(item, winston->maximum_turn);
    if (!item->item_flags[1]) {
        cinfo = baddie_slots;
        for (slot = 0; slot < NUM_SLOTS; slot++, cinfo++) {
            if (items[cinfo->item_num].object_number != 360)
                continue;
            else {
                items[cinfo->item_num].status = INVISIBLE;
                CreatureDie(cinfo->item_num, 0);
                break;
            }
        }
    }
    if (item->hit_points <= 0) {
        winston->maximum_turn = 0;
        switch (item->current_anim_state) {
        case 10:
            if (item->hit_status)
                item->goal_anim_state = 9;
            else if (item->item_flags[0]-- < 0)
                item->goal_anim_state = 13;
            break;
        case 9:
            if (item->hit_status)
                item->goal_anim_state = 9;
            else if (item->item_flags[0]-- < 0)
                item->goal_anim_state = 13;
            break;
        case 13:
            if (item->hit_status)
                item->goal_anim_state = 9;
            else if (item->item_flags[0]-- < 0)
                item->goal_anim_state = 11;
            break;
        case 11:
            item->hit_points = 16;
            if (GetRandomControl() & 1)
                winston->flags = 999;
            break;
        default:
            item->anim_number = objects[361].anim_index + 16;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = 10;
            item->goal_anim_state = 10;
            item->item_flags[0] = 150;
        }
    } else {
        switch (item->current_anim_state) {
        case 1:
            winston->maximum_turn = 364;
            if (winston->flags == 999)
                item->goal_anim_state = 12;
            else if (lara.target == item)
                item->goal_anim_state = 3;
            else if (info.distance > 0x240000 || !info.ahead) {
                if (item->goal_anim_state != 2) {
                    item->goal_anim_state = 2;
                    SoundEffect(345, &item->pos, 0);
                }
            }
            break;
        case 2:
            winston->maximum_turn = 364;
            if (lara.target == item)
                item->goal_anim_state = 1;
            else if (info.distance < 0x240000) {
                if (info.ahead) {
                    item->goal_anim_state = 1;
                    if (winston->flags & 1)
                        winston->flags--;
                } else if (!(winston->flags & 1)) {
                    SoundEffect(344, &item->pos, 0);
                    SoundEffect(347, &item->pos, 0);
                    winston->flags |= 1;
                }
            }
            break;
        case 12:
            winston->maximum_turn = 0;
            winston->flags = 0;
            break;
        case 3:
            winston->maximum_turn = 364;
            if (item->required_anim_state)
                item->goal_anim_state = item->required_anim_state;
            if (item->hit_status)
                item->goal_anim_state = 6;
            else if (lara.target != item)
                item->goal_anim_state = 1;
            break;
        case 4:
            winston->maximum_turn = 364;
            if (item->required_anim_state)
                item->goal_anim_state = item->required_anim_state;
            if (item->hit_status)
                item->goal_anim_state = 7;
            else if (lara.target != item)
                item->goal_anim_state = 3;
            break;
        case 5:
            winston->maximum_turn = 364;
            if (item->required_anim_state)
                item->goal_anim_state = item->required_anim_state;
            if (item->hit_status)
                item->goal_anim_state = 8;
            else if (lara.target != item)
                item->goal_anim_state = 3;
            break;
        case 6:
            if (!(GetRandomControl() & 1))
                item->required_anim_state = 4;
            else
                item->required_anim_state = 5;
            break;
        case 7:
            item->required_anim_state = 3;
            break;
        case 8:
            item->required_anim_state = 3;
            break;
        }
    }
    if (GetRandomControl() < 0x100)
        SoundEffect(347, &item->pos, 0);
    CreatureAnimation(item_number, angle, 0);
}

void OldWinstonControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *winston;
    sint16 angle;
    AI_INFO info;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    winston = (CREATURE_INFO *)item->data;
    CreatureAIInfo(item, &info);
    GetCreatureMood(item, &info, 1);
    CreatureMood(item, &info, 1);
    angle = CreatureTurn(item, winston->maximum_turn);
    if (item->current_anim_state == 1) {
        if (info.distance > 0x240000 || !info.ahead) {
            if (item->goal_anim_state != 2) {
                item->goal_anim_state = 2;
                SoundEffect(345, &item->pos, 0);
            }
        }
    } else if (info.distance < 0x240000) {
        if (info.ahead) {
            item->goal_anim_state = 1;
            if (winston->flags & 1)
                winston->flags--;
        } else if (!(winston->flags & 1)) {
            SoundEffect(344, &item->pos, 0);
            SoundEffect(347, &item->pos, 0);
            winston->flags |= 1;
        }
    }
    if (item->touch_bits && !(winston->flags & 2)) {
        SoundEffect(346, &item->pos, 0);
        SoundEffect(347, &item->pos, 0);
        winston->flags |= 2;
    } else if (!item->touch_bits && winston->flags & 2)
        winston->flags -= 2;
    if (GetRandomDraw() < 0x100)
        SoundEffect(347, &item->pos, 0);
    CreatureAnimation(item_number, angle, 0);
}
