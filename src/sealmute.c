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
void CreatureJoint(ITEM_INFO *item, sint16 joint, sint16 required);
void CreatureTilt(ITEM_INFO *item, sint16 angle);
sint16 CreatureEffect(ITEM_INFO *item, BITE_INFO *bite, sint16 (*generate)(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 yrot, sint16 room_number));
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);
void GetAITarget(CREATURE_INFO *creature);
void AlertAllGuards(sint16 item_number);
sint16 AIGuard(CREATURE_INFO *creature);
int TargetVisible(ITEM_INFO *item, AI_INFO *info);
int Targetable(ITEM_INFO *item, AI_INFO *info);

int abs(int x);
#pragma intrinsic(abs)
extern sint32 wibble;
void TriggerFireFlame(sint32 x, sint32 y, sint32 z, sint32 body_part, sint32 type);
void TriggerSealmuteGasThrower_sealmute(ITEM_INFO *item, BITE_INFO *bite, sint16 speed);

extern BITE_INFO seal_gas;

void SealmuteControl(sint16 item_number)
{
    ITEM_INFO *item, *target, *real_enemy;
    CREATURE_INFO *seal, *cinfo;
    sint16 angle, head, torso_y, torso_x, tilt, slot;
    sint32 lara_dx, lara_dz, x, z, distance;
    AI_INFO info, lara_info;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    seal = (CREATURE_INFO *)item->data;
    head = torso_y = torso_x = angle = tilt = 0;
    if (item->item_flags[0] > 80)
        item->hit_points = 0;
    if (item->hit_points <= 0) {
        if (item->current_anim_state != 3) {
            item->anim_number = objects[item->object_number].anim_index + 5;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = 3;
            seal->flags = 0;
        } else if (item->item_flags[0] > 80) {
            PHD_VECTOR pos;
            sint32 bright, r, g, b, rnd;

            for (r = 9; r < 17; r++) {
                if (!(wibble & 4)) {
                    pos.x = 0;
                    pos.y = 0;
                    pos.z = 0;
                    GetJointAbsPosition(item, &pos, r);
                    TriggerFireFlame(pos.x, pos.y, pos.z, -1, 255);
                }
            }
            bright = item->frame_number - anims[item->anim_number].frame_base;
            if (bright > 16) {
                bright = anims[item->anim_number].frame_end - item->frame_number;
                if (bright > 16)
                    bright = 16;
            }
            rnd = GetRandomControl();
            r = 31 - ((rnd >> 4) & 3);
            g = 24 - ((rnd >> 6) & 7);
            b = rnd & 7;
            r = (r * bright) >> 4;
            g = (g * bright) >> 4;
            b = (b * bright) >> 4;
            TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 12, r, g, b);
        } else {
            if (item->frame_number >= anims[item->anim_number].frame_base + 1 && item->frame_number <= anims[item->anim_number].frame_end - 8) {
                uint32 length;

                length = item->frame_number - anims[item->anim_number].frame_base + 1;
                if (length > 24) {
                    length = anims[item->anim_number].frame_end - item->frame_number - 8;
                    if (length <= 0)
                        length = 1;
                    if (length > 24)
                        length = (GetRandomControl() & 15) + 8;
                }
                TriggerSealmuteGasThrower_sealmute(item, &seal_gas, length);
            }
        }
    } else {
        if (item->ai_bits)
            GetAITarget(seal);
        else {
            seal->enemy = lara_item;
            lara_dz = lara_item->pos.z_pos - item->pos.z_pos;
            lara_dx = lara_item->pos.x_pos - item->pos.x_pos;
            lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
            cinfo = baddie_slots;
            for (slot = 0; slot < 5; slot++, cinfo++) {
                if (cinfo->item_num == NO_ITEM || cinfo->item_num == item_number)
                    continue;
                target = &items[cinfo->item_num];
                if ((target->object_number != LARA && target->object_number != 50) || target->hit_points <= 0)
                    continue;
                x = target->pos.x_pos - item->pos.x_pos;
                z = target->pos.z_pos - item->pos.z_pos;
                distance = x * x + z * z;
                if (distance < lara_info.distance)
                    seal->enemy = target;
            }
        }
        CreatureAIInfo(item, &info);
        if (seal->enemy == lara_item) {
            lara_info.angle = info.angle;
            lara_info.distance = info.distance;
        } else {
            lara_dz = lara_item->pos.z_pos - item->pos.z_pos;
            lara_dx = lara_item->pos.x_pos - item->pos.x_pos;
            lara_info.angle = phd_atan(lara_dz, lara_dx) - item->pos.y_rot;
            lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
        }
        if (info.zone_number == info.enemy_zone) {
            GetCreatureMood(item, &info, 1);
            if (seal->enemy == lara_item && lara.poisoned >= 0x100)
                seal->mood = ESCAPE_MOOD;
            CreatureMood(item, &info, 1);
        } else {
            GetCreatureMood(item, &info, 0);
            if (seal->enemy == lara_item && lara.poisoned >= 0x100)
                seal->mood = ESCAPE_MOOD;
            CreatureMood(item, &info, 0);
        }
        angle = CreatureTurn(item, seal->maximum_turn);
        real_enemy = seal->enemy;
        seal->enemy = lara_item;
        if (lara_info.distance < 0x100000 || item->hit_status || TargetVisible(item, &lara_info))
            AlertAllGuards(item_number);
        seal->enemy = real_enemy;
        switch (item->current_anim_state) {
        case 0:
            seal->maximum_turn = 0;
            seal->flags = 0;
            head = info.angle;
            if (item->ai_bits & 1) {
                head = AIGuard(seal);
                item->goal_anim_state = 0;
                break;
            } else if (item->ai_bits & 4) {
                item->goal_anim_state = 1;
                head = 0;
            } else if (seal->mood == ESCAPE_MOOD)
                item->goal_anim_state = 1;
            else if (Targetable(item, &info) && info.distance < 0x400000)
                item->goal_anim_state = 2;
            else if (item->required_anim_state)
                item->goal_anim_state = item->required_anim_state;
            else
                item->goal_anim_state = 1;
            break;
        case 1:
            seal->maximum_turn = 3 * ONE_DEGREE;
            if (info.ahead)
                head = info.angle;
            if (item->ai_bits & 4) {
                item->goal_anim_state = 1;
                head = 0;
            } else if (Targetable(item, &info) && info.distance < 0x400000)
                item->goal_anim_state = 0;
            break;
        case 2:
            if (abs(info.angle) < 3 * ONE_DEGREE)
                item->pos.y_rot += info.angle;
            else if (info.angle < 0)
                item->pos.y_rot -= 3 * ONE_DEGREE;
            else
                item->pos.y_rot += 3 * ONE_DEGREE;
            if (info.ahead) {
                torso_y = info.angle >> 1;
                torso_x = info.x_angle;
            }
            if (item->frame_number >= anims[item->anim_number].frame_base + 35 && item->frame_number <= anims[item->anim_number].frame_base + 58) {
                if (seal->flags < 24)
                    seal->flags += 3;
                if (seal->flags < 24)
                    TriggerSealmuteGasThrower_sealmute(item, &seal_gas, seal->flags);
                else
                    TriggerSealmuteGasThrower_sealmute(item, &seal_gas, (GetRandomControl() & 15) + 8);
                if (seal->enemy != lara_item)
                    seal->enemy->hit_status = 1;
            }
        }
    }
    CreatureTilt(item, tilt);
    CreatureJoint(item, 0, torso_x);
    CreatureJoint(item, 1, torso_y);
    CreatureJoint(item, 2, head);
    CreatureAnimation(item_number, angle, tilt);
}
