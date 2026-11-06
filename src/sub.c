#include <windows.h>
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

int GetFrames(ITEM_INFO *item, sint16 *frmptr[], int *rate);
void S_PrintShadow(int size, sint16 *bptr, ITEM_INFO *item, int unknown);
void phd_PushMatrix(void);
void phd_TranslateAbs(sint32 x, sint32 y, sint32 z);
void phd_RotYXZ(sint16 ry, sint16 rx, sint16 rz);
int S_GetObjectBounds(sint16 *bptr);
void CalculateObjectLighting(ITEM_INFO *item, sint16 *frame);
int phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void gar_RotYXZsuperpack(sint16 **pprot, int skip);
void phd_PutPolygons(sint16 *objptr, int clip);
void phd_RotX(sint16 rx);
void phd_RotZ(sint16 rz);
void InitInterpolate(int frac, int rate);
void phd_TranslateRel_ID(sint32 x, sint32 y, sint32 z, sint32 x2, sint32 y2, sint32 z2);
void gar_RotYXZsuperpack_I(sint16 **pprot1, sint16 **pprot2, int skip);
void phd_PutPolygons_I(sint16 *ptr, int clip);
void phd_PushMatrix_I(void);
void phd_TranslateRel_I(sint32 x, sint32 y, sint32 z);
void phd_RotX_I(sint16 ang);
void phd_RotZ_I(sint16 ang);
void InterpolateMatrix(void);
void phd_PopMatrix_I(void);
int TriggerActive(ITEM_INFO *item);
void S_DrawWakeFX(ITEM_INFO *item);
extern sint32 *bones;

void SubDraw(ITEM_INFO *v)
{
    sint16 *frmptr[2];
    OBJECT_INFO *object;
    int clip, rate, frac;

    frac = GetFrames(v, frmptr, &rate);
    object = &objects[v->object_number];
    S_PrintShadow(256, frmptr[0], v, 0);
    phd_PushMatrix();
    phd_TranslateAbs(v->pos.x_pos, v->pos.y_pos + 128, v->pos.z_pos);
    phd_RotYXZ(v->pos.y_rot, v->pos.x_rot, v->pos.z_rot);
    if ((clip = S_GetObjectBounds(frmptr[0]))) {
        sint32 *bone;
        sint16 **meshpp;
        sint16 *rotation;
        SUBINFO *sub = (SUBINFO *)v->data;


        CalculateObjectLighting(v, frmptr[0]);
        meshpp = &meshes[object->mesh_index];
        bone = bones + object->bone_index;
        if (!frac) {
            phd_TranslateRel(*(frmptr[0] + 6), *(frmptr[0] + 7), *(frmptr[0] + 8));
            rotation = frmptr[0] + 9;
            gar_RotYXZsuperpack(&rotation, 0);
            phd_PutPolygons(*meshpp++, clip);
            phd_PushMatrix();
            phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
            gar_RotYXZsuperpack(&rotation, 0);
            phd_RotX(v->pos.z_rot + (sub->RotX >> 13));
            phd_PutPolygons(*meshpp++, clip);
            phd_PopMatrix();
            phd_PushMatrix();
            phd_TranslateRel(*(bone + 5), *(bone + 6), *(bone + 7));
            gar_RotYXZsuperpack(&rotation, 0);
            phd_RotX(-v->pos.z_rot + (sub->RotX >> 13));
            phd_PutPolygons(*meshpp++, clip);
            phd_PopMatrix();
            phd_PushMatrix();
            phd_TranslateRel(*(bone + 9), *(bone + 10), *(bone + 11));
            gar_RotYXZsuperpack(&rotation, 0);
            phd_RotZ(sub->FanRot);
            phd_PutPolygons(*meshpp++, clip);
            phd_PopMatrix();
        } else {
            sint16 *rotation2;

            InitInterpolate(frac, rate);
            rotation = frmptr[0] + 9;
            rotation2 = frmptr[1] + 9;
            phd_TranslateRel_ID(*(frmptr[0] + 6), *(frmptr[0] + 7), *(frmptr[0] + 8), *(frmptr[1] + 6), *(frmptr[1] + 7), *(frmptr[1] + 8));
            gar_RotYXZsuperpack_I(&rotation, &rotation2, 0);
            phd_PutPolygons_I(*meshpp++, clip);
            phd_PushMatrix_I();
            phd_TranslateRel_I(*(bone + 1), *(bone + 2), *(bone + 3));
            gar_RotYXZsuperpack_I(&rotation, &rotation2, 0);
            phd_RotX_I(v->pos.z_rot + (sub->RotX >> 13));
            InterpolateMatrix();
            phd_PutPolygons_I(*meshpp++, clip);
            phd_PopMatrix_I();
            phd_PushMatrix_I();
            phd_TranslateRel_I(*(bone + 5), *(bone + 6), *(bone + 7));
            gar_RotYXZsuperpack_I(&rotation, &rotation2, 0);
            phd_RotX_I(-v->pos.z_rot + (sub->RotX >> 13));
            phd_PutPolygons_I(*meshpp++, clip);
            phd_PopMatrix_I();
            phd_PushMatrix_I();
            phd_TranslateRel_I(*(bone + 9), *(bone + 10), *(bone + 11));
            gar_RotYXZsuperpack_I(&rotation, &rotation2, 0);
            phd_RotZ_I(sub->FanRot);
            phd_PutPolygons_I(*meshpp++, clip);
            phd_PopMatrix_I();
        }
    }
    phd_PopMatrix();
    if (TriggerActive(v))
        S_DrawWakeFX(v);
}

void UserInput(ITEM_INFO *v, ITEM_INFO *l, SUBINFO *sub);
void DoCurrent(ITEM_INFO *item);
void FireSubHarpoon(ITEM_INFO *v);
void BackgroundCollision(ITEM_INFO *v, ITEM_INFO *l, SUBINFO *sub);
sint32 GetWaterHeight(sint32 x, sint32 y, sint32 z, sint16 room_number);
void AnimateItem(ITEM_INFO *item);

int SubControl(void)
{
    sint32 h;
    SUBINFO *sub;
    ITEM_INFO *v, *l;
    FLOOR_INFO *floor;
    sint16 room_number;

    l = lara_item;
    v = &items[lara.skidoo];
    sub = (SUBINFO *)v->data;
    if (!(sub->Flags & 8)) {
        UserInput(v, l, sub);
        v->speed = sub->Vel >> 16;
        v->pos.x_rot += sub->RotX >> 16;
        v->pos.y_rot += sub->Rot >> 16;
        v->pos.z_rot = sub->Rot >> 12;
        if (v->pos.x_rot > 80 * ONE_DEGREE)
            v->pos.x_rot = 80 * ONE_DEGREE;
        else if (v->pos.x_rot < -80 * ONE_DEGREE)
            v->pos.x_rot = -80 * ONE_DEGREE;
        v->pos.x_pos += (((phd_sin(v->pos.y_rot) * v->speed) >> W2V_SHIFT) * phd_cos(v->pos.x_rot)) >> W2V_SHIFT;
        v->pos.y_pos -= (phd_sin(v->pos.x_rot) * v->speed) >> W2V_SHIFT;
        v->pos.z_pos += (((phd_cos(v->pos.y_rot) * v->speed) >> W2V_SHIFT) * phd_cos(v->pos.x_rot)) >> W2V_SHIFT;
    }

    room_number = v->room_number;
    floor = GetFloor(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, &room_number);
    v->floor = GetHeight(floor, v->pos.x_pos, v->pos.y_pos, v->pos.z_pos);

    if ((sub->Flags & 1) && !(sub->Flags & 8)) {
        h = GetWaterHeight(v->pos.x_pos, v->pos.y_pos, v->pos.z_pos, room_number);
        if (h != NO_HEIGHT && !(room[v->room_number].flags & UNDERWATER)) {
            if ((h - v->pos.y_pos) >= -210)
                v->pos.y_pos = h + 210;
            if (!(sub->Flags & SF_SURFACE)) {
                SoundEffect(36, &lara_item->pos, 2);
                sub->Flags &= ~4;
            }
            sub->Flags |= SF_SURFACE;
        } else if (h != NO_HEIGHT && (h - v->pos.y_pos) >= -210) {
            v->pos.y_pos = h + 210;
            if (!(sub->Flags & SF_SURFACE)) {
                SoundEffect(36, &lara_item->pos, 2);
                sub->Flags &= ~4;
            }
            sub->Flags |= SF_SURFACE;
        } else
            sub->Flags &= ~SF_SURFACE;

        if (!(sub->Flags & SF_SURFACE)) {
            if (l->hit_points >= 0) {
                lara.air--;
                if (lara.air < 0) {
                    lara.air = -1;
                    l->hit_points -= 5;
                }
            }
        } else {
            if (l->hit_points >= 0) {
                lara.air += 10;
                if (lara.air > 1800)
                    lara.air = 1800;
            }
        }
    }

    TestTriggers(trigger_index, 0);

    if (lara.skidoo != NO_ITEM && !(sub->Flags & 8)) {
        DoCurrent(v);
        if ((input & IN_ACTION) && (sub->Flags & 1) && !sub->WeaponTimer) {
            FireSubHarpoon(v);
            sub->WeaponTimer = 15;
        }
        if (room_number != v->room_number) {
            ItemNewRoom(lara.skidoo, room_number);
            ItemNewRoom(lara.item_number, room_number);
        }
        l->pos.x_pos = v->pos.x_pos;
        l->pos.y_pos = v->pos.y_pos + 128;
        l->pos.z_pos = v->pos.z_pos;
        l->pos.x_rot = v->pos.x_rot;
        l->pos.y_rot = v->pos.y_rot;
        l->pos.z_rot = v->pos.z_rot;
        AnimateItem(l);
        BackgroundCollision(v, l, sub);
        if (sub->Flags & 1)
            SoundEffect(346, (PHD_3DPOS *)&v->pos.x_pos, 2 | 4 | 0x1000000 | (v->speed << 16));
        v->anim_number = objects[19].anim_index - objects[VEHICLE_ANIM].anim_index + l->anim_number;
        v->frame_number = anims[v->anim_number].frame_base + (l->frame_number - anims[l->anim_number].frame_base);
        if (sub->Flags & SF_SURFACE)
            camera.target_elevation = -60 * ONE_DEGREE;
        else
            camera.target_elevation = 0;
        return 1;
    } else if (sub->Flags & 8) {
        AnimateItem(l);
        if (room_number != v->room_number)
            ItemNewRoom(lara.skidoo, room_number);
        BackgroundCollision(v, l, sub);
        sub->RotX = 0;
        v->anim_number = 5;
        v->frame_number = anims[objects[19].anim_index + 5].frame_base;
        v->goal_anim_state = v->current_anim_state = 5;
        v->fallspeed = 0;
        v->speed = 0;
        v->gravity_status = 1;
        AnimateItem(v);
        return 1;
    } else
        return 0;
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

extern BITE_INFO sub_bites[6];
extern uint8 SubWakeShade;
extern sint32 wibble;

void TriggerSubMist(sint32 x, sint32 y, sint32 z, sint32 speed, sint16 angle);
void CreateBubble(PHD_3DPOS *pos, sint16 room_number, sint32 size, sint32 sizerange);
int LOS(GAME_VECTOR *start, GAME_VECTOR *target);
void TriggerDynamic(sint32 x, sint32 y, sint32 z, sint32 falloff, sint32 r, sint32 g, sint32 b);
void DoWake_sub(ITEM_INFO *v, sint16 leftright);
void UpdateWakeFX_sub(void);

void SubEffects(sint16 item_number)
{
    ITEM_INFO *v;
    SUBINFO *sub;
    PHD_VECTOR pos;
    sint32 lp;

    v = &items[item_number];
    sub = (SUBINFO *)v->data;

    if (lara.skidoo == item_number) {
        if (!sub->Vel)
            sub->FanRot += ONE_DEGREE * 2;
        else
            sub->FanRot += sub->Vel >> 12;

        if (sub->Vel) {
            pos.x = sub_bites[0].x;
            pos.y = sub_bites[0].y;
            pos.z = sub_bites[0].z;
            GetJointAbsPosition(v, &pos, sub_bites[0].mesh_num);
            TriggerSubMist(pos.x, pos.y + 128, pos.z, abs(sub->Vel) >> 16, v->pos.y_rot + 0x8000);

            if ((GetRandomControl() & 1) == 0) {
                PHD_3DPOS pos3d;
                sint16 room_number;

                pos3d.x_pos = pos.x + (GetRandomControl() & 63) - 32;
                pos3d.y_pos = pos.y + 128;
                pos3d.z_pos = pos.z + (GetRandomControl() & 63) - 32;
                room_number = v->room_number;
                GetFloor(pos3d.x_pos, pos3d.y_pos, pos3d.z_pos, &room_number);
                CreateBubble(&pos3d, room_number, 4, 8);
            }
        }
    }

    for (lp = 0; lp < 2; lp++) {
        GAME_VECTOR source, target;
        sint32 r;

        r = 31 - (GetRandomControl() & 3);
        pos.x = sub_bites[1].x;
        pos.y = sub_bites[1].y;
        pos.z = sub_bites[1].z << (lp * 6);
        GetJointAbsPosition(v, &pos, sub_bites[1].mesh_num);

        if (lp == 1) {
            target.x = pos.x;
            target.y = pos.y;
            target.z = pos.z;
            target.room_number = v->room_number;
            LOS(&source, &target);
            pos.x = target.x;
            pos.y = target.y;
            pos.z = target.z;
        } else {
            source.x = pos.x;
            source.y = pos.y;
            source.z = pos.z;
            source.room_number = v->room_number;
        }

        TriggerDynamic(pos.x, pos.y, pos.z, 16 + (lp << 3), r, r, r);
    }

    if (!(wibble & 15) && sub->Vel) {
        DoWake_sub(v, 0);
        DoWake_sub(v, 1);
    }

    if (sub->Vel == 0) {
        if (SubWakeShade)
            SubWakeShade--;
    } else {
        if (SubWakeShade < 16)
            SubWakeShade++;
    }

    if (sub->WeaponTimer)
        sub->WeaponTimer--;

    UpdateWakeFX_sub();
}

static uint8 SubCurrentStartWake;

void DoWake_sub(ITEM_INFO *v, sint16 leftright)
{
    PHD_VECTOR pos;
    sint32 lp;

    if (!TriggerActive(v))
        return;

    if (SubWakePts[SubCurrentStartWake][leftright].life)
        return;
    SubWakePts[SubCurrentStartWake][leftright].life = 0x20;
    for (lp = 0; lp < 2; lp++) {
        pos.x = sub_bites[2 + (leftright << 1) + lp].x;
        pos.y = sub_bites[2 + (leftright << 1) + lp].y;
        pos.z = sub_bites[2 + (leftright << 1) + lp].z;
        GetJointAbsPosition(v, &pos, sub_bites[2 + (leftright << 1) + lp].mesh_num);
        SubWakePts[SubCurrentStartWake][leftright].x[lp] = pos.x;
        SubWakePts[SubCurrentStartWake][leftright].y[lp] = pos.y + 128;
        SubWakePts[SubCurrentStartWake][leftright].z[lp] = pos.z;
    }

    if (leftright == 1) {
        SubCurrentStartWake++;
        SubCurrentStartWake &= NUM_WAKE_SPRITES - 1;
    }
}


void UpdateWakeFX_sub(void)
{
    int i, j;

    for (i = 0; i < 2; i++) {
        for (j = 0; j < NUM_WAKE_SPRITES; j++) {
            if (SubWakePts[j][i].life)
                SubWakePts[j][i].life--;
        }
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

int GetOnSub(sint16 item_number, COLL_INFO *coll);
void CreateFlare(int thrown);
void ObjectCollisionSub(int item_num, ITEM_INFO *laraitem, COLL_INFO *coll);

void SubCollision(sint16 item_number, ITEM_INFO *l, COLL_INFO *coll)
{
    int geton;
    ITEM_INFO *v;

    if (l->hit_points < 0 || lara.skidoo != NO_ITEM)
        return;

    v = &items[item_number];
    if ((geton = GetOnSub(item_number, coll))) {
        lara.skidoo = item_number;
        lara.water_status = LARA_ABOVEWATER;
        if (lara.gun_type == LG_FLARE) {
            CreateFlare(0);
            undraw_flare_meshes();
            lara.flare_control_left = 0;
            lara.request_gun_type = lara.gun_type = LG_ARMLESS;
        }
        lara.gun_status = LG_HANDSBUSY;

        v->hit_points = 1;
        l->pos.x_pos = v->pos.x_pos;
        l->pos.y_pos = v->pos.y_pos;
        l->pos.z_pos = v->pos.z_pos;
        l->pos.y_rot = v->pos.y_rot;
        if (l->current_anim_state == AS_SURFTREAD || l->current_anim_state == AS_SURFSWIM) {
            l->anim_number = objects[VEHICLE_ANIM].anim_index + 10;
            l->frame_number = anims[objects[19].anim_index + 10].frame_base;
            l->current_anim_state = l->goal_anim_state = 8;
        } else {
            l->anim_number = objects[VEHICLE_ANIM].anim_index + 13;
            l->frame_number = anims[objects[19].anim_index + 13].frame_base;
            l->current_anim_state = l->goal_anim_state = 8;
        }
        AnimateItem(l);
    } else {
        v->pos.y_pos += 128;
        ObjectCollisionSub(item_number, l, coll);
        v->pos.y_pos -= 128;
    }
}

void GetLaraHandAbsPosition(PHD_VECTOR *vec, int joint);
int mgLOS(GAME_VECTOR *start, GAME_VECTOR *target, int push);
void UpdateLaraRoom(ITEM_INFO *item, int height);
sint32 GetWaterDepth(sint32 x, sint32 y, sint32 z, sint16 room_number);
int CanGetOff_sub(ITEM_INFO *v);

void UserInput(ITEM_INFO *v, ITEM_INFO *l, SUBINFO *sub)
{
    sint16 anim, frame;
    PHD_VECTOR vec;
    GAME_VECTOR VPos, LPos;
    sint32 wd, wh, hfw;

    CanGetOff_sub(v);
    anim = l->anim_number - objects[VEHICLE_ANIM].anim_index;
    frame = l->frame_number - anims[l->anim_number].frame_base;
    switch (l->current_anim_state) {
    case 4:
        if (l->hit_points <= 0) {
            l->goal_anim_state = 0;
            break;
        }
        if (input & IN_LEFT)
            sub->Rot -= 0x400000;
        else if (input & IN_RIGHT)
            sub->Rot += 0x400000;
        if (sub->Flags & SF_SURFACE) {
            if (v->pos.x_rot > ONE_DEGREE * 50)
                v->pos.x_rot -= ONE_DEGREE;
            else if (v->pos.x_rot < ONE_DEGREE * 50)
                v->pos.x_rot += ONE_DEGREE;
        } else {
            if (input & IN_FORWARD)
                sub->RotX -= (ONE_DEGREE * 2) << 16;
            else if (input & IN_BACK)
                sub->RotX += (ONE_DEGREE * 2) << 16;
        }
        if (input & IN_JUMP) {
            if ((sub->Flags & SF_SURFACE) && (input & IN_FORWARD) && v->pos.x_rot > -(ONE_DEGREE * 15))
                sub->Flags |= 4;
            sub->Vel += 0x40000;
        } else {
            l->goal_anim_state = 5;
        }
        break;
    case 5:
        if (l->hit_points <= 0) {
            l->goal_anim_state = 0;
            break;
        }
        if (input & IN_LEFT)
            sub->Rot -= 0x200000;
        else if (input & IN_RIGHT)
            sub->Rot += 0x200000;
        if (sub->Flags & SF_SURFACE) {
            if (v->pos.x_rot > ONE_DEGREE * 50)
                v->pos.x_rot -= ONE_DEGREE;
            else if (v->pos.x_rot < ONE_DEGREE * 50)
                v->pos.x_rot += ONE_DEGREE;
        } else {
            if (input & IN_FORWARD)
                sub->RotX -= (ONE_DEGREE * 2) << 16;
            else if (input & IN_BACK)
                sub->RotX += (ONE_DEGREE * 2) << 16;
        }
        if ((input & IN_ROLL) && CanGetOff_sub(v)) {
            if (sub->Flags & SF_SURFACE)
                l->goal_anim_state = 2;
            else
                l->goal_anim_state = 9;
            sub->Flags &= ~1;
            StopSoundEffect(346);
            SoundEffect(348, (PHD_3DPOS *)&v->pos.x_pos, 2);
        } else if (input & IN_JUMP) {
            if ((sub->Flags & SF_SURFACE) && (input & IN_FORWARD) && v->pos.x_rot > -(ONE_DEGREE * 15))
                sub->Flags |= 4;
            l->goal_anim_state = 4;
        }
        break;
    case 8:
        if (anim == 11) {
            v->pos.y_pos += 4;
            v->pos.x_rot += ONE_DEGREE;
            if (frame == 30)
                SoundEffect(347, (PHD_3DPOS *)&v->pos.x_pos, 2);
            if (frame == 50)
                sub->Flags |= 1;
        } else if (anim == 13) {
            if (frame == 30)
                SoundEffect(347, (PHD_3DPOS *)&v->pos.x_pos, 2);
            if (frame == 42)
                sub->Flags |= 1;
        }
        break;
    case 9:
        if (anim == 12 && frame == 42) {
            vec.x = 0;
            vec.y = 0;
            vec.z = 0;
            GetLaraHandAbsPosition(&vec, 2);
            LPos.x = vec.x;
            LPos.y = vec.y;
            LPos.z = vec.z;
            LPos.room_number = v->room_number;
            VPos.x = v->pos.x_pos;
            VPos.y = v->pos.y_pos;
            VPos.z = v->pos.z_pos;
            VPos.room_number = v->room_number;
            mgLOS(&VPos, &LPos, 0);
            l->pos.x_pos = LPos.x;
            l->pos.y_pos = LPos.y;
            l->pos.z_pos = LPos.z;
            l->anim_number = 108;
            l->frame_number = anims[108].frame_base;
            l->current_anim_state = AS_TREAD;
            l->fallspeed = 0;
            l->gravity_status = 0;
            l->pos.x_rot = l->pos.z_rot = 0;
            UpdateLaraRoom(l, 0);
            lara.water_status = LARA_UNDERWATER;
            lara.gun_status = LG_ARMLESS;
            lara.skidoo = NO_ITEM;
            v->hit_points = 0;
        }
        break;
    case 2:
        if (anim == 9 && frame == 51) {
            vec.x = 0;
            vec.y = 0;
            vec.z = 0;
            wd = GetWaterDepth(l->pos.x_pos, l->pos.y_pos, l->pos.z_pos, l->room_number);
            wh = GetWaterHeight(l->pos.x_pos, l->pos.y_pos, l->pos.z_pos, l->room_number);
            if (wh != NO_HEIGHT)
                hfw = l->pos.y_pos - wh;
            else
                hfw = NO_HEIGHT;
            GetLaraHandAbsPosition(&vec, 2);
            l->pos.x_pos = vec.x;
            l->pos.y_pos = l->pos.y_pos + 1 - hfw;
            l->pos.y_pos = vec.y;
            l->pos.z_pos = vec.z;
            l->anim_number = 114;
            l->frame_number = anims[114].frame_base;
            l->current_anim_state = l->goal_anim_state = AS_SURFTREAD;
            l->fallspeed = 0;
            l->gravity_status = 0;
            l->pos.x_rot = l->pos.z_rot = 0;
            UpdateLaraRoom(l, -LARA_HITE / 2);
            lara.water_status = LARA_SURFACE;
            lara.water_surface_dist = -hfw;
            lara.dive_count = 11;
            lara.torso_x_rot = lara.torso_y_rot = 0;
            lara.head_x_rot = lara.head_y_rot = 0;
            lara.gun_status = LG_ARMLESS;
            lara.skidoo = NO_ITEM;
            v->hit_points = 0;
        }
        break;
    case 0:
        if ((anim == 0 && frame == 16) || (anim == 0 && frame == 17)) {
            vec.x = 0;
            vec.y = 0;
            vec.z = 0;
            GetLaraHandAbsPosition(&vec, 2);
            l->pos.x_pos = vec.x;
            l->pos.y_pos = vec.y;
            l->pos.z_pos = vec.z;
            l->anim_number = 124;
            l->frame_number = anims[124].frame_base + 17;
            l->current_anim_state = l->goal_anim_state = AS_UWDEATH;
            l->fallspeed = 0;
            l->gravity_status = 0;
            l->pos.x_rot = l->pos.z_rot = 0;
            sub->Flags |= 8;
        }
        v->speed = 0;
        break;
    }
    if (sub->Flags & 4) {
        if (v->pos.x_rot > -(ONE_DEGREE * 15))
            v->pos.x_rot -= ONE_DEGREE * 5;
        else
            sub->Flags &= ~4;
    }
    if (sub->Vel > 0) {
        if ((sub->Vel -= 0x18000) < 0)
            sub->Vel = 0;
    } else if (sub->Vel < 0) {
        if ((sub->Vel += 0x18000) > 0)
            sub->Vel = 0;
    }
    if (sub->Vel > 0x400000)
        sub->Vel = 0x400000;
    else if (sub->Vel < -0x400000)
        sub->Vel = -0x400000;
    if (sub->Rot > 0) {
        if ((sub->Rot -= 0x100000) < 0)
            sub->Rot = 0;
    } else if (sub->Rot < 0) {
        if ((sub->Rot += 0x100000) > 0)
            sub->Rot = 0;
    }
    if (sub->RotX > 0) {
        if ((sub->RotX -= ONE_DEGREE << 16) < 0)
            sub->RotX = 0;
    } else if (sub->RotX < 0) {
        if ((sub->RotX += ONE_DEGREE << 16) > 0)
            sub->RotX = 0;
    }
    if (sub->Rot > 0x1c00000)
        sub->Rot = 0x1c00000;
    else if (sub->Rot < -0x1c00000)
        sub->Rot = -0x1c00000;
    if (sub->RotX > (ONE_DEGREE * 2) << 16)
        sub->RotX = (ONE_DEGREE * 2) << 16;
    else if (sub->RotX < -((ONE_DEGREE * 2) << 16))
        sub->RotX = -((ONE_DEGREE * 2) << 16);
}

int CanGetOff_sub(ITEM_INFO *v)
{
    SUBINFO *sub;
    FLOOR_INFO *floor;
    sint32 rad, x, y, z, height, ceiling;
    sint16 room_number, angle;

    sub = (SUBINFO *)v->data;
    if (lara.current_xvel || lara.current_zvel || sub->Vel)
        return 0;

    angle = v->pos.y_rot + 0x8000;
    rad = WALL_L * phd_cos(v->pos.x_rot) >> W2V_SHIFT;
    x = v->pos.x_pos + ((rad * phd_sin(angle)) >> W2V_SHIFT);
    z = v->pos.z_pos + ((rad * phd_cos(angle)) >> W2V_SHIFT);
    y = v->pos.y_pos - ((WALL_L * phd_sin(-v->pos.x_rot)) >> W2V_SHIFT);

    room_number = v->room_number;
    floor = GetFloor(x, y, z, &room_number);
    height = GetHeight(floor, x, y, z);
    if (height == NO_HEIGHT || y > height)
        return 0;

    ceiling = GetCeiling(floor, x, y, z);
    if (height - ceiling < 256 || y < ceiling || ceiling == NO_HEIGHT)
        return 0;

    return 1;
}
