#include "game.h"

typedef struct {
    sint16 ObjectNum;
    sint8 Draw1Anim2;
    sint8 Draw1Anim;
    sint8 Draw2Anim;
    sint8 RecoilAnim;
} PISTOL_DEF;

extern PISTOL_DEF PistolTable[4];

void set_arm_info(LARA_ARM *arm, int frame);
void draw_pistol_meshes(int weapon_type);
void ready_pistols(int weapon_type);

void draw_pistols(int weapon_type)
{
    sint16 ani;
    PISTOL_DEF *p;

    p = &PistolTable[lara.gun_type];
    ani = lara.left_arm.frame_number + 1;
    if (ani < p->Draw1Anim || ani > p->RecoilAnim - 1)
        ani = p->Draw1Anim;
    else if (ani == p->Draw2Anim)
    {
        draw_pistol_meshes(weapon_type);
        SoundEffect(6, &lara_item->pos, 0);
    }
    else if (ani == p->RecoilAnim - 1)
    {
        ready_pistols(weapon_type);
        ani = 0;
    }
    set_arm_info(&lara.right_arm, ani);
    set_arm_info(&lara.left_arm, ani);
}

void set_arm_info(LARA_ARM *arm, int frame)
{
    int anim_base;
    PISTOL_DEF *p;

    p = &PistolTable[lara.gun_type];
    anim_base = objects[p->ObjectNum].anim_index;
    if (frame < p->Draw1Anim)
        arm->anim_number = anim_base;
    else if (frame < p->Draw2Anim)
        arm->anim_number = anim_base + 1;
    else if (frame < p->RecoilAnim)
        arm->anim_number = anim_base + 2;
    else
        arm->anim_number = anim_base + 3;
    arm->frame_base = anims[arm->anim_number].frame_ptr;
    arm->frame_number = frame;
}

void ready_pistols(int weapon_type)
{
    lara.gun_status = LG_READY;
    lara.left_arm.x_rot = lara.left_arm.y_rot = lara.left_arm.z_rot = 0;
    lara.right_arm.x_rot = lara.right_arm.y_rot = lara.right_arm.z_rot = 0;
    lara.left_arm.frame_number = lara.right_arm.frame_number = 0;
    lara.target = NULL;
    lara.left_arm.lock = lara.right_arm.lock = 0;
    lara.left_arm.frame_base = lara.right_arm.frame_base = objects[WeaponObject(weapon_type)].frame_base;
}

void draw_pistol_meshes(int weapon_type)
{
    int obj;

    obj = WeaponObject(weapon_type);
    lara.mesh_ptrs[HAND_R] = meshes[objects[obj].mesh_index + HAND_R];
    lara.mesh_ptrs[THIGH_R] = meshes[objects[LARA].mesh_index + THIGH_R];
    if (weapon_type != LG_MAGNUMS)
    {
        lara.mesh_ptrs[HAND_L] = meshes[objects[obj].mesh_index + HAND_L];
        lara.mesh_ptrs[THIGH_L] = meshes[objects[LARA].mesh_index + THIGH_L];
    }
}

void undraw_pistol_mesh_left(int weapon_type)
{
    if (weapon_type != LG_MAGNUMS)
    {
        int obj;

        obj = WeaponObject(weapon_type);
        lara.mesh_ptrs[THIGH_L] = meshes[objects[obj].mesh_index + THIGH_L];
        lara.mesh_ptrs[HAND_L] = meshes[objects[LARA].mesh_index + HAND_L];
    }
}

void undraw_pistol_mesh_right(int weapon_type)
{
    int obj;

    obj = WeaponObject(weapon_type);
    lara.mesh_ptrs[THIGH_R] = meshes[objects[obj].mesh_index + THIGH_R];
    lara.mesh_ptrs[HAND_R] = meshes[objects[LARA].mesh_index + HAND_R];
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

void LaraTargetInfo(WEAPON_INFO *winfo);
void LaraGetNewTarget(WEAPON_INFO *winfo);
void AimWeapon(WEAPON_INFO *winfo, LARA_ARM *arm);
void AnimatePistols(int weapon_type);
void GetLaraHandAbsPosition(PHD_VECTOR *vec, int hand);

void PistolHandler(int weapon_type)
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
    AimWeapon(winfo, &lara.right_arm);
    if (lara.left_arm.lock && !lara.right_arm.lock)
    {
        lara.torso_y_rot = lara.left_arm.y_rot / 2;
        lara.torso_x_rot = lara.left_arm.x_rot / 2;
        if (camera.old_type != LOOK_CAMERA)
        {
            lara.head_y_rot = lara.torso_y_rot;
            lara.head_x_rot = lara.torso_x_rot;
        }
    }
    else if (lara.right_arm.lock && !lara.left_arm.lock)
    {
        lara.torso_y_rot = lara.right_arm.y_rot / 2;
        lara.torso_x_rot = lara.right_arm.x_rot / 2;
        if (camera.old_type != LOOK_CAMERA)
        {
            lara.head_y_rot = lara.torso_y_rot;
            lara.head_x_rot = lara.torso_x_rot;
        }
    }
    else if (lara.left_arm.lock && lara.right_arm.lock)
    {
        lara.torso_y_rot = (lara.left_arm.y_rot + lara.right_arm.y_rot) / 4;
        lara.torso_x_rot = (lara.left_arm.x_rot + lara.right_arm.x_rot) / 4;
        if (camera.old_type != LOOK_CAMERA)
        {
            lara.head_y_rot = lara.torso_y_rot;
            lara.head_x_rot = lara.torso_x_rot;
        }
    }
    AnimatePistols(weapon_type);
    if (lara.left_arm.flash_gun || lara.right_arm.flash_gun)
    {
        PHD_VECTOR pos;

        pos.x = (GetRandomControl() & 255) - 128;
        pos.y = (GetRandomControl() & 127) - 63;
        pos.z = (GetRandomControl() & 255) - 128;
        GetLaraHandAbsPosition(&pos, lara.left_arm.flash_gun ? 0 : 1);
        TriggerDynamic(pos.x, pos.y, pos.z, 10, (GetRandomControl() & 7) + 24, (GetRandomControl() & 3) + 16, GetRandomControl() & 7);
    }
}
