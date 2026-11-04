#ifndef ANIM_H
#define ANIM_H

#include "types.h"
#include "items.h"
#include "collide.h"

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint16 room_number;
    sint16 box_number;
} GAME_VECTOR;

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint16 data;
    sint16 flags;
} OBJECT_VECTOR;

typedef struct object_info {
    sint16 nmeshes;
    sint16 mesh_index;
    sint32 bone_index;
    sint16 *frame_base;
    void (*initialise)(sint16 item_number);
    void (*control)(sint16 item_number);
    void (*floor)(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
    void (*ceiling)(ITEM_INFO *item, sint32 x, sint32 y, sint32 z, sint32 *height);
    void (*draw_routine)(ITEM_INFO *item);
    void (*collision)(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
    sint16 anim_index;
    sint16 hit_points;
    sint16 pivot_length;
    sint16 radius;
    sint16 shadow_size;
    uint16 bite_offset;
    uint16 loaded:1;
    uint16 intelligent:1;
    uint16 non_lot:1;
    uint16 save_position:1;
    uint16 save_hitpoints:1;
    uint16 save_flags:1;
    uint16 save_anim:1;
    uint16 semi_transparent:1;
    uint16 water_creature:1;
} OBJECT_INFO;

extern OBJECT_INFO objects[];
extern sint16 **meshes;

#endif
