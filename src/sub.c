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

sint32 GetFreeSpark(void);

void BackgroundCollision(ITEM_INFO *v, ITEM_INFO *l, SUBINFO *sub)
{
    int height;
    COLL_INFO cinfo, *coll = &cinfo;

    coll->bad_pos = -NO_HEIGHT;
    coll->bad_neg = -400;
    coll->bad_ceiling = 400;
    coll->old.x = v->pos.x_pos;
    coll->old.y = v->pos.y_pos;
    coll->old.z = v->pos.z_pos;
    coll->radius = 300;
    coll->trigger = NULL;
    coll->slopes_are_walls = 0;
    coll->slopes_are_pits = 0;
    coll->lava_is_pit = 0;
    coll->enable_hit = 0;
    coll->enable_baddie_push = 1;
    if (v->pos.x_rot >= -16384 && v->pos.x_rot <= 16384)
        coll->facing = lara.move_angle = v->pos.y_rot;
    else
        coll->facing = lara.move_angle = v->pos.y_rot - 32768;
    height = phd_sin(v->pos.x_rot) * 1024 >> W2V_SHIFT;
    if (height < 0)
        height = -height;
    if (height < 200)
        height = 200;
    coll->bad_neg = -height;
    GetCollisionInfo(coll, v->pos.x_pos, v->pos.y_pos + height / 2, v->pos.z_pos, v->room_number, height);
    ShiftItem(v, coll);
    if (coll->coll_type == COLL_FRONT) {
        if (sub->RotX > 0x1ffe0000)
            sub->RotX += 0x16c0000;
        else if (sub->RotX < -0x1ffe0000)
            sub->RotX -= 0x16c0000;
        else
            sub->Vel = 0;
    } else if (coll->coll_type == COLL_TOP) {
        if (sub->RotX >= -0x1ffe0000)
            sub->RotX -= 0x16c0000;
    } else if (coll->coll_type == COLL_TOPFRONT)
        sub->Vel = 0;
    else if (coll->coll_type == COLL_LEFT)
        v->pos.y_rot += 5 * 182;
    else if (coll->coll_type == COLL_RIGHT)
        v->pos.y_rot -= 5 * 182;
    else if (coll->coll_type == COLL_CLAMP) {
        v->pos.x_pos = coll->old.x;
        v->pos.y_pos = coll->old.y;
        v->pos.z_pos = coll->old.z;
        sub->Vel = 0;
        return;
    }
    if (coll->mid_floor < 0) {
        v->pos.y_pos += coll->mid_floor;
        sub->RotX += 0x16c0000;
    }
}

void TriggerSubMist(sint32 x, sint32 y, sint32 z, sint32 speed, sint16 angle)
{
    sint32 size, xv, zv;
    SPARKS *sptr;

    sptr = &sparks[GetFreeSpark()];
    sptr->On = 1;
    sptr->sR = 0;
    sptr->sG = 0;
    sptr->sB = 0;
    sptr->dR = 64;
    sptr->dG = 64;
    sptr->dB = 64;
    sptr->ColFadeSpeed = 4 + (GetRandomControl() & 3);
    sptr->FadeToBlack = 12;
    sptr->sLife = sptr->Life = (GetRandomControl() & 3) + 20;
    sptr->TransType = 2;
    sptr->extras = 0;
    sptr->Dynamic = -1;
    sptr->x = x + ((GetRandomControl() & 15) - 8);
    sptr->y = y + ((GetRandomControl() & 15) - 8);
    sptr->z = z + ((GetRandomControl() & 15) - 8);
    zv = (speed * phd_cos(angle)) >> (W2V_SHIFT + 2);
    xv = (speed * phd_sin(angle)) >> (W2V_SHIFT + 2);
    sptr->Xvel = xv + ((GetRandomControl() & 127) - 64);
    sptr->Yvel = 0;
    sptr->Zvel = zv + ((GetRandomControl() & 127) - 64);
    sptr->Friction = 3;
    if (GetRandomControl() & 1) {
        sptr->Flags = 2 | 8 | 16 | 512;
        sptr->RotAng = GetRandomControl() & 4095;
        if (GetRandomControl() & 1)
            sptr->RotAdd = -(GetRandomControl() & 15) - 16;
        else
            sptr->RotAdd = (GetRandomControl() & 15) + 16;
    } else
        sptr->Flags = 2 | 8 | 512;
    sptr->Def = objects[296].mesh_index;
    sptr->Scalar = 3;
    sptr->Gravity = sptr->MaxYvel = 0;
    size = (GetRandomControl() & 7) + (speed >> 1) + 16;
    sptr->Width = sptr->sWidth = size >> 2;
    sptr->Height = sptr->sHeight = size >> 2;
    sptr->dWidth = size;
    sptr->dHeight = size;
}
