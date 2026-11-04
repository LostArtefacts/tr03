#include "game.h"

void draw_flare_meshes(void)
{
    lara.mesh_ptrs[HAND_L] = meshes[objects[FLARE].mesh_index + HAND_L];
}

void undraw_flare_meshes(void)
{
    lara.mesh_ptrs[HAND_L] = meshes[objects[LARA].mesh_index + HAND_L];
}

void ready_flare(void)
{
    lara.gun_status = LG_ARMLESS;
    lara.left_arm.x_rot = lara.left_arm.y_rot = lara.left_arm.z_rot = 0;
    lara.right_arm.x_rot = lara.right_arm.y_rot = lara.right_arm.z_rot = 0;
    lara.left_arm.lock = lara.right_arm.lock = 0;
    lara.target = NULL;
}
