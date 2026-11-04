#ifndef ROOM_H
#define ROOM_H

#include "types.h"

#define WALL_L 1024
#define NO_HEIGHT (-0x7f00)
#define STEP_L (WALL_L / 4)
#define WALL_SHIFT 10

#define NO_BOX 0x7ff
#define BLOCKED 0x4000
#define NO_ROOM 0xff

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

typedef struct {
    sint16 nx;
    sint16 ny;
    sint16 nz;
    sint16 pad;
} SUNLIGHT;

typedef struct {
    sint32 intensity;
    sint32 falloff;
} SPOTLIGHT;

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    uint8 r;
    uint8 g;
    uint8 b;
    uint8 type;
    union {
        SUNLIGHT sun;
        SPOTLIGHT spot;
    } l;
} LIGHT_INFO;

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint16 y_rot;
    sint16 shade;
    sint16 shadeB;
    sint16 static_number;
} MESH_INFO;

typedef struct room_info {
    sint16 *data;
    sint16 *door;
    FLOOR_INFO *floor;
    LIGHT_INFO *light;
    MESH_INFO *mesh;
    sint32 x;
    sint32 y;
    sint32 z;
    sint32 minfloor;
    sint32 maxceiling;
    sint16 x_size;
    sint16 y_size;
    sint16 ambient;
    sint16 lighting;
    sint16 num_lights;
    sint16 num_meshes;
    sint16 ReverbType;
    char MeshEffect;
    char bound_active;
    sint16 left;
    sint16 right;
    sint16 top;
    sint16 bottom;
    sint16 test_left;
    sint16 test_right;
    sint16 test_top;
    sint16 test_bottom;
    sint16 item_number;
    sint16 fx_number;
    sint16 flipped_room;
    uint16 flags;
} ROOM_INFO;

extern ROOM_INFO *room;
extern sint16 number_rooms;
extern BOX_INFO *boxes;

FLOOR_INFO *GetFloor(sint32 x, sint32 y, sint32 z, sint16 *room_number);
extern sint16 *trigger_index;

sint32 GetHeight(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);
void TestTriggers(sint16 *data, int heavy);
sint32 GetCeiling(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);

#endif
