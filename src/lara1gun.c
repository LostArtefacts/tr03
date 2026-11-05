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

void LaraTargetInfo(WEAPON_INFO *winfo);
void LaraGetNewTarget(WEAPON_INFO *winfo);
void AimWeapon(WEAPON_INFO *winfo, LARA_ARM *arm);
void AnimatePistols(int weapon_type);
void AnimateShotgun(int weapon_type);
void GetLaraHandAbsPosition(PHD_VECTOR *vec, int hand);

extern sint32 FuckYou;

void RifleHandler(int weapon_type)
{
    WEAPON_INFO *winfo;

    winfo = &weapons[weapon_type];
    if (input & IN_ACTION)
        LaraTargetInfo(winfo);
    else
        lara.target = NULL;
    if (lara.target == NULL)
        LaraGetNewTarget(winfo);
    AimWeapon(winfo, &lara.left_arm);
    if (lara.left_arm.lock) {
        lara.torso_y_rot = lara.left_arm.y_rot;
        lara.torso_x_rot = lara.left_arm.x_rot;
        if (camera.old_type != LOOK_CAMERA)
            lara.head_x_rot = lara.head_y_rot = 0;
    }
    lara.torso_x_rot += FuckYou;
    lara.left_arm.x_rot += FuckYou;
    if (weapon_type == LG_MAGNUMS)
        AnimatePistols(weapon_type);
    else
        AnimateShotgun(weapon_type);
    if (lara.right_arm.flash_gun) {
        if (weapon_type == LG_SHOTGUN || weapon_type == LG_M16) {
            TriggerDynamic(lara_item->pos.x_pos + (phd_sin(lara_item->pos.y_rot) >> (W2V_SHIFT - 10)) + ((GetRandomControl() & 255) - 128), lara_item->pos.y_pos - WALL_L / 2 + ((GetRandomControl() & 127) - 63), lara_item->pos.z_pos + (phd_cos(lara_item->pos.y_rot) >> (W2V_SHIFT - 10)) + ((GetRandomControl() & 255) - 128), (weapon_type == LG_SHOTGUN) ? 12 : 11, (GetRandomControl() & 7) + 24, (GetRandomControl() & 3) + 16, GetRandomControl() & 7);
        } else if (weapon_type == LG_MAGNUMS) {
            PHD_VECTOR pos;

            pos.x = (GetRandomControl() & 255) - 128;
            pos.y = (GetRandomControl() & 127) - 63;
            pos.z = (GetRandomControl() & 255) - 128;
            GetLaraHandAbsPosition(&pos, 1);
            TriggerDynamic(pos.x, pos.y, pos.z, 12, (GetRandomControl() & 7) + 24, (GetRandomControl() & 3) + 16, GetRandomControl() & 7);
        }
    }
}

void TriggerGunSmoke(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv, sint32 initial, sint32 weapon, sint32 count);
void TriggerShotgunSparks(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv);

void FireShotgun(void)
{
    int i, r, fired, x, y, z, lp;
    PHD_VECTOR pos;
    PHD_ANGLE angles[2];
    PHD_ANGLE dangles[2];

    angles[0] = lara.left_arm.y_rot + lara_item->pos.y_rot;
    angles[1] = lara.left_arm.x_rot;
    if (!lara.left_arm.lock) {
        angles[0] += lara.torso_y_rot;
        angles[1] += lara.torso_x_rot;
    }
    fired = 0;
    for (i = 0; i < 6; i++) {
        r = (int)((GetRandomControl() - 16384) * (20 * ONE_DEGREE)) / 65536;
        dangles[0] = angles[0] + r;
        r = (int)((GetRandomControl() - 16384) * (20 * ONE_DEGREE)) / 65536;
        dangles[1] = angles[1] + r;
        if (FireWeapon(LG_SHOTGUN, lara.target, lara_item, dangles))
            fired = 1;
    }
    if (fired) {
        pos.x = 0;
        pos.y = 228;
        pos.z = 32;
        GetLaraHandAbsPosition(&pos, 1);
        x = pos.x;
        y = pos.y;
        z = pos.z;
        pos.x = 0;
        pos.y = 228 + 1280;
        pos.z = 32;
        GetLaraHandAbsPosition(&pos, 1);
        SmokeCountL = 32;
        SmokeWeapon = LG_SHOTGUN;
        for (lp = 0; lp < 7; lp++)
            TriggerGunSmoke(x, y, z, pos.x - x, pos.y - y, pos.z - z, 1, SmokeWeapon, 32);
        for (lp = 0; lp < 12; lp++)
            TriggerShotgunSparks(x, y, z, (pos.x - x) << 1, (pos.y - y) << 1, (pos.z - z) << 1);
        lara.right_arm.flash_gun = weapons[LG_SHOTGUN].flash_time;
        SoundEffect(105, &lara_item->pos, 0x1400000 | 4);
        SoundEffect(weapons[LG_SHOTGUN].sample_num, &lara_item->pos, 0);
    }
}

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

void phd_PushMatrix(void);
void phd_PushUnitMatrix(void);
void phd_RotYXZ(sint16 ry, sint16 rx, sint16 rz);
void phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void TriggerRocketFlame(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv, sint32 item_no);

void FireRocket(void)
{
    sint16 item_number, lp;
    ITEM_INFO *item;
    PHD_VECTOR pos;
    sint32 x, y, z, wx, wy, wz, xv, yv, zv;

    if (lara.rocket.ammo <= 0)
        return;

    lara.has_fired = 1;

    item_number = CreateItem();
    if (item_number != NO_ITEM) {
        item = &items[item_number];

        item->object_number = 309;
        item->room_number = lara_item->room_number;
        pos.x = 0;
        pos.y = 180;
        pos.z = 72;

        GetLaraHandAbsPosition(&pos, 1);
        item->pos.x_pos = x = pos.x;
        item->pos.y_pos = y = pos.y;
        item->pos.z_pos = z = pos.z;
        pos.x = 0;
        pos.y = 180 + 1024;
        pos.z = 72;
        GetLaraHandAbsPosition(&pos, 1);

        SmokeCountL = 32;
        SmokeWeapon = LG_ROCKET;
        for (lp = 0; lp < 5; lp++)
            TriggerGunSmoke(x, y, z, pos.x - x, pos.y - y, pos.z - z, 1, LG_ROCKET, 32);

        InitialiseItem(item_number);

        item->pos.x_rot = lara_item->pos.x_rot + lara.left_arm.x_rot;
        item->pos.y_rot = lara_item->pos.y_rot + lara.left_arm.y_rot;
        item->pos.z_rot = 0;

        if (!lara.left_arm.lock) {
            item->pos.x_rot += lara.torso_x_rot;
            item->pos.y_rot += lara.torso_y_rot;
        }

        item->speed = 512 >> 5;
        item->item_flags[0] = 0;

        AddActiveItem(item_number);

        if (!savegame.bonus_flag)
            lara.rocket.ammo--;
        savegame.ammo_used++;

        phd_PushUnitMatrix();
        phd_mxptr[3] = 0;
        phd_mxptr[7] = 0;
        phd_mxptr[11] = 0;
        phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
        phd_PushMatrix();
        phd_TranslateRel(0, 0, -128);
        wx = phd_mxptr[3] >> W2V_SHIFT;
        wy = phd_mxptr[7] >> W2V_SHIFT;
        wz = phd_mxptr[11] >> W2V_SHIFT;
        phd_PopMatrix();

        for (lp = 0; lp < 8; lp++) {
            phd_PushMatrix();
            phd_TranslateRel(0, 0, -(GetRandomControl() & 2047));
            xv = phd_mxptr[3] >> W2V_SHIFT;
            yv = phd_mxptr[7] >> W2V_SHIFT;
            zv = phd_mxptr[11] >> W2V_SHIFT;
            phd_PopMatrix();
            TriggerRocketFlame(wx, wy, wz, xv - wx, yv - wy, zv - wz, item_number);
        }
        phd_PopMatrix();

        SoundEffect(105, &lara_item->pos, 0x2000004);
    }
}

void FireGrenade(void)
{
    sint16 item_number, lp;
    ITEM_INFO *item;
    PHD_VECTOR pos;
    FLOOR_INFO *floor;
    sint32 x, y, z;

    if (lara.grenade.ammo <= 0)
        return;
    lara.has_fired = 1;
    item_number = CreateItem();
    if (item_number != NO_ITEM) {
        item = &items[item_number];
        item->shade = 0x4210 | 0x8000;
        item->object_number = 311;
        item->room_number = lara_item->room_number;
        pos.x = 0;
        pos.y = 180 + 96;
        pos.z = 80;
        GetLaraHandAbsPosition(&pos, 1);
        item->pos.x_pos = x = pos.x;
        item->pos.y_pos = y = pos.y;
        item->pos.z_pos = z = pos.z;
        floor = GetFloor(pos.x, pos.y, pos.z, &item->room_number);
        if (GetHeight(floor, pos.x, pos.y, pos.z) < pos.y) {
            item->pos.x_pos = lara_item->pos.x_pos;
            item->pos.y_pos = pos.y;
            item->pos.z_pos = lara_item->pos.z_pos;
            item->room_number = lara_item->room_number;
        }
        pos.x = 0;
        pos.y = 180 + 1024;
        pos.z = 80;
        GetLaraHandAbsPosition(&pos, 1);
        SmokeCountL = 32;
        SmokeWeapon = LG_GRENADE;
        for (lp = 0; lp < 5; lp++)
            TriggerGunSmoke(x, y, z, pos.x - x, pos.y - y, pos.z - z, 1, LG_GRENADE, 32);
        InitialiseItem(item_number);
        item->pos.x_rot = lara_item->pos.x_rot + lara.left_arm.x_rot;
        item->pos.y_rot = lara_item->pos.y_rot + lara.left_arm.y_rot;
        item->pos.z_rot = 0;
        if (!lara.left_arm.lock) {
            item->pos.x_rot += lara.torso_x_rot;
            item->pos.y_rot += lara.torso_y_rot;
        }
        item->speed = 128;
        item->fallspeed = (-item->speed * phd_sin(item->pos.x_rot)) >> W2V_SHIFT;
        item->current_anim_state = item->pos.x_rot;
        item->goal_anim_state = item->pos.y_rot;
        item->required_anim_state = 0;
        item->hit_points = 4 * 30;
        AddActiveItem(item_number);
        if (!savegame.bonus_flag)
            lara.grenade.ammo--;
        savegame.ammo_used++;
    }
}

void draw_shotgun(int weapon_type)
{
    ITEM_INFO *item;

    if (lara.weapon_item == NO_ITEM) {
        lara.weapon_item = CreateItem();
        item = &items[lara.weapon_item];
        item->object_number = WeaponObject(weapon_type);
        if (weapon_type == LG_ROCKET)
            item->anim_number = objects[ROCKET_GUN].anim_index + 1;
        else if (weapon_type == LG_GRENADE)
            item->anim_number = objects[GRENADE_GUN].anim_index;
        else
            item->anim_number = objects[item->object_number].anim_index + 1;
        item->frame_number = anims[item->anim_number].frame_base;
        item->current_anim_state = item->goal_anim_state = 1;
        item->status = ACTIVE;
        item->room_number = NO_ROOM;
        lara.left_arm.frame_base = lara.right_arm.frame_base = objects[item->object_number].frame_base;
    } else
        item = &items[lara.weapon_item];
    AnimateItem(item);
    if (item->current_anim_state == 0 || item->current_anim_state == 6)
        ready_shotgun(weapon_type);
    else if (item->frame_number - anims[item->anim_number].frame_base == weapons[weapon_type].draw_frame)
        draw_shotgun_meshes(weapon_type);
    else if (lara.water_status == LARA_UNDERWATER)
        item->goal_anim_state = 6;
    lara.left_arm.frame_base = lara.right_arm.frame_base = anims[item->anim_number].frame_ptr;
    lara.left_arm.frame_number = lara.right_arm.frame_number = item->frame_number - anims[item->anim_number].frame_base;
    lara.left_arm.anim_number = lara.right_arm.anim_number = item->anim_number;
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

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint16 InnerXZoff;
    sint16 InnerXZsize;
    sint16 InnerYsize;
    sint16 InnerXZvel;
    sint16 InnerYvel;
    sint16 InnerGravity;
    sint16 InnerFriction;
    sint16 MiddleXZoff;
    sint16 MiddleXZsize;
    sint16 MiddleYsize;
    sint16 MiddleXZvel;
    sint16 MiddleYvel;
    sint16 MiddleGravity;
    sint16 MiddleFriction;
    sint16 OuterXZoff;
    sint16 OuterXZsize;
    sint16 OuterXZvel;
    sint16 OuterFriction;
} SPLASH_SETUP;

extern SPLASH_SETUP splash_setup;

void SetupSplash(SPLASH_SETUP *spl);
void TriggerExplosionBubble(sint32 x, sint32 y, sint32 z, sint16 roomnum);
void TriggerExplosionSparks(sint32 x, sint32 y, sint32 z, sint32 extra_trigs, sint32 dyn, sint32 uw, sint16 roomnum);
sint32 GetWaterHeight(sint32 x, sint32 y, sint32 z, sint16 room_number);

void TriggerUnderwaterExplosion(ITEM_INFO *item)
{
    sint32 wh, hfw, lp;

    TriggerExplosionBubble(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
    TriggerExplosionSparks(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 2, -2, 1, item->room_number);
    for (lp = 0; lp < 3; lp++)
        TriggerExplosionSparks(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, 2, -1, 1, item->room_number);
    wh = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
    if (wh != NO_HEIGHT) {
        hfw = item->pos.y_pos - wh;
        if (hfw < 2048) {
            hfw = 2048 - hfw;
            splash_setup.x = item->pos.x_pos;
            splash_setup.y = room[item->room_number].maxceiling;
            splash_setup.z = item->pos.z_pos;
            splash_setup.InnerXZoff = 16 + (hfw >> 6);
            splash_setup.InnerXZsize = 12 + (hfw >> 6);
            splash_setup.InnerYsize = -96;
            splash_setup.InnerXZvel = 0xa0;
            splash_setup.InnerYvel = (-hfw << 3) - 4096;
            splash_setup.InnerGravity = 0x60;
            splash_setup.InnerFriction = 7;
            splash_setup.MiddleXZoff = 24 + (hfw >> 6);
            splash_setup.MiddleXZsize = 24 + (hfw >> 6);
            splash_setup.MiddleYsize = -64;
            splash_setup.MiddleXZvel = 0xe0;
            splash_setup.MiddleYvel = (-hfw << 2) - 3072;
            splash_setup.MiddleGravity = 0x38;
            splash_setup.MiddleFriction = 8;
            splash_setup.OuterXZoff = 32 + (hfw >> 6);
            splash_setup.OuterXZsize = 32 + (hfw >> 6);
            splash_setup.OuterXZvel = 0x110;
            splash_setup.OuterFriction = 9;
            SetupSplash(&splash_setup);
        }
    }
}
