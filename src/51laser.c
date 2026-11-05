#include <stdio.h>
#include <string.h>
#include "game.h"

void TriggerDynamic(sint32 x, sint32 y, sint32 z, sint32 falloff, sint32 r, sint32 g, sint32 b);
int TestBoundsCollide(ITEM_INFO *item, ITEM_INFO *laraitem, sint32 radius);
void DoLotsOfBloodD(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num, int number);

int abs(int x);
#pragma intrinsic(abs)

void ControlArea51Laser(sint16 item_number)
{
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    sint32 destx, destz, dx, dz, rad, r, g, b;
    sint16 room_number;

    item = &items[item_number];

    item->current_anim_state = 0;

    rad = (GetRandomControl() & 1) + 8;
    r = (GetRandomControl() & 3) + 24;
    g = GetRandomControl() & 3;
    b = GetRandomControl() & 1;

    TriggerDynamic(item->pos.x_pos, item->pos.y_pos - 64, item->pos.z_pos, rad, r, g, b);
    item->mesh_bits = 0xffffffff - (GetRandomControl() & 20);

    destx = (item->item_flags[1] & 255) << 9;
    destz = ((item->item_flags[1] >> 8) & 255) << 9;

    dx = destx - item->pos.x_pos;
    dz = destz - item->pos.z_pos;

    if (abs(dx) < 768 && abs(dz) < 768) {
        dx = ((item->item_flags[0] & 255) << 9) + ((item->item_flags[2] * -2560 * phd_sin(item->pos.y_rot)) >> W2V_SHIFT);
        dz = (((item->item_flags[0] >> 8) & 255) << 9) + ((item->item_flags[2] * -2560 * phd_cos(item->pos.y_rot)) >> W2V_SHIFT);
        item->item_flags[1] = ((dx >> 9) & 255) | (((dz >> 9) & 255) << 8);
        item->shadeB = 32;
    }

    if (item->item_flags[2] == 1) {
        if (item->shadeB) {
            if (item->item_flags[3] == 0) {
                item->shadeB--;
                if (item->shadeB == 0)
                    item->item_flags[2] = -item->item_flags[2];
            } else if (item->item_flags[3] <= 4) {
                item->item_flags[3] = 0;
            } else {
                item->item_flags[3] -= item->item_flags[3] >> 2;
            }
        } else {
            item->item_flags[3] += 5;
            if (item->item_flags[3] > 512)
                item->item_flags[3] = 512;
        }
    } else {
        if (item->shadeB) {
            if (item->item_flags[3] == 0) {
                item->shadeB--;
                if (item->shadeB == 0)
                    item->item_flags[2] = -item->item_flags[2];
            } else if (item->item_flags[3] >= -4) {
                item->item_flags[3] = 0;
            } else {
                item->item_flags[3] -= item->item_flags[3] >> 2;
            }
        } else {
            item->item_flags[3] -= 5;
            if (item->item_flags[3] < -512)
                item->item_flags[3] = -512;
        }
    }

    item->pos.z_pos += (item->item_flags[3] * phd_cos(item->pos.y_rot)) >> (W2V_SHIFT + 2);
    item->pos.x_pos += (item->item_flags[3] * phd_sin(item->pos.y_rot)) >> (W2V_SHIFT + 2);

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (room_number != item->room_number)
        ItemNewRoom(item_number, room_number);

    if (TestBoundsCollide(item, lara_item, 64)) {
        lara_item->hit_points -= 25;
        DoLotsOfBloodD(lara_item->pos.x_pos, item->pos.y_pos - (GetRandomControl() & 255) - 32, lara_item->pos.z_pos,
                       (GetRandomControl() & 127) + 128, GetRandomControl() << 1, lara_item->room_number, 3);
    }

    AnimateItem(item);
}

void InitialiseArea51Laser(sint16 item_number)
{
    ITEM_INFO *item;
    sint32 dx, dz;

    item = &items[item_number];
    dx = item->pos.x_pos + ((2560 * phd_sin(item->pos.y_rot)) >> W2V_SHIFT);
    dz = item->pos.z_pos + ((2560 * phd_cos(item->pos.y_rot)) >> W2V_SHIFT);
    item->item_flags[0] = ((item->pos.x_pos >> 9) & 255) | (((item->pos.z_pos >> 9) & 255) << 8);
    item->item_flags[1] = ((dx >> 9) & 255) | (((dz >> 9) & 255) << 8);
    item->item_flags[2] = 1;
    item->item_flags[3] = 0;
    item->shadeB = 0;
}
