#include "game.h"

void draw_shotgun_meshes(int weapon_type)
{
    int obj;

    obj = WeaponObject(weapon_type);
    lara.mesh_ptrs[HAND_R] = meshes[objects[obj].mesh_index + HAND_R];
    lara.back_gun = 0;
}

void undraw_shotgun_meshes(int weapon_type)
{
    int obj;

    obj = WeaponObject(weapon_type);
    lara.mesh_ptrs[HAND_R] = meshes[objects[LARA].mesh_index + HAND_R];
    lara.back_gun = obj;
}

void ready_shotgun(int weapon_type)
{
    lara.gun_status = LG_READY;
    lara.left_arm.x_rot = lara.left_arm.y_rot = lara.left_arm.z_rot = 0;
    lara.right_arm.x_rot = lara.right_arm.y_rot = lara.right_arm.z_rot = 0;
    lara.left_arm.frame_number = lara.right_arm.frame_number = 0;
    lara.left_arm.lock = lara.right_arm.lock = 0;
    lara.target = NULL;
    lara.left_arm.frame_base = lara.right_arm.frame_base = objects[WeaponObject(weapon_type)].frame_base;
}

typedef struct {
    sint16 lock_angles[4];
    sint16 left_angles[4];
    sint16 right_angles[4];
    sint16 aim_speed;
    sint16 shot_accuracy;
    sint16 gun_height;
    sint16 target_dist;
    char damage;
    char recoil_frame;
    char flash_time;
    char draw_frame;
    sint16 sample_num;
} WEAPON_INFO;

extern WEAPON_INFO weapons[];
extern sint32 SmokeCountL;
extern sint32 SmokeWeapon;

int FireWeapon(int weapon_type, ITEM_INFO *target, ITEM_INFO *src, PHD_ANGLE *angles);
void TriggerGunShell(sint16 leftright, sint32 type, sint32 weapon);

void FireM16(int running)
{
    PHD_ANGLE angles[2];

    angles[0] = lara.left_arm.y_rot + lara_item->pos.y_rot;
    angles[1] = lara.left_arm.x_rot;
    if (!lara.left_arm.lock) {
        angles[0] += lara.torso_y_rot;
        angles[1] += lara.torso_x_rot;
    }
    if (running) {
        weapons[M16].shot_accuracy = ONE_DEGREE * 12;
        weapons[M16].damage = 1;
    } else {
        weapons[M16].shot_accuracy = ONE_DEGREE * 4;
        weapons[M16].damage = 3;
    }
    if (FireWeapon(LG_M16, lara.target, lara_item, angles)) {
        SmokeCountL = 24;
        SmokeWeapon = LG_M16;
        TriggerGunShell(1, 366, LG_M16);
        lara.right_arm.flash_gun = weapons[LG_M16].flash_time;
    }
}

sint16 CreateItem(void);
void InitialiseItem(sint16 item_num);
void GetLaraHandAbsPosition(PHD_VECTOR *vec, int hand);
void find_target_point(ITEM_INFO *item, GAME_VECTOR *target);

void FireHarpoon(void)
{
    sint16 item_number;
    ITEM_INFO *item;
    GAME_VECTOR pos;
    int distance;

    if (lara.harpoon.ammo <= 0)
        return;
    item_number = CreateItem();
    if (item_number != NO_ITEM) {
        item = &items[item_number];
        item->shade = 0x4210 | 0x8000;
        item->object_number = 310;
        item->room_number = lara_item->room_number;
        pos.x = -2;
        pos.y = 273 + 100;
        pos.z = 77;
        GetLaraHandAbsPosition((PHD_VECTOR *)&pos, 1);
        item->pos.x_pos = pos.x;
        item->pos.y_pos = pos.y;
        item->pos.z_pos = pos.z;
        InitialiseItem(item_number);
        if (lara.target) {
            find_target_point(lara.target, &pos);
            item->pos.y_rot = phd_atan(pos.z - item->pos.z_pos, pos.x - item->pos.x_pos);
            distance = phd_sqrt((pos.z - item->pos.z_pos) * (pos.z - item->pos.z_pos) +
                                (pos.x - item->pos.x_pos) * (pos.x - item->pos.x_pos));
            item->pos.x_rot = -phd_atan(distance, pos.y - item->pos.y_pos);
        } else {
            item->pos.x_rot = lara_item->pos.x_rot + lara.torso_x_rot;
            item->pos.y_rot = lara_item->pos.y_rot + lara.torso_y_rot;
        }
        item->pos.z_rot = 0;
        item->fallspeed = (sint16)(-256 * phd_sin(item->pos.x_rot) >> W2V_SHIFT);
        item->speed = (sint16)(256 * phd_cos(item->pos.x_rot) >> W2V_SHIFT);
        item->hit_points = 256;
        AddActiveItem(item_number);
        if (!savegame.bonus_flag)
            lara.harpoon.ammo--;
        savegame.ammo_used++;
    }
}

void undraw_shotgun(int weapon_type)
{
    ITEM_INFO *item;

    item = &items[lara.weapon_item];
    if (lara.water_status == LARA_SURFACE)
        item->goal_anim_state = 9;
    else
        item->goal_anim_state = 3;
    AnimateItem(item);
    if (item->status == DEACTIVATED) {
        lara.gun_status = LG_ARMLESS;
        lara.target = NULL;
        lara.left_arm.lock = lara.right_arm.lock = 0;
        KillItem(lara.weapon_item);
        lara.weapon_item = NO_ITEM;
        lara.left_arm.frame_number = lara.right_arm.frame_number = 0;
    } else if (item->current_anim_state == 3 && item->frame_number - anims[item->anim_number].frame_base == 21)
        undraw_shotgun_meshes(weapon_type);
    lara.left_arm.frame_base = lara.right_arm.frame_base = anims[item->anim_number].frame_ptr;
    lara.left_arm.frame_number = lara.right_arm.frame_number = item->frame_number - anims[item->anim_number].frame_base;
    lara.left_arm.anim_number = lara.right_arm.anim_number = item->anim_number;
}
