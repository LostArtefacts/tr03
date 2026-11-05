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
void CreatureDie(sint16 item_number, int explode);
sint16 CreatureEffect(ITEM_INFO *item, BITE_INFO *bite, sint16 (*generate)(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 yrot, sint16 room_number));
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);
void GetAITarget(CREATURE_INFO *creature);
void AlertAllGuards(sint16 item_number);
sint16 AIGuard(CREATURE_INFO *creature);
int TargetVisible(ITEM_INFO *item, AI_INFO *info);
int Targetable(ITEM_INFO *item, AI_INFO *info);
void TriggerExplosionSparks(sint32 x, sint32 y, sint32 z, sint32 extra_trigs, sint32 dyn, sint32 uw, sint16 roomnum);
void TriggerPlasma_clawmute(sint16 item_number);
void TriggerPlasmaBall_clawmute(ITEM_INFO *item, sint32 type, PHD_VECTOR *pos1, sint16 room_number, sint16 angle);

int abs(int x);
#pragma intrinsic(abs)

extern BITE_INFO claw_bite_left;
extern BITE_INFO claw_bite_right;

void ClawmuteControl(sint16 item_number)
{
    ITEM_INFO *item, *real_enemy;
    CREATURE_INFO *claw;
    sint16 angle, head, torso_y, torso_x, tilt, random;
    sint32 lara_dx, lara_dz;
    AI_INFO info, lara_info;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    claw = (CREATURE_INFO *)item->data;
    head = torso_y = torso_x = angle = tilt = 0;
    if (item->hit_points <= 0) {
        if (item->current_anim_state != 8) {
            item->anim_number = objects[item->object_number].anim_index + 20;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = 8;
        }
        if (item->frame_number == anims[item->anim_number].frame_end - 1) {
            sint32 lp;

            CreatureDie(item_number, 1);
            TriggerExplosionSparks(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 3, -2, 2, 0);
            for (lp = 0; lp < 2; lp++)
                TriggerExplosionSparks(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 3, -1, 2, 0);
            SoundEffect(106, &item->pos, 0);
            return;
        }
    } else {
        if (item->ai_bits)
            GetAITarget(claw);
        CreatureAIInfo(item, &info);
        if (claw->enemy == lara_item) {
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
            CreatureMood(item, &info, 1);
        } else {
            GetCreatureMood(item, &info, 0);
            CreatureMood(item, &info, 0);
        }
        angle = CreatureTurn(item, claw->maximum_turn);
        real_enemy = claw->enemy;
        claw->enemy = lara_item;
        if (lara_info.distance < 0x100000 || item->hit_status || TargetVisible(item, &lara_info))
            AlertAllGuards(item_number);
        claw->enemy = real_enemy;
        switch (item->current_anim_state) {
        case 0:
            claw->maximum_turn = 0;
            claw->flags = 0;
            head = info.angle;
            if (item->ai_bits & 1) {
                head = AIGuard(claw);
                item->goal_anim_state = 0;
                break;
            } else if (item->ai_bits & 4) {
                item->goal_anim_state = 1;
                head = 0;
            } else if (claw->mood == ESCAPE_MOOD)
                item->goal_anim_state = 2;
            else if (info.bite && info.distance < 0x100000) {
                torso_y = info.angle;
                torso_x = info.x_angle;
                if (info.angle < 0)
                    item->goal_anim_state = 6;
                else
                    item->goal_anim_state = 7;
            } else if (info.bite && info.distance < 0x1c6e39) {
                torso_y = info.angle;
                torso_x = info.x_angle;
                item->goal_anim_state = 9;
            } else if (Targetable(item, &info) && ((info.distance > 0x900000 && !item->item_flags[0]) || info.zone_number != info.enemy_zone))
                item->goal_anim_state = 10;
            else if (claw->mood == BORED_MOOD) {
                random = (sint16)GetRandomControl();
                item->goal_anim_state = 1;
            } else if (item->required_anim_state)
                item->goal_anim_state = item->required_anim_state;
            else
                item->goal_anim_state = 2;
            break;
        case 1:
            claw->maximum_turn = 3 * ONE_DEGREE;
            if (info.ahead)
                head = info.angle;
            if (item->ai_bits & 4) {
                item->goal_anim_state = 1;
                head = 0;
            } else if (info.bite && info.distance < 0x1c6e39) {
                claw->maximum_turn = 3 * ONE_DEGREE;
                if (info.angle < 0)
                    item->goal_anim_state = 4;
                else
                    item->goal_anim_state = 5;
            } else if (Targetable(item, &info) && ((info.distance > 0x900000 && !item->item_flags[0]) || info.zone_number != info.enemy_zone)) {
                claw->maximum_turn = 3 * ONE_DEGREE;
                item->goal_anim_state = 0;
            } else if (claw->mood == ESCAPE_MOOD || claw->mood == ATTACK_MOOD)
                item->goal_anim_state = 2;
            break;
        case 2:
            claw->maximum_turn = 4 * ONE_DEGREE;
            if (info.ahead)
                head = info.angle;
            if (item->ai_bits & 1)
                item->goal_anim_state = 0;
            else if (claw->mood == BORED_MOOD)
                item->goal_anim_state = 0;
            else if (claw->flags && info.ahead)
                item->goal_anim_state = 0;
            else if (info.bite && info.distance < 0x400000) {
                if (lara_item->speed == 0)
                    item->goal_anim_state = 0;
                else
                    item->goal_anim_state = 3;
            } else if (Targetable(item, &info) && ((info.distance > 0x900000 && !item->item_flags[0]) || info.zone_number != info.enemy_zone)) {
                claw->maximum_turn = 3 * ONE_DEGREE;
                item->goal_anim_state = 0;
            }
            claw->flags = 0;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 9:
        case 3:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
            }
            if (!claw->flags && (item->touch_bits & 0x90)) {
                lara_item->hit_status = 1;
                lara_item->hit_points -= 100;
                CreatureEffect(item, &claw_bite_left, DoBloodSplat);
                CreatureEffect(item, &claw_bite_right, DoBloodSplat);
                claw->flags = 1;
            }
            item->item_flags[0] = 0;
            break;
        case 10:
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
            if (item->frame_number == anims[item->anim_number].frame_base && !(GetRandomControl() & 3))
                item->item_flags[0] = 1;
            if (item->frame_number - anims[item->anim_number].frame_base < 28)
                TriggerPlasma_clawmute(item_number);
            else if (item->frame_number - anims[item->anim_number].frame_base == 28)
                TriggerPlasmaBall_clawmute(item, 0, NULL, item->room_number, item->pos.y_rot);
            {
                PHD_VECTOR pos1;
                sint32 bright, r, g, b, rnd;

                bright = item->frame_number - anims[item->anim_number].frame_base;
                if (bright > 16) {
                    bright = anims[item->anim_number].frame_base + 28 + 16 - item->frame_number;
                    if (bright > 16)
                        bright = 16;
                }
                if (bright > 0) {
                    rnd = GetRandomControl();
                    b = 31 - ((rnd >> 4) & 3);
                    g = 24 - ((rnd >> 6) & 3);
                    r = rnd & 7;
                    r = (r * bright) >> 4;
                    g = (g * bright) >> 4;
                    b = (b * bright) >> 4;
                    pos1.x = -32;
                    pos1.y = -16;
                    pos1.z = -192;
                    GetJointAbsPosition(item, &pos1, 13);
                    TriggerDynamic(pos1.x, pos1.y, pos1.z, 13, r, g, b);
                }
            }
        }
    }
    CreatureTilt(item, tilt);
    CreatureJoint(item, 0, torso_x);
    CreatureJoint(item, 1, torso_y);
    CreatureJoint(item, 2, head);
    CreatureAnimation(item_number, angle, 0);
}

sint32 GetFreeSpark(void);

void TriggerPlasma_clawmute(sint16 item_number)
{
    sint32 size;
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - items[item_number].pos.x_pos;
    dz = lara_item->pos.z_pos - items[item_number].pos.z_pos;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sB = 255;
    sptr->sG = 48 + (GetRandomControl() & 31);
    sptr->sR = 48;
    sptr->dB = 192 + (GetRandomControl() & 63);
    sptr->dG = 128 + (GetRandomControl() & 63);
    sptr->dR = 32;
    sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
    sptr->FadeToBlack = 8;
    sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 24;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = (GetRandomControl() & 15) - 8;
    sptr->y = 0;
    sptr->z = (GetRandomControl() & 15) - 8;
    sptr->Xvel = (GetRandomControl() & 31) - 16;
    sptr->Yvel = (GetRandomControl() & 15) + 16;
    sptr->Zvel = (GetRandomControl() & 31) - 16;
    sptr->Friction = 3;
    if (GetRandomControl() & 1) {
        sptr->Flags = 2 | 8 | 16 | 128 | 512 | 4096;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 15) - 16;
        else
            sptr->RotAdd = (GetRandomControl() & 15) + 16;
    } else
        sptr->Flags = 2 | 8 | 128 | 512 | 4096;
    sptr->Gravity = (GetRandomControl() & 31) + 16;
    sptr->MaxYvel = (GetRandomControl() & 7) + 16;
    sptr->FxObj = item_number;
    sptr->NodeNumber = 6;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 1;
    size = (GetRandomControl() & 31) + 64;
    sptr->Width = sptr->sWidth = size;
    sptr->Height = sptr->sHeight = size;
    sptr->dWidth = size >> 2;
    sptr->dHeight = size >> 2;
}

void phd_GetVectorAngles(sint32 x, sint32 y, sint32 z, PHD_ANGLE *dest);
sint16 CreateEffect(sint16 room_num);

void TriggerPlasmaBall_clawmute(ITEM_INFO *item, sint32 type, PHD_VECTOR *pos1, sint16 room_number, sint16 angle)
{
    FX_INFO *fx;
    PHD_VECTOR pos;
    sint16 fx_number;
    PHD_ANGLE angles[2];
    sint32 speed;

    if (type == 0) {
        pos.x = -32;
        pos.y = -16;
        pos.z = -192;
        GetJointAbsPosition(item, &pos, 13);
        speed = (GetRandomControl() & 7) + 8;
        phd_GetVectorAngles(lara_item->pos.x_pos - pos.x, lara_item->pos.y_pos - pos.y - 256, lara_item->pos.z_pos - pos.z, angles);
        angles[0] = item->pos.y_rot;
    } else {
        pos.x = pos1->x;
        pos.y = pos1->y;
        pos.z = pos1->z;
        speed = (GetRandomControl() & 15) + 16;
        angles[0] = GetRandomControl() << 1;
        angles[1] = 0x2000;
    }
    fx_number = CreateEffect(room_number);
    if (fx_number != NO_ITEM) {
        fx = &effects[fx_number];
        fx->pos.x_pos = pos.x;
        fx->pos.y_pos = pos.y;
        fx->pos.z_pos = pos.z;
        fx->pos.y_rot = angles[0];
        fx->pos.x_rot = angles[1];
        fx->object_number = 368;
        fx->speed = speed;
        fx->fallspeed = 0;
        fx->flag1 = type;
    }
}

extern sint32 wibble;
void TriggerPlasmaBallFlame_clawmute(sint16 fx_number, sint32 type, sint32 xv, sint32 yv, sint32 zv);
void KillEffect(sint16 fx_num);
void EffectNewRoom(sint16 fx_num, sint16 room_number);
int ItemNearLara(PHD_3DPOS *pos, sint32 radius);

void ControlClawmutePlasmaBall(sint16 fx_number)
{
    FX_INFO *fx;
    FLOOR_INFO *floor;
    sint32 old_x, old_y, old_z, x, speed;
    sint32 rnd, r, g, b;
    uint8 radtab[2] = {13, 7};
    sint16 room_number;

    fx = &effects[fx_number];
    old_x = fx->pos.x_pos;
    old_y = fx->pos.y_pos;
    old_z = fx->pos.z_pos;
    if (fx->speed < 384 && fx->flag1 == 0)
        fx->speed += (fx->speed >> 3) + 4;
    if (fx->flag1 == 1) {
        fx->fallspeed++;
        if (fx->speed > 8)
            fx->speed -= 2;
        if (fx->pos.x_rot > -0x3c00)
            fx->pos.x_rot -= 0x100;
    }
    speed = (fx->speed * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT;
    fx->pos.z_pos += (speed * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT;
    fx->pos.x_pos += (speed * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT;
    fx->pos.y_pos += -((fx->speed * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT) + fx->fallspeed;
    if (wibble & 4) {
        if (fx->flag1 == 0)
            TriggerPlasmaBallFlame_clawmute(fx_number, fx->flag1, 0, 0, 0);
        else
            TriggerPlasmaBallFlame_clawmute(fx_number, fx->flag1, 0, abs(old_y - fx->pos.y_pos) << 3, 0);
    }
    room_number = fx->room_number;
    floor = GetFloor(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, &room_number);
    if (fx->pos.y_pos >= GetHeight(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos) || fx->pos.y_pos < GetCeiling(floor, fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos)) {
        PHD_VECTOR pos;

        if (fx->flag1 == 0) {
            pos.x = old_x;
            pos.y = old_y;
            pos.z = old_z;
            r = 5 + (GetRandomControl() & 3);
            for (x = 0; x < r; x++)
                TriggerPlasmaBall_clawmute(NULL, 1, &pos, fx->room_number, fx->pos.y_rot);
        }
        KillEffect(fx_number);
        return;
    }
    if (room[room_number].flags & UNDERWATER) {
        KillEffect(fx_number);
        return;
    }
    if (ItemNearLara(&fx->pos, 200) && fx->flag1 == 0) {
        PHD_VECTOR pos;

        pos.x = fx->pos.x_pos;
        pos.y = fx->pos.y_pos;
        pos.z = fx->pos.z_pos;
        r = 3 + (GetRandomControl() & 1);
        for (x = 0; x < r; x++)
            TriggerPlasmaBall_clawmute(NULL, 1, &pos, fx->room_number, fx->pos.y_rot);
        lara_item->hit_points -= 200;
        lara_item->hit_status = 1;
        KillEffect(fx_number);
        return;
    }
    if (room_number != fx->room_number)
        EffectNewRoom(fx_number, lara_item->room_number);
    if (radtab[fx->flag1]) {
        rnd = GetRandomControl();
        b = 31 - ((rnd >> 4) & 3);
        g = 24 - ((rnd >> 6) & 3);
        r = rnd & 7;
        TriggerDynamic(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, radtab[fx->flag1], r, g, b);
    }
}

void TriggerPlasmaBallFlame_clawmute(sint16 fx_number, sint32 type, sint32 xv, sint32 yv, sint32 zv)
{
    SPARKS *sptr;
    sint32 size;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - effects[fx_number].pos.x_pos;
    dz = lara_item->pos.z_pos - effects[fx_number].pos.z_pos;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sB = 255;
    sptr->sG = 48 + (GetRandomControl() & 31);
    sptr->sR = 48;
    sptr->dB = 192 + (GetRandomControl() & 63);
    sptr->dG = 128 + (GetRandomControl() & 63);
    sptr->dR = 32;
    sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
    sptr->FadeToBlack = 8;
    sptr->sLife = sptr->Life = (GetRandomControl() & 7) + 24;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = (GetRandomControl() & 15) - 8;
    sptr->y = 0;
    sptr->z = (GetRandomControl() & 15) - 8;
    sptr->Xvel = xv + (GetRandomControl() & 255) - 128;
    sptr->Yvel = yv;
    sptr->Zvel = zv + (GetRandomControl() & 255) - 128;
    sptr->Friction = 5;
    if (GetRandomControl() & 1) {
        sptr->Flags = 2 | 8 | 16 | 512 | 64;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 15) - 16;
        else
            sptr->RotAdd = (GetRandomControl() & 15) + 16;
    } else {
        sptr->Flags = 2 | 8 | 512 | 64;
    }
    sptr->FxObj = fx_number;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 1;
    size = (GetRandomControl() & 31) + 64;
    sptr->Width = sptr->sWidth = size;
    sptr->Height = sptr->sHeight = size;
    sptr->dWidth = size >> 2;
    sptr->dHeight = size >> 2;
    if (type == 0) {
        sptr->Yvel = (GetRandomControl() & 511) - 256;
        sptr->Xvel <<= 1;
        sptr->Zvel <<= 1;
        sptr->Scalar = 2;
        sptr->Friction = 5 | (5 << 4);
        sptr->dWidth >>= 1;
        sptr->dHeight >>= 1;
    }
    sptr->Gravity = sptr->MaxYvel = 0;
}
