#include "game.h"
#include <stdlib.h>

extern sint16 rcossin_tbl[];

extern uint32 CurrentLevel;
extern sint32 wibble;

void TriggerAlertLight(sint32 x, sint32 y, sint32 z, sint32 r, sint32 g, sint32 b, sint32 angle, sint16 room_no);

void ControlStrobeLight(sint16 item_number)
{
    ITEM_INFO *item;
    sint32 angle, sin, cos;

    item = &items[item_number];
    if (TriggerActive(item)) {
        if (CurrentLevel == 15 && !item->really_active)
            return;
        item->pos.y_rot += ONE_DEGREE << 4;
        angle = ((item->pos.y_rot + 0x5800) >> 4) & 4095;
        TriggerAlertLight(item->pos.x_pos, item->pos.y_pos - 512, item->pos.z_pos, 31, 8, 0, angle, item->room_number);
        sin = ((sint16)rcossin_tbl[angle << 1]) >> 4;
        cos = ((sint16)rcossin_tbl[(angle << 1) + 1]) >> 4;
        TriggerDynamic(item->pos.x_pos + sin, item->pos.y_pos - 768, item->pos.z_pos + cos, 6, 31, 12, 0);
        if ((wibble & 127) == 0)
            SoundEffect(208, &item->pos, 0);
        item->item_flags[0]++;
        if (item->item_flags[0] > 1800) {
            item->really_active = 0;
            item->item_flags[0] = 0;
        }
    }
}

void ControlPulseLight(sint16 item_number)
{
    ITEM_INFO *item;
    sint32 angle, sin;

    item = &items[item_number];
    if (TriggerActive(item))
    {
        item->item_flags[0] += ONE_DEGREE << 2;
        angle = (item->item_flags[0] >> 4) & 4095;
        sin = rcossin_tbl[angle << 1] >> 7;
        sin = abs(sin);
        if (sin > 31)
            sin = 31;
        else if (sin < 8)
        {
            sin = 8;
            item->item_flags[0] += 2048;
        }
        TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, sin, 31, 12, 0);
    }
}

void ControlOnOffLight(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    if (TriggerActive(item))
        TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 16, 31, 31, 31);
}

void ControlElectricalLight(sint16 item_number)
{
    ITEM_INFO *item;
    sint32 r, g, b;

    item = &items[item_number];
    if (TriggerActive(item)) {
        if (item->item_flags[0] < 16) {
            r = g = (GetRandomControl() & 7) << 2;
            b = r + (GetRandomControl() & 3);
            item->item_flags[0]++;
        } else if (item->item_flags[0] < 96) {
            if ((wibble & 63) == 0 || (GetRandomControl() & 7) == 0) {
                r = g = 24 - (GetRandomControl() & 7);
                b = r + (GetRandomControl() & 3);
            } else {
                r = g = GetRandomControl() & 7;
                b = r + (GetRandomControl() & 3);
            }
            item->item_flags[0]++;
        } else if (item->item_flags[0] < 160) {
            r = g = 12 - (GetRandomControl() & 3);
            b = r + (GetRandomControl() & 3);
            if ((GetRandomControl() & 31) == 0 && item->item_flags[0] > 128)
                item->item_flags[0] = 160;
            else
                item->item_flags[0]++;
        } else {
            r = g = 31 - (GetRandomControl() & 3);
            b = 31 - (GetRandomControl() & 1);
        }
        TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 16, r, g, b);
    } else {
        item->item_flags[0] = 0;
    }
}

void ControlBeaconLight(sint16 item_number)
{
    ITEM_INFO *item;
    sint32 r, g, b;

    item = &items[item_number];
    if (TriggerActive(item))
    {
        item->item_flags[0]++;
        item->item_flags[0] &= 63;
        if (item->item_flags[0] < 3)
        {
            r = g = 31 - (GetRandomControl() & 1);
            b = 31 - (GetRandomControl() & 3);
            TriggerDynamic(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 16, r, g, b);
        }
    }
}
