#include "game.h"

void LaserSplitterToggle(ITEM_INFO *item);

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
