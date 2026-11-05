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
sint32 GetWaterHeight(sint32 x, sint32 y, sint32 z, sint16 room_number);
RIPPLE_STRUCT *SetupRipple(sint32 x, sint32 y, sint32 z, sint32 size, sint32 moretrans);

extern sint32 wibble;

void OrcaControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *orca;
    sint16 angle, head;
    AI_INFO info;
    int lara_alive;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    orca = (CREATURE_INFO *)item->data;
    angle = head = 0;
    lara_alive = lara_item->hit_points > 0;
    item->hit_points = DONT_TARGET;
    CreatureAIInfo(item, &info);
    GetCreatureMood(item, &info, 1);
    if (!(room[lara_item->room_number].flags & UNDERWATER) && lara.skidoo == NO_ITEM)
        orca->mood = BORED_MOOD;
    CreatureMood(item, &info, 1);
    angle = CreatureTurn(item, orca->maximum_turn);
    switch (item->current_anim_state) {
    case 0:
        orca->flags = 0;
        orca->maximum_turn = 364;
        if (orca->mood == BORED_MOOD) {
            if (GetRandomControl() & 0xff)
                item->goal_anim_state = 0;
            else
                item->goal_anim_state = 2;
            break;
        } else if (info.ahead && info.distance < 0x90000)
            item->goal_anim_state = 4;
        else if (orca->mood == ESCAPE_MOOD)
            item->goal_anim_state = 1;
        else if (info.distance > 0x240000) {
            if (info.angle < 0x5000 && info.angle > -0x5000) {
                if (!(GetRandomControl() & 0x3f))
                    item->goal_anim_state = 6;
                else
                    item->goal_anim_state = 1;
            } else
                item->goal_anim_state = 7;
        }
        break;
    case 1:
        orca->flags = 0;
        orca->maximum_turn = 364;
        if (orca->mood == BORED_MOOD) {
            if (GetRandomControl() & 0xff)
                item->goal_anim_state = 0;
            else
                item->goal_anim_state = 2;
        } else if (orca->mood == ESCAPE_MOOD)
            break;
        else if (info.ahead && info.distance < 0x240000 && info.zone_number == info.enemy_zone)
            item->goal_anim_state = 0;
        else if (info.distance > 0x240000 && !(GetRandomControl() & 0x7f))
            item->goal_anim_state = 2;
        else if (info.distance > 0x240000 && !info.ahead)
            item->goal_anim_state = 0;
        break;
    case 7:
        orca->maximum_turn = 0;
        if (item->frame_number == anims[item->anim_number].frame_base + 59) {
            if (item->pos.y_rot < 0)
                item->pos.y_rot += 0x8000;
            else
                item->pos.y_rot -= 0x8000;
            item->pos.x_rot = -item->pos.x_rot;
        }
        break;
    }
    CreatureAnimation(item_number, angle, 0);
    CreatureUnderwater(item, 204);
    if (wibble & 4) {
        PHD_VECTOR pos;
        sint16 room_number;
        sint32 wh;
        RIPPLE_STRUCT *r;

        room_number = item->room_number;
        pos.x = -32;
        pos.y = 16;
        pos.z = -300;
        GetJointAbsPosition(item, &pos, 5);
        GetFloor(pos.x, pos.y, pos.z, &room_number);
        if ((wh = GetWaterHeight(pos.x, pos.y, pos.z, room_number)) != NO_HEIGHT) {
            if (pos.y < wh) {
                r = SetupRipple(pos.x, wh, pos.z, -2 - (GetRandomControl() & 1), 0);
                r->init = 0;
            }
        }
    }
}
