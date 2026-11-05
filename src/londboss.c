#include "game.h"

void DrawLondonBossShield(ITEM_INFO *item);
void DrawSummonRings(void);
void DrawKnockBackRings(void);
void UpdateElectricityPoints(void);
void LaraElectricDeath(sint32 copy, ITEM_INFO *item);

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
extern sint32 LondonBossRadii[5];
extern sint32 LondonBossHeights[5];
extern SHIELD_POINTS LondonBossShield[40];

void S_DrawLondonBoss(ITEM_INFO *item)
{
    DrawAnimatingItem(item);
    if (bossdata.explode_count)
    {
        DrawLondonBossShield(item);
        DrawExplosionRings();
    }
    else
    {
        DrawSummonRings();
        DrawKnockBackRings();
    }
    if (item->hit_points <= 0 && bossdata.explode_count == 0)
    {
        UpdateElectricityPoints();
        LaraElectricDeath(0, item);
        LaraElectricDeath(1, item);
    }
}

void InitialiseLondonBoss(sint16 item_number)
{
    SHIELD_POINTS *shptr;
    sint32 lp, lp1, angle, y;

    bossdata.explode_count = bossdata.ring_count = bossdata.dead = bossdata.dropped_icon = 0;
    shptr = &LondonBossShield[0];
    for (lp = 0; lp < 5; lp++) {
        y = LondonBossHeights[lp];
        angle = 0;
        for (lp1 = 0; lp1 < 8; lp1++) {
            shptr->x = (rcossin_tbl[angle << 1] * LondonBossRadii[lp]) >> 11;
            shptr->y = y;
            shptr->z = (rcossin_tbl[(angle << 1) + 1] * LondonBossRadii[lp]) >> 11;
            shptr->rgb = 0;
            angle += 512;
            shptr++;
        }
    }
}

sint16 CreateItem(void);
void phd_GetVectorAngles(sint32 x, sint32 y, sint32 z, PHD_ANGLE *dest);
void InitialiseItem(sint16 item_num);

void TriggerLaserBolt(PHD_VECTOR *pos, ITEM_INFO *item, sint32 type, sint16 yang)
{
    sint16 item_number;
    ITEM_INFO *bolt_item;
    PHD_ANGLE angles[2];

    item_number = CreateItem();
    if (item_number != NO_ITEM) {
        bolt_item = &items[item_number];
        bolt_item->object_number = 371;
        bolt_item->room_number = item->room_number;
        bolt_item->pos.x_pos = pos->x;
        bolt_item->pos.y_pos = pos->y;
        bolt_item->pos.z_pos = pos->z;
        InitialiseItem(item_number);
        if (type == 2) {
            bolt_item->pos.y_pos += item->pos.y_pos - 384;
            bolt_item->pos.x_rot = -pos->y << 5;
            bolt_item->pos.y_rot = GetRandomControl() << 1;
        } else {
            phd_GetVectorAngles(lara_item->pos.x_pos - pos->x, lara_item->pos.y_pos - 256 - pos->y,
                                lara_item->pos.z_pos - pos->z, angles);
            bolt_item->pos.x_rot = angles[1];
            bolt_item->pos.y_rot = yang;
            bolt_item->pos.z_rot = 0;
        }
        if (type != 1) {
            bolt_item->speed = 16;
            bolt_item->item_flags[0] = -24;
            bolt_item->item_flags[1] = 4;
            if (type == 2)
                bolt_item->item_flags[2] = 1;
        } else {
            bolt_item->speed = 24;
            bolt_item->item_flags[0] = 31;
            bolt_item->item_flags[1] = 16;
        }
        AddActiveItem(item_number);
    }
}
