#ifndef ROOM_H
#define ROOM_H

#include "types.h"

#define WALL_L 1024
#define NO_HEIGHT (-0x7f00)
#define STEP_L (WALL_L / 4)
#define WALL_SHIFT 10

#define NO_BOX 0x7ff
#define BLOCKED 0x4000

typedef struct floor_info {
    uint16 index;
    uint16 fx:4;
    uint16 box:11;
    uint16 stopper:1;
    uint8 pit_room;
    sint8 floor;
    uint8 sky_room;
    sint8 ceiling;
} FLOOR_INFO;

typedef struct box_info {
    uint8 left;
    uint8 right;
    uint8 top;
    uint8 bottom;
    sint16 height;
    sint16 overlap_index;
} BOX_INFO;

typedef struct doorpos_data {
    FLOOR_INFO *floor;
    FLOOR_INFO data;
    sint16 block;
} DOORPOS_DATA;

extern BOX_INFO *boxes;

#endif
