#include "game.h"
#include <stdlib.h>

extern sint32 level_items;
extern PHD_VECTOR TrigDynamics[3];

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
