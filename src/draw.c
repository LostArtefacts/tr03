#include "game.h"
#include <string.h>

#define S_LongMemCpy(d, s, n) memcpy(d, s, (n) * sizeof(sint32))

extern int IM_rate, IM_frac;
extern sint32 *IMptr;
extern sint32 IMstack[];
extern sint16 interpolated_bounds[6];

void DrawRooms(sint16 current_room);
void DrawGameInfo(int timed);
void S_OutputPolyList(void);
int S_DumpScreen(void);
void S_AnimateTextures(int nframes);
void phd_PushMatrix(void);
void phd_RotX(sint16 rx);
void phd_RotY(sint16 ry);
void phd_RotZ(sint16 rz);
void phd_RotYXZ(sint16 ry, sint16 rx, sint16 rz);
void phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void phd_PutPolygons(sint16 *objptr, int clip);
void gar_RotYXZsuperpack(sint16 **pprot, int skip);
void InterpolateMatrix(void);
int GetFrames(ITEM_INFO *item, sint16 *frmptr[], int *rate);

sint32 DrawPhaseGame(void)
{
    DrawRooms(camera.pos.room_number);
    DrawGameInfo(1);
    S_OutputPolyList();
    camera.number_frames = S_DumpScreen();
    S_AnimateTextures(camera.number_frames);
    return camera.number_frames;
}

extern int CurrentRoom;
extern int camera_underwater;
extern sint32 outside;
extern sint32 phd_left, phd_right, phd_top, phd_bottom;

void phd_TranslateAbs(sint32 x, sint32 y, sint32 z);
void S_SetupBelowWater(int underwater);
void S_SetupAboveWater(int underwater);
void S_LightRoom(ROOM_INFO *r);
void S_InsertRoom(sint16 *objptr, int outside);
void ClipRoom(ROOM_INFO *r);

void PrintRooms(sint16 room_number)
{
    ROOM_INFO *r;

    CurrentRoom = room_number;
    r = &room[room_number];
    if (r->flags & UNDERWATER)
        S_SetupBelowWater(camera_underwater);
    else
        S_SetupAboveWater(camera_underwater);
    phd_TranslateAbs(r->x, r->y, r->z);
    phd_left = r->left;
    phd_right = r->right;
    phd_top = r->top;
    phd_bottom = r->bottom;
    S_LightRoom(r);
    if (outside > 0 && !(r->flags & 64)) {
        S_InsertRoom(r->data, 1);
    } else {
        if (outside >= 0)
            ClipRoom(r);
        S_InsertRoom(r->data, 0);
    }
}

extern sint32 bound_start, bound_end;
extern sint32 bound_list[128];
extern sint32 mid_sort;
extern sint32 number_draw_rooms;
extern sint16 draw_rooms[];
extern sint32 outside_left, outside_right, outside_top, outside_bottom;
extern sint32 w2v_matrix[12];
void SetRoomBounds(sint16 *objptr, int room_number, ROOM_INFO *parent);

void GetRoomBounds(void)
{
    ROOM_INFO *r;
    sint16 *door;
    int i, room_number;

    while (bound_start != bound_end) {
        room_number = bound_list[(bound_start++) % 128];
        r = &room[room_number];
        r->bound_active -= 2;
        mid_sort = (r->bound_active >> 8) + 1;
        if (r->test_left < r->left)
            r->left = r->test_left;
        if (r->test_top < r->top)
            r->top = r->test_top;
        if (r->test_right > r->right)
            r->right = r->test_right;
        if (r->test_bottom > r->bottom)
            r->bottom = r->test_bottom;
        if (!(r->bound_active & 1)) {
            draw_rooms[number_draw_rooms++] = room_number;
            r->bound_active |= 1;
            if (r->flags & 8)
                outside = 8;
        }
        if (r->flags & 8) {
            if (r->left < outside_left)
                outside_left = r->left;
            if (r->right > outside_right)
                outside_right = r->right;
            if (r->top < outside_top)
                outside_top = r->top;
            if (r->bottom > outside_bottom)
                outside_bottom = r->bottom;
        }
        phd_PushMatrix();
        phd_TranslateAbs(r->x, r->y, r->z);
        if ((door = r->door)) {
            for (i = *(door++); i > 0; i--, door += 15) {
                room_number = *(door++);
                if (door[0] * (r->x + door[3] - w2v_matrix[3]) + door[1] * (r->y + door[4] - w2v_matrix[7]) + door[2] * (r->z + door[5] - w2v_matrix[11]) >= 0)
                    continue;
                SetRoomBounds(door, room_number, r);
            }
        }
        phd_PopMatrix();
    }
}

struct door_vbuf {
    sint32 xv;
    sint32 yv;
    sint32 zv;
};

extern struct door_vbuf vbufdoor[4];
extern sint32 phd_zfar, phd_persp, phd_centerx, phd_centery;
extern sint16 phd_winxmax, phd_winymax;

void SetRoomBounds(sint16 *objptr, int room_number, ROOM_INFO *parent)
{
    int i, z_behind, z_toofar;
    sint32 left, right, top, bottom;
    sint32 xv, yv, zv, xs, ys;
    sint32 *mptr;
    ROOM_INFO *r;
    struct door_vbuf *dest, *last;

    r = &room[room_number];
    if (r->left <= parent->test_left && r->right >= parent->test_right && r->top <= parent->test_top && r->bottom >= parent->test_bottom)
        return;

    left = parent->test_right;
    right = parent->test_left;
    top = parent->test_bottom;
    bottom = parent->test_top;
    objptr += 3;
    mptr = phd_mxptr;
    dest = vbufdoor;
    z_behind = z_toofar = 0;
    for (i = 0; i < 4; i++, dest++, objptr += 3) {
        dest->xv = xv = (sint32)(*(mptr + 0) * *objptr + *(mptr + 1) * *(objptr + 1) + *(mptr + 2) * *(objptr + 2) + *(mptr + 3));
        dest->yv = yv = (sint32)(*(mptr + 4) * *objptr + *(mptr + 5) * *(objptr + 1) + *(mptr + 6) * *(objptr + 2) + *(mptr + 7));
        dest->zv = zv = (sint32)(*(mptr + 8) * *objptr + *(mptr + 9) * *(objptr + 1) + *(mptr + 10) * *(objptr + 2) + *(mptr + 11));
        if (zv > 0) {
            if (zv > phd_zfar)
                z_toofar++;
            zv /= phd_persp;
            if (zv) {
                xs = xv / zv + phd_centerx;
                ys = yv / zv + phd_centery;
            } else {
                xs = (xv >= 0) ? phd_right : phd_left;
                ys = (yv >= 0) ? phd_bottom : phd_top;
            }
            if (xs - 1 < left)
                left = xs - 1;
            if (xs + 1 > right)
                right = xs + 1;
            if (ys - 1 < top)
                top = ys - 1;
            if (ys + 1 > bottom)
                bottom = ys + 1;
        } else
            z_behind++;
    }

    if (z_behind == 4)
        return;
    if (z_toofar == 4 && outside == 0)
        return;

    if (z_behind > 0) {
        dest = vbufdoor;
        last = dest + 3;
        for (i = 0; i < 4; i++, last = dest, dest++) {
            if ((dest->zv <= 0) ^ (last->zv <= 0)) {
                if (dest->xv < 0 && last->xv < 0)
                    left = 0;
                else if (dest->xv > 0 && last->xv > 0)
                    right = phd_winxmax;
                else {
                    left = 0;
                    right = phd_winxmax;
                }
                if (dest->yv < 0 && last->yv < 0)
                    top = 0;
                else if (dest->yv > 0 && last->yv > 0)
                    bottom = phd_winymax;
                else {
                    top = 0;
                    bottom = phd_winymax;
                }
            }
        }
    }

    if (left < parent->test_left)
        left = parent->test_left;
    if (right > parent->test_right)
        right = parent->test_right;
    if (top < parent->test_top)
        top = parent->test_top;
    if (bottom > parent->test_bottom)
        bottom = parent->test_bottom;
    if (left >= right || top >= bottom)
        return;

    if (r->bound_active & 2) {
        if (left < r->test_left)
            r->test_left = (sint16)left;
        if (top < r->test_top)
            r->test_top = (sint16)top;
        if (right > r->test_right)
            r->test_right = (sint16)right;
        if (bottom > r->test_bottom)
            r->test_bottom = (sint16)bottom;
    } else {
        bound_list[(bound_end++) % 128] = room_number;
        r->bound_active |= 2;
        r->test_left = (sint16)left;
        r->test_right = (sint16)right;
        r->test_top = (sint16)top;
        r->test_bottom = (sint16)bottom;
    }
}

extern sint32 phd_znear, phd_zfar;

void DrawDummyItem();
void S_DrawSprite(uint32 flags, sint32 x, sint32 y, sint32 z, sint16 sprnum, sint16 shade, sint16 scale);
void S_CalculateLight(sint32 x, sint32 y, sint32 z, sint32 room_number, ITEM_LIGHT *il);

void DrawEffect(sint16 fx_num)
{
    FX_INFO *fx;
    OBJECT_INFO *object;

    fx = &effects[fx_num];
    if (objects[fx->object_number].draw_routine == DrawDummyItem)
        return;
    object = &objects[fx->object_number];
    if (!object->loaded)
        return;
    if (fx->object_number == 299) {
        S_DrawSprite((uint16)fx->pos.x_rot | (fx->pos.y_rot << 16), fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, objects[299].mesh_index,
                     fx->shade, fx->frame_number);
        return;
    }
    if (object->nmeshes < 0)
        S_DrawSprite(0x01000000 | (object->semi_transparent ? 0x02000000 : 0) | 0x08000000, fx->pos.x_pos,
                     fx->pos.y_pos, fx->pos.z_pos, (sint16)(object->mesh_index - fx->frame_number), fx->shade, 0);
    else {
        phd_PushMatrix();
        phd_TranslateAbs(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos);
        if (phd_mxptr[11] > phd_znear && phd_mxptr[11] < phd_zfar) {
            phd_RotYXZ(fx->pos.y_rot, fx->pos.x_rot, fx->pos.z_rot);
            if (object->nmeshes) {
                S_CalculateLight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, fx->room_number, NULL);
                phd_PutPolygons(meshes[object->mesh_index], -1);
            } else {
                S_CalculateLight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, fx->room_number, NULL);
                phd_PutPolygons(meshes[fx->frame_number], -1);
            }
        }
        phd_PopMatrix();
    }
}

void InitInterpolate(int frac, int rate)
{
    IM_frac = frac;
    IM_rate = rate;
    IMptr = IMstack;
    S_LongMemCpy(IMstack, phd_mxptr, 12);
}

void phd_PopMatrix_I(void)
{
    phd_PopMatrix();
    IMptr -= 12;
}

void phd_PushMatrix_I(void)
{
    phd_PushMatrix();
    S_LongMemCpy(IMptr + 12, IMptr, 12);
    IMptr += 12;
}

void phd_RotY_I(sint16 ang)
{
    sint32 *oldm;

    phd_RotY(ang);
    oldm = phd_mxptr;
    phd_mxptr = IMptr;
    phd_RotY(ang);
    phd_mxptr = oldm;
}

void phd_RotX_I(sint16 ang)
{
    sint32 *oldm;

    phd_RotX(ang);
    oldm = phd_mxptr;
    phd_mxptr = IMptr;
    phd_RotX(ang);
    phd_mxptr = oldm;
}

void phd_RotZ_I(sint16 ang)
{
    sint32 *oldm;

    phd_RotZ(ang);
    oldm = phd_mxptr;
    phd_mxptr = IMptr;
    phd_RotZ(ang);
    phd_mxptr = oldm;
}

void phd_TranslateRel_I(sint32 x, sint32 y, sint32 z)
{
    sint32 *oldm;

    phd_TranslateRel(x, y, z);
    oldm = phd_mxptr;
    phd_mxptr = IMptr;
    phd_TranslateRel(x, y, z);
    phd_mxptr = oldm;
}

void phd_TranslateRel_ID(sint32 x, sint32 y, sint32 z, sint32 x2, sint32 y2, sint32 z2)
{
    sint32 *oldm;

    phd_TranslateRel(x, y, z);
    oldm = phd_mxptr;
    phd_mxptr = IMptr;
    phd_TranslateRel(x2, y2, z2);
    phd_mxptr = oldm;
}

void phd_RotYXZ_I(sint16 y, sint16 x, sint16 z)
{
    sint32 *oldm;

    phd_RotYXZ(y, x, z);
    oldm = phd_mxptr;
    phd_mxptr = IMptr;
    phd_RotYXZ(y, x, z);
    phd_mxptr = oldm;
}

void gar_RotYXZsuperpack_I(sint16 **pprot1, sint16 **pprot2, int skip)
{
    sint32 *oldm;

    gar_RotYXZsuperpack(pprot1, skip);
    oldm = phd_mxptr;
    phd_mxptr = IMptr;
    gar_RotYXZsuperpack(pprot2, skip);
    phd_mxptr = oldm;
}

void phd_RotYXZpack(sint32 rots);

void gar_RotYXZsuperpack(sint16 **pprot, int skip)
{
    uint16 *prot;
    sint32 packed;

    while (skip) {
        prot = *pprot;
        if (*prot & (3 << 14))
            (*pprot) += 1;
        else
            (*pprot) += 2;
        skip--;
    }

    prot = *pprot;
    switch (*prot >> 14) {
    case 0:
        packed = (*prot << 16) + *(prot + 1);
        phd_RotYXZpack(packed);
        (*pprot) += 2;
        return;
    case 1:
        phd_RotX((sint16)((*prot & 1023) << 6));
        break;
    case 2:
        phd_RotY((sint16)((*prot & 1023) << 6));
        break;
    default:
        phd_RotZ((sint16)((*prot & 1023) << 6));
        break;
    }

    (*pprot) += 1;
}

void phd_PutPolygons_I(sint16 *ptr, int clip)
{
    phd_PushMatrix();
    InterpolateMatrix();
    phd_PutPolygons(ptr, clip);
    phd_PopMatrix();
}

void InterpolateMatrix(void)
{
    sint32 *mptr, *iptr;
    sint32 frac, rate;

    mptr = phd_mxptr;
    iptr = IMptr;
    frac = IM_frac;
    rate = IM_rate;

    if (rate == 2 || (frac == 2 && rate == 4)) {
        mptr[0] = (iptr[0] + mptr[0]) >> 1;
        mptr[1] = (iptr[1] + mptr[1]) >> 1;
        mptr[2] = (iptr[2] + mptr[2]) >> 1;
        mptr[3] = (iptr[3] + mptr[3]) >> 1;

        mptr[4] = (iptr[4] + mptr[4]) >> 1;
        mptr[5] = (iptr[5] + mptr[5]) >> 1;
        mptr[6] = (iptr[6] + mptr[6]) >> 1;
        mptr[7] = (iptr[7] + mptr[7]) >> 1;

        mptr[8] = (iptr[8] + mptr[8]) >> 1;
        mptr[9] = (iptr[9] + mptr[9]) >> 1;
        mptr[10] = (iptr[10] + mptr[10]) >> 1;
        mptr[11] = (iptr[11] + mptr[11]) >> 1;
    } else if (frac == 1) {
        mptr[0] += (iptr[0] - mptr[0]) >> 2;
        mptr[1] += (iptr[1] - mptr[1]) >> 2;
        mptr[2] += (iptr[2] - mptr[2]) >> 2;
        mptr[3] += (iptr[3] - mptr[3]) >> 2;

        mptr[4] += (iptr[4] - mptr[4]) >> 2;
        mptr[5] += (iptr[5] - mptr[5]) >> 2;
        mptr[6] += (iptr[6] - mptr[6]) >> 2;
        mptr[7] += (iptr[7] - mptr[7]) >> 2;

        mptr[8] += (iptr[8] - mptr[8]) >> 2;
        mptr[9] += (iptr[9] - mptr[9]) >> 2;
        mptr[10] += (iptr[10] - mptr[10]) >> 2;
        mptr[11] += (iptr[11] - mptr[11]) >> 2;
    } else {
        mptr[0] = iptr[0] - ((iptr[0] - mptr[0]) >> 2);
        mptr[1] = iptr[1] - ((iptr[1] - mptr[1]) >> 2);
        mptr[2] = iptr[2] - ((iptr[2] - mptr[2]) >> 2);
        mptr[3] = iptr[3] - ((iptr[3] - mptr[3]) >> 2);

        mptr[4] = iptr[4] - ((iptr[4] - mptr[4]) >> 2);
        mptr[5] = iptr[5] - ((iptr[5] - mptr[5]) >> 2);
        mptr[6] = iptr[6] - ((iptr[6] - mptr[6]) >> 2);
        mptr[7] = iptr[7] - ((iptr[7] - mptr[7]) >> 2);

        mptr[8] = iptr[8] - ((iptr[8] - mptr[8]) >> 2);
        mptr[9] = iptr[9] - ((iptr[9] - mptr[9]) >> 2);
        mptr[10] = iptr[10] - ((iptr[10] - mptr[10]) >> 2);
        mptr[11] = iptr[11] - ((iptr[11] - mptr[11]) >> 2);
    }
}

void InterpolateArmMatrix(void)
{
    sint32 *mptr, *iptr;
    sint32 frac, rate;

    mptr = phd_mxptr;
    iptr = IMptr;
    frac = IM_frac;
    rate = IM_rate;

    mptr[0] = mptr[0 - 24];
    mptr[1] = mptr[1 - 24];
    mptr[2] = mptr[2 - 24];
    mptr[4] = mptr[4 - 24];
    mptr[5] = mptr[5 - 24];
    mptr[6] = mptr[6 - 24];
    mptr[8] = mptr[8 - 24];
    mptr[9] = mptr[9 - 24];
    mptr[10] = mptr[10 - 24];

    if (rate == 2 || (frac == 2 && rate == 4)) {
        mptr[3] = (iptr[3] + mptr[3]) >> 1;
        mptr[7] = (iptr[7] + mptr[7]) >> 1;
        mptr[11] = (iptr[11] + mptr[11]) >> 1;
    } else if (frac == 1) {
        mptr[3] += (iptr[3] - mptr[3]) >> 2;
        mptr[7] += (iptr[7] - mptr[7]) >> 2;
        mptr[11] += (iptr[11] - mptr[11]) >> 2;
    } else {
        mptr[3] = iptr[3] - ((iptr[3] - mptr[3]) >> 2);
        mptr[7] = iptr[7] - ((iptr[7] - mptr[7]) >> 2);
        mptr[11] = iptr[11] - ((iptr[11] - mptr[11]) >> 2);
    }
}

extern sint32 wibble;

void S_CalculateStaticLight(sint16 adder);
void S_DrawSprite(uint32 flags, sint32 x, sint32 y, sint32 z, sint16 sprnum, sint16 shade, sint16 scale);

void DrawGunFlash(int weapon_type, int clip)
{
    sint16 light;
    int g_len, nZOff;

    switch (weapon_type) {
    case LG_FLARE:
        return;
    case LG_MAGNUMS:
        light = 24 | (18 << 5);
        g_len = 215;
        nZOff = 65;
        break;
    case LG_UZIS:
        light = 24 | (18 << 5);
        g_len = 150;
        nZOff = 50;
        break;
    case LG_M16:
        phd_TranslateRel(0, 332, 96);
        phd_RotYXZ(0, -85 * 182, (sint16)((GetRandomDraw() * 2 & 0x4000) + 0x2000 + (GetRandomDraw() & 4095) - 2048));
        S_CalculateStaticLight(24 | (18 << 5));
        phd_PutPolygons(meshes[objects[301].mesh_index], clip);
        S_DrawSprite(0x02000000 | 0x10000000 | 0x20000000 | 0x3f | (0x30 << 8) | (0x08 << 16) | 0x04000000, 0, 0, -65,
                     objects[299].mesh_index, 0, 0xc0);
        return;
    case LG_SHOTGUN:
        return;
    default:
        light = 24 | (18 << 5);
        g_len = 150;
        nZOff = 40;
        break;
    }
    phd_TranslateRel(0, g_len, nZOff);
    phd_RotYXZ(0, -90 * 182, (sint16)(wibble << 8));
    S_CalculateStaticLight(light);
    phd_PutPolygons(meshes[objects[300].mesh_index], clip);
    S_DrawSprite(0x02000000 | 0x10000000 | 0x20000000 | 0x3f | (0x38 << 8) | (0x8 << 16) | 0x04000000, 0, 0, 0,
                 objects[299].mesh_index, 0, 0xc0);
}

extern int GnGameMode;

void GetLaraHandAbsPosition(PHD_VECTOR *vec, int hand);
void S_CalculateLight(sint32 x, sint32 y, sint32 z, sint32 room_number, ITEM_LIGHT *il);
void S_CalculateStaticMeshLight(sint32 x, sint32 y, sint32 z, sint32 shade, sint32 shadeB, ROOM_INFO *ri);
void phd_PushUnitMatrix(void);

#define M03 3
#define M13 7
#define M23 11

void CalculateObjectLighting(ITEM_INFO *item, sint16 *frame)
{
    sint16 room_no;
    PHD_VECTOR pos;
    sint32 *mptr;
    sint32 x, y, z;

    if (item->shade < 0) {
        if (GnGameMode == 3) {
            pos.x = 0;
            pos.y = 0;
            pos.z = 0;
            GetJointAbsPosition(item, &pos, 0);
            room_no = item->room_number;
            GetFloor(pos.x, pos.y, pos.z, &room_no);
            S_CalculateLight(pos.x, pos.y, pos.z, room_no, &item->il);
        } else if (item->object_number == 110)
            S_CalculateLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number, &item->il);
        else {
            phd_PushUnitMatrix();
            mptr = phd_mxptr;
            mptr[M03] = mptr[M13] = mptr[M23] = 0;
            phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
            x = (phd_mxptr[M03] >> W2V_SHIFT) + item->pos.x_pos;
            y = (phd_mxptr[M13] >> W2V_SHIFT) + item->pos.y_pos;
            z = (phd_mxptr[M23] >> W2V_SHIFT) + item->pos.z_pos;
            phd_PopMatrix();
            S_CalculateLight(x, y, z, item->room_number, &item->il);
        }
    } else
        S_CalculateStaticMeshLight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->shade & 0x7fff, item->shadeB,
                                   &room[item->room_number]);
}

void CalculateObjectLightingLara(void)
{
    sint16 room_no;
    PHD_VECTOR pos;

    pos.x = 0;
    pos.y = 0;
    pos.z = 0;
    if (GnGameMode == 3) {
        GetJointAbsPosition(lara_item, &pos, 7);
        room_no = lara_item->room_number;
    } else {
        GetLaraHandAbsPosition(&pos, 3);
        room_no = lara_item->room_number;
        GetFloor(pos.x, pos.y, pos.z, &room_no);
    }
    S_CalculateLight(pos.x, pos.y, pos.z, room_no, &lara_item->il);
}

int GetFrames(ITEM_INFO *item, sint16 *frmptr[], int *rate)
{
    ANIM_STRUCT *anim;
    int frm;
    int first, second;
    int frame_size;
    int interp, rat;

    frm = item->frame_number;
    anim = &anims[item->anim_number];
    frmptr[0] = frmptr[1] = anim->frame_ptr;
    rat = *rate = anim->interpolation & 0xff;
    frame_size = anim->interpolation >> 8;
    frm -= anim->frame_base;
    first = frm / rat;
    interp = frm % rat;
    frmptr[0] += first * frame_size;
    frmptr[1] = frmptr[0] + frame_size;
    if (interp == 0)
        return 0;
    second = first * rat + rat;
    if (second > anim->frame_end)
        *rate = anim->frame_end - (second - rat);
    return interp;
}

sint16 *GetBoundsAccurate(ITEM_INFO *item)
{
    int i, frac, rate;
    sint16 *bptr, *frmptr[2];

    frac = GetFrames(item, frmptr, &rate);
    if (frac == 0)
        return frmptr[0];
    bptr = interpolated_bounds;
    for (i = 0; i < 6; i++, bptr++, frmptr[0]++, frmptr[1]++)
        *bptr = *frmptr[0] + ((*frmptr[1] - *frmptr[0]) * frac) / rate;
    return interpolated_bounds;
}

sint16 *GetBestFrame(ITEM_INFO *item)
{
    int frac, rate;
    sint16 *frmptr[2];

    frac = GetFrames(item, frmptr, &rate);
    if (frac <= (rate >> 1))
        return frmptr[0];
    else
        return frmptr[1];
}

extern sint32 nPolyType;
extern int bLaraOn, bRoomOn, bEffectOn;
extern uint32 CurrentLevel;

void S_InitialisePolyList(int clear);
void S_InsertBackground(sint16 *objptr);
void PrintObjects(sint16 room_number);
void DrawLara(ITEM_INFO *item);
void S_DrawSparks(void);
void S_DrawSplashes(void);
void S_DrawBat(void);
void DoRain(void);
void DoSnow(void);
void DrawDummyItem();
void UpdateElectricityPoints(void);
void LaraElectricDeath(sint32 copy, ITEM_INFO *item);

void DrawRooms(sint16 current_room)
{
    int i;
    ROOM_INFO *r;

    CurrentRoom = current_room;
    r = &room[current_room];
    phd_left = r->test_left = 0;
    phd_top = r->test_top = 0;
    phd_right = r->test_right = phd_winxmax;
    phd_bottom = r->test_bottom = phd_winymax;
    outside = r->flags & 8;
    camera_underwater = r->flags & UNDERWATER;
    r->bound_active = 2;
    bound_list[0] = current_room;
    bound_start = 0;
    bound_end = 1;
    number_draw_rooms = 0;
    if (outside) {
        outside_left = outside_top = 0;
        outside_right = phd_winxmax;
        outside_bottom = phd_winymax;
    } else {
        outside_left = phd_winxmax;
        outside_top = phd_winymax;
        outside_right = outside_bottom = 0;
    }
    GetRoomBounds();
    mid_sort = 0;

    if (outside) {
        phd_left = outside_left;
        phd_right = outside_right;
        phd_top = outside_top;
        phd_bottom = outside_bottom;
        if (objects[355].loaded) {
            sint16 *frame;

            S_SetupAboveWater(camera_underwater);
            phd_PushMatrix();
            *(phd_mxptr + 3) = *(phd_mxptr + 7) = *(phd_mxptr + 11) = 0;
            frame = anims[objects[355].anim_index].frame_ptr + 9;
            gar_RotYXZsuperpack(&frame, 0);
            S_InitialisePolyList(0);
            S_InsertBackground(meshes[objects[355].mesh_index]);
            phd_PopMatrix();
        } else {
            S_InitialisePolyList(1);
            outside = -1;
        }
    } else
        S_InitialisePolyList(0);

    if (objects[0].loaded && !(lara_item->flags & ONESHOT)) {
        if (room[lara_item->room_number].flags & UNDERWATER)
            S_SetupBelowWater(camera_underwater);
        else
            S_SetupAboveWater(camera_underwater);
        mid_sort = room[lara_item->room_number].bound_active >> 8;
        if (mid_sort)
            mid_sort--;
        nPolyType = 2;
        if (bLaraOn)
            DrawLara(lara_item);
    }

    if (bRoomOn) {
        nPolyType = 3;
        for (i = 0; i < number_draw_rooms; i++)
            PrintRooms(draw_rooms[i]);
    }
    for (i = 0; i < number_draw_rooms; i++)
        PrintObjects(draw_rooms[i]);

    if (bEffectOn) {
        nPolyType = 6;
        S_DrawSparks();
        S_DrawSplashes();
        S_DrawBat();
        if (CurrentLevel == 16 || CurrentLevel == 19)
            DoSnow();
        if (CurrentLevel == 1 || CurrentLevel == 9)
            DoRain();
        DrawDummyItem();
    }

    if (lara.electric) {
        if (lara.electric < 16)
            lara.electric++;
        UpdateElectricityPoints();
        LaraElectricDeath(0, lara_item);
        LaraElectricDeath(1, lara_item);
    }
}

typedef struct {
    sint16 mesh_number;
    sint16 flags;
    sint16 x_minp;
    sint16 x_maxp;
    sint16 y_minp;
    sint16 y_maxp;
    sint16 z_minp;
    sint16 z_maxp;
    sint16 x_minc;
    sint16 x_maxc;
    sint16 y_minc;
    sint16 y_maxc;
    sint16 z_minc;
    sint16 z_maxc;
} STATIC_INFO;

extern STATIC_INFO static_objects[];
extern MESH_INFO *CurrentMesh;
extern int bObjectOn, bAObjectOn;

int S_GetObjectBounds(sint16 *bptr);
void S_CalculateStaticMeshLight(sint32 x, sint32 y, sint32 z, sint32 shade, sint32 shadeB, ROOM_INFO *ri);
sint32 GetRandomDraw(void);
void DoLotsOfBloodD(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 direction, sint16 room_num, int number);
void DrawEffect(sint16 fx_num);

void PrintObjects(sint16 room_number)
{
    ROOM_INFO *r;
    MESH_INFO *mesh;
    ITEM_INFO *item;
    sint16 i, item_num, clip;

    CurrentRoom = room_number;
    r = &room[room_number];
    if (r->flags & UNDERWATER)
        S_SetupBelowWater(camera_underwater);
    else
        S_SetupAboveWater(camera_underwater);
    r->bound_active = 0;

    phd_PushMatrix();
    phd_TranslateAbs(r->x, r->y, r->z);
    phd_left = r->left;
    phd_right = r->right;
    phd_top = r->top;
    phd_bottom = r->bottom;
    mesh = r->mesh;
    for (i = r->num_meshes; i > 0; i--, mesh++) {
        CurrentMesh = mesh;
        nPolyType = 4;
        if (static_objects[mesh->static_number].flags & 2) {
            phd_PushMatrix();
            phd_TranslateAbs(mesh->x, mesh->y, mesh->z);
            phd_RotY(mesh->y_rot);
            if ((clip = S_GetObjectBounds(&static_objects[mesh->static_number].x_minp))) {
                S_CalculateStaticMeshLight(mesh->x, mesh->y, mesh->z, mesh->shade, mesh->shadeB, r);
                if (bObjectOn)
                    phd_PutPolygons(meshes[static_objects[mesh->static_number].mesh_number], clip);
            }
            phd_PopMatrix();
        }
    }

    phd_left = 0;
    phd_top = 0;
    phd_right = phd_winxmax + 1;
    phd_bottom = phd_winymax + 1;
    nPolyType = 5;
    for (item_num = r->item_number; item_num != NO_ITEM; item_num = items[item_num].next_item) {
        item = &items[item_num];
        if (item->status != INVISIBLE && bAObjectOn) {
            if (objects[item->object_number].intelligent)
                nPolyType = 5;
            else if ((item->object_number >= 148 && item->object_number <= 157) || (item->object_number >= 12 && item->object_number <= 16))
                nPolyType = 5;
            else
                nPolyType = 4;
            (*objects[item->object_number].draw_routine)(item);
        }
        if (item->after_death < 32 && item->after_death > 0) {
            item->after_death++;
            if (item->after_death == 2 || item->after_death == 5 || item->after_death == 11 || item->after_death == 20
                || item->after_death == 27 || item->after_death == 32 || (GetRandomDraw() & 7) == 0)
                DoLotsOfBloodD(item->pos.x_pos, item->pos.y_pos - 64, item->pos.z_pos, 0, GetRandomDraw() << 1, item->room_number, 1);
        }
    }

    nPolyType = 6;
    for (item_num = r->fx_number; item_num != NO_ITEM; item_num = effects[item_num].next_fx)
        DrawEffect(item_num);
    phd_PopMatrix();
    r->left = phd_winxmax;
    r->top = phd_winymax;
    r->right = r->bottom = 0;
}

#define M00 0
#define M01 1
#define M02 2
#define M10 4
#define M11 5
#define M12 6
#define M20 8
#define M21 9
#define M22 10

#define PutPolyLara(A, B) \
    if (item->mesh_bits & (1 << (A))) { \
        if (IsJointUnderwater[A]) \
            S_SetupBelowWater(camera_underwater); \
        else \
            S_SetupAboveWater(camera_underwater); \
        phd_PutPolygons(lara.mesh_ptrs[A], B); \
    }

extern uint8 GotJointPos[];
extern uint8 IsJointUnderwater[];
extern sint32 *bones;

void DrawLaraInt(ITEM_INFO *item, sint16 *frame1, sint16 *frame2, int frac, int rate);
void S_PrintShadow(sint16 size, sint16 *bptr, ITEM_INFO *item, int unknown);
void CalculateObjectLightingLara(void);
void DrawHair(void);
void DrawGunFlash(int weapon_type, int clip);
void TriggerFlareSparks(sint32 x, sint32 y, sint32 z, sint32 xv, sint32 yv, sint32 zv, sint32 smoke, sint32 FromLara);

void DrawLara(ITEM_INFO *item)
{
    PHD_VECTOR pos;
    OBJECT_INFO *object;
    sint32 *bone, *bonew;
    sint16 *rotation, *rotationw;
    sint16 *frame;
    int clip, fire_arms;
    sint16 *frmptr[2];
    int frac, rate, size;
    int left, right, top, bottom;
    sint32 do_flare = 0;
    sint32 savemox[12];

    left = phd_left;
    right = phd_right;
    top = phd_top;
    bottom = phd_bottom;
    phd_left = phd_top = 0;
    phd_bottom = phd_winymax;
    phd_right = phd_winxmax;

    if (!GotJointPos[FOOT_L])
        GetLaraHandAbsPosition(&pos, 5);
    if (!GotJointPos[FOOT_R])
        GetLaraHandAbsPosition(&pos, 6);
    if (!GotJointPos[HAND_L])
        GetLaraHandAbsPosition(&pos, 0);
    if (!GotJointPos[HAND_R])
        GetLaraHandAbsPosition(&pos, 1);
    if (!GotJointPos[HEAD])
        GetLaraHandAbsPosition(&pos, 4);

    if (lara.hit_direction < 0 && (frac = GetFrames(item, frmptr, &rate))) {
        DrawLaraInt(item, frmptr[0], frmptr[1], frac, rate);
        phd_left = left;
        phd_right = right;
        phd_top = top;
        phd_bottom = bottom;
        return;
    }

    object = &objects[item->object_number];
    if (lara.hit_direction >= 0) {
        sint16 spaz;
        ANIM_STRUCT *anim;

        switch (lara.hit_direction) {
        case 0:
            if (lara.IsDucked)
                spaz = 294;
            else
                spaz = 125;
            break;
        case 2:
            if (lara.IsDucked)
                spaz = 293;
            else
                spaz = 126;
            break;
        case 1:
            if (lara.IsDucked)
                spaz = 295;
            else
                spaz = 127;
            break;
        default:
            if (lara.IsDucked)
                spaz = 296;
            else
                spaz = 128;
            break;
        }
        anim = &anims[spaz];
        frame = anim->frame_ptr;
        size = anim->interpolation >> 8;
        frame += lara.hit_frame * size;
    } else
        frame = frmptr[0];

    if (lara.skidoo == NO_ITEM || (lara.extra_anim && item->current_anim_state == 12))
        S_PrintShadow(object->shadow_size, frame, item, 0);
    S_LongMemCpy(savemox, phd_mxptr, 12);

    phd_PushMatrix();
    phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
    if (!(clip = S_GetObjectBounds(frame))) {
        if (GnGameMode == 3) {
            phd_PopMatrix();
            return;
        } else
            clip = -1;
    }

    phd_PushMatrix();
    CalculateObjectLightingLara();
    bone = bones + object->bone_index;
    rotation = frame + 9;
    phd_TranslateRel((sint32)*(frame + 6), (sint32)*(frame + 7), (sint32)*(frame + 8));
    gar_RotYXZsuperpack(&rotation, 0);
    PutPolyLara(HIPS, clip);

    phd_PushMatrix();
    phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
    gar_RotYXZsuperpack(&rotation, 0);
    PutPolyLara(THIGH_L, clip);
    phd_TranslateRel(*(bone + 1 + 4), *(bone + 2 + 4), *(bone + 3 + 4));
    gar_RotYXZsuperpack(&rotation, 0);
    PutPolyLara(CALF_L, clip);
    phd_TranslateRel(*(bone + 1 + 8), *(bone + 2 + 8), *(bone + 3 + 8));
    gar_RotYXZsuperpack(&rotation, 0);
    PutPolyLara(FOOT_L, clip);
    phd_PopMatrix();

    phd_PushMatrix();
    phd_TranslateRel(*(bone + 1 + 12), *(bone + 2 + 12), *(bone + 3 + 12));
    gar_RotYXZsuperpack(&rotation, 0);
    PutPolyLara(THIGH_R, clip);
    phd_TranslateRel(*(bone + 1 + 16), *(bone + 2 + 16), *(bone + 3 + 16));
    gar_RotYXZsuperpack(&rotation, 0);
    PutPolyLara(CALF_R, clip);
    phd_TranslateRel(*(bone + 1 + 20), *(bone + 2 + 20), *(bone + 3 + 20));
    gar_RotYXZsuperpack(&rotation, 0);
    PutPolyLara(FOOT_R, clip);
    phd_PopMatrix();

    phd_TranslateRel(*(bone + 1 + 24), *(bone + 2 + 24), *(bone + 3 + 24));
    if (lara.weapon_item != NO_ITEM && lara.gun_type == LG_M16
        && (items[lara.weapon_item].current_anim_state == 0
            || items[lara.weapon_item].current_anim_state == 2
            || items[lara.weapon_item].current_anim_state == 4)) {
        rotation = lara.right_arm.frame_base
            + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
        gar_RotYXZsuperpack(&rotation, 7);
    } else
        gar_RotYXZsuperpack(&rotation, 0);
    phd_RotYXZ(lara.torso_y_rot, lara.torso_x_rot, lara.torso_z_rot);
    PutPolyLara(TORSO, clip);

    phd_PushMatrix();
    phd_TranslateRel(*(bone + 1 + 52), *(bone + 2 + 52), *(bone + 3 + 52));
    rotationw = rotation;
    gar_RotYXZsuperpack(&rotationw, 6);
    phd_RotYXZ(lara.head_y_rot, lara.head_x_rot, lara.head_z_rot);
    PutPolyLara(HEAD, clip);
    S_LongMemCpy(phd_mxptr, savemox, 12);
    DrawHair();
    phd_PopMatrix();

    if (lara.back_gun) {
        phd_PushMatrix();
        bonew = bones + objects[lara.back_gun].bone_index;
        phd_TranslateRel(*(bonew + 1 + 52), *(bonew + 2 + 52), *(bonew + 3 + 52));
        rotationw = objects[lara.back_gun].frame_base + 9;
        gar_RotYXZsuperpack(&rotationw, 14);
        phd_PutPolygons(meshes[objects[lara.back_gun].mesh_index + HEAD], clip);
        phd_PopMatrix();
    }

    fire_arms = LG_UNARMED;
    if (lara.gun_status == LG_READY || lara.gun_status == LG_SPECIAL
        || lara.gun_status == LG_DRAW || lara.gun_status == LG_UNDRAW)
        fire_arms = lara.gun_type;

    switch (fire_arms) {
    case LG_UNARMED:
    case LG_FLARE:
        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
        gar_RotYXZsuperpack(&rotation, 0);
        PutPolyLara(UARM_R, clip);
        phd_TranslateRel(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
        gar_RotYXZsuperpack(&rotation, 0);
        PutPolyLara(LARM_R, clip);
        phd_TranslateRel(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
        gar_RotYXZsuperpack(&rotation, 0);
        PutPolyLara(HAND_R, clip);
        if (GnGameMode == 3 && lara.right_arm.flash_gun)
            S_LongMemCpy(savemox, phd_mxptr, 12);
        phd_PopMatrix();

        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
        if (lara.flare_control_left) {
            rotation = lara.left_arm.frame_base
                + (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) * (anims[lara.left_arm.anim_number].interpolation >> 8) + 9;
            gar_RotYXZsuperpack(&rotation, 11);
        } else
            gar_RotYXZsuperpack(&rotation, 0);
        PutPolyLara(UARM_L, clip);
        phd_TranslateRel(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
        gar_RotYXZsuperpack(&rotation, 0);
        PutPolyLara(LARM_L, clip);
        phd_TranslateRel(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
        gar_RotYXZsuperpack(&rotation, 0);
        PutPolyLara(HAND_L, clip);
        if (lara.gun_type == LG_FLARE && lara.left_arm.flash_gun) {
            do_flare = 1;
            DrawGunFlash(LG_FLARE, clip);
        }
        if (GnGameMode == 3) {
            if (lara.left_arm.flash_gun)
                DrawGunFlash(LG_PISTOLS, clip);
            if (lara.right_arm.flash_gun) {
                S_LongMemCpy(phd_mxptr, savemox, 12);
                DrawGunFlash(LG_PISTOLS, clip);
            }
        }
        break;

    case LG_PISTOLS:
    case LG_UZIS:
        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
        *(phd_mxptr + M00) = *(phd_mxptr + M00 - 24);
        *(phd_mxptr + M01) = *(phd_mxptr + M01 - 24);
        *(phd_mxptr + M02) = *(phd_mxptr + M02 - 24);
        *(phd_mxptr + M10) = *(phd_mxptr + M10 - 24);
        *(phd_mxptr + M11) = *(phd_mxptr + M11 - 24);
        *(phd_mxptr + M12) = *(phd_mxptr + M12 - 24);
        *(phd_mxptr + M20) = *(phd_mxptr + M20 - 24);
        *(phd_mxptr + M21) = *(phd_mxptr + M21 - 24);
        *(phd_mxptr + M22) = *(phd_mxptr + M22 - 24);
        phd_RotYXZ(lara.right_arm.y_rot, lara.right_arm.x_rot, lara.right_arm.z_rot);
        rotation = lara.right_arm.frame_base
            + (lara.right_arm.frame_number - anims[lara.right_arm.anim_number].frame_base) * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
        gar_RotYXZsuperpack(&rotation, 8);
        PutPolyLara(UARM_R, clip);
        phd_TranslateRel(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
        gar_RotYXZsuperpack(&rotation, 0);
        PutPolyLara(LARM_R, clip);
        phd_TranslateRel(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
        gar_RotYXZsuperpack(&rotation, 0);
        PutPolyLara(HAND_R, clip);
        if (lara.right_arm.flash_gun)
            S_LongMemCpy(savemox, phd_mxptr, 12);
        phd_PopMatrix();
        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
        *(phd_mxptr + M00) = *(phd_mxptr + M00 - 24);
        *(phd_mxptr + M01) = *(phd_mxptr + M01 - 24);
        *(phd_mxptr + M02) = *(phd_mxptr + M02 - 24);
        *(phd_mxptr + M10) = *(phd_mxptr + M10 - 24);
        *(phd_mxptr + M11) = *(phd_mxptr + M11 - 24);
        *(phd_mxptr + M12) = *(phd_mxptr + M12 - 24);
        *(phd_mxptr + M20) = *(phd_mxptr + M20 - 24);
        *(phd_mxptr + M21) = *(phd_mxptr + M21 - 24);
        *(phd_mxptr + M22) = *(phd_mxptr + M22 - 24);
        phd_RotYXZ(lara.left_arm.y_rot, lara.left_arm.x_rot, lara.left_arm.z_rot);
        rotation = lara.left_arm.frame_base
            + (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) * (anims[lara.left_arm.anim_number].interpolation >> 8) + 9;
        gar_RotYXZsuperpack(&rotation, 11);
        PutPolyLara(UARM_L, clip);
        phd_TranslateRel(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
        gar_RotYXZsuperpack(&rotation, 0);
        PutPolyLara(LARM_L, clip);
        phd_TranslateRel(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
        gar_RotYXZsuperpack(&rotation, 0);
        PutPolyLara(HAND_L, clip);
        if (lara.left_arm.flash_gun)
            DrawGunFlash(fire_arms, clip);
        if (lara.right_arm.flash_gun) {
            S_LongMemCpy(phd_mxptr, savemox, 12);
            DrawGunFlash(fire_arms, clip);
        }
        break;

    case LG_MAGNUMS:
        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
        *(phd_mxptr + M00) = *(phd_mxptr + M00 - 24);
        *(phd_mxptr + M01) = *(phd_mxptr + M01 - 24);
        *(phd_mxptr + M02) = *(phd_mxptr + M02 - 24);
        *(phd_mxptr + M10) = *(phd_mxptr + M10 - 24);
        *(phd_mxptr + M11) = *(phd_mxptr + M11 - 24);
        *(phd_mxptr + M12) = *(phd_mxptr + M12 - 24);
        *(phd_mxptr + M20) = *(phd_mxptr + M20 - 24);
        *(phd_mxptr + M21) = *(phd_mxptr + M21 - 24);
        *(phd_mxptr + M22) = *(phd_mxptr + M22 - 24);
        phd_RotYXZ(lara.torso_y_rot, lara.torso_x_rot, lara.torso_z_rot);
        rotation = lara.right_arm.frame_base
            + (lara.right_arm.frame_number - anims[lara.right_arm.anim_number].frame_base) * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
        gar_RotYXZsuperpack(&rotation, 8);
        PutPolyLara(UARM_R, clip);
        phd_TranslateRel(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
        gar_RotYXZsuperpack(&rotation, 0);
        PutPolyLara(LARM_R, clip);
        phd_TranslateRel(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
        gar_RotYXZsuperpack(&rotation, 0);
        PutPolyLara(HAND_R, clip);
        if (lara.right_arm.flash_gun)
            S_LongMemCpy(savemox, phd_mxptr, 12);
        phd_PopMatrix();
        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
        *(phd_mxptr + M00) = *(phd_mxptr + M00 - 24);
        *(phd_mxptr + M01) = *(phd_mxptr + M01 - 24);
        *(phd_mxptr + M02) = *(phd_mxptr + M02 - 24);
        *(phd_mxptr + M10) = *(phd_mxptr + M10 - 24);
        *(phd_mxptr + M11) = *(phd_mxptr + M11 - 24);
        *(phd_mxptr + M12) = *(phd_mxptr + M12 - 24);
        *(phd_mxptr + M20) = *(phd_mxptr + M20 - 24);
        *(phd_mxptr + M21) = *(phd_mxptr + M21 - 24);
        *(phd_mxptr + M22) = *(phd_mxptr + M22 - 24);
        phd_RotYXZ(lara.torso_y_rot, lara.torso_x_rot, lara.torso_z_rot);
        rotation = lara.left_arm.frame_base
            + (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) * (anims[lara.left_arm.anim_number].interpolation >> 8) + 9;
        gar_RotYXZsuperpack(&rotation, 11);
        PutPolyLara(UARM_L, clip);
        phd_TranslateRel(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
        gar_RotYXZsuperpack(&rotation, 0);
        PutPolyLara(LARM_L, clip);
        phd_TranslateRel(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
        gar_RotYXZsuperpack(&rotation, 0);
        PutPolyLara(HAND_L, clip);
        if (lara.left_arm.flash_gun)
            DrawGunFlash(fire_arms, clip);
        if (lara.right_arm.flash_gun) {
            S_LongMemCpy(phd_mxptr, savemox, 12);
            DrawGunFlash(fire_arms, clip);
        }
        break;

    case LG_SHOTGUN:
    case LG_HARPOON:
    case LG_ROCKET:
    case LG_GRENADE:
    case LG_M16:
        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
        rotation = lara.right_arm.frame_base
            + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
        gar_RotYXZsuperpack(&rotation, 8);
        PutPolyLara(UARM_R, clip);
        phd_TranslateRel(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
        gar_RotYXZsuperpack(&rotation, 0);
        PutPolyLara(LARM_R, clip);
        phd_TranslateRel(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
        gar_RotYXZsuperpack(&rotation, 0);
        PutPolyLara(HAND_R, clip);
        if (lara.right_arm.flash_gun)
            S_LongMemCpy(savemox, phd_mxptr, 12);
        phd_PopMatrix();

        phd_PushMatrix();
        phd_TranslateRel(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
        gar_RotYXZsuperpack(&rotation, 0);
        PutPolyLara(UARM_L, clip);
        phd_TranslateRel(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
        gar_RotYXZsuperpack(&rotation, 0);
        PutPolyLara(LARM_L, clip);
        phd_TranslateRel(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
        gar_RotYXZsuperpack(&rotation, 0);
        PutPolyLara(HAND_L, clip);
        if (lara.right_arm.flash_gun) {
            S_LongMemCpy(phd_mxptr, savemox, 12);
            DrawGunFlash(fire_arms, clip);
        }
        break;
    }

    phd_PopMatrix();
    phd_PopMatrix();
    phd_PopMatrix();
    phd_left = left;
    phd_right = right;
    phd_top = top;
    phd_bottom = bottom;

    if (do_flare) {
        PHD_VECTOR pos;
        sint32 xv, yv, zv, lp;

        pos.x = 8;
        pos.y = 36;
        pos.z = 32;
        GetLaraHandAbsPosition(&pos, 0);
        xv = pos.x;
        yv = pos.y;
        zv = pos.z;
        pos.x = 8;
        pos.y = 36;
        pos.z = 1024 + (GetRandomDraw() & 255);
        GetLaraHandAbsPosition(&pos, 0);
        for (lp = 0; lp < (GetRandomDraw() & 3) + 4; lp++)
            TriggerFlareSparks(xv, yv, zv, pos.x - xv, pos.y - yv, pos.z - zv, lp >> 2, 0);
    }
}

#define PutPolyLara_I(A, B) \
    if (item->mesh_bits & (1 << (A))) { \
        if (IsJointUnderwater[A]) \
            S_SetupBelowWater(camera_underwater); \
        else \
            S_SetupAboveWater(camera_underwater); \
        phd_PutPolygons_I(lara.mesh_ptrs[A], B); \
    }

void DrawLaraInt(ITEM_INFO *item, sint16 *frame1, sint16 *frame2, int frac, int rate)
{
    OBJECT_INFO *object;
    sint32 *bone, *bonew;
    sint16 *rotation1, *rotation2, *rotationw1, *rotationw2;
    sint16 *bounds;
    int clip, fire_arms;
    sint32 do_flare = 0;
    sint32 savemox1[12];

    object = &objects[item->object_number];
    bounds = GetBoundsAccurate(item);
    if (lara.skidoo == NO_ITEM)
        S_PrintShadow(object->shadow_size, bounds, item, 0);
    S_LongMemCpy(savemox1, phd_mxptr, 12);

    phd_PushMatrix();
    phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);
    if (!(clip = S_GetObjectBounds(frame1))) {
        if (GnGameMode == 3) {
            phd_PopMatrix();
            return;
        } else
            clip = -1;
    }

    phd_PushMatrix();
    CalculateObjectLightingLara();
    bone = bones + object->bone_index;
    rotation1 = frame1 + 9;
    rotation2 = frame2 + 9;
    InitInterpolate(frac, rate);
    phd_TranslateRel_ID((sint32)*(frame1 + 6), (sint32)*(frame1 + 7), (sint32)*(frame1 + 8),
        (sint32)*(frame2 + 6), (sint32)*(frame2 + 7), (sint32)*(frame2 + 8));
    gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
    PutPolyLara_I(HIPS, clip);

    phd_PushMatrix_I();
    phd_TranslateRel_I(*(bone + 1), *(bone + 2), *(bone + 3));
    gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
    PutPolyLara_I(THIGH_L, clip);
    phd_TranslateRel_I(*(bone + 1 + 4), *(bone + 2 + 4), *(bone + 3 + 4));
    gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
    PutPolyLara_I(CALF_L, clip);
    phd_TranslateRel_I(*(bone + 1 + 8), *(bone + 2 + 8), *(bone + 3 + 8));
    gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
    PutPolyLara_I(FOOT_L, clip);
    phd_PopMatrix_I();

    phd_PushMatrix_I();
    phd_TranslateRel_I(*(bone + 1 + 12), *(bone + 2 + 12), *(bone + 3 + 12));
    gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
    PutPolyLara_I(THIGH_R, clip);
    phd_TranslateRel_I(*(bone + 1 + 16), *(bone + 2 + 16), *(bone + 3 + 16));
    gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
    PutPolyLara_I(CALF_R, clip);
    phd_TranslateRel_I(*(bone + 1 + 20), *(bone + 2 + 20), *(bone + 3 + 20));
    gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
    PutPolyLara_I(FOOT_R, clip);
    phd_PopMatrix_I();

    phd_TranslateRel_I(*(bone + 1 + 24), *(bone + 2 + 24), *(bone + 3 + 24));
    if (lara.weapon_item != NO_ITEM && lara.gun_type == LG_M16
        && (items[lara.weapon_item].current_anim_state == 0
            || items[lara.weapon_item].current_anim_state == 2
            || items[lara.weapon_item].current_anim_state == 4)) {
        rotation1 = rotation2 = lara.right_arm.frame_base
            + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
        gar_RotYXZsuperpack_I(&rotation1, &rotation2, 7);
    } else
        gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
    phd_RotYXZ_I(lara.torso_y_rot, lara.torso_x_rot, lara.torso_z_rot);
    PutPolyLara_I(TORSO, clip);

    phd_PushMatrix_I();
    phd_TranslateRel_I(*(bone + 1 + 52), *(bone + 2 + 52), *(bone + 3 + 52));
    rotationw1 = rotation1;
    rotationw2 = rotation2;
    gar_RotYXZsuperpack_I(&rotationw1, &rotationw2, 6);
    phd_RotYXZ_I(lara.head_y_rot, lara.head_x_rot, lara.head_z_rot);
    PutPolyLara_I(HEAD, clip);
    S_LongMemCpy(phd_mxptr, savemox1, 12);
    DrawHair();
    phd_PopMatrix_I();

    if (lara.back_gun) {
        phd_PushMatrix_I();
        bonew = bones + objects[lara.back_gun].bone_index;
        phd_TranslateRel_I(*(bonew + 1 + 52), *(bonew + 2 + 52), *(bonew + 3 + 52));
        rotationw1 = rotationw2 = objects[lara.back_gun].frame_base + 9;
        gar_RotYXZsuperpack_I(&rotationw1, &rotationw2, 14);
        phd_PutPolygons_I(meshes[objects[lara.back_gun].mesh_index + HEAD], clip);
        phd_PopMatrix_I();
    }

    fire_arms = LG_UNARMED;
    if (lara.gun_status == LG_READY || lara.gun_status == LG_SPECIAL
        || lara.gun_status == LG_DRAW || lara.gun_status == LG_UNDRAW)
        fire_arms = lara.gun_type;

    switch (fire_arms) {
    case LG_UNARMED:
    case LG_FLARE:
        phd_PushMatrix_I();
        phd_TranslateRel_I(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
        gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        PutPolyLara_I(UARM_R, clip);
        phd_TranslateRel_I(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
        gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        PutPolyLara_I(LARM_R, clip);
        phd_TranslateRel_I(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
        gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        PutPolyLara_I(HAND_R, clip);
        phd_PopMatrix_I();

        phd_PushMatrix_I();
        phd_TranslateRel_I(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
        if (lara.flare_control_left) {
            rotation1 = rotation2 = lara.left_arm.frame_base
                + (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) * (anims[lara.left_arm.anim_number].interpolation >> 8) + 9;
            gar_RotYXZsuperpack_I(&rotation1, &rotation2, 11);
        } else
            gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        PutPolyLara_I(UARM_L, clip);
        phd_TranslateRel_I(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
        gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        PutPolyLara_I(LARM_L, clip);
        phd_TranslateRel_I(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
        gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        PutPolyLara_I(HAND_L, clip);
        if (lara.gun_type == LG_FLARE && lara.left_arm.flash_gun) {
            phd_TranslateRel_I(11, 32, 80);
            phd_RotX_I(-90 * 182);
            do_flare = 1;
        }
        break;

    case LG_PISTOLS:
    case LG_UZIS:
        phd_PushMatrix_I();
        phd_TranslateRel_I(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
        InterpolateArmMatrix();
        phd_RotYXZ(lara.right_arm.y_rot, lara.right_arm.x_rot, lara.right_arm.z_rot);
        rotation1 = lara.right_arm.frame_base
            + (lara.right_arm.frame_number - anims[lara.right_arm.anim_number].frame_base) * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
        gar_RotYXZsuperpack(&rotation1, 8);
        PutPolyLara(UARM_R, clip);
        phd_TranslateRel(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
        gar_RotYXZsuperpack(&rotation1, 0);
        PutPolyLara(LARM_R, clip);
        phd_TranslateRel(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
        gar_RotYXZsuperpack(&rotation1, 0);
        PutPolyLara(HAND_R, clip);
        if (lara.right_arm.flash_gun)
            S_LongMemCpy(savemox1, phd_mxptr, 12);
        phd_PopMatrix_I();

        phd_PushMatrix_I();
        phd_TranslateRel_I(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
        InterpolateArmMatrix();
        phd_RotYXZ(lara.left_arm.y_rot, lara.left_arm.x_rot, lara.left_arm.z_rot);
        rotation1 = lara.left_arm.frame_base
            + (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) * (anims[lara.left_arm.anim_number].interpolation >> 8) + 9;
        gar_RotYXZsuperpack(&rotation1, 11);
        PutPolyLara(UARM_L, clip);
        phd_TranslateRel(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
        gar_RotYXZsuperpack(&rotation1, 0);
        PutPolyLara(LARM_L, clip);
        phd_TranslateRel(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
        gar_RotYXZsuperpack(&rotation1, 0);
        PutPolyLara(HAND_L, clip);
        if (lara.left_arm.flash_gun)
            DrawGunFlash(fire_arms, clip);
        if (lara.right_arm.flash_gun) {
            S_LongMemCpy(phd_mxptr, savemox1, 12);
            DrawGunFlash(fire_arms, clip);
        }
        break;

    case LG_MAGNUMS:
        phd_PushMatrix_I();
        phd_TranslateRel_I(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
        InterpolateArmMatrix();
        phd_RotYXZ_I(lara.torso_y_rot, lara.torso_x_rot, lara.torso_z_rot);
        rotation1 = lara.right_arm.frame_base
            + (lara.right_arm.frame_number - anims[lara.right_arm.anim_number].frame_base) * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
        gar_RotYXZsuperpack(&rotation1, 8);
        PutPolyLara(UARM_R, clip);
        phd_TranslateRel(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
        gar_RotYXZsuperpack(&rotation1, 0);
        PutPolyLara(LARM_R, clip);
        phd_TranslateRel(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
        gar_RotYXZsuperpack(&rotation1, 0);
        PutPolyLara(HAND_R, clip);
        if (lara.right_arm.flash_gun)
            S_LongMemCpy(savemox1, phd_mxptr, 12);
        phd_PopMatrix_I();

        phd_PushMatrix_I();
        phd_TranslateRel_I(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
        InterpolateArmMatrix();
        phd_RotYXZ_I(lara.torso_y_rot, lara.torso_x_rot, lara.torso_z_rot);
        rotation1 = lara.left_arm.frame_base
            + (lara.left_arm.frame_number - anims[lara.left_arm.anim_number].frame_base) * (anims[lara.left_arm.anim_number].interpolation >> 8) + 9;
        gar_RotYXZsuperpack(&rotation1, 11);
        PutPolyLara(UARM_L, clip);
        phd_TranslateRel(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
        gar_RotYXZsuperpack(&rotation1, 0);
        PutPolyLara(LARM_L, clip);
        phd_TranslateRel(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
        gar_RotYXZsuperpack(&rotation1, 0);
        PutPolyLara(HAND_L, clip);
        if (lara.left_arm.flash_gun)
            DrawGunFlash(fire_arms, clip);
        if (lara.right_arm.flash_gun) {
            S_LongMemCpy(phd_mxptr, savemox1, 12);
            DrawGunFlash(fire_arms, clip);
        }
        break;

    case LG_SHOTGUN:
    case LG_HARPOON:
    case LG_ROCKET:
    case LG_GRENADE:
    case LG_M16:
        phd_PushMatrix_I();
        phd_TranslateRel_I(*(bone + 1 + 28), *(bone + 2 + 28), *(bone + 3 + 28));
        rotation1 = rotation2 = lara.right_arm.frame_base
            + lara.right_arm.frame_number * (anims[lara.right_arm.anim_number].interpolation >> 8) + 9;
        gar_RotYXZsuperpack_I(&rotation1, &rotation2, 8);
        PutPolyLara_I(UARM_R, clip);
        phd_TranslateRel_I(*(bone + 1 + 32), *(bone + 2 + 32), *(bone + 3 + 32));
        gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        PutPolyLara_I(LARM_R, clip);
        phd_TranslateRel_I(*(bone + 1 + 36), *(bone + 2 + 36), *(bone + 3 + 36));
        gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        PutPolyLara_I(HAND_R, clip);
        if (lara.right_arm.flash_gun)
            S_LongMemCpy(savemox1, phd_mxptr, 12);
        phd_PopMatrix_I();

        phd_PushMatrix_I();
        phd_TranslateRel_I(*(bone + 1 + 40), *(bone + 2 + 40), *(bone + 3 + 40));
        gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        PutPolyLara_I(UARM_L, clip);
        phd_TranslateRel_I(*(bone + 1 + 44), *(bone + 2 + 44), *(bone + 3 + 44));
        gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        PutPolyLara_I(LARM_L, clip);
        phd_TranslateRel_I(*(bone + 1 + 48), *(bone + 2 + 48), *(bone + 3 + 48));
        gar_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
        PutPolyLara_I(HAND_L, clip);
        if (lara.right_arm.flash_gun) {
            S_LongMemCpy(phd_mxptr, savemox1, 12);
            DrawGunFlash(fire_arms, clip);
        }
        break;
    }

    phd_PopMatrix();
    phd_PopMatrix();
    phd_PopMatrix();

    if (do_flare) {
        PHD_VECTOR pos;
        sint32 xv, yv, zv, lp;

        pos.x = 8;
        pos.y = 36;
        pos.z = 32;
        GetLaraHandAbsPosition(&pos, 0);
        xv = pos.x;
        yv = pos.y;
        zv = pos.z;
        pos.x = 8;
        pos.y = 36;
        pos.z = 1024 + (GetRandomDraw() & 255);
        GetLaraHandAbsPosition(&pos, 0);
        for (lp = 0; lp < (GetRandomDraw() & 3) + 4; lp++)
            TriggerFlareSparks(xv, yv, zv, pos.x - xv, pos.y - yv, pos.z - zv, lp >> 2, 0);
    }
}
