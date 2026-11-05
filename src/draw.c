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
