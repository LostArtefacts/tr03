#ifndef LARA_H
#define LARA_H

#include "types.h"
#include "items.h"
#include "larafire.h"

#define ONE_DEGREE 182
#define FASTFALL_SPEED 128
#define LARA_HITE 762
#define LARA_DUCK_HEIGHT 400
#define LARA_RAD 100
#define STEPUP_HEIGHT ((STEP_L * 3) / 2)

enum lara_gun_types {
    LG_UNARMED,
    LG_PISTOLS,
    LG_MAGNUMS,
    LG_UZIS,
    LG_SHOTGUN,
    LG_M16,
    LG_ROCKET,
    LG_GRENADE,
    LG_HARPOON,
    LG_FLARE,
    LG_SKIDOO,
    NUM_WEAPONS
};

enum lara_gun_states {
    LG_ARMLESS,
    LG_HANDSBUSY,
    LG_DRAW,
    LG_UNDRAW,
    LG_READY,
    LG_SPECIAL
};

enum lara_water_states {
    LARA_ABOVEWATER,
    LARA_UNDERWATER,
    LARA_SURFACE,
    LARA_CHEAT,
    LARA_WADE
};

typedef struct lara_info {
    sint16 item_number;
    sint16 gun_status;
    sint16 gun_type;
    sint16 request_gun_type;
    sint16 last_gun_type;
    sint16 calc_fallspeed;
    sint16 water_status;
    sint16 climb_status;
    sint16 pose_count;
    sint16 hit_frame;
    sint16 hit_direction;
    sint16 air;
    sint16 dive_count;
    sint16 death_count;
    sint16 current_active;
    sint16 current_xvel;
    sint16 current_yvel;
    sint16 current_zvel;
    sint16 hit_effect_count;
    sint16 flare_age;
    sint16 skidoo;
    sint16 weapon_item;
    sint16 back_gun;
    sint16 flare_frame;
    sint16 poisoned;
    sint16 electric;
    uint16 flare_control_left:1;
    uint16 flare_control_right:1;
    uint16 extra_anim:1;
    uint16 look:1;
    uint16 burn:1;
    uint16 keep_ducked:1;
    uint16 CanMonkeySwing:1;
    uint16 MineL:1;
    uint16 MineR:1;
    uint16 BurnGreen:1;
    uint16 IsDucked:1;
    uint16 has_fired:1;
    sint32 water_surface_dist;
    PHD_VECTOR last_pos;
    struct fx_info *hit_effect;
    sint32 mesh_effects;
    sint16 *mesh_ptrs[15];
    ITEM_INFO *target;
    PHD_ANGLE target_angles[2];
    PHD_ANGLE turn_rate;
    PHD_ANGLE move_angle;
    PHD_ANGLE head_y_rot, head_x_rot, head_z_rot;
    PHD_ANGLE torso_y_rot, torso_x_rot, torso_z_rot;
    LARA_ARM left_arm, right_arm;
    AMMO_INFO pistols;
    AMMO_INFO magnums;
    AMMO_INFO uzis;
    AMMO_INFO shotgun;
    AMMO_INFO harpoon;
    AMMO_INFO rocket;
    AMMO_INFO grenade;
    AMMO_INFO m16;
    struct creature_info *creature;
} LARA_INFO;

extern LARA_INFO lara;
extern ITEM_INFO *lara_item;

#endif
