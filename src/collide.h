#ifndef COLLIDE_H
#define COLLIDE_H

#include "3dglodef.h"
#include "items.h"

#define COLL_NONE 0
#define COLL_FRONT 1
#define COLL_LEFT 2
#define COLL_RIGHT 4
#define COLL_TOP 8
#define COLL_TOPFRONT 16
#define COLL_CLAMP 32

typedef struct {
    sint32 mid_floor;
    sint32 mid_ceiling;
    sint32 mid_type;
    sint32 front_floor;
    sint32 front_ceiling;
    sint32 front_type;
    sint32 left_floor;
    sint32 left_ceiling;
    sint32 left_type;
    sint32 right_floor;
    sint32 right_ceiling;
    sint32 right_type;
    sint32 left_floor2;
    sint32 left_ceiling2;
    sint32 left_type2;
    sint32 right_floor2;
    sint32 right_ceiling2;
    sint32 right_type2;
    sint32 radius;
    sint32 bad_pos, bad_neg;
    sint32 bad_ceiling;
    PHD_VECTOR shift;
    PHD_VECTOR old;
    sint16 old_anim_state;
    sint16 old_anim_number;
    sint16 old_frame_number;
    sint16 facing;
    sint16 quadrant;
    sint16 coll_type;
    sint16 *trigger;
    signed char tilt_x, tilt_z;
    char hit_by_baddie;
    char hit_static;
    uint16 slopes_are_walls:2;
    uint16 slopes_are_pits:1;
    uint16 lava_is_pit:1;
    uint16 enable_baddie_push:1;
    uint16 enable_hit:1;
    uint16 hit_ceiling:1;
} COLL_INFO;

void ShiftItem(ITEM_INFO *item, COLL_INFO *coll);
void DoorCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);

void GetCollisionInfo(COLL_INFO *coll, sint32 xpos, sint32 ypos, sint32 zpos, sint16 room_number, sint32 objheight);

#endif
