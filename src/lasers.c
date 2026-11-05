#include "specific/standard.h"
#include "game.h"

void LaserSplitterToggle(ITEM_INFO *item);
int LOS(GAME_VECTOR *start, GAME_VECTOR *target);
void DoLotsOfBloodD(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num, int number);
void S_DrawLaserBeam(GAME_VECTOR *src, GAME_VECTOR *dest, uint8 r, uint8 g, uint8 b);

int LaraOnLOS(GAME_VECTOR *start, GAME_VECTOR *target)
{
    int j, failure;
    sint32 x, y, z, dx, dy, dz, distance;
    sint32 direction;
    sint16 *bounds, *zextent, *xextent;
    ITEM_INFO *item;

    dx = target->x - start->x;
    dy = target->y - start->y;
    dz = target->z - start->z;
    item = lara_item;
    bounds = GetBoundsAccurate(item);
    direction = (uint16)(item->pos.y_rot + 0x2000) / 0x4000;
    if (direction & 1) {
        zextent = &bounds[0];
        xextent = &bounds[4];
    } else {
        zextent = &bounds[4];
        xextent = &bounds[0];
    }
    failure = 0;
    if (ABS(dz) > ABS(dx)) {
        distance = item->pos.z_pos + zextent[0] - start->z;
        for (j = 0; j < 2; j++) {
            if ((distance & 0x80000000) == (dz & 0x80000000)) {
                y = dy * distance / dz;
                if (y > item->pos.y_pos + bounds[2] - start->y && y < item->pos.y_pos + bounds[3] - start->y) {
                    x = dx * distance / dz;
                    if (x < item->pos.x_pos + xextent[0] - start->x)
                        failure |= 1;
                    else if (x > item->pos.x_pos + xextent[1] - start->x)
                        failure |= 2;
                    else
                        return 1;
                }
            }
            distance = item->pos.z_pos + zextent[1] - start->z;
        }
        if (failure == 3)
            return 1;
    } else {
        distance = item->pos.x_pos + xextent[0] - start->x;
        for (j = 0; j < 2; j++) {
            if ((distance & 0x80000000) == (dx & 0x80000000)) {
                y = dy * distance / dx;
                if (y > item->pos.y_pos + bounds[2] - start->y && y < item->pos.y_pos + bounds[3] - start->y) {
                    z = dz * distance / dx;
                    if (z < item->pos.z_pos + zextent[0] - start->z)
                        failure |= 1;
                    else if (z > item->pos.z_pos + zextent[1] - start->z)
                        failure |= 2;
                    else
                        return 1;
                }
            }
            distance = item->pos.x_pos + xextent[1] - start->x;
        }
        if (failure == 3)
            return 1;
    }
    return 0;
}

void LaserControl(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];
    LaserSplitterToggle(item);
}

void LaserSplitterToggle(ITEM_INFO *item)
{
    sint16 room_number;
    sint32 x, z, dx, dz;
    FLOOR_INFO *floor;
    int active;

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (!(boxes[floor->box].overlap_index & 0x8000))
        return;
    active = TriggerActive(item);
    if (active == ((boxes[floor->box].overlap_index & BLOCKED) == BLOCKED))
        return;
    switch (item->pos.y_rot) {
    case 0:
        dz = -WALL_L;
        dx = 0;
        break;
    case 16384:
        dz = 0;
        dx = -WALL_L;
        break;
    case -32768:
        dz = WALL_L;
        dx = 0;
        break;
    default:
        dz = 0;
        dx = WALL_L;
        break;
    }
    x = item->pos.x_pos;
    z = item->pos.z_pos;
    while (floor->box != NO_BOX && (boxes[floor->box].overlap_index & 0x8000)) {
        if (active)
            boxes[floor->box].overlap_index |= BLOCKED;
        else
            boxes[floor->box].overlap_index &= ~BLOCKED;
        x += dx;
        z += dz;
        floor = GetFloor(x, item->pos.y_pos, z, &room_number);
    }
}

extern sint32 level_items;

void S_DrawLaser(ITEM_INFO *item)
{
    GAME_VECTOR src, dest;
    sint32 x, z, lp, y;

    if (!TriggerActive(item)) {
        return;
    }

    y = 0;

    switch (item->pos.y_rot) {
    case 0:
        z = 511;
        x = 0;
        break;
    case 16384:
        z = 0;
        x = 511;
        break;
    case -32768:
        z = -511;
        x = 0;
        break;
    default:
        z = 0;
        x = -511;
        break;
    }

    for (lp = 0; lp < item->hit_points; lp++) {
        src.room_number = item->room_number;
        src.x = item->pos.x_pos + x;
        src.y = item->pos.y_pos + y;
        src.z = item->pos.z_pos + z;

        dest.x = item->pos.x_pos - (x << 5);
        dest.y = item->pos.y_pos + y;
        dest.z = item->pos.z_pos - (z << 5);

        LOS(&src, &dest);

        if (LaraOnLOS(&src, &dest)) {
            sint32 lp;
            ITEM_INFO *target;

            if (item->object_number != 66) {
                if (item->object_number == 68) {
                    lara_item->hit_points = 0;
                } else {
                    lara_item->hit_points -= 10;
                }
                DoLotsOfBloodD(lara_item->pos.x_pos, item->pos.y_pos + y, lara_item->pos.z_pos, (GetRandomDraw() & 127) + 128,
                               GetRandomDraw() << 1, lara_item->room_number, 1);
            }

            if (item->pos.y_rot == 0 || item->pos.y_rot == -32768) {
                dest.z = lara_item->pos.z_pos;
            } else {
                dest.x = lara_item->pos.x_pos;
            }

            for (lp = 0, target = &items[0]; lp < level_items; lp++, target++) {
                if (target->object_number == 318 || target->object_number == 64) {
                    if (TriggerActive(target)) {
                        target->really_active = 1;
                    }
                }
            }
        }

        if (item->object_number == 66) {
            S_DrawLaserBeam(&src, &dest, 16, 0, 16);
        } else if (item->object_number == 67) {
            S_DrawLaserBeam(&src, &dest, 0, 0, 16);
        } else {
            S_DrawLaserBeam(&src, &dest, 0, 2, 16);
        }

        y -= 256;
    }
}
