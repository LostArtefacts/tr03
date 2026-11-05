#include "game.h"

typedef struct {
    sint16 RotX;
    sint16 RotY;
    sint16 StartRotY;
    char Flags;
    char FireCount;
} BIGGUNINFO;

extern sint32 SmokeCountL;
extern sint32 SmokeWeapon;

void TriggerGunSmoke(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv, sint32 initial, sint32 weapon, sint32 count);
sint16 CreateItem(void);
void InitialiseItem(sint16 item_num);
void phd_PushMatrix(void);
void phd_PushUnitMatrix(void);
void phd_RotYXZ(sint16 ry, sint16 rx, sint16 rz);
void phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void TriggerRocketFlame(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv, sint32 item_no);

void FireBigGun(ITEM_INFO *v)
{
    sint16 item_number;

    if ((item_number = CreateItem()) != NO_ITEM) {
        int lp;
        PHD_VECTOR pos;
        ITEM_INFO *item;
        long wx, wy, wz, xv, yv, zv;
        BIGGUNINFO *gun = (BIGGUNINFO *)v->data;

        item = &items[item_number];

        item->object_number = 309;
        item->room_number = lara_item->room_number;

        pos.x = 0;
        pos.y = 0;
        pos.z = 256;
        GetJointAbsPosition(v, &pos, 2);

        item->pos.x_pos = pos.x;
        item->pos.y_pos = pos.y;
        item->pos.z_pos = pos.z;

        InitialiseItem(item_number);

        item->pos.x_rot = -((gun->RotX - 32) * ONE_DEGREE);
        item->pos.y_rot = v->pos.y_rot;
        item->pos.z_rot = 0;
        item->speed = 512 >> 5;
        item->item_flags[0] = 1;

        AddActiveItem(item_number);

        SoundEffect(77, &item->pos, 0);
        SoundEffect(105, &item->pos, 0x2000004);

        SmokeCountL = 32;
        SmokeWeapon = LG_ROCKET;

        for (lp = 0; lp < 5; lp++) {
            TriggerGunSmoke(pos.x, pos.y, pos.z, 0, 0, 0, 1, LG_ROCKET, 32);
        }

        phd_PushUnitMatrix();
        phd_mxptr[3] = 0;
        phd_mxptr[7] = 0;
        phd_mxptr[11] = 0;
        phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
        phd_PushMatrix();
        phd_TranslateRel(0, 0, -128);
        wx = phd_mxptr[3] >> W2V_SHIFT;
        wy = phd_mxptr[7] >> W2V_SHIFT;
        wz = phd_mxptr[11] >> W2V_SHIFT;
        phd_PopMatrix();

        for (lp = 0; lp < 8; lp++) {
            phd_PushMatrix();
            phd_TranslateRel(0, 0, -(GetRandomControl() & 2047));
            xv = phd_mxptr[3] >> W2V_SHIFT;
            yv = phd_mxptr[7] >> W2V_SHIFT;
            zv = phd_mxptr[11] >> W2V_SHIFT;
            phd_PopMatrix();
            TriggerRocketFlame(wx, wy, wz, xv - wx, yv - wy, zv - wz, item_number);
        }
        phd_PopMatrix();
    }
}

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

extern sint32 *bones;

int GetFrames(ITEM_INFO *item, sint16 *frmptr[], int *rate);
void phd_TranslateAbs(sint32 x, sint32 y, sint32 z);
int S_GetObjectBounds(sint16 *bptr);
void CalculateObjectLighting(ITEM_INFO *item, sint16 *frame);
void gar_RotYXZsuperpack(sint16 **pprot, int skip);
void gar_RotYXZsuperpack_I(sint16 **pprot1, sint16 **pprot2, int skip);
void phd_PutPolygons(sint16 *objptr, int clip);
void phd_PutPolygons_I(sint16 *ptr, int clip);
void InitInterpolate(int frac, int rate);
void phd_TranslateRel_I(sint32 x, sint32 y, sint32 z);
void phd_TranslateRel_ID(sint32 x, sint32 y, sint32 z, sint32 x2, sint32 y2, sint32 z2);

void BigGunDraw(ITEM_INFO *v)
{
    sint16 *frmptr[2];
    OBJECT_INFO *object;
    int clip, rate, frac;

    frac = GetFrames(v, frmptr, &rate);

    object = &objects[v->object_number];

    phd_PushMatrix();
    phd_TranslateAbs(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);
    phd_RotYXZ(v->pos.y_rot, v->pos.x_rot, v->pos.z_rot);

    if ((clip = S_GetObjectBounds(frmptr[0]))) {
        sint32 *bone;
        sint16 **meshpp;
        sint16 *rotation;
        BIGGUNINFO *gun = (BIGGUNINFO *)v->data;

        CalculateObjectLighting(v, frmptr[0]);

        meshpp = &meshes[object->mesh_index];
        bone = bones + object->bone_index;

        if (!frac) {
            phd_TranslateRel((sint32)*(frmptr[0] + 6), (sint32)*(frmptr[0] + 7), (sint32)*(frmptr[0] + 8));
            rotation = frmptr[0] + 9;
            gar_RotYXZsuperpack(&rotation, 0);
            phd_PutPolygons(*meshpp++, clip);

            phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
            gar_RotYXZsuperpack(&rotation, 0);
            phd_PutPolygons(*meshpp++, clip);

            phd_TranslateRel(*(bone + 1 + 4), *(bone + 2 + 4), *(bone + 3 + 4));
            gar_RotYXZsuperpack(&rotation, 0);
            phd_PutPolygons(*meshpp++, clip);
        } else {
            sint16 *rotation2;

            InitInterpolate(frac, rate);
            rotation = frmptr[0] + 9;
            rotation2 = frmptr[1] + 9;

            phd_TranslateRel_ID((sint32)*(frmptr[0] + 6), (sint32)*(frmptr[0] + 7), (sint32)*(frmptr[0] + 8),
                                (sint32)*(frmptr[1] + 6), (sint32)*(frmptr[1] + 7), (sint32)*(frmptr[1] + 8));
            gar_RotYXZsuperpack_I(&rotation, &rotation2, 0);
            phd_PutPolygons_I(*meshpp++, clip);

            phd_TranslateRel_I(*(bone + 1), *(bone + 2), *(bone + 3));
            gar_RotYXZsuperpack_I(&rotation, &rotation2, 0);
            phd_PutPolygons_I(*meshpp++, clip);

            phd_TranslateRel_I(*(bone + 1 + 4), *(bone + 2 + 4), *(bone + 3 + 4));
            gar_RotYXZsuperpack_I(&rotation, &rotation2, 0);
            phd_PutPolygons_I(*meshpp++, clip);
        }
    }
    phd_PopMatrix();
}
