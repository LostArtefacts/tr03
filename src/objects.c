#include "game.h"

typedef struct {
    sint32 start_height;
    sint32 wait_time;
} LIFT_INFO;

enum height_types { WALL, SMALL_SLOPE, BIG_SLOPE, DIAGONAL, SPLIT_TRI };

extern int height_type;
extern int OnObject;

void LiftFloorCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *floor, sint32 *ceiling);

void LiftFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    sint32 floor, ceiling;

    LiftFloorCeiling(item, x, y, z, &floor, &ceiling);
    if (floor < *height)
    {
        OnObject = 1;
        *height = floor;
    }
}

void LiftCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    sint32 floor, ceiling;

    LiftFloorCeiling(item, x, y, z, &floor, &ceiling);
    if (ceiling > *height)
        *height = ceiling;
}

void BridgeFlatFloor(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    if (y <= item->pos.y_pos)
    {
        *height = item->pos.y_pos;
        height_type = WALL;
        OnObject = 1;
        if (item == lara_item)
            lara_item->item_flags[0] = 1;
    }
}

void BridgeFlatCeiling(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height)
{
    if (y > item->pos.y_pos)
        *height = item->pos.y_pos + STEP_L;
}

sint32 GetOffset(ITEM_INFO *item, sint32 x, sint32 z)
{
    if (item->pos.y_rot == 0)
        return (WALL_L - x) & (WALL_L - 1);
    else if (item->pos.y_rot == -0x8000)
        return x & (WALL_L - 1);
    else if (item->pos.y_rot == 0x4000)
        return z & (WALL_L - 1);
    return (WALL_L - z) & (WALL_L - 1);
}
