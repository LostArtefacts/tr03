#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "game.h"

extern sint16 HoldStates[14];

int CheckForHoldingState(int state)
{
    sint16 *holds = HoldStates;

    if (lara.extra_anim)
        return 0;

    while (*holds >= 0)
    {
        if (state == *holds)
            return 1;
        holds++;
    }
    return 0;
}

sint16 *GetBestFrame(ITEM_INFO *item);

void find_target_point(ITEM_INFO *item, GAME_VECTOR *target)
{
    sint16 *bounds;
    sint32 x, y, z, c, s;

    bounds = GetBestFrame(item);
    x = (sint32)((bounds[0] + bounds[1]) / 2);
    y = (sint32)(bounds[2] + (bounds[3] - bounds[2]) / 3);
    z = (sint32)((bounds[4] + bounds[5]) / 2);
    c = phd_cos(item->pos.y_rot);
    s = phd_sin(item->pos.y_rot);
    target->x = item->pos.x_pos + ((c * x + s * z) >> W2V_SHIFT);
    target->y = item->pos.y_pos + y;
    target->z = item->pos.z_pos + ((c * z - s * x) >> W2V_SHIFT);
    target->room_number = item->room_number;
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

void SmashWindow(sint16 item_number);

void AimWeapon(WEAPON_INFO *winfo, LARA_ARM *arm)
{
    PHD_ANGLE curr, speed;
    PHD_ANGLE destx, desty;

    speed = winfo->aim_speed;
    if (arm->lock) {
        desty = lara.target_angles[0];
        destx = lara.target_angles[1];
    } else {
        desty = destx = 0;
    }
    curr = arm->y_rot;
    if (curr >= desty - speed && curr <= desty + speed)
        curr = desty;
    else if (curr < desty)
        curr += speed;
    else
        curr -= speed;
    arm->y_rot = curr;
    curr = arm->x_rot;
    if (curr >= destx - speed && curr <= destx + speed)
        curr = destx;
    else if (curr < destx)
        curr += speed;
    else
        curr -= speed;
    arm->x_rot = curr;
    arm->z_rot = 0;
}

void TriggerRicochetSpark(GAME_VECTOR *pos, sint32 angle, int size);
sint16 DoBloodSplat(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num);

void HitTarget(ITEM_INFO *item, GAME_VECTOR *hitpos, int damage)
{
    sint32 lp;

    if (item->hit_points > 0 && item->hit_points <= damage)
        savegame.kills++;
    item->hit_points -= damage;
    item->hit_status = 1;
    if (item->data != NULL)
        ((CREATURE_INFO *)item->data)->hurt_by_lara = 1;
    if (hitpos) {
        if (item->object_number != 36 && item->object_number != ROBOT_SENTRY_GUN && item->object_number != TARGETS) {
            DoBloodSplat(hitpos->x, hitpos->y, hitpos->z, item->speed, item->pos.y_rot, item->room_number);
        } else if (item->object_number == ROBOT_SENTRY_GUN) {
            for (lp = 0; lp < 3; lp++)
                TriggerRicochetSpark(hitpos, GetRandomControl() & 4095, 0);
            SoundEffect(10, &item->pos, 0);
        }
    }
}

void SmashItem(sint16 item_number, int weapon_type)
{
    ITEM_INFO *item;

    item = &items[item_number];
    switch (item->object_number)
    {
    case SMASH_WINDOW:
    case SMASH_OBJECT1:
    case SMASH_OBJECT2:
    case SMASH_OBJECT3:
        SmashWindow(item_number);
        break;
    case CARCASS:
    case EXTRAFX6:
        if (item->status != ACTIVE)
        {
            item->status = ACTIVE;
            AddActiveItem(item_number);
        }
        break;
    }
}

int WeaponObject(int weapon_type)
{
    switch (weapon_type)
    {
    case LG_MAGNUMS:
        return MAGNUM;
    case LG_UZIS:
        return UZI;
    case LG_SHOTGUN:
        return SHOTGUN;
    case LG_ROCKET:
        return ROCKET_GUN;
    case LG_GRENADE:
        return GRENADE_GUN;
    case LG_HARPOON:
        return HARPOON_GUN;
    case LG_M16:
        return M16;
    default:
        return PISTOLS;
    }
}

void draw_pistol_meshes(int weapon_type);
void draw_shotgun_meshes(int weapon_type);

void InitialiseNewWeapon(void)
{
    lara.left_arm.frame_number = lara.right_arm.frame_number = 0;
    lara.left_arm.x_rot = lara.left_arm.y_rot = lara.left_arm.z_rot = 0;
    lara.right_arm.x_rot = lara.right_arm.y_rot = lara.right_arm.z_rot = 0;
    lara.target = NULL;
    lara.left_arm.lock = lara.right_arm.lock = 0;
    lara.left_arm.flash_gun = lara.right_arm.flash_gun = 0;
    switch (lara.gun_type) {
    case LG_PISTOLS:
    case LG_UZIS:
        lara.left_arm.frame_base = lara.right_arm.frame_base = objects[PISTOLS].frame_base;
        if (lara.gun_status != LG_ARMLESS)
            draw_pistol_meshes(lara.gun_type);
        break;
    case LG_SHOTGUN:
    case LG_HARPOON:
    case LG_M16:
    case LG_ROCKET:
    case LG_GRENADE:
    case LG_MAGNUMS:
        lara.left_arm.frame_base = lara.right_arm.frame_base = objects[WeaponObject(lara.gun_type)].frame_base;
        if (lara.gun_status != LG_ARMLESS)
            draw_shotgun_meshes(lara.gun_type);
        break;
    case LG_FLARE:
        lara.left_arm.frame_base = lara.right_arm.frame_base = objects[FLARE].frame_base;
        if (lara.gun_status != LG_ARMLESS)
            draw_flare_meshes();
        break;
    default:
        lara.left_arm.frame_base = lara.right_arm.frame_base = anims[lara_item->anim_number].frame_ptr;
        break;
    }
}

void phd_GetVectorAngles(sint32 x, sint32 y, sint32 z, PHD_ANGLE *dest);

void LaraTargetInfo(WEAPON_INFO *winfo)
{
    ITEM_INFO *item;
    PHD_ANGLE ang[2];
    GAME_VECTOR src, target;

    if ((item = lara.target) == NULL) {
        lara.left_arm.lock = lara.right_arm.lock = 0;
        lara.target_angles[0] = lara.target_angles[1] = 0;
        return;
    }
    src.x = lara_item->pos.x_pos;
    src.y = lara_item->pos.y_pos - 650;
    src.z = lara_item->pos.z_pos;
    src.room_number = lara_item->room_number;
    find_target_point(item, &target);
    phd_GetVectorAngles(target.x - src.x, target.y - src.y, target.z - src.z, ang);
    ang[0] -= lara_item->pos.y_rot;
    ang[1] -= lara_item->pos.x_rot;
    if (LOS(&src, &target)) {
        if (ang[0] >= winfo->lock_angles[0] && ang[0] <= winfo->lock_angles[1] && ang[1] >= winfo->lock_angles[2] &&
            ang[1] <= winfo->lock_angles[3])
            lara.left_arm.lock = lara.right_arm.lock = 1;
        else {
            if (lara.left_arm.lock) {
                if (ang[0] < winfo->left_angles[0] || ang[0] > winfo->left_angles[1] || ang[1] < winfo->left_angles[2] ||
                    ang[1] > winfo->left_angles[3])
                    lara.left_arm.lock = 0;
            }
            if (lara.right_arm.lock) {
                if (ang[0] < winfo->right_angles[0] || ang[0] > winfo->right_angles[1] ||
                    ang[1] < winfo->right_angles[2] || ang[1] > winfo->right_angles[3])
                    lara.right_arm.lock = 0;
            }
        }
    } else
        lara.left_arm.lock = lara.right_arm.lock = 0;
    lara.target_angles[0] = ang[0];
    lara.target_angles[1] = ang[1];
}

extern sint16 next_item_active;

void LaraGetNewTarget(WEAPON_INFO *winfo)
{
    int x, y, z;
    int dist, maxdist, maxdist2, bestdist;
    ITEM_INFO *item, *bestitem;
    PHD_ANGLE ang[2], bestyrot, yrot;
    GAME_VECTOR src, target;
    sint16 item_num;

    item = NULL;
    src.x = lara_item->pos.x_pos;
    src.y = lara_item->pos.y_pos - 650;
    src.z = lara_item->pos.z_pos;
    src.room_number = lara_item->room_number;
    bestitem = NULL;
    bestyrot = 32767;
    bestdist = 0x7fffffff;
    maxdist = winfo->target_dist;
    maxdist2 = maxdist * maxdist;
    for (item_num = next_item_active; item_num != NO_ITEM; item_num = item->next_active) {
        if (item_num == NO_ITEM || item_num == lara.item_number)
            continue;
        item = &items[item_num];
        if (item->hit_points <= 0)
            continue;
        if (objects[item->object_number].intelligent == 0)
            continue;
        x = item->pos.x_pos - src.x;
        y = item->pos.y_pos - src.y;
        z = item->pos.z_pos - src.z;
        if (ABS(x) > maxdist || ABS(y) > maxdist || ABS(z) > maxdist)
            continue;
        dist = x * x + y * y + z * z;
        if (dist < maxdist2) {
            find_target_point(item, &target);
            if (LOS(&src, &target)) {
                phd_GetVectorAngles(target.x - src.x, target.y - src.y, target.z - src.z, ang);
                ang[0] -= lara_item->pos.y_rot + lara.torso_y_rot;
                ang[1] -= lara_item->pos.x_rot + lara.torso_x_rot;
                if (ang[0] >= winfo->lock_angles[0] && ang[0] <= winfo->lock_angles[1] && ang[1] >= winfo->lock_angles[2] && ang[1] <= winfo->lock_angles[3]) {
                    yrot = ABS(ang[0]);
                    if (yrot < bestyrot + ONE_DEGREE * 15 && dist < bestdist) {
                        bestdist = dist;
                        bestyrot = yrot;
                        bestitem = item;
                    }
                }
            }
        }
    }
    lara.target = bestitem;
    LaraTargetInfo(winfo);
}

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint32 r;
} SPHERE;

extern char TribeBossShieldOn;
extern uint32 CurrentLevel;
extern WEAPON_INFO weapons[];

void phd_GenerateW2V(PHD_3DPOS *viewpos);
int GetSpheres(ITEM_INFO *item, SPHERE *ptr, int WorldSpace);
int ObjectOnLOS(GAME_VECTOR *start, GAME_VECTOR *target);
int LOS(GAME_VECTOR *start, GAME_VECTOR *target);
void Richochet(GAME_VECTOR *pos);
void FindClosestShieldPoint(sint32 x, sint32 y, sint32 z, ITEM_INFO *item);
sint32 mGetAngle(sint32 x1, sint32 y1, sint32 x2, sint32 y2);

int FireWeapon(int weapon_type, ITEM_INFO *target, ITEM_INFO *src, PHD_ANGLE *angles)
{
    int r, i, nums, bestdist, best;
    AMMO_INFO *ammo;
    WEAPON_INFO *winfo;
    PHD_3DPOS view;
    SPHERE *sptr, slist[33];
    GAME_VECTOR vsrc, vdest;
    FLOOR_INFO *floor;
    sint16 room_number;
    sint16 smash_item, angle;
    sint32 ricochet_angle;
    sint32 x, z;

    switch (weapon_type) {
    default:
        ammo = &lara.pistols;
        ammo->ammo = 1000;
        break;
    case LG_MAGNUMS:
        ammo = &lara.magnums;
        if (savegame.bonus_flag)
            ammo->ammo = 1000;
        break;
    case LG_UZIS:
        ammo = &lara.uzis;
        if (savegame.bonus_flag)
            ammo->ammo = 1000;
        break;
    case LG_M16:
        ammo = &lara.m16;
        if (savegame.bonus_flag)
            ammo->ammo = 1000;
        break;
    case LG_SHOTGUN:
        ammo = &lara.shotgun;
        if (savegame.bonus_flag)
            ammo->ammo = 1000;
        break;
    }

    if (ammo->ammo > 0) {
        ammo->ammo--;
        winfo = &weapons[weapon_type];
        view.x_pos = src->pos.x_pos;
        view.y_pos = src->pos.y_pos - winfo->gun_height;
        view.z_pos = src->pos.z_pos;
        r = (int)((GetRandomControl() - 16384) * winfo->shot_accuracy) / 65536;
        view.x_rot = *(angles + 1) + (sint16)r;
        r = (int)((GetRandomControl() - 16384) * winfo->shot_accuracy) / 65536;
        view.y_rot = *(angles + 0) + (sint16)r;
        view.z_rot = 0;
        phd_GenerateW2V(&view);

        nums = GetSpheres(target, slist, 0);
        sptr = &slist[0];
        best = -1;
        bestdist = 0x7fffffff;
        for (i = 0; i < nums; i++, sptr++) {
            r = sptr->r;
            if (ABS(sptr->x) < r && ABS(sptr->y) < r && sptr->z > r && (sptr->x * sptr->x) + (sptr->y * sptr->y) <= (r * r)) {
                if (sptr->z - r < bestdist) {
                    bestdist = sptr->z - r;
                    best = i;
                }
            }
        }

        lara.has_fired = 1;
        savegame.ammo_used++;
        vsrc.x = view.x_pos;
        vsrc.y = view.y_pos;
        vsrc.z = view.z_pos;
        room_number = src->room_number;
        floor = GetFloor(view.x_pos, view.y_pos, view.z_pos, &room_number);
        vsrc.room_number = room_number;

        if (best >= 0) {
            savegame.ammo_hit++;
            vdest.x = vsrc.x + ((*(phd_mxptr + 8) * bestdist) >> W2V_SHIFT);
            vdest.y = vsrc.y + ((*(phd_mxptr + 9) * bestdist) >> W2V_SHIFT);
            vdest.z = vsrc.z + ((*(phd_mxptr + 10) * bestdist) >> W2V_SHIFT);
            smash_item = ObjectOnLOS(&vsrc, &vdest);
            if (smash_item != NO_ITEM) {
                if (!(items[smash_item].object_number == 102 && CurrentLevel == 6))
                    SmashItem(smash_item, weapon_type);
            }
            if (target->object_number != 70 && target->object_number != 361 && target->object_number != 57
                && (target->object_number != 36 || TribeBossShieldOn == 0))
                HitTarget(target, &vdest, winfo->damage);
            else if (target->object_number == 36) {
                sint32 dx, dy, dz;

                dx = (vdest.x - vsrc.x) >> 5;
                dy = (vdest.y - vsrc.y) >> 5;
                dz = (vdest.z - vsrc.z) >> 5;
                FindClosestShieldPoint(vdest.x - dx, vdest.y - dy, vdest.z - dz, target);
            } else if (target->object_number == 361 || target->object_number == 57) {
                target->hit_status = 1;
                target->hit_points--;
                ricochet_angle = (mGetAngle(lara_item->pos.z_pos, lara_item->pos.x_pos, target->pos.z_pos, target->pos.x_pos) >> 4) & 4095;
                TriggerRicochetSpark(&vdest, ricochet_angle, 16);
                SoundEffect(10, &target->pos, 0);
            } else {
                z = target->pos.z_pos - lara_item->pos.z_pos;
                x = target->pos.x_pos - lara_item->pos.x_pos;
                angle = 0x8000 + phd_atan(z, x) - target->pos.y_rot;
                if ((target->current_anim_state > 1 && target->current_anim_state < 5) && angle < 0x4000 && angle > -0x4000) {
                    target->hit_status = 1;
                    ricochet_angle = (mGetAngle(lara_item->pos.z_pos, lara_item->pos.x_pos, target->pos.z_pos, target->pos.x_pos) >> 4) & 4095;
                    TriggerRicochetSpark(&vdest, ricochet_angle, 16);
                    SoundEffect(10, &target->pos, 0);
                } else
                    HitTarget(target, &vdest, winfo->damage);
            }
            return 1;
        } else {
            vdest.x = vsrc.x + ((*(phd_mxptr + 8) * winfo->target_dist) >> W2V_SHIFT);
            vdest.y = vsrc.y + ((*(phd_mxptr + 9) * winfo->target_dist) >> W2V_SHIFT);
            vdest.z = vsrc.z + ((*(phd_mxptr + 10) * winfo->target_dist) >> W2V_SHIFT);
            best = LOS(&vsrc, &vdest);
            smash_item = ObjectOnLOS(&vsrc, &vdest);
            if (smash_item != NO_ITEM) {
                if (!(items[smash_item].object_number == 102 && CurrentLevel == 6))
                    SmashItem(smash_item, weapon_type);
                else
                    Richochet(&vdest);
            } else if (!best)
                Richochet(&vdest);
            return -1;
        }
    }
    ammo->ammo = 0;
    return 0;
}

int Inv_RequestItem(int item_num);
void CreateFlare(int thrown);
void undraw_flare_meshes(void);
void draw_pistols(int weapon_type);
void undraw_pistols(int weapon_type);
void draw_shotgun(int weapon_type);
void undraw_shotgun(int weapon_type);
void draw_flare(void);
void undraw_flare(void);
void DoFlareInHand(int flare_age);
void set_flare_arm(int frame);
void PistolHandler(int weapon_type);
void RifleHandler(int weapon_type);

void LaraGun(void)
{
    if (lara.left_arm.flash_gun > 0)
        lara.left_arm.flash_gun--;
    if (lara.right_arm.flash_gun > 0)
        lara.right_arm.flash_gun--;

    if (lara_item->hit_points <= 0)
        lara.gun_status = LG_ARMLESS;
    else if (lara.gun_status == LG_ARMLESS) {
        if (input & IN_DRAW)
            lara.request_gun_type = lara.last_gun_type;
        else if (input & IN_F) {
            if (lara.gun_type == LG_FLARE)
                lara.gun_status = LG_UNDRAW;
            else if (Inv_RequestItem(178))
                lara.request_gun_type = LG_FLARE;
        }
        if (lara.request_gun_type != lara.gun_type || (input & IN_DRAW)) {
            if (lara_item->current_anim_state == 71
                && (lara.request_gun_type == LG_SHOTGUN || lara.request_gun_type == LG_M16 || lara.request_gun_type == LG_ROCKET
                    || lara.request_gun_type == LG_GRENADE || lara.request_gun_type == LG_HARPOON))
                return;
            if (lara.request_gun_type == LG_FLARE
                || (lara.skidoo == NO_ITEM
                    && (lara.request_gun_type == LG_HARPOON || lara.water_status == LARA_ABOVEWATER
                        || (lara.water_status == LARA_WADE && lara.water_surface_dist > -weapons[lara.gun_type].gun_height)))) {
                if (lara.gun_type == LG_FLARE) {
                    CreateFlare(0);
                    undraw_flare_meshes();
                    lara.flare_control_left = 0;
                }
                lara.gun_type = lara.request_gun_type;
                InitialiseNewWeapon();
                lara.gun_status = LG_DRAW;
                lara.left_arm.frame_number = lara.right_arm.frame_number = 0;
            } else {
                lara.last_gun_type = lara.request_gun_type;
                if (lara.gun_type == LG_FLARE)
                    lara.request_gun_type = LG_FLARE;
                else
                    lara.gun_type = lara.request_gun_type;
            }
        }
    } else if (lara.gun_status == LG_READY) {
        if ((input & IN_F) && Inv_RequestItem(178))
            lara.request_gun_type = LG_FLARE;
        if ((input & IN_DRAW) || lara.request_gun_type != lara.gun_type)
            lara.gun_status = LG_UNDRAW;
        else if (lara.gun_type != LG_HARPOON && lara.water_status != LARA_ABOVEWATER
                 && (lara.water_status != LARA_WADE || lara.water_surface_dist < -weapons[lara.gun_type].gun_height))
            lara.gun_status = LG_UNDRAW;
    }

    switch (lara.gun_status) {
    case LG_DRAW:
        if (lara.gun_type != LG_FLARE && lara.gun_type != LG_UNARMED)
            lara.last_gun_type = lara.gun_type;
        switch (lara.gun_type) {
        case LG_PISTOLS:
        case LG_MAGNUMS:
        case LG_UZIS:
            if (camera.type != CINEMATIC_CAMERA && camera.type != LOOK_CAMERA)
                camera.type = COMBAT_CAMERA;
            draw_pistols(lara.gun_type);
            break;
        case LG_SHOTGUN:
        case LG_HARPOON:
        case LG_ROCKET:
        case LG_GRENADE:
        case LG_M16:
            if (camera.type != CINEMATIC_CAMERA && camera.type != LOOK_CAMERA)
                camera.type = COMBAT_CAMERA;
            draw_shotgun(lara.gun_type);
            break;
        case LG_FLARE:
            draw_flare();
            break;
        default:
            lara.gun_status = LG_ARMLESS;
            break;
        }
        break;
    case LG_UNDRAW:
        lara.mesh_ptrs[14] = meshes[objects[0].mesh_index + 14];
        switch (lara.gun_type) {
        case LG_PISTOLS:
        case LG_MAGNUMS:
        case LG_UZIS:
            undraw_pistols(lara.gun_type);
            break;
        case LG_SHOTGUN:
        case LG_HARPOON:
        case LG_ROCKET:
        case LG_GRENADE:
        case LG_M16:
            undraw_shotgun(lara.gun_type);
            break;
        case LG_FLARE:
            undraw_flare();
            break;
        default:
            break;
        }
        break;
    case LG_SPECIAL:
        draw_flare();
        break;
    case LG_READY:
        if (lara.pistols.ammo && (input & IN_ACTION))
            lara.mesh_ptrs[14] = meshes[objects[5].mesh_index + 14];
        else
            lara.mesh_ptrs[14] = meshes[objects[0].mesh_index + 14];
        if (camera.type != CINEMATIC_CAMERA && camera.type != LOOK_CAMERA)
            camera.type = COMBAT_CAMERA;
        if (input & IN_ACTION) {
            AMMO_INFO *ammo;

            switch (lara.gun_type) {
            case LG_SHOTGUN:
                ammo = &lara.shotgun;
                break;
            case LG_MAGNUMS:
                ammo = &lara.magnums;
                break;
            case LG_UZIS:
                ammo = &lara.uzis;
                break;
            case LG_ROCKET:
                ammo = &lara.rocket;
                break;
            case LG_GRENADE:
                ammo = &lara.grenade;
                break;
            case LG_HARPOON:
                ammo = &lara.harpoon;
                break;
            case LG_M16:
                ammo = &lara.m16;
                break;
            default:
                ammo = &lara.pistols;
            }
            if (ammo->ammo <= 0) {
                ammo->ammo = 0;
                SoundEffect(48, &lara_item->pos, 0);
                if (Inv_RequestItem(160))
                    lara.request_gun_type = LG_PISTOLS;
                else
                    lara.request_gun_type = LG_UNARMED;
                break;
            }
        }
        switch (lara.gun_type) {
        case LG_PISTOLS:
        case LG_UZIS:
            PistolHandler(lara.gun_type);
            break;
        case LG_SHOTGUN:
        case LG_HARPOON:
        case LG_M16:
        case LG_ROCKET:
        case LG_GRENADE:
        case LG_MAGNUMS:
            RifleHandler(lara.gun_type);
            break;
        }
        break;
    case LG_ARMLESS:
        if (lara.gun_type == LG_FLARE) {
            if (lara.skidoo != NO_ITEM || CheckForHoldingState(lara_item->current_anim_state)) {
                if (!lara.flare_control_left) {
                    lara.left_arm.frame_number = 95;
                    lara.flare_control_left = 1;
                } else if (lara.left_arm.frame_number != 0) {
                    lara.left_arm.frame_number++;
                    if (lara.left_arm.frame_number == 110)
                        lara.left_arm.frame_number = 0;
                }
            } else
                lara.flare_control_left = 0;
            DoFlareInHand(lara.flare_age);
            set_flare_arm(lara.left_arm.frame_number);
        }
        break;
    case LG_HANDSBUSY:
        if (lara.gun_type == LG_FLARE) {
            lara.flare_control_left = (lara.skidoo != NO_ITEM || CheckForHoldingState(lara_item->current_anim_state));
            DoFlareInHand(lara.flare_age);
            set_flare_arm(lara.left_arm.frame_number);
        }
        break;
    }
}

#define BIG_SLOPE 2
#define DIAGONAL 3

extern int height_type;
extern sint32 OnObject;
extern sint32 tiltxoff, tiltyoff;

int abs(int x);
#pragma intrinsic(abs)

sint32 GetHeight(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);

void DoProperDetection(sint16 item_number, sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv)
{
    ITEM_INFO *item;
    sint16 room_number;
    sint32 ceiling, height, oldtype, oldonobj, oldheight;
    sint32 bs, yang;
    FLOOR_INFO *floor;

    item = &items[item_number];
    room_number = item->room_number;
    floor = GetFloor(x, y, z, &room_number);
    oldheight = GetHeight(floor, x, y, z);
    oldonobj = OnObject;
    oldtype = height_type;
    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    height = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

    if (item->pos.y_pos >= height) {
        bs = 0;
        if ((height_type == BIG_SLOPE || height_type == DIAGONAL) && oldheight < height) {
            yang = (sint32)((uint16)item->pos.y_rot);
            if (tiltyoff < 0) {
                if (yang >= 0x8000)
                    bs = 1;
            } else if (tiltyoff > 0) {
                if (yang <= 0x8000)
                    bs = 1;
            }
            if (tiltxoff < 0) {
                if (yang >= 0x4000 && yang <= 0xc000)
                    bs = 1;
            } else if (tiltxoff > 0) {
                if (yang <= 0x4000 || yang >= 0xc000)
                    bs = 1;
            }
        }

        if (y > (height + 32) && bs == 0
            && (((x >> WALL_SHIFT) != (item->pos.x_pos >> WALL_SHIFT))
                || ((z >> WALL_SHIFT) != (item->pos.z_pos >> WALL_SHIFT)))) {
            sint32 xs;

            if ((x & (~(WALL_L - 1))) != (item->pos.x_pos & (~(WALL_L - 1)))
                && (z & (~(WALL_L - 1))) != (item->pos.z_pos & (~(WALL_L - 1)))) {
                if (abs(x - item->pos.x_pos) < abs(z - item->pos.z_pos))
                    xs = 1;
                else
                    xs = 0;
            } else
                xs = 1;

            if ((x & (~(WALL_L - 1))) != (item->pos.x_pos & (~(WALL_L - 1))) && xs) {
                if (xv <= 0)
                    item->pos.y_rot = 0x4000 + (0xc000 - item->pos.y_rot);
                else
                    item->pos.y_rot = 0xc000 + (0x4000 - item->pos.y_rot);
            } else
                item->pos.y_rot = 0x8000 - item->pos.y_rot;
            item->speed >>= 1;
            item->pos.x_pos = x;
            item->pos.y_pos = y;
            item->pos.z_pos = z;
        } else if (height_type == BIG_SLOPE || height_type == DIAGONAL) {
            item->speed -= item->speed >> 2;
            if (tiltyoff < 0 && ((abs(tiltyoff) - abs(tiltxoff) >= 2))) {
                if (((uint16)item->pos.y_rot) > 0x8000) {
                    item->pos.y_rot = 0x4000 + (0xc000 - (uint16)item->pos.y_rot - 1);
                    if (item->fallspeed > 0)
                        item->fallspeed = -(item->fallspeed >> 1);
                } else {
                    if (item->speed < 32) {
                        item->speed -= tiltyoff << 1;
                        if ((uint16)item->pos.y_rot > 0x4000 && (uint16)item->pos.y_rot < 0xc000) {
                            item->pos.y_rot -= 4096;
                            if ((uint16)item->pos.y_rot < 0x4000)
                                item->pos.y_rot = 0x4000;
                        } else if ((uint16)item->pos.y_rot < 0x4000) {
                            item->pos.y_rot += 4096;
                            if ((uint16)item->pos.y_rot > 0x4000)
                                item->pos.y_rot = 0x4000;
                        }
                    }
                    if (item->fallspeed > 0)
                        item->fallspeed = -(item->fallspeed >> 1);
                    else
                        item->fallspeed = 0;
                }
            } else if (tiltyoff > 0 && ((abs(tiltyoff) - abs(tiltxoff) >= 2))) {
                if (((uint16)item->pos.y_rot) < 0x8000) {
                    item->pos.y_rot = 0xc000 + (0x4000 - (uint16)item->pos.y_rot - 1);
                    if (item->fallspeed > 0)
                        item->fallspeed = -(item->fallspeed >> 1);
                } else {
                    if (item->speed < 32) {
                        item->speed += tiltyoff << 1;
                        if ((uint16)item->pos.y_rot > 0xc000 || (uint16)item->pos.y_rot < 0x4000) {
                            item->pos.y_rot -= 4096;
                            if ((uint16)item->pos.y_rot < 0xc000)
                                item->pos.y_rot = 0xc000;
                        } else if ((uint16)item->pos.y_rot < 0xc000) {
                            item->pos.y_rot += 4096;
                            if ((uint16)item->pos.y_rot > 0xc000)
                                item->pos.y_rot = 0xc000;
                        }
                    }
                    if (item->fallspeed > 0)
                        item->fallspeed = -(item->fallspeed >> 1);
                    else
                        item->fallspeed = 0;
                }
            } else if (tiltxoff < 0 && ((abs(tiltxoff) - abs(tiltyoff) >= 2))) {
                if (((uint16)item->pos.y_rot) > 0x4000 && ((uint16)item->pos.y_rot) < 0xc000) {
                    item->pos.y_rot = (0x8000 - item->pos.y_rot - 1);
                    if (item->fallspeed > 0)
                        item->fallspeed = -(item->fallspeed >> 1);
                } else {
                    if (item->speed < 32) {
                        item->speed -= tiltxoff << 1;
                        if ((uint16)item->pos.y_rot < 0x8000) {
                            item->pos.y_rot -= 4096;
                            if ((uint16)item->pos.y_rot > 0xf000)
                                item->pos.y_rot = 0;
                        } else if ((uint16)item->pos.y_rot >= 0x8000) {
                            item->pos.y_rot += 4096;
                            if ((uint16)item->pos.y_rot < 0x1000)
                                item->pos.y_rot = 0;
                        }
                    }
                    if (item->fallspeed > 0)
                        item->fallspeed = -(item->fallspeed >> 1);
                    else
                        item->fallspeed = 0;
                }
            } else if (tiltxoff > 0 && ((abs(tiltxoff) - abs(tiltyoff) >= 2))) {
                if (((uint16)item->pos.y_rot) > 0xc000 || ((uint16)item->pos.y_rot) < 0x4000) {
                    item->pos.y_rot = (0x8000 - item->pos.y_rot - 1);
                    if (item->fallspeed > 0)
                        item->fallspeed = -(item->fallspeed >> 1);
                } else {
                    if (item->speed < 32) {
                        item->speed += tiltxoff << 1;
                        if ((uint16)item->pos.y_rot > 0x8000) {
                            item->pos.y_rot -= 4096;
                            if ((uint16)item->pos.y_rot < 0x8000)
                                item->pos.y_rot = 0x8000;
                        } else if ((uint16)item->pos.y_rot < 0x8000) {
                            item->pos.y_rot += 4096;
                            if ((uint16)item->pos.y_rot > 0x8000)
                                item->pos.y_rot = 0x8000;
                        }
                    }
                    if (item->fallspeed > 0)
                        item->fallspeed = -(item->fallspeed >> 1);
                    else
                        item->fallspeed = 0;
                }
            } else if (tiltyoff < 0 && tiltxoff < 0) {
                if (((uint16)item->pos.y_rot) > 0x6000 && ((uint16)item->pos.y_rot) < 0xe000) {
                    item->pos.y_rot = 0x2000 + (0xa000 - (uint16)item->pos.y_rot - 1);
                    if (item->fallspeed > 0)
                        item->fallspeed = -(item->fallspeed >> 1);
                } else {
                    if (item->speed < 32) {
                        item->speed += (-tiltyoff) + (-tiltxoff);
                        if ((uint16)item->pos.y_rot > 0x2000 && (uint16)item->pos.y_rot < 0xa000) {
                            item->pos.y_rot -= 4096;
                            if ((uint16)item->pos.y_rot < 0x2000)
                                item->pos.y_rot = 0x2000;
                        } else if ((uint16)item->pos.y_rot != 0x2000) {
                            item->pos.y_rot += 4096;
                            if ((uint16)item->pos.y_rot > 0x2000)
                                item->pos.y_rot = 0x2000;
                        }
                    }
                    if (item->fallspeed > 0)
                        item->fallspeed = -(item->fallspeed >> 1);
                    else
                        item->fallspeed = 0;
                }
            } else if (tiltyoff < 0 && tiltxoff > 0) {
                if (((uint16)item->pos.y_rot) > 0xa000 || ((uint16)item->pos.y_rot) < 0x2000) {
                    item->pos.y_rot = 0x6000 + (0xe000 - (uint16)item->pos.y_rot - 1);
                    if (item->fallspeed > 0)
                        item->fallspeed = -(item->fallspeed >> 1);
                } else {
                    if (item->speed < 32) {
                        item->speed += (-tiltyoff) + tiltxoff;
                        if ((uint16)item->pos.y_rot < 0xe000 && (uint16)item->pos.y_rot > 0x6000) {
                            item->pos.y_rot -= 4096;
                            if ((uint16)item->pos.y_rot < 0x6000)
                                item->pos.y_rot = 0x6000;
                        } else if ((uint16)item->pos.y_rot != 0x6000) {
                            item->pos.y_rot += 4096;
                            if ((uint16)item->pos.y_rot > 0x6000)
                                item->pos.y_rot = 0x6000;
                        }
                    }
                    if (item->fallspeed > 0)
                        item->fallspeed = -(item->fallspeed >> 1);
                    else
                        item->fallspeed = 0;
                }
            } else if (tiltyoff > 0 && tiltxoff > 0) {
                if (((uint16)item->pos.y_rot) > 0xe000 || ((uint16)item->pos.y_rot) < 0x6000) {
                    item->pos.y_rot = 0xa000 + (0x2000 - (uint16)item->pos.y_rot - 1);
                    if (item->fallspeed > 0)
                        item->fallspeed = -(item->fallspeed >> 1);
                } else {
                    if (item->speed < 32) {
                        item->speed += tiltyoff + tiltxoff;
                        if ((uint16)item->pos.y_rot < 0x2000 || (uint16)item->pos.y_rot > 0xa000) {
                            item->pos.y_rot -= 4096;
                            if ((uint16)item->pos.y_rot < 0xa000)
                                item->pos.y_rot = 0xa000;
                        } else if ((uint16)item->pos.y_rot != 0xa000) {
                            item->pos.y_rot += 4096;
                            if ((uint16)item->pos.y_rot > 0xa000)
                                item->pos.y_rot = 0xa000;
                        }
                    }
                    if (item->fallspeed > 0)
                        item->fallspeed = -(item->fallspeed >> 1);
                    else
                        item->fallspeed = 0;
                }
            } else if (tiltyoff > 0 && tiltxoff < 0) {
                if (((uint16)item->pos.y_rot) > 0x2000 && ((uint16)item->pos.y_rot) < 0xa000) {
                    item->pos.y_rot = 0xe000 + (0x6000 - (uint16)item->pos.y_rot - 1);
                    if (item->fallspeed > 0)
                        item->fallspeed = -(item->fallspeed >> 1);
                } else {
                    if (item->speed < 32) {
                        item->speed += tiltyoff + (-tiltxoff);
                        if ((uint16)item->pos.y_rot < 0x6000 || (uint16)item->pos.y_rot > 0xe000) {
                            item->pos.y_rot -= 4096;
                            if ((uint16)item->pos.y_rot < 0xe000)
                                item->pos.y_rot = 0xe000;
                        } else if ((uint16)item->pos.y_rot != 0xe000) {
                            item->pos.y_rot += 4096;
                            if ((uint16)item->pos.y_rot > 0xe000)
                                item->pos.y_rot = 0xe000;
                        }
                    }
                    if (item->fallspeed > 0)
                        item->fallspeed = -(item->fallspeed >> 1);
                    else
                        item->fallspeed = 0;
                }
            }
            item->pos.x_pos = x;
            item->pos.y_pos = y;
            item->pos.z_pos = z;
        } else {
            if (item->fallspeed > 0) {
                if (item->fallspeed > 16) {
                    if (item->object_number == 311)
                        item->fallspeed = -(item->fallspeed - (item->fallspeed >> 1));
                    else {
                        item->fallspeed = -(item->fallspeed >> 2);
                        if (item->fallspeed < -100)
                            item->fallspeed = -100;
                    }
                } else {
                    item->fallspeed = 0;
                    if (item->object_number == 311) {
                        item->required_anim_state = 1;
                        item->pos.x_rot = 0;
                        item->speed--;
                    } else
                        item->speed -= 3;
                    if (item->speed < 0)
                        item->speed = 0;
                }
            }
            item->pos.y_pos = height;
        }
    } else {
        if (yv >= 0) {
            room_number = item->room_number;
            floor = GetFloor(item->pos.x_pos, y, item->pos.z_pos, &room_number);
            oldheight = GetHeight(floor, item->pos.x_pos, y, item->pos.z_pos);
            oldonobj = OnObject;
            room_number = item->room_number;
            floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
            GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
            if (item->pos.y_pos >= oldheight && oldonobj) {
                if (item->fallspeed > 0) {
                    if (item->fallspeed > 16) {
                        if (item->object_number == 311)
                            item->fallspeed = -(item->fallspeed - (item->fallspeed >> 1));
                        else {
                            item->fallspeed = -(item->fallspeed >> 2);
                            if (item->fallspeed < -100)
                                item->fallspeed = -100;
                        }
                    } else {
                        item->fallspeed = 0;
                        if (item->object_number == 311) {
                            item->required_anim_state = 1;
                            item->pos.x_rot = 0;
                            item->speed--;
                        } else
                            item->speed -= 3;
                        if (item->speed < 0)
                            item->speed = 0;
                    }
                }
                item->pos.y_pos = oldheight;
            }
        }

        room_number = item->room_number;
        floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
        ceiling = GetCeiling(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
        if (item->pos.y_pos < ceiling) {
            if (y < ceiling
                && (((x >> WALL_SHIFT) != (item->pos.x_pos >> WALL_SHIFT))
                    || ((z >> WALL_SHIFT) != (item->pos.z_pos >> WALL_SHIFT)))) {
                if ((x & (~(WALL_L - 1))) != (item->pos.x_pos & (~(WALL_L - 1)))) {
                    if (xv <= 0)
                        item->pos.y_rot = 0x4000 + (0xc000 - item->pos.y_rot);
                    else
                        item->pos.y_rot = 0xc000 + (0x4000 - item->pos.y_rot);
                } else {
                    item->pos.y_rot = 0x8000 - item->pos.y_rot;
                }
                if (item->object_number == 311)
                    item->speed -= item->speed >> 3;
                else
                    item->speed >>= 1;
                item->pos.x_pos = x;
                item->pos.y_pos = y;
                item->pos.z_pos = z;
            } else
                item->pos.y_pos = ceiling;
            if (item->fallspeed < 0)
                item->fallspeed = -item->fallspeed;
        }
    }

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (room_number != item->room_number)
        ItemNewRoom(item_number, room_number);
}
