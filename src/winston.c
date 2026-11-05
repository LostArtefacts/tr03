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
