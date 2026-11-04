#include "game.h"

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
