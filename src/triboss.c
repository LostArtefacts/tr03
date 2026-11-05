#include "game.h"
#include <stdlib.h>

extern sint32 level_items;
extern PHD_VECTOR TrigDynamics[3];

typedef struct {
    sint16 x;
    sint16 y;
    sint16 z;
    uint8 rsub;
    uint8 gsub;
    uint8 bsub;
    uint8 pad[3];
    sint32 rgb;
} SHIELD_POINTS;

extern sint16 rcossin_tbl[];
extern sint32 TribeBossRadii[5];
extern sint32 TribeBossHeights[5];
extern SHIELD_POINTS TribeBossShield[40];
extern char TribeBossShieldOn;
extern char lizard_man_active;
extern char shield_active;

void UpdateElectricityPoints(void);
void TriggerTribeBossHeadElectricity(ITEM_INFO *item, sint32 copy);
void DrawTribeBossShield(ITEM_INFO *item);

void S_DrawTribeBoss(ITEM_INFO *item)
{
    DrawAnimatingItem(item);
    UpdateElectricityPoints();
    TrigDynamics[0].x = 0;
    TrigDynamics[1].x = 0;
    TrigDynamics[2].x = 0;
    if (!bossdata.explode_count)
    {
        TriggerTribeBossHeadElectricity(item, 0);
        TriggerTribeBossHeadElectricity(item, 1);
    }
    else
        DrawExplosionRings();
    DrawTribeBossShield(item);
}

void TribeBossDie(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    item->collidable = 0;
    item->hit_points = DONT_TARGET;
    KillItem(item_number);
    DisableBaddieAI(item_number);
    item->flags |= ONESHOT;
}

sint16 FindLizardManItemNumber(sint16 room_number);

void InitialiseTribeBoss(sint16 item_number)
{
    SHIELD_POINTS *shptr;
    sint32 lp, lp1, angle, y;

    bossdata.LizmanItem = FindLizardManItemNumber(items[item_number].room_number);
    bossdata.LizmanRoom = items[bossdata.LizmanItem].room_number;
    for (lp = 0; lp < 3; lp++)
        TrigDynamics[lp].x = 0;
    bossdata.attack_count = bossdata.attack_flag = bossdata.death_count = bossdata.attack_head_count =
        lizard_man_active = TribeBossShieldOn = shield_active = bossdata.explode_count =
        bossdata.ring_count = bossdata.dead = bossdata.dropped_icon = 0;
    shptr = &TribeBossShield[0];
    for (lp = 0; lp < 5; lp++) {
        y = TribeBossHeights[lp];
        angle = 0;
        for (lp1 = 0; lp1 < 8; lp1++) {
            shptr->x = (rcossin_tbl[angle << 1] * TribeBossRadii[lp]) >> 11;
            shptr->y = y;
            shptr->z = (rcossin_tbl[(angle << 1) + 1] * TribeBossRadii[lp]) >> 11;
            shptr->rgb = 0;
            angle += 512;
            shptr++;
        }
    }
}

void CreatureJoint(ITEM_INFO *item, sint16 joint, sint16 required);

void RotateHeadXAngle(ITEM_INFO *item)
{
    PHD_VECTOR pos1, pos2;
    sint32 x, y, z;
    sint16 angle;

    pos1.x = pos1.y = pos1.z = 0;
    GetJointAbsPosition(lara_item, &pos1, 0);
    pos2.x = pos2.y = pos2.z = 0;
    GetJointAbsPosition(item, &pos2, 0);
    x = abs(pos2.x - pos1.x);
    y = pos2.y - pos1.y;
    z = abs(pos2.z - pos1.z);
    x = phd_sqrt(x * x + z * z);
    angle = phd_atan(x, y);
    if (abs(angle) < 0x2000)
        CreatureJoint(item, 2, angle);
    else
        CreatureJoint(item, 2, 0);
}

extern sint32 lizman_summon_coords[][4];

int EnableBaddieAI(sint16 item_number, int Always);
void RemoveDrawnItem(sint16 item_num);

void TriggerLizardMan(void)
{
    ROOM_INFO *r;
    ITEM_INFO *item;
    sint16 room_number;

    item = &items[bossdata.LizmanItem];
    item->object_number = LIZARD_MAN;
    item->room_number = bossdata.LizmanRoom;
    item->pos.x_pos = lizman_summon_coords[bossdata.attack_type - 1][0];
    item->pos.y_pos = lizman_summon_coords[bossdata.attack_type - 1][1];
    item->pos.z_pos = lizman_summon_coords[bossdata.attack_type - 1][2];
    item->anim_number = objects[item->object_number].anim_index;
    item->frame_number = anims[item->anim_number].frame_base;
    item->current_anim_state = item->goal_anim_state = anims[item->anim_number].current_anim_state;
    item->required_anim_state = 0;
    item->pos.y_rot = (bossdata.attack_type == 1) ? 0x8000 : 0;
    item->pos.x_rot = item->pos.z_rot = 0;
    item->flags = item->timer = 0;
    item->data = NULL;
    item->mesh_bits = 0xffffffff;
    item->hit_points = objects[item->object_number].hit_points;
    item->collidable = 1;
    item->active = 0;
    item->status = ACTIVE;
    AddActiveItem(bossdata.LizmanItem);
    EnableBaddieAI(bossdata.LizmanItem, 1);
    room_number = item->room_number;
    GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (item->room_number != room_number)
        ItemNewRoom(bossdata.LizmanItem, room_number);
    lizard_man_active = 1;
    RemoveDrawnItem(bossdata.LizmanItem);
    r = &room[item->room_number];
    item->next_item = r->item_number;
    r->item_number = bossdata.LizmanItem;
}

sint32 GetFreeSpark(void);

void TriggerSummonSmoke(sint32 x, sint32 y, sint32 z)
{
    SPARKS *sptr;
    sint32 size;

    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 16;
    sptr->sG = 64;
    sptr->sB = 0;
    sptr->dR = 8;
    sptr->dG = 32;
    sptr->dB = 0;
    sptr->ColFadeSpeed = 16 + (GetRandomControl() & 7);
    sptr->FadeToBlack = 64;
    sptr->sLife = sptr->Life = (GetRandomControl() & 15) + 96;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 127) - 64);
    sptr->y = y - (GetRandomControl() & 31);
    sptr->z = z + ((GetRandomControl() & 127) - 64);
    sptr->Xvel = (GetRandomControl() & 255) - 128;
    sptr->Yvel = -(GetRandomControl() & 15) - 16;
    sptr->Zvel = (GetRandomControl() & 255) - 128;
    sptr->Friction = 0;
    if (GetRandomControl() & 1) {
        sptr->Flags = 2 | 8 | 16 | 512 | 256;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 7) - 4;
        else
            sptr->RotAdd = (GetRandomControl() & 7) + 4;
    } else
        sptr->Flags = 2 | 8 | 512 | 256;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 3;
    sptr->Gravity = -(GetRandomControl() & 7) - 8;
    sptr->MaxYvel = -(GetRandomControl() & 7) - 4;
    size = (GetRandomControl() & 31) + 128;
    sptr->Width = sptr->sWidth = size >> 1;
    sptr->dWidth = size;
    size += (GetRandomControl() & 31) + 32;
    sptr->Height = sptr->sHeight = size >> 1;
    sptr->dHeight = size;
}

sint16 FindLizardManItemNumber(sint16 room_number)
{
    ITEM_INFO *target;
    sint16 target_number;

    for (target_number = 0; target_number < level_items; target_number++)
    {
        target = &items[target_number];
        if (target->object_number == LIZARD_MAN && target->room_number == room_number)
            return target_number;
    }
    return -1;
}
