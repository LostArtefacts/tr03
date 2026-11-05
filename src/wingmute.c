#include "game.h"

#define WING_START_ANIM 2
#define WING_WAIT 2

void InitialiseWingmute(sint16 item_number)
{
    ITEM_INFO *item;

    InitialiseCreature(item_number);
    item = &items[item_number];
    item->anim_number = objects[MUTANT1].anim_index + WING_START_ANIM;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = WING_WAIT;
    item->item_flags[1] = GetRandomControl() & 0x7f;
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
void TriggerWingMuteParticles(sint16 item_number);

extern BITE_INFO wing_bite;
extern sint16 rcossin_tbl[];

int abs(int x);
#pragma intrinsic(abs)

void WingmuteControl(sint16 item_number)
{
    ITEM_INFO *item;
    CREATURE_INFO *wing;
    AI_INFO info;
    sint16 angle = 0;
    PHD_VECTOR pos;
    sint32 g;

    if (!CreatureActive(item_number))
        return;
    item = &items[item_number];
    wing = (CREATURE_INFO *)item->data;
    if (item->hit_points <= 0) {
        switch (item->current_anim_state) {
        case 5:
            if (item->pos.y_pos > item->floor) {
                item->pos.y_pos = item->floor;
                item->gravity_status = 0;
                item->fallspeed = 0;
                item->goal_anim_state = 6;
            }
            break;
        case 6:
            item->pos.y_pos = item->floor;
            break;
        default:
            item->anim_number = objects[MUTANT1].anim_index + 5;
            item->frame_number = anims[item->anim_number].frame_base;
            item->current_anim_state = 5;
            item->gravity_status = 1;
            item->speed = 0;
            break;
        }
        item->pos.x_rot = 0;
    } else {
        CreatureAIInfo(item, &info);
        GetCreatureMood(item, &info, 1);
        CreatureMood(item, &info, 1);
        angle = CreatureTurn(item, wing->maximum_turn);
        switch (item->current_anim_state) {
        case WING_WAIT:
            item->pos.y_pos = item->floor;
            wing->maximum_turn = 182;
            if (item->hit_status || info.distance < 0x900000 || wing->hurt_by_lara || item->ai_bits == 8)
                item->goal_anim_state = 3;
            break;
        case 1:
            item->pos.y_pos = item->pos.y_pos + 51;
            if (item->pos.y_pos > item->floor)
                item->pos.y_pos = item->floor;
            break;
        case 0:
            wing->flags = 0;
            wing->maximum_turn = 546;
            if (item->required_anim_state)
                item->goal_anim_state = item->required_anim_state;
            else if (item->hit_status || GetRandomControl() < 0x180 || item->ai_bits == 8)
                item->goal_anim_state = 7;
            else if ((wing->mood == BORED_MOOD || GetRandomControl() < 0x80) && !wing->hurt_by_lara &&
                     !(item->ai_bits == 8))
                item->goal_anim_state = 1;
            else if (info.ahead && info.distance < 0x40000)
                item->goal_anim_state = 4;
            break;
        case 7:
            wing->flags = 0;
            wing->maximum_turn = 546;
            if (item->required_anim_state)
                item->goal_anim_state = item->required_anim_state;
            else if ((wing->mood == BORED_MOOD || GetRandomControl() < 0x80) && !wing->hurt_by_lara &&
                     !(item->ai_bits == 8))
                item->goal_anim_state = 0;
            else if (info.ahead && info.distance < 0x40000)
                item->goal_anim_state = 4;
            break;
        case 4:
            wing->maximum_turn = 546;
            if (info.ahead && info.distance < 0x40000)
                item->goal_anim_state = 4;
            else if (info.distance < 0x40000)
                item->goal_anim_state = 0;
            else {
                item->goal_anim_state = 0;
                item->required_anim_state = 7;
            }
            if (!wing->flags && (item->touch_bits & 0x1000)) {
                lara_item->hit_points -= 50;
                lara_item->hit_status = 1;
                CreatureEffect(item, &wing_bite, DoBloodSplat);
                wing->flags = 1;
            }
            break;
        }
    }
    g = abs(rcossin_tbl[item->item_flags[1] << 7] >> 7);
    if (g > 31)
        g = 31;
    pos.x = 0;
    pos.y = 0;
    pos.z = 0;
    GetJointAbsPosition(item, &pos, 10);
    TriggerDynamic(pos.x, pos.y, pos.z, 10, 0, g, 0);
    item->item_flags[1]++;
    item->item_flags[1] &= 63;
    TriggerWingMuteParticles(item_number);
    TriggerWingMuteParticles(item_number);
    CreatureAnimation(item_number, angle, 0);
}

sint32 GetFreeSpark(void);

void TriggerWingMuteParticles(sint16 item_number)
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
    sptr->sG = (GetRandomControl() & 63) + 32;
    sptr->sB = sptr->sG >> 1;
    sptr->sR = sptr->sG >> 2;
    sptr->dG = (GetRandomControl() & 31) + 224;
    sptr->dB = sptr->dG >> 1;
    sptr->dR = sptr->dG >> 2;
    sptr->ColFadeSpeed = 4;
    sptr->FadeToBlack = 2;
    sptr->sLife = sptr->Life = 8;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = (GetRandomControl() & 15) - 8;
    sptr->y = (GetRandomControl() & 15) - 8;
    sptr->z = (GetRandomControl() & 127) - 64;
    sptr->Xvel = (GetRandomControl() & 31) - 16;
    sptr->Yvel = (GetRandomControl() & 31) - 16;
    sptr->Zvel = (GetRandomControl() & 31) - 16;
    sptr->Friction = 2 | (2 << 4);
    sptr->Flags = 2 | 128 | 4096 | 8;
    sptr->Gravity = sptr->MaxYvel = 0;
    sptr->FxObj = item_number;
    sptr->NodeNumber = 1;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 3;
    size = (GetRandomControl() & 3) + 3;
    sptr->Width = sptr->sWidth = size;
    sptr->Height = sptr->sHeight = size;
    sptr->dWidth = size >> 1;
    sptr->dHeight = size >> 1;
}
