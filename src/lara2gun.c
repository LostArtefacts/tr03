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
