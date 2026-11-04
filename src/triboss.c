#include "game.h"

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
