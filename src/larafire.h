#ifndef LARAFIRE_H
#define LARAFIRE_H

#include "types.h"

typedef struct lara_arm {
    sint16 *frame_base;
    sint16 frame_number;
    sint16 anim_number;
    sint16 lock;
    sint16 y_rot;
    sint16 x_rot;
    sint16 z_rot;
    sint16 flash_gun;
} LARA_ARM;

typedef struct ammo_info {
    sint32 ammo;
} AMMO_INFO;

#endif
