#ifndef ITEMS_H
#define ITEMS_H

#include "3dglodef.h"

#define NOT_ACTIVE 0
#define ACTIVE 1
#define DEACTIVATED 2
#define INVISIBLE 3
#define ONESHOT 0x100

typedef struct item_light {
    PHD_VECTOR sun;
    PHD_VECTOR bulb;
    PHD_VECTOR dynamic;
    uint8 sunr;
    uint8 sung;
    uint8 sunb;
    sint8 init;
    uint8 bulbr;
    uint8 bulbg;
    uint8 bulbb;
    uint8 ambient;
    uint8 dynamicr;
    uint8 dynamicg;
    uint8 dynamicb;
    uint8 pad2;
} ITEM_LIGHT;

typedef struct item_info {
    sint32 floor;
    uint32 touch_bits;
    uint32 mesh_bits;
    sint16 object_number;
    sint16 current_anim_state;
    sint16 goal_anim_state;
    sint16 required_anim_state;
    sint16 anim_number;
    sint16 frame_number;
    sint16 room_number;
    sint16 next_item;
    sint16 next_active;
    sint16 speed;
    sint16 fallspeed;
    sint16 hit_points;
    uint16 box_number;
    sint16 timer;
    sint16 flags;
    sint16 shade;
    sint16 shadeB;
    sint16 carried_item;
    sint16 after_death;
    uint16 fired_weapon;
    sint16 item_flags[4];
    void *data;
    PHD_3DPOS pos;
    ITEM_LIGHT il;
    uint16 active:1;
    uint16 status:2;
    uint16 gravity_status:1;
    uint16 hit_status:1;
    uint16 collidable:1;
    uint16 looked_at:1;
    uint16 dynamic_light:1;
    uint16 clear_body:1;
    uint16 ai_bits:5;
    uint16 really_active:1;
} ITEM_INFO;

extern ITEM_INFO *items;

#define NO_ITEM -1

typedef struct fx_info {
    PHD_3DPOS pos;
    sint16 room_number;
    sint16 object_number;
    sint16 next_fx;
    sint16 next_active;
    sint16 speed;
    sint16 fallspeed;
    sint16 frame_number;
    sint16 counter;
    sint16 shade;
    sint16 flag1;
    sint16 flag2;
} FX_INFO;

#endif
