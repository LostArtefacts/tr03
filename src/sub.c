#include "game.h"
#include <stdlib.h>

#define NUM_WAKE_SPRITES 32
#define SF_SURFACE 2

typedef struct {
    sint32 Vel;
    sint32 Rot;
    sint32 RotX;
    sint16 FanRot;
    char Flags;
    char WeaponTimer;
} SUBINFO;

typedef struct {
    sint32 x[2];
    sint32 y[2];
    sint32 z[2];
    uint8 life;
    uint8 pad[3];
} SUB_WAKE_PTS;

extern SUB_WAKE_PTS SubWakePts[NUM_WAKE_SPRITES][2];

void SubInitialise(sint16 item_number)
{
    ITEM_INFO *v;
    SUBINFO *sub;
    sint32 i;

    v = &items[item_number];
    sub = v->data = game_malloc(sizeof(SUBINFO), 0);
    sub->Vel = sub->Rot = 0;
    sub->Flags = SF_SURFACE;
    sub->WeaponTimer = 0;
    for (i = 0; i < NUM_WAKE_SPRITES; i++)
    {
        SubWakePts[i][0].life = 0;
        SubWakePts[i][1].life = 0;
    }
}

int GetOnSub(sint16 item_number, COLL_INFO *coll)
{
    int dist;
    sint32 x, y, z;
    ITEM_INFO *v, *l;
    FLOOR_INFO *floor;
    sint16 room_number;

    l = lara_item;
    v = &items[item_number];
    if (!(input & IN_ACTION) || lara.gun_status != LG_ARMLESS || l->gravity_status)
        return 0;
    y = abs(l->pos.y_pos - (v->pos.y_pos - 128));
    if (y > 256)
        return 0;
    x = l->pos.x_pos - v->pos.x_pos;
    z = l->pos.z_pos - v->pos.z_pos;
    dist = x * x + z * z;
    if (dist > 512 * 512)
        return 0;
    room_number = v->room_number;
    floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);
    if (GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos) < -32000)
        return 0;
    return 1;
}

extern char SubHarpoonLR;

sint16 CreateItem(void);
void InitialiseItem(sint16 item_num);

void FireSubHarpoon(ITEM_INFO *v)
{
    sint16 item_number;
    PHD_VECTOR pos;
    ITEM_INFO *item;

    if (lara.harpoon.ammo <= 0)
        return;
    if ((item_number = CreateItem()) != NO_ITEM) {
        item = &items[item_number];
        item->object_number = 310;
        item->shade = 0x4210 | 0x8000;
        item->room_number = v->room_number;
        pos.x = SubHarpoonLR ? 22 : -22;
        pos.y = 24;
        pos.z = 230;
        GetJointAbsPosition(v, &pos, 3);
        item->pos.x_pos = pos.x;
        item->pos.y_pos = pos.y;
        item->pos.z_pos = pos.z;
        InitialiseItem(item_number);
        item->pos.x_rot = v->pos.x_rot;
        item->pos.y_rot = v->pos.y_rot;
        item->pos.z_rot = 0;
        item->fallspeed = (sint16)(-256 * phd_sin(item->pos.x_rot) >> W2V_SHIFT);
        item->speed = (sint16)(256 * phd_cos(item->pos.x_rot) >> W2V_SHIFT);
        item->hit_points = 256;
        item->item_flags[0] = 1;
        AddActiveItem(item_number);
        SoundEffect(23, &lara_item->pos, 2);
        if (!savegame.bonus_flag)
            lara.harpoon.ammo--;
        savegame.ammo_used++;
        SubHarpoonLR ^= 1;
    }
}
