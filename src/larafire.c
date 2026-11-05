#include "game.h"

extern sint16 HoldStates[14];

void SmashWindow(sint16 item_number);

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
