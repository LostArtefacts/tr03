#include "game.h"

sint32 GetFreeSpark(void);

void TriggerFlamethrowerFlame(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv, sint32 fxnum);
void phd_GetVectorAngles(sint32 x, sint32 y, sint32 z, PHD_ANGLE *dest);

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

#define FLAMER_STOP 1
#define FLAMER_WALK 2
#define FLAMER_WAIT 4
#define FLAMER_SHOOT2 6
#define FLAMER_DEATH 7
#define FLAMER_AIM2 9
#define FLAMER_AIM3 10
#define FLAMER_SHOOT3 11

#define SQUARE(x) ((x) * (x))
#define FLAMER_WALK_TURN (ONE_DEGREE * 5)
#define FLAMER_SHOOT1_RANGE SQUARE(WALL_L * 4)
#define FLAMER_AWARE_DISTANCE SQUARE(WALL_L)
#define FLAMER_DIE_ANIM 19
#define FLAMER_STOP_ANIM 12
#define SEAL_X_OFFSET 0x800
#define LV_CHAMBER 19
#define FLAMETHROWER_BLOKE 50
#define BURNT_MUTANT 42
#define GUARD 1
#define PATROL1 4

int CreatureActive(sint16 item_number);
void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info);
void GetCreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
void CreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn);
int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);
void CreatureJoint(ITEM_INFO *item, sint16 joint, sint16 required);
void GetAITarget(CREATURE_INFO *creature);
void AlertAllGuards(sint16 item_number);
sint16 AIGuard(CREATURE_INFO *creature);
int TargetVisible(ITEM_INFO *item, AI_INFO *info);
int Targetable(ITEM_INFO *item, AI_INFO *info);
sint16 TriggerFlameThrower(ITEM_INFO *item, BITE_INFO *bite, sint16 speed);
void TriggerPilotFlame(sint32 itemnum);

extern uint32 CurrentLevel;
extern BITE_INFO flamer_gun;

void FlamerControl(sint16 item_number)
{
    PHD_VECTOR pos1;
    ITEM_INFO *item, *target, *real_enemy;
    CREATURE_INFO *flamer, *cinfo;
    sint16 angle, torso_y, torso_x, head, tilt, rnd, meta_mood, slot;
    sint32 lara_dx, lara_dz, x, z, distance, best_distance;
    AI_INFO info, lara_info;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    flamer = (CREATURE_INFO *)item->data;
    torso_y = torso_x = head = angle = tilt = 0;
    pos1.x = flamer_gun.x;
    pos1.y = flamer_gun.y;
    pos1.z = flamer_gun.z;
    GetJointAbsPosition(item, &pos1, flamer_gun.mesh_num);
    rnd = GetRandomControl();
    if (item->current_anim_state != FLAMER_SHOOT2 && item->current_anim_state != FLAMER_SHOOT3) {
        TriggerDynamic(pos1.x, pos1.y, pos1.z, (rnd & 3) + 6, 24 - ((rnd >> 4) & 3), 16 - ((rnd >> 6) & 3), rnd & 3);
        TriggerPilotFlame(item_number);
    } else
        TriggerDynamic(pos1.x, pos1.y, pos1.z, (rnd & 3) + 10, 31 - ((rnd >> 4) & 3), 24 - ((rnd >> 6) & 3), rnd & 7);
    if (item->hit_points <= 0) {
        if (item->current_anim_state != FLAMER_DEATH) {
            item->anim_number = objects[item->object_number].anim_index + FLAMER_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = FLAMER_DEATH;
        }
    } else {
        if (item->ai_bits)
            GetAITarget(flamer);
        else if (flamer->hurt_by_lara || CurrentLevel == LV_CHAMBER)
            flamer->enemy = lara_item;
        else {
            flamer->enemy = NULL;
            best_distance = 0x7fffffff;
            cinfo = baddie_slots;
            for (slot = 0; slot < NUM_SLOTS; slot++, cinfo++) {
                if (cinfo->item_num == NO_ITEM || cinfo->item_num == item_number)
                    continue;
                target = &items[cinfo->item_num];
                if (target->object_number == LARA || target->object_number == WHITE_SOLDIER
                    || target->object_number == FLAMETHROWER_BLOKE || target->hit_points <= 0)
                    continue;
                x = target->pos.x_pos - item->pos.x_pos;
                z = target->pos.z_pos - item->pos.z_pos;
                distance = x * x + z * z;
                if (distance < best_distance) {
                    flamer->enemy = target;
                    best_distance = distance;
                }
            }
        }
        CreatureAIInfo(item, &info);
        if (flamer->enemy == lara_item) {
            lara_info.angle = info.angle;
            lara_info.distance = info.distance;
            if (!flamer->hurt_by_lara && CurrentLevel != LV_CHAMBER)
                flamer->enemy = NULL;
        } else {
            lara_dz = lara_item->pos.z_pos - item->pos.z_pos;
            lara_dx = lara_item->pos.x_pos - item->pos.x_pos;
            lara_info.angle = phd_atan(lara_dz, lara_dx) - item->pos.y_rot;
            lara_info.distance = lara_dz * lara_dz + lara_dx * lara_dx;
            info.x_angle -= SEAL_X_OFFSET;
        }
        meta_mood = 0;
        GetCreatureMood(item, &info, meta_mood);
        CreatureMood(item, &info, meta_mood);
        angle = CreatureTurn(item, flamer->maximum_turn);
        real_enemy = flamer->enemy;
        if (item->hit_status
            || ((lara_info.distance < FLAMER_AWARE_DISTANCE || TargetVisible(item, &lara_info))
                && CurrentLevel == LV_CHAMBER)) {
            if (!flamer->alerted)
                SoundEffect(300, &item->pos, 0);
            AlertAllGuards(item_number);
        }
        switch (item->current_anim_state) {
        case FLAMER_STOP:
            head = lara_info.angle;
            flamer->flags = 0;
            flamer->maximum_turn = 0;
            if (item->ai_bits & GUARD) {
                head = AIGuard(flamer);
                if (!(GetRandomControl() & 0xff))
                    item->goal_anim_state = FLAMER_WAIT;
                break;
            } else if (item->ai_bits & PATROL1)
                item->goal_anim_state = FLAMER_WALK;
            else if (flamer->mood == ESCAPE_MOOD)
                item->goal_anim_state = FLAMER_WALK;
            else if (Targetable(item, &info)
                     && (real_enemy != lara_item || flamer->hurt_by_lara || CurrentLevel == LV_CHAMBER)) {
                if (info.distance < FLAMER_SHOOT1_RANGE)
                    item->goal_anim_state = FLAMER_AIM3;
                else
                    item->goal_anim_state = FLAMER_WALK;
            } else if (flamer->mood == BORED_MOOD && info.ahead && !(GetRandomControl() & 0xff))
                item->goal_anim_state = FLAMER_WAIT;
            else if (flamer->mood == ATTACK_MOOD || !(GetRandomControl() & 0xff))
                item->goal_anim_state = FLAMER_WALK;
            break;
        case FLAMER_WAIT:
            head = lara_info.angle;
            if (item->ai_bits & GUARD) {
                head = AIGuard(flamer);
                if (!(GetRandomControl() & 0xff))
                    item->goal_anim_state = FLAMER_STOP;
                break;
            } else if ((Targetable(item, &info) && info.distance < FLAMER_SHOOT1_RANGE
                        && (real_enemy != lara_item || flamer->hurt_by_lara || CurrentLevel == LV_CHAMBER))
                       || flamer->mood != BORED_MOOD || !(GetRandomControl() & 0xff))
                item->goal_anim_state = FLAMER_STOP;
            break;
        case FLAMER_WALK:
            head = lara_info.angle;
            flamer->flags = 0;
            flamer->maximum_turn = FLAMER_WALK_TURN;
            if (item->ai_bits & GUARD) {
                item->anim_number = objects[item->object_number].anim_index + FLAMER_STOP_ANIM;
                item->frame_number = anims[item->anim_number].frame_base;
                item->current_anim_state = FLAMER_STOP;
                item->goal_anim_state = FLAMER_STOP;
            } else if (item->ai_bits & PATROL1)
                item->goal_anim_state = FLAMER_WALK;
            else if (flamer->mood == ESCAPE_MOOD)
                item->goal_anim_state = FLAMER_WALK;
            else if (Targetable(item, &info)
                     && (real_enemy != lara_item || flamer->hurt_by_lara || CurrentLevel == LV_CHAMBER)) {
                if (info.distance < FLAMER_SHOOT1_RANGE)
                    item->goal_anim_state = FLAMER_STOP;
                else
                    item->goal_anim_state = FLAMER_AIM2;
            } else if (flamer->mood == BORED_MOOD && info.ahead)
                item->goal_anim_state = FLAMER_STOP;
            else
                item->goal_anim_state = FLAMER_WALK;
            break;
        case FLAMER_AIM3:
            flamer->flags = 0;
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
                if (Targetable(item, &info) && info.distance < FLAMER_SHOOT1_RANGE
                    && (real_enemy != lara_item || flamer->hurt_by_lara || CurrentLevel == LV_CHAMBER))
                    item->goal_anim_state = FLAMER_SHOOT3;
                else
                    item->goal_anim_state = FLAMER_STOP;
            }
            break;
        case FLAMER_AIM2:
            flamer->flags = 0;
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
                if (Targetable(item, &info) && info.distance < FLAMER_SHOOT1_RANGE
                    && (real_enemy != lara_item || flamer->hurt_by_lara || CurrentLevel == LV_CHAMBER))
                    item->goal_anim_state = FLAMER_SHOOT2;
                else
                    item->goal_anim_state = FLAMER_WALK;
            }
            break;
        case FLAMER_SHOOT3:
            if (flamer->flags < 40)
                flamer->flags += (flamer->flags >> 2) + 1;
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
                if (Targetable(item, &info) && info.distance < FLAMER_SHOOT1_RANGE
                    && (real_enemy != lara_item || flamer->hurt_by_lara || CurrentLevel == LV_CHAMBER))
                    item->goal_anim_state = FLAMER_SHOOT3;
                else
                    item->goal_anim_state = FLAMER_STOP;
            } else
                item->goal_anim_state = FLAMER_STOP;
            if (flamer->flags < 40)
                TriggerFlameThrower(item, &flamer_gun, flamer->flags);
            else {
                TriggerFlameThrower(item, &flamer_gun, (GetRandomControl() & 31) + 12);
                if (real_enemy) {
                    if (real_enemy->object_number == BURNT_MUTANT)
                        real_enemy->item_flags[0]++;
                }
            }
            SoundEffect(204, &item->pos, 0);
            break;
        case FLAMER_SHOOT2:
            if (flamer->flags < 40)
                flamer->flags += (flamer->flags >> 2) + 1;
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
                if (Targetable(item, &info) && info.distance < FLAMER_SHOOT1_RANGE
                    && (real_enemy != lara_item || flamer->hurt_by_lara || CurrentLevel == LV_CHAMBER))
                    item->goal_anim_state = FLAMER_SHOOT2;
                else
                    item->goal_anim_state = FLAMER_WALK;
            } else
                item->goal_anim_state = FLAMER_WALK;
            if (flamer->flags < 40)
                TriggerFlameThrower(item, &flamer_gun, flamer->flags);
            else {
                TriggerFlameThrower(item, &flamer_gun, (GetRandomControl() & 31) + 12);
                if (real_enemy) {
                    if (real_enemy->object_number == BURNT_MUTANT)
                        real_enemy->item_flags[0]++;
                }
            }
            SoundEffect(204, &item->pos, 0);
            break;
        }
    }
    CreatureTilt(item, tilt);
    CreatureJoint(item, 0, torso_y);
    CreatureJoint(item, 1, torso_x);
    CreatureJoint(item, 2, head);
    CreatureAnimation(item_number, angle, 0);
}

sint16 TriggerFlameThrower(ITEM_INFO *item, BITE_INFO *bite, sint16 speed)
{
    PHD_VECTOR pos1, pos2;
    FX_INFO *fx;
    sint16 fx_number;
    PHD_ANGLE angles[2];
    sint32 xv, yv, zv, vel, spd, lp;

    fx_number = CreateEffect(item->room_number);
    if (fx_number != NO_ITEM) {
        fx = &effects[fx_number];
        pos1.x = bite->x;
        pos1.y = bite->y;
        pos1.z = bite->z;
        GetJointAbsPosition(item, &pos1, bite->mesh_num);
        pos2.x = bite->x;
        pos2.y = bite->y << 1;
        pos2.z = bite->z;
        GetJointAbsPosition(item, &pos2, bite->mesh_num);
        phd_GetVectorAngles(pos2.x - pos1.x, pos2.y - pos1.y, pos2.z - pos1.z, angles);
        fx->pos.x_pos = pos1.x;
        fx->pos.y_pos = pos1.y;
        fx->pos.z_pos = pos1.z;
        fx->room_number = item->room_number;
        fx->pos.x_rot = angles[1];
        fx->pos.z_rot = 0;
        fx->pos.y_rot = angles[0];
        fx->speed = speed << 2;
        fx->object_number = 307;
        fx->counter = 20;
        fx->flag1 = 0;
        TriggerFlamethrowerFlame(0, 0, 0, 0, 0, 0, fx_number);
        for (lp = 0; lp < 2; lp++) {
            spd = (GetRandomControl() % (speed << 2)) + 32;
            vel = (spd * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT;
            zv = (vel * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT;
            xv = (vel * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT;
            yv = -((spd * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT);
            TriggerFlamethrowerFlame(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, xv << 5, yv << 5, zv << 5, -1);
        }
        vel = ((speed << 1) * phd_cos(fx->pos.x_rot)) >> W2V_SHIFT;
        zv = (vel * phd_cos(fx->pos.y_rot)) >> W2V_SHIFT;
        xv = (vel * phd_sin(fx->pos.y_rot)) >> W2V_SHIFT;
        yv = -(((speed << 1) * phd_sin(fx->pos.x_rot)) >> W2V_SHIFT);
        TriggerFlamethrowerFlame(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, xv << 5, yv << 5, zv << 5, -2);
    }
    return fx_number;
}

void TriggerFlamethrowerFlame(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv, sint32 fxnum)
{
    sint32 size;
    SPARKS *sptr;

    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 48 + (GetRandomControl() & 31);
    sptr->sG = sptr->sR;
    sptr->sB = 192 + (GetRandomControl() & 63);
    sptr->dR = 192 + (GetRandomControl() & 63);
    sptr->dG = 128 + (GetRandomControl() & 63);
    sptr->dB = 32;
    if (xv || yv || zv) {
        sptr->ColFadeSpeed = 6;
        sptr->FadeToBlack = 2;
        sptr->sLife = sptr->Life = (GetRandomControl() & 1) + 12;
    } else {
        sptr->ColFadeSpeed = 8;
        sptr->FadeToBlack = 16;
        sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 20;
    }
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 31) - 16);
    sptr->y = y;
    sptr->z = z + ((GetRandomControl() & 31) - 16);
    sptr->Xvel = ((GetRandomControl() & 15) - 16) + xv;
    sptr->Yvel = yv;
    sptr->Zvel = ((GetRandomControl() & 15) - 16) + zv;
    sptr->Friction = 0;
    if (GetRandomControl() & 1) {
        if (fxnum >= 0)
            sptr->Flags = 2 | 8 | 16 | 512 | 64;
        else
            sptr->Flags = 2 | 8 | 16 | 512;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 15) - 16;
        else
            sptr->RotAdd = (GetRandomControl() & 15) + 16;
    } else {
        if (fxnum >= 0)
            sptr->Flags = 2 | 8 | 512 | 64;
        else
            sptr->Flags = 2 | 8 | 512;
    }
    sptr->FxObj = fxnum;
    sptr->Gravity = sptr->MaxYvel = 0;
    sptr->Def = objects[296].mesh_index;
    size = (GetRandomControl() & 31) + 64;
    if (xv || yv || zv) {
        sptr->sWidth = sptr->Width = size >> 5;
        sptr->sHeight = sptr->Height = size >> 5;
        if (fxnum == -2)
            sptr->Scalar = 2;
        else
            sptr->Scalar = 3;
    } else {
        sptr->sWidth = sptr->Width = size >> 4;
        sptr->sHeight = sptr->Height = size >> 4;
        sptr->Scalar = 4;
    }
    sptr->dWidth = size >> 1;
    sptr->dHeight = size >> 1;
}

void TriggerPilotFlame(sint32 itemnum)
{
    sint32 size;
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - items[itemnum].pos.x_pos;
    dz = lara_item->pos.z_pos - items[itemnum].pos.z_pos;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 48 + (GetRandomControl() & 31);
    sptr->sG = sptr->sR;
    sptr->sB = 192 + (GetRandomControl() & 63);
    sptr->dR = 192 + (GetRandomControl() & 63);
    sptr->dG = 128 + (GetRandomControl() & 63);
    sptr->dB = 32;
    sptr->ColFadeSpeed = 12 + (GetRandomControl() & 3);
    sptr->FadeToBlack = 4;
    sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 20;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = (GetRandomControl() & 31) - 16;
    sptr->y = (GetRandomControl() & 31) - 16;
    sptr->z = (GetRandomControl() & 31) - 16;
    sptr->Xvel = (GetRandomControl() & 31) - 16;
    sptr->Yvel = -(GetRandomControl() & 3);
    sptr->Zvel = (GetRandomControl() & 31) - 16;
    sptr->Flags = 2 | 8 | 512 | 128 | 4096;
    sptr->FxObj = itemnum;
    sptr->NodeNumber = 0;
    sptr->Friction = 4;
    sptr->Gravity = -(GetRandomControl() & 3) - 2;
    sptr->MaxYvel = -(GetRandomControl() & 3) - 4;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 0;
    size = (GetRandomControl() & 7) + 32;
    sptr->Width = sptr->sWidth = size >> 1;
    sptr->Height = sptr->sHeight = size >> 1;
    sptr->dWidth = size;
    sptr->dHeight = size;
}

#ifdef TR3_EXE
BITE_INFO flamer_gun = { 0, 340, 64, 7 };
#endif
