#include "game.h"

extern sint32 compys_attack_lara;

void InitialiseCompy(sint16 item_number)
{
    InitialiseCreature(item_number);
    compys_attack_lara = 0;
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

extern BITE_INFO compy_hit;
extern sint32 compy_scared_timer;

int abs(int x);
#pragma intrinsic(abs)

void CompyControl(sint16 item_number)
{
    ITEM_INFO *item, *enemy;
    CREATURE_INFO *compy;
    sint16 angle, head, neck, tilt, random, room_number, compys_carcass, linknum;
    sint32 dx, dz;
    AI_INFO info;
    FLOOR_INFO *floor;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    compy = (CREATURE_INFO *)item->data;
    head = neck = angle = tilt = 0;
    if (!item->item_flags[1]) {
        linknum = room[item->room_number].item_number;
        if (items[linknum].object_number == 354)
            item->item_flags[1] = linknum;
        else {
            for (; linknum != NO_ITEM; linknum = items[linknum].next_item) {
                if (items[linknum].object_number == 354) {
                    item->item_flags[1] = linknum;
                    break;
                }
            }
        }
    }
    compys_carcass = item->item_flags[1];
    if (item->hit_points <= 0) {
        if (item->current_anim_state != 4) {
            item->anim_number = objects[item->object_number].anim_index + 6;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = 4;
        }
    } else {
        CreatureAIInfo(item, &info);
        enemy = compy->enemy;
        if (compy->mood == BORED_MOOD && compys_carcass) {
            dx = items[compys_carcass].pos.x_pos - item->pos.x_pos;
            dz = items[compys_carcass].pos.z_pos - item->pos.z_pos;
            info.distance = dx * dx + dz * dz;
            info.angle = phd_atan(dz, dx) - item->pos.y_rot;
            info.ahead = info.angle > -0x4000 && info.angle < 0x4000;
        }
        random = ((item_number & 7) * 0x200) - 0x700;
        if (!compy_scared_timer && !compys_attack_lara &&
            ((info.enemy_facing < 0x3000 && info.enemy_facing > -0x3000 && lara_item->speed > 15) ||
             lara_item->current_anim_state == AS_ROLL || item->hit_status)) {
            item->item_flags[0] = (random + 0x700) >> 7;
            compy_scared_timer = 280;
        } else if (compy_scared_timer) {
            if (item->item_flags[0] > 0)
                item->item_flags[0]--;
            else {
                compy->mood = ESCAPE_MOOD;
                compy_scared_timer--;
            }
            if (GetRandomControl() < 0x1f && item->timer > 180)
                compys_attack_lara = 1;
        } else if (info.zone_number != info.enemy_zone)
            compy->mood = BORED_MOOD;
        else
            compy->mood = ATTACK_MOOD;
        switch (compy->mood) {
        case ATTACK_MOOD:
            compy->target.x = enemy->pos.x_pos + (1024 * phd_sin(item->pos.y_rot + random) >> W2V_SHIFT);
            compy->target.y = enemy->pos.y_pos;
            compy->target.z = enemy->pos.z_pos + (1024 * phd_cos(item->pos.y_rot + random) >> W2V_SHIFT);
            break;
        case STALK_MOOD:
        case ESCAPE_MOOD:
            compy->target.x = item->pos.x_pos + (1024 * phd_sin(info.angle + 0x8000 + random) >> W2V_SHIFT);
            compy->target.z = item->pos.z_pos + (1024 * phd_cos(info.angle + 0x8000 + random) >> W2V_SHIFT);
            room_number = item->room_number;
            floor = GetFloor(compy->target.x, item->pos.y_pos, compy->target.z, &room_number);
            if (abs(boxes[floor->box].height - item->pos.y_pos) > 256) {
                compy->mood = BORED_MOOD;
                item->item_flags[0] = compy_scared_timer;
            }
            break;
        case BORED_MOOD:
            if (compys_carcass) {
                compy->target.x = items[compys_carcass].pos.x_pos;
                compy->target.z = items[compys_carcass].pos.z_pos;
            }
            break;
        }
        angle = CreatureTurn(item, compy->maximum_turn);
        if (info.ahead)
            head = info.angle;
        neck = -(info.angle / 4);
        item->timer++;
        if (item->hit_status && item->timer > 200 && GetRandomControl() < 0x1f * 100)
            compys_attack_lara = 1;
        switch (item->current_anim_state) {
        case 0:
            compy->maximum_turn = 546;
            compy->flags &= ~1;
            if (compy->mood == ATTACK_MOOD) {
                if (info.ahead && info.distance < 116281 * 4) {
                    if (compys_attack_lara) {
                        if (GetRandomControl() < 0x4000)
                            item->goal_anim_state = 3;
                        else
                            item->goal_anim_state = 2;
                    } else
                        item->goal_anim_state = 0;
                } else if (info.distance > 116281 * (9 - 4 * compys_attack_lara))
                    item->goal_anim_state = 1;
            } else if (compy->mood == BORED_MOOD) {
                if (info.ahead && info.distance < 116281 * 3 && compys_carcass) {
                    if (GetRandomControl() < 0x4000)
                        item->goal_anim_state = 3;
                    else
                        item->goal_anim_state = 2;
                } else if (info.distance > 116281 * 3)
                    item->goal_anim_state = 1;
            } else {
                if (GetRandomControl() < 0x1000)
                    item->goal_anim_state = 2;
                else
                    item->goal_anim_state = 1;
            }
            break;
        case 1:
            compy->flags &= ~1;
            compy->maximum_turn = 1820;
            if (info.angle < 0x3000 && info.angle > -0x3000 &&
                info.distance < 116281 * (9 - 4 * compys_attack_lara)) {
                item->goal_anim_state = 0;
            }
            break;
        case 3:
        case 2:
            compy->maximum_turn = 1820;
            if (!(compy->flags & 1) && (item->touch_bits & 4) && compys_attack_lara) {
                compy->flags |= 1;
                lara_item->hit_points -= 90;
                lara_item->hit_status = 1;
                CreatureEffect(item, &compy_hit, DoBloodSplat);
            } else if (!(compy->flags & 1) && info.distance < 116281 && info.ahead && compys_carcass &&
                       compy->mood != ATTACK_MOOD) {
                compy->flags |= 1;
                CreatureEffect(item, &compy_hit, DoBloodSplat);
            }
            break;
        }
    }
    tilt = angle >> 1;
    CreatureTilt(item, tilt);
    CreatureJoint(item, 0, head);
    CreatureJoint(item, 1, neck);
    CreatureAnimation(item_number, angle, tilt);
}

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint16 InnerXZoff;
    sint16 InnerXZsize;
    sint16 InnerYsize;
    sint16 InnerXZvel;
    sint16 InnerYvel;
    sint16 InnerGravity;
    sint16 InnerFriction;
    sint16 MiddleXZoff;
    sint16 MiddleXZsize;
    sint16 MiddleYsize;
    sint16 MiddleXZvel;
    sint16 MiddleYvel;
    sint16 MiddleGravity;
    sint16 MiddleFriction;
    sint16 OuterXZoff;
    sint16 OuterXZsize;
    sint16 OuterXZvel;
    sint16 OuterFriction;
} SPLASH_SETUP;

extern SPLASH_SETUP splash_setup;
extern sint16 CarcassItem;

void SetupSplash(SPLASH_SETUP *spl);

void CarcassControl(sint16 item_number)
{
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    sint16 room_number, old_room, maxfs;
    sint32 h;

    item = &items[item_number];
    if (item->status != ACTIVE)
        return;
    old_room = item->room_number;
    item->pos.y_pos += item->fallspeed;
    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    h = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    if (item->room_number != room_number)
        ItemNewRoom(item_number, room_number);
    if (item->pos.y_pos >= h) {
        item->pos.y_pos = h;
        item->fallspeed = 0;
        item->pos.z_rot = 0x4000;
        return;
    }
    item->pos.z_rot += item->fallspeed;
    item->fallspeed += (room[room_number].flags & UNDERWATER) ? 1 : 8;
    maxfs = (room[old_room].flags & UNDERWATER) ? 64 : 512;
    if (item->fallspeed > maxfs)
        item->fallspeed = maxfs;
    if ((room[room_number].flags & UNDERWATER) && !(room[old_room].flags & UNDERWATER)) {
        splash_setup.x = item->pos.x_pos;
        splash_setup.y = room[room_number].maxceiling;
        splash_setup.z = item->pos.z_pos;
        splash_setup.InnerXZoff = 16;
        splash_setup.InnerXZsize = 16;
        splash_setup.InnerYsize = -96;
        splash_setup.InnerXZvel = 0xa0;
        splash_setup.InnerYvel = -item->fallspeed * 72;
        splash_setup.InnerGravity = 0x80;
        splash_setup.InnerFriction = 7;
        splash_setup.MiddleXZoff = 24;
        splash_setup.MiddleXZsize = 32;
        splash_setup.MiddleYsize = -64;
        splash_setup.MiddleXZvel = 0xe0;
        splash_setup.MiddleYvel = -item->fallspeed * 36;
        splash_setup.MiddleGravity = 0x48;
        splash_setup.MiddleFriction = 8;
        splash_setup.OuterXZoff = 32;
        splash_setup.OuterXZsize = 32;
        splash_setup.OuterXZvel = 0x110;
        splash_setup.OuterFriction = 9;
        SetupSplash(&splash_setup);
        item->fallspeed = 16;
        item->pos.y_pos = room[room_number].maxceiling + 1;
    }
    if (room[item->room_number].flags & UNDERWATER)
        CarcassItem = item_number;
    else
        CarcassItem = -1;
}

#ifdef TR3_EXE
BITE_INFO compy_hit = { 0, 0, 0, 2 };
#endif
