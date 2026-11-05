#include "game.h"

typedef struct {
    sint16 RotX;
    sint16 RotY;
    sint16 StartRotY;
    char Flags;
    char FireCount;
} BIGGUNINFO;

void BigGunInitialise(sint16 item_number)
{
    ITEM_INFO *v;
    BIGGUNINFO *gun;

    v = &items[item_number];
    gun = v->data = game_malloc(sizeof(BIGGUNINFO), 0);
    gun->Flags = gun->FireCount = 0;
    gun->RotX = 30;
    gun->RotY = 0;
    gun->StartRotY = v->pos.y_rot;
}

#define BIGGUN 18

int CanUseGun(ITEM_INFO *v, ITEM_INFO *l);
void CreateFlare(int thrown);
void ObjectCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);

void BigGunCollision(sint16 item_number, ITEM_INFO *l, COLL_INFO *coll)
{
    ITEM_INFO *v;
    BIGGUNINFO *gun;

    if (l->hit_points < 0 || lara.skidoo != NO_ITEM)
        return;
    v = &items[item_number];
    if (CanUseGun(v, l)) {
        lara.skidoo = item_number;
        if (lara.gun_type == LG_FLARE) {
            CreateFlare(0);
            undraw_flare_meshes();
            lara.flare_control_left = 0;
            lara.request_gun_type = lara.gun_type = LG_ARMLESS;
        }
        lara.gun_status = LG_HANDSBUSY;
        l->pos.x_pos = v->pos.x_pos;
        l->pos.y_pos = v->pos.y_pos;
        l->pos.z_pos = v->pos.z_pos;
        l->pos.x_rot = v->pos.x_rot;
        l->pos.y_rot = v->pos.y_rot;
        l->pos.z_rot = v->pos.z_rot;
        l->anim_number = objects[VEHICLE_ANIM].anim_index;
        l->frame_number = anims[objects[BIGGUN].anim_index].frame_base;
        l->current_anim_state = l->goal_anim_state = 0;
        gun = (BIGGUNINFO *)v->data;
        gun->Flags = 0;
        gun->RotX = 30;
    } else
        ObjectCollision(item_number, l, coll);
}

int CanUseGun(ITEM_INFO *v, ITEM_INFO *l)
{
    int dist;
    sint32 x, z;

    if (!(input & IN_ACTION) || lara.gun_status != LG_ARMLESS || l->gravity_status)
        return 0;

    x = l->pos.x_pos - v->pos.x_pos;
    z = l->pos.z_pos - v->pos.z_pos;
    dist = x * x + z * z;
    if (dist > 30000)
        return 0;
    else
        return 1;
}
