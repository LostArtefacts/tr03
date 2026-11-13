#include "game.h"

#define SHIVA_START_ANIM 14
#define SHIVA_START 4

sint32 GetFreeSpark(void);

extern sint32 effect_mesh;
extern sint32 *bones;

int GetFrames(ITEM_INFO *item, sint16 *frmptr[], int *rate);
void S_PrintShadow(int size, sint16 *bptr, ITEM_INFO *item, int underwater);
void phd_PushMatrix(void);
void phd_TranslateAbs(sint32 x, sint32 y, sint32 z);
void phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void phd_RotX(sint16 rx);
void phd_RotY(sint16 ry);
void phd_RotZ(sint16 rz);
void phd_RotYXZ(sint16 ry, sint16 rx, sint16 rz);
int S_GetObjectBounds(sint16 *bptr);
void CalculateObjectLighting(ITEM_INFO *item, sint16 *frame);
void gar_RotYXZsuperpack(sint16 **pprot, int skip);
void phd_PutPolygons(sint16 *objptr, int clip);
void InitInterpolate(int frac, int rate);
void phd_TranslateRel_ID(sint32 x, sint32 y, sint32 z, sint32 x2, sint32 y2, sint32 z2);
void gar_RotYXZsuperpack_I(sint16 **pprot1, sint16 **pprot2, int skip);
void phd_PutPolygons_I(sint16 *ptr, int clip);
void phd_PopMatrix_I(void);
void phd_PushMatrix_I(void);
void phd_TranslateRel_I(sint32 x, sint32 y, sint32 z);
void phd_RotX_I(sint16 ang);
void phd_RotY_I(sint16 ang);
void phd_RotZ_I(sint16 ang);

void TriggerShivaSmoke(sint32 x, sint32 y, sint32 z, sint32 uw);

void DrawShiva(ITEM_INFO *item)
{
    int clip, i, poppush;
    OBJECT_INFO *object;
    sint32 *bone;
    sint16 *rotation1, *rotation2;
    sint16 *extra_rotation;
    sint16 **meshpp, **swappp;
    uint32 bit;
    sint16 *frmptr[2];
    int rate, frac;
    PHD_VECTOR pos;

    frac = GetFrames(item, frmptr, &rate);
    if (item->hit_points <= 0 && item->status != ACTIVE && item->mesh_bits != 0) {
        item->mesh_bits = item->mesh_bits >> 1;
        pos.x = 0;
        pos.y = 0;
        pos.z = 256;
        GetJointAbsPosition(item, &pos, effect_mesh++);
        TriggerShivaSmoke(pos.x, pos.y, pos.z, 1);
    }

    object = &objects[item->object_number];
    if (object->shadow_size)
        S_PrintShadow(object->shadow_size, frmptr[0], item, 0);

    phd_PushMatrix();
    phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
    if ((clip = S_GetObjectBounds(frmptr[0]))) {
        CalculateObjectLighting(item, frmptr[0]);
        extra_rotation = (sint16 *)item->data;
        meshpp = &meshes[objects[70].mesh_index];
        swappp = &meshes[objects[122].mesh_index];
        bone = bones + object->bone_index;
        bit = 1;
        if (!frac) {
            phd_TranslateRel((sint32)*(frmptr[0] + 6), (sint32)*(frmptr[0] + 7), (sint32)*(frmptr[0] + 8));
            rotation1 = frmptr[0] + 9;
            gar_RotYXZsuperpack(&rotation1, 0);
            if (bit & item->mesh_bits)
                phd_PutPolygons(*meshpp, clip);
            else
                phd_PutPolygons(*swappp, clip);
            meshpp++;
            swappp++;
            for (i = object->nmeshes - 1; i > 0; i--, bone += 4, meshpp++, swappp++) {
                poppush = *bone;
                if (poppush & 1)
                    phd_PopMatrix();
                if (poppush & 2)
                    phd_PushMatrix();
                phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
                gar_RotYXZsuperpack(&rotation1, 0);
                if (extra_rotation && (poppush & 0x1c)) {
                    if (poppush & 8)
                        phd_RotY(*(extra_rotation++));
                    if (poppush & 4)
                        phd_RotX(*(extra_rotation++));
                    if (poppush & 16)
                        phd_RotZ(*(extra_rotation++));
                }
                bit <<= 1;
                if (bit & item->mesh_bits)
                    phd_PutPolygons(*meshpp, clip);
                else
                    phd_PutPolygons(*swappp, clip);
            }
        } else {
            InitInterpolate(frac, rate);
            phd_TranslateRel_ID((sint32)*(frmptr[0] + 6), (sint32)*(frmptr[0] + 7), (sint32)*(frmptr[0] + 8),
                                (sint32)*(frmptr[1] + 6), (sint32)*(frmptr[1] + 7), (sint32)*(frmptr[1] + 8));
            rotation1 = frmptr[0] + 9;
            rotation2 = frmptr[1] + 9;
            gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
            if (bit & item->mesh_bits)
                phd_PutPolygons_I(*meshpp, clip);
            else
                phd_PutPolygons_I(*swappp, clip);
            meshpp++;
            swappp++;
            for (i = object->nmeshes - 1; i > 0; i--, bone += 4, meshpp++, swappp++) {
                poppush = *bone;
                if (poppush & 1)
                    phd_PopMatrix_I();
                if (poppush & 2)
                    phd_PushMatrix_I();
                phd_TranslateRel_I(*(bone + 1), *(bone + 2), *(bone + 3));
                gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
                if (extra_rotation && (poppush & 0x1c)) {
                    if (poppush & 8)
                        phd_RotY_I(*(extra_rotation++));
                    if (poppush & 4)
                        phd_RotX_I(*(extra_rotation++));
                    if (poppush & 16)
                        phd_RotZ_I(*(extra_rotation++));
                }
                bit <<= 1;
                if (bit & item->mesh_bits)
                    phd_PutPolygons_I(*meshpp, clip);
                else if (item->hit_points > 0 || item->status == ACTIVE || bit != 0x400 || item->carried_item == -1)
                    phd_PutPolygons_I(*swappp, clip);
            }
        }
    }
    phd_PopMatrix();
}

void TriggerShivaSmoke(sint32 x, sint32 y, sint32 z, sint32 uw)
{
    sint32 size;
    SPARKS *sptr;
    sint32 dx, dz;

    dx = lara_item->pos.x_pos - x;
    dz = lara_item->pos.z_pos - z;
    if (dx < -0x4000 || dx > 0x4000 || dz < -0x4000 || dz > 0x4000)
        return;
    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    if (uw) {
        sptr->sR = 0;
        sptr->sG = 0;
        sptr->sB = 0;
        sptr->dR = 192;
        sptr->dG = 192;
        sptr->dB = 208;
    } else {
        sptr->sR = 144;
        sptr->sG = 144;
        sptr->sB = 144;
        sptr->dR = 64;
        sptr->dG = 64;
        sptr->dB = 64;
    }
    sptr->ColFadeSpeed = 8;
    sptr->FadeToBlack = 64;
    sptr->sLife = sptr->Life = (GetRandomControl() & 31) + 96;
    if (uw)
        sptr->TransType = 2;
    else
        sptr->TransType = 3;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + (GetRandomControl() & 31) - 16;
    sptr->y = y + (GetRandomControl() & 31) - 16;
    sptr->z = z + (GetRandomControl() & 31) - 16;
    sptr->Xvel = ((GetRandomControl() & 4095) - 2048) >> 2;
    sptr->Yvel = (GetRandomControl() & 255) - 128;
    sptr->Zvel = ((GetRandomControl() & 4095) - 2048) >> 2;
    if (uw) {
        sptr->Yvel >>= 4;
        sptr->y += 32;
        sptr->Friction = 4 | (1 << 4);
    } else
        sptr->Friction = 6;
    sptr->Flags = 2 | 8 | 16 | 512;
    sptr->RotAng = GetRandomControl() & 4095;
    if (GetRandomControl() & 1)
        sptr->RotAdd = -(GetRandomControl() & 15) - 16;
    else
        sptr->RotAdd = (GetRandomControl() & 15) + 16;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 3;
    if (uw)
        sptr->Gravity = sptr->MaxYvel = 0;
    else {
        sptr->Gravity = -(GetRandomControl() & 3) - 3;
        sptr->MaxYvel = -(GetRandomControl() & 3) - 4;
    }
    size = (GetRandomControl() & 31) + 128;
    sptr->Width = sptr->sWidth = size >> 2;
    sptr->dWidth = size;
    size += (GetRandomControl() & 31) + 32;
    sptr->Height = sptr->sHeight = size >> 3;
    sptr->dHeight = size;
}

void InitialiseShiva(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (item->object_number == SHIVA)
    {
        item->anim_number = objects[SHIVA].anim_index + SHIVA_START_ANIM;
        item->frame_number = anims[item->anim_number].frame_base;
        item->current_anim_state = item->goal_anim_state = SHIVA_START;
    }
    item->status = NOT_ACTIVE;
    item->mesh_bits = 0;
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

#define SHIVA_WAIT 0
#define SHIVA_WALK 1
#define SHIVA_WAIT_DEF 2
#define SHIVA_WALK_DEF 3
#define SHIVA_PINCER 5
#define SHIVA_KILL 6
#define SHIVA_CHOPPER 7
#define SHIVA_WALKBACK 8
#define SHIVA_DEATH 9

#define SHIVA_WALK_TURN (ONE_DEGREE * 4)
#define SHIVA_WALK_CHANCE 0x400
#define SHIVA_PINCER_RANGE SQUARE(WALL_L * 5 / 4)
#define SHIVA_CHOPPER_RANGE SQUARE(WALL_L * 4 / 3)
#define SHIVA_DIE_ANIM 22
#define SHIVA_KILL_ANIM 18
#define SHIVA_HURT_FLAG 4
#define SHIVA_PINCER_DAMAGE 150
#define SHIVA_CHOPPER_DAMAGE 180
#define SQUARE(x) ((x) * (x))

int CreatureActive(sint16 item_number);
void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info);
void GetCreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
void CreatureMood(ITEM_INFO *item, AI_INFO *info, int violent);
sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn);
int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);
void CreatureJoint(ITEM_INFO *item, sint16 joint, sint16 required);
void CreatureKill(ITEM_INFO *item, int kill_anim, int kill_state, int lara_kill_state);
void TriggerExplosionSparks(sint32 x, sint32 y, sint32 z, sint32 extra_trigs, sint32 dyn, sint32 uw, sint16 roomnum);
void ShivaDamage(ITEM_INFO *item, CREATURE_INFO *shiva, int damage);

#define SHIVA_TOUCHL 0x2400
#define SHIVA_TOUCHR 0x2400000

extern BITE_INFO shiva_left;
extern BITE_INFO shiva_right;

sint16 CreatureEffect(ITEM_INFO *item, BITE_INFO *bite, sint16 (*generate)(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 yrot, sint16 room_number));
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);

void ShivaControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *shiva;
    sint16 angle, head_x, head_y, torso_x, torso_y, tilt, room_number;
    sint32 x, z;
    int random, lara_alive;
    AI_INFO info;
    PHD_VECTOR pos;
    FLOOR_INFO *floor;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    shiva = (CREATURE_INFO *)item->data;
    head_x = head_y = torso_x = torso_y = angle = tilt = 0;
    lara_alive = (lara_item->hit_points > 0);
    if (item->hit_points <= 0) {
        if (item->current_anim_state != SHIVA_DEATH) {
            item->anim_number = objects[item->object_number].anim_index + SHIVA_DIE_ANIM;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = SHIVA_DEATH;
        }
    } else {
        CreatureAIInfo(item, &info);
        GetCreatureMood(item, &info, 1);
        CreatureMood(item, &info, 1);
        if (shiva->mood == ESCAPE_MOOD) {
            shiva->target.x = lara_item->pos.x_pos;
            shiva->target.z = lara_item->pos.z_pos;
        }
        angle = CreatureTurn(item, shiva->maximum_turn);
        if (item->current_anim_state != SHIVA_START)
            item->mesh_bits = 0xffffffff;
        switch (item->current_anim_state) {
        case SHIVA_START:
            shiva->maximum_turn = 0;
            if (!shiva->flags) {
                if (item->mesh_bits == 0)
                    effect_mesh = 0;
                item->mesh_bits = (item->mesh_bits << 1) + 1;
                shiva->flags = 1;
                pos.x = 0;
                pos.y = 0;
                pos.z = 256;
                GetJointAbsPosition(item, &pos, effect_mesh++);
                TriggerExplosionSparks(pos.x, pos.y, pos.z, 2, 0, 0, item->room_number);
                TriggerShivaSmoke(pos.x, pos.y, pos.z, 1);
            } else
                shiva->flags--;
            if (item->mesh_bits == 0x7fffffff) {
                item->goal_anim_state = SHIVA_WAIT;
                effect_mesh = 0;
                shiva->flags = -45;
            }
            break;
        case SHIVA_WAIT:
            if (info.ahead)
                head_y = info.angle;
            if (shiva->flags < 0) {
                shiva->flags++;
                TriggerShivaSmoke(item->pos.x_pos + (GetRandomControl() & 0x5ff) - 0x300,
                                  pos.y - (GetRandomControl() & 0x5ff),
                                  item->pos.z_pos + (GetRandomControl() & 0x5ff) - 0x300, 1);
                break;
            }
            if (shiva->flags == 1)
                shiva->flags = 0;
            shiva->maximum_turn = 0;
            if (shiva->mood == ESCAPE_MOOD) {
                room_number = item->room_number;
                x = item->pos.x_pos + (WALL_L * phd_sin(item->pos.y_rot + 0x8000) >> W2V_SHIFT);
                z = item->pos.z_pos + (WALL_L * phd_cos(item->pos.y_rot + 0x8000) >> W2V_SHIFT);
                floor = GetFloor(x, item->pos.y_pos, z, &room_number);
                if (!shiva->flags && floor->box != NO_BOX && !(boxes[floor->box].overlap_index & 0x8000))
                    item->goal_anim_state = SHIVA_WALKBACK;
                else
                    item->goal_anim_state = SHIVA_WAIT_DEF;
            } else if (shiva->mood == BORED_MOOD) {
                random = GetRandomControl();
                if (random < SHIVA_WALK_CHANCE)
                    item->goal_anim_state = SHIVA_WALK;
            } else if (info.bite && info.distance < SHIVA_PINCER_RANGE) {
                item->goal_anim_state = SHIVA_PINCER;
                shiva->flags = 0;
            } else if (info.bite && info.distance < SHIVA_CHOPPER_RANGE) {
                item->goal_anim_state = SHIVA_CHOPPER;
                shiva->flags = 0;
            } else if (item->hit_status && info.ahead) {
                shiva->flags = SHIVA_HURT_FLAG;
                item->goal_anim_state = SHIVA_WAIT_DEF;
            } else
                item->goal_anim_state = SHIVA_WALK;
            break;
        case SHIVA_WAIT_DEF:
            if (info.ahead)
                head_y = info.angle;
            shiva->maximum_turn = 0;
            if (item->hit_status || shiva->mood == ESCAPE_MOOD)
                shiva->flags = SHIVA_HURT_FLAG;
            if ((info.bite && info.distance < SHIVA_CHOPPER_RANGE)
                || (item->frame_number == anims[item->anim_number].frame_base && !shiva->flags) || !info.ahead) {
                item->goal_anim_state = SHIVA_WAIT;
                shiva->flags = 0;
            } else if (shiva->flags)
                item->goal_anim_state = SHIVA_WAIT_DEF;
            if (item->frame_number == anims[item->anim_number].frame_base && shiva->flags > 1)
                shiva->flags -= 2;
            break;
        case SHIVA_WALK:
            if (info.ahead)
                head_y = info.angle;
            shiva->maximum_turn = SHIVA_WALK_TURN;
            if (shiva->mood == ESCAPE_MOOD)
                item->goal_anim_state = SHIVA_WAIT;
            else if (shiva->mood == BORED_MOOD)
                item->goal_anim_state = SHIVA_WAIT;
            else if (info.bite && info.distance < SHIVA_CHOPPER_RANGE) {
                item->goal_anim_state = SHIVA_WAIT;
                shiva->flags = 0;
            } else if (item->hit_status) {
                shiva->flags = SHIVA_HURT_FLAG;
                item->goal_anim_state = SHIVA_WALK_DEF;
            }
            break;
        case SHIVA_WALK_DEF:
            if (info.ahead)
                head_y = info.angle;
            shiva->maximum_turn = SHIVA_WALK_TURN;
            if (item->hit_status)
                shiva->flags = SHIVA_HURT_FLAG;
            if ((info.bite && info.distance < SHIVA_PINCER_RANGE)
                || (item->frame_number == anims[item->anim_number].frame_base && !shiva->flags)) {
                item->goal_anim_state = SHIVA_WALK;
                shiva->flags = 0;
            } else if (shiva->flags)
                item->goal_anim_state = SHIVA_WALK_DEF;
            if (item->frame_number == anims[item->anim_number].frame_base)
                shiva->flags = 0;
            break;
        case SHIVA_WALKBACK:
            if (info.ahead)
                head_y = info.angle;
            shiva->maximum_turn = SHIVA_WALK_TURN;
            if ((info.ahead && info.distance < SHIVA_CHOPPER_RANGE)
                || (item->frame_number == anims[item->anim_number].frame_base && !shiva->flags))
                item->goal_anim_state = SHIVA_WAIT;
            else if (item->hit_status) {
                shiva->flags = SHIVA_HURT_FLAG;
                item->goal_anim_state = SHIVA_WAIT;
            }
            break;
        case SHIVA_PINCER:
            if (info.ahead) {
                torso_y = info.angle;
                torso_x = info.x_angle;
                head_y = info.angle;
            }
            shiva->maximum_turn = SHIVA_WALK_TURN;
            ShivaDamage(item, shiva, SHIVA_PINCER_DAMAGE);
            break;
        case SHIVA_CHOPPER:
            torso_y = info.angle;
            head_y = info.angle;
            if (info.x_angle > 0)
                torso_x = info.x_angle;
            shiva->maximum_turn = SHIVA_WALK_TURN;
            ShivaDamage(item, shiva, SHIVA_CHOPPER_DAMAGE);
            break;
        case SHIVA_KILL:
            torso_y = torso_x = head_x = head_y = shiva->maximum_turn = 0;
            if (item->frame_number == anims[item->anim_number].frame_base + 10
                || item->frame_number == anims[item->anim_number].frame_base + 21
                || item->frame_number == anims[item->anim_number].frame_base + 33) {
                CreatureEffect(item, &shiva_right, DoBloodSplat);
                CreatureEffect(item, &shiva_left, DoBloodSplat);
            }
        }
    }
    if (lara_alive && lara_item->hit_points <= 0) {
        CreatureKill(item, SHIVA_KILL_ANIM, SHIVA_KILL, 2);
        return;
    }
    CreatureTilt(item, tilt);
    head_y -= torso_y;
    CreatureJoint(item, 0, torso_y);
    CreatureJoint(item, 1, torso_x);
    CreatureJoint(item, 2, head_y);
    CreatureJoint(item, 3, head_x);
    CreatureAnimation(item_number, angle, 0);
}

void ShivaDamage(ITEM_INFO *item, CREATURE_INFO *shiva, int damage)
{
    if (!shiva->flags && (item->touch_bits & SHIVA_TOUCHR)) {
        lara_item->hit_points -= damage;
        lara_item->hit_status = 1;
        CreatureEffect(item, &shiva_right, DoBloodSplat);
        shiva->flags = 1;
        SoundEffect(318, &item->pos, 0);
    }
    if (!shiva->flags && (item->touch_bits & SHIVA_TOUCHL)) {
        lara_item->hit_points -= damage;
        lara_item->hit_status = 1;
        CreatureEffect(item, &shiva_left, DoBloodSplat);
        shiva->flags = 1;
        SoundEffect(318, &item->pos, 0);
    }
}
