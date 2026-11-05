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

sint16 CreateItem(void);
void InitialiseItem(sint16 item_num);

void CreateFlare(int thrown)
{
    sint16 flare_item;
    ITEM_INFO *flare;
    PHD_VECTOR pos;
    FLOOR_INFO *floor;

    flare_item = CreateItem();
    if (flare_item != NO_ITEM) {
        flare = &items[flare_item];
        flare->object_number = 179;
        flare->room_number = lara_item->room_number;
        pos.x = -16;
        pos.y = 32;
        pos.z = 42;
        GetLaraHandAbsPosition(&pos, 0);
        floor = GetFloor(pos.x, pos.y, pos.z, &flare->room_number);
        if (GetHeight(floor, pos.x, pos.y, pos.z) < pos.y) {
            flare->pos.x_pos = lara_item->pos.x_pos;
            flare->pos.y_pos = pos.y;
            flare->pos.z_pos = lara_item->pos.z_pos;
            flare->pos.y_rot = lara_item->pos.y_rot;
            flare->room_number = lara_item->room_number;
        } else {
            flare->pos.x_pos = pos.x;
            flare->pos.y_pos = pos.y;
            flare->pos.z_pos = pos.z;
            if (thrown)
                flare->pos.y_rot = lara_item->pos.y_rot;
            else
                flare->pos.y_rot = lara_item->pos.y_rot - 0x2000;
        }
        InitialiseItem(flare_item);
        flare->pos.z_rot = 0;
        flare->pos.x_rot = 0;
        flare->shade = -1;
        if (thrown) {
            flare->speed = lara_item->speed + 50;
            flare->fallspeed = lara_item->fallspeed - 50;
        } else {
            flare->speed = lara_item->speed + 10;
            flare->fallspeed = lara_item->fallspeed + 50;
        }
        if (DoFlareLight((PHD_VECTOR *)&flare->pos, lara.flare_age))
            flare->data = (void *)(lara.flare_age | 0x8000);
        else
            flare->data = (void *)(lara.flare_age & 0x7fff);
        AddActiveItem(flare_item);
        flare->status = ACTIVE;
    }
}

void ready_flare(void);
int Inv_RemoveItem(int item_num);

void draw_flare(void)
{
    sint16 ani;

    if (lara_item->current_anim_state == AS_FLAREPICKUP || lara_item->current_anim_state == AS_PICKUP) {
        DoFlareInHand(lara.flare_age);
        lara.flare_control_left = 0;
        lara.left_arm.frame_number = FL_2HOLD_F - 2;
        set_flare_arm(lara.left_arm.frame_number);
        return;
    }
    ani = lara.left_arm.frame_number;
    ani++;
    lara.flare_control_left = 1;
    if (ani < FL_DRAW_F || ani > FL_2HOLD_F - 1)
        ani = FL_DRAW_F;
    else if (ani == 46) {
        draw_flare_meshes();
        if (!savegame.bonus_flag)
            Inv_RemoveItem(178);
    } else if (ani >= FL_IGNITE_F && ani <= FL_2HOLD_F - 2) {
        if (ani == FL_IGNITE_F) {
            if (room[lara_item->room_number].flags & UNDERWATER)
                SoundEffect(11, &lara_item->pos, UNDERWATER);
            else
                SoundEffect(11, &lara_item->pos, 0);
            lara.flare_age = 0;
        }
        DoFlareInHand(lara.flare_age);
    } else if (ani == FL_2HOLD_F - 1) {
        ready_flare();
        ani = 0;
        DoFlareInHand(lara.flare_age);
    }
    lara.left_arm.frame_number = ani;
    set_flare_arm(lara.left_arm.frame_number);
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

void DoProperDetection(sint16 item_number, sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv);

void FlareControl(sint16 item_number)
{
    ITEM_INFO *flare;
    sint32 x, y, z, flare_age, xv, yv, zv;

    flare = &items[item_number];
    if (room[flare->room_number].flags & 0x80) {
        KillItem(item_number);
        return;
    }
    if (flare->fallspeed) {
        flare->pos.x_rot += 182 * 3;
        flare->pos.z_rot += 182 * 5;
    } else {
        flare->pos.x_rot = 0;
        flare->pos.z_rot = 0;
    }
    x = flare->pos.x_pos;
    y = flare->pos.y_pos;
    z = flare->pos.z_pos;
    flare->pos.x_pos += (xv = flare->speed * phd_sin(flare->pos.y_rot) >> W2V_SHIFT);
    flare->pos.z_pos += (zv = flare->speed * phd_cos(flare->pos.y_rot) >> W2V_SHIFT);
    if (room[flare->room_number].flags & UNDERWATER) {
        flare->fallspeed += (5 - flare->fallspeed) / 2;
        flare->speed += (5 - flare->speed) / 2;
    } else
        flare->fallspeed += 6;
    flare->pos.y_pos += (yv = flare->fallspeed);
    DoProperDetection(item_number, x, y, z, xv, yv, zv);
    flare_age = (sint32)flare->data & 0x7fff;
    if (flare_age < 900)
        flare_age++;
    else if (flare->fallspeed == 0 && flare->speed == 0) {
        KillItem(item_number);
        return;
    }
    if (DoFlareLight((PHD_VECTOR *)&flare->pos, flare_age)) {
        flare_age |= 0x8000;
        if (room[flare->room_number].flags & UNDERWATER) {
            SoundEffect(12, &flare->pos, 1);
            if (GetRandomControl() < 0x4000)
                CreateBubble((PHD_3DPOS *)&flare->pos, flare->room_number, 8, 8);
        } else
            SoundEffect(12, &flare->pos, 4 + 0x2000000);
    }
    flare->data = (void *)flare_age;
}
