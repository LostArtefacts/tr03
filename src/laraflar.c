#include "game.h"

#define FL_THROW_F 1
#define FL_DRAW_F 33
#define FL_IGNITE_F 72
#define FL_2HOLD_F 95

void GetLaraHandAbsPosition(PHD_VECTOR *vec, int hand);
int DoFlareLight(PHD_VECTOR *pos, int flare_age);
void CreateBubble(PHD_3DPOS *pos, sint16 room_number, sint32 size, sint32 sizerange);

void DoFlareInHand(int flare_age)
{
    PHD_VECTOR pos;

    pos.x = 11;
    pos.y = 32;
    pos.z = 41;
    GetLaraHandAbsPosition(&pos, 0);
    lara.left_arm.flash_gun = DoFlareLight(&pos, flare_age);
    if (lara.flare_age < 900) {
        lara.flare_age++;
        if (room[lara_item->room_number].flags & UNDERWATER) {
            SoundEffect(12, &lara_item->pos, 1);
            if (GetRandomControl() < 0x4000)
                CreateBubble((PHD_3DPOS *)&pos, lara_item->room_number, 8, 8);
        } else {
            SoundEffect(12, &lara_item->pos, 0x2000004);
        }
    } else if (lara.gun_status == LG_ARMLESS) {
        lara.gun_status = LG_UNDRAW;
    }
}

void set_flare_arm(int frame)
{
    int anim_base;

    anim_base = objects[FLARE].anim_index;
    if (frame < FL_THROW_F)
        lara.left_arm.anim_number = anim_base;
    else if (frame < FL_DRAW_F)
        lara.left_arm.anim_number = anim_base + 1;
    else if (frame < FL_IGNITE_F)
        lara.left_arm.anim_number = anim_base + 2;
    else if (frame < FL_2HOLD_F)
        lara.left_arm.anim_number = anim_base + 3;
    else
        lara.left_arm.anim_number = anim_base + 4;
    lara.left_arm.frame_base = anims[lara.left_arm.anim_number].frame_ptr;
}

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
