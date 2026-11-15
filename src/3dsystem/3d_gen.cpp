#include "../specific/standard.h"
extern "C" {
#include "../types.h"
extern sint32 phd_right;
}

extern "C" {

#include "../3dglodef.h"

#define M00 0
#define M01 1
#define M02 2
#define M03 3
#define M10 4
#define M11 5
#define M12 6
#define M13 7
#define M20 8
#define M21 9
#define M22 10
#define M23 11

extern sint32 w2v_matrix[12];
extern int phd_zfar;
extern int phd_persp;
extern int phd_oopersp;
extern int phd_winwidth;
extern int phd_winheight;
extern float f_znear;
extern float f_zfar;
extern float f_persp;
extern float f_oneopersp;
extern float f_oneoznear;
extern float f_perspoznear;
extern float one;
extern float LfAspectCorrection;

void SetupZRange(void);
void InitZTable(void);
void do_quickysorty(int left, int right, sint32 buffer[][3]);

void phd_GetVectorAngles(sint32 x, sint32 y, sint32 z, PHD_ANGLE *dest)
{
    PHD_ANGLE pitch;

    *(dest + 0) = phd_atan(z, x);
    while ((sint16)x != x || (sint16)y != y || (sint16)z != z) {
        x >>= 2;
        y >>= 2;
        z >>= 2;
    }
    pitch = phd_atan(phd_sqrt(x * x + z * z), y);
    if ((y > 0 && pitch > 0) || (y < 0 && pitch < 0))
        pitch = -pitch;
    *(dest + 1) = pitch;
}

void phd_GenerateW2V(PHD_3DPOS *viewpos);
void phd_LookAt(sint32 xsrc, sint32 ysrc, sint32 zsrc, sint32 xtar, sint32 ytar, sint32 ztar, PHD_ANGLE roll);
void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz);
void phd_RotYXZpack(sint32 rpack);
void phd_PutPolygons(sint16 *objptr, int clipstatus);
void S_InsertRoom(sint16 *objptr, int outside);
sint16 *calc_back_light(sint16 *objptr);
void S_InsertBackground(sint16 *objptr);

void phd_RotX(PHD_ANGLE rx)
{
    sint32 sx, cx, r0, r1;
    sint32 *mptr;

    if (rx) {
        mptr = phd_mxptr;
        sx = phd_sin(rx);
        cx = phd_cos(rx);

        r0 = *(mptr + M01) * cx + *(mptr + M02) * sx;
        r1 = *(mptr + M02) * cx - *(mptr + M01) * sx;
        *(mptr + M01) = r0 >> W2V_SHIFT;
        *(mptr + M02) = r1 >> W2V_SHIFT;

        r0 = *(mptr + M11) * cx + *(mptr + M12) * sx;
        r1 = *(mptr + M12) * cx - *(mptr + M11) * sx;
        *(mptr + M11) = r0 >> W2V_SHIFT;
        *(mptr + M12) = r1 >> W2V_SHIFT;

        r0 = *(mptr + M21) * cx + *(mptr + M22) * sx;
        r1 = *(mptr + M22) * cx - *(mptr + M21) * sx;
        *(mptr + M21) = r0 >> W2V_SHIFT;
        *(mptr + M22) = r1 >> W2V_SHIFT;
    }
}

void phd_LookAt(sint32 xsrc, sint32 ysrc, sint32 zsrc, sint32 xtar, sint32 ytar, sint32 ztar, PHD_ANGLE roll);

int phd_TranslateRel(sint32 x, sint32 y, sint32 z)
{
    sint32 *mptr;

    mptr = phd_mxptr;
    *(mptr + M03) += *(mptr + M00) * x + *(mptr + M01) * y + *(mptr + M02) * z;
    *(mptr + M13) += *(mptr + M10) * x + *(mptr + M11) * y + *(mptr + M12) * z;
    *(mptr + M23) += *(mptr + M20) * x + *(mptr + M21) * y + *(mptr + M22) * z;
    if ((x = ABS(*(mptr + M03))) > phd_zfar || (y = ABS(*(mptr + M13))) > phd_zfar || (z = ABS(*(mptr + M23))) > phd_zfar)
        return 0;
    return 1;
}

void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz);
void phd_RotYXZpack(sint32 rpack);

void phd_TranslateAbs(sint32 x, sint32 y, sint32 z)
{
    sint32 *mptr = phd_mxptr;

    x -= *(w2v_matrix + M03);
    y -= *(w2v_matrix + M13);
    z -= *(w2v_matrix + M23);
    *(mptr + M03) = *(mptr + M00) * x + *(mptr + M01) * y + *(mptr + M02) * z;
    *(mptr + M13) = *(mptr + M10) * x + *(mptr + M11) * y + *(mptr + M12) * z;
    *(mptr + M23) = *(mptr + M20) * x + *(mptr + M21) * y + *(mptr + M22) * z;
}

void phd_SortPolyList(int number, sint32 buffer[][3])
{
    sint32 i;

    if (number) {
        for (i = 0; i < number; i++)
            buffer[i][1] += i;
        do_quickysorty(0, number - 1, buffer);
    }
}

void AlterFOV(PHD_ANGLE fov)
{
    fov /= 2;
    phd_persp = ((phd_winwidth / 2) * phd_cos(fov)) / phd_sin(fov);
    phd_oopersp = sint32(one) / phd_persp;
    f_persp = float(phd_persp);
    f_oneopersp = one / f_persp;
    f_perspoznear = f_persp / f_znear;
    LfAspectCorrection = (4.0f / 3.0f) / (float(phd_winwidth) / float(phd_winheight));
    InitZTable();
}

extern int phd_znear;

void SetZNear(int nNewZNear)
{
    phd_znear = nNewZNear;
    f_znear = float(phd_znear);
    f_oneoznear = one / f_znear;
    f_perspoznear = f_persp / f_znear;
    SetupZRange();
}

void SetZFar(int nNewZFar)
{
    phd_zfar = nNewZFar;
    f_zfar = float(phd_zfar);
    SetupZRange();
}

#include "../specific/winmain.h"

extern sint16 phd_winxmin;
extern sint16 phd_winymin;
extern sint16 phd_winxmax;
extern sint16 phd_winymax;
extern sint32 phd_left, phd_top, phd_bottom;
extern sint32 phd_centerx, phd_centery;
extern int phd_scrwidth;
extern int phd_scrheight;
extern int phd_viewdist;
extern float f_centerx, f_centery;
extern sint32 *phd_mxptr;
extern sint32 matrix_stack[];
extern RECT phd_WindowRect;
extern sint16 *(*InsertObjectGT3)(sint16 *pFaceInfo, int nFaces, int nSortType);
extern sint16 *(*InsertObjectGT4)(sint16 *pFaceInfo, int nFaces, int nSortType);
extern sint16 *(*InsertObjectG3)(sint16 *pFaceInfo, int nFaces, int nSortType);
extern sint16 *(*InsertObjectG4)(sint16 *pFaceInfo, int nFaces, int nSortType);
extern sint16 *(*RoomInsertObjectGT3)(sint16 *pFaceInfo, int nFaces, int nSortType);
extern sint16 *(*RoomInsertObjectGT4)(sint16 *pFaceInfo, int nFaces, int nSortType);
extern void (*InsertFlatRect)(sint32 min_x, sint32 min_y, sint32 max_x, sint32 max_y, int nZDepth, int colour);
extern void (*InsertLine)(sint32 x1, sint32 y1, sint32 x2, sint32 y2, sint32 z, char color);
extern void (*InsertSprite)(int zdepth, int x1, int y1, int x2, int y2, int nSprite, int shade, int shade1, int nDrawType, int offset);
extern void (*InsertTrans8)(void *vbuf, sint16 shade);
extern void (*InsertTransQuad)(sint32 sx, sint32 sy, sint32 w, sint32 h, sint32 z);
sint16 *HWI_InsertObjectGT3_Sorted(sint16 *pFaceInfo, int nFaces, int nSortType);
sint16 *HWI_InsertObjectGT4_Sorted(sint16 *pFaceInfo, int nFaces, int nSortType);
sint16 *HWI_InsertObjectG3_Sorted(sint16 *pFaceInfo, int nFaces, int nSortType);
sint16 *HWI_InsertObjectG4_Sorted(sint16 *pFaceInfo, int nFaces, int nSortType);
void HWI_InsertFlatRect_Sorted(sint32 min_x, sint32 min_y, sint32 max_x, sint32 max_y, int nZDepth, int colour);
void HWI_InsertLine_Sorted(sint32 x1, sint32 y1, sint32 x2, sint32 y2, sint32 z, char color);
void HWI_InsertSprite_Sorted(int zdepth, int x1, int y1, int x2, int y2, int nSprite, int shade, int shade1, int nDrawType, int offset);
void HWI_InsertTrans8_Sorted(void *vbuf, sint16 shade);
void HWI_InsertTransQuad_Sorted(sint32 sx, sint32 sy, sint32 w, sint32 h, sint32 z);

void phd_InitWindow(int x, int y, int width, int height, int nearz, int farz, int view_angle, int scrwidth, int scrheight)
{
    phd_winxmin = (sint16)x;
    phd_winymin = (sint16)y;
    phd_winxmax = (sint16)(width - 1);
    phd_winymax = (sint16)(height - 1);
    phd_winwidth = width;
    phd_winheight = height;
    phd_centerx = width / 2;
    phd_centery = height / 2;
    phd_znear = nearz << 14;
    phd_zfar = farz << 14;
    phd_viewdist = farz;
    phd_scrwidth = scrwidth;
    phd_scrheight = scrheight;
    f_centerx = float(phd_centerx);
    f_centery = float(phd_centery);
    AlterFOV((PHD_ANGLE)(182 * view_angle));
    SetZNear(phd_znear);
    SetZFar(phd_zfar);
    InitZTable();
    phd_left = 0;
    phd_top = 0;
    phd_right = phd_winxmax;
    phd_bottom = phd_winymax;
    phd_mxptr = matrix_stack;
    phd_WindowRect.left = phd_winxmin;
    phd_WindowRect.top = phd_winymin;
    phd_WindowRect.right = phd_winxmin + phd_winwidth;
    phd_WindowRect.bottom = phd_winymin + phd_winheight;

    switch (App.nRenderMode) {
    case 0:
        InsertObjectGT3 = HWI_InsertObjectGT3_Sorted;
        InsertObjectGT4 = HWI_InsertObjectGT4_Sorted;
        InsertObjectG3 = HWI_InsertObjectG3_Sorted;
        InsertObjectG4 = HWI_InsertObjectG4_Sorted;
        InsertFlatRect = HWI_InsertFlatRect_Sorted;
        InsertLine = HWI_InsertLine_Sorted;
        RoomInsertObjectGT3 = HWI_InsertObjectGT3_Sorted;
        RoomInsertObjectGT4 = HWI_InsertObjectGT4_Sorted;
        InsertSprite = HWI_InsertSprite_Sorted;
        InsertTrans8 = HWI_InsertTrans8_Sorted;
        InsertTransQuad = HWI_InsertTransQuad_Sorted;
        break;

    case 1:
        InsertObjectGT3 = HWI_InsertObjectGT3_Sorted;
        InsertObjectGT4 = HWI_InsertObjectGT4_Sorted;
        InsertObjectG3 = HWI_InsertObjectG3_Sorted;
        InsertObjectG4 = HWI_InsertObjectG4_Sorted;
        InsertFlatRect = HWI_InsertFlatRect_Sorted;
        InsertLine = HWI_InsertLine_Sorted;
        RoomInsertObjectGT3 = HWI_InsertObjectGT3_Sorted;
        RoomInsertObjectGT4 = HWI_InsertObjectGT4_Sorted;
        InsertSprite = HWI_InsertSprite_Sorted;
        InsertTrans8 = HWI_InsertTrans8_Sorted;
        InsertTransQuad = HWI_InsertTransQuad_Sorted;
        break;
    }
}

void phd_NormaliseVector(sint32 x, sint32 y, sint32 z, sint32 *dest)
{
    sint32 len;

    if (x || y || z) {
        while ((sint16)x != x || (sint16)y != y || (sint16)z != z) {
            x >>= 2;
            y >>= 2;
            z >>= 2;
        }
        len = phd_sqrt(x * x + y * y + z * z);
        *(dest + 0) = (x << W2V_SHIFT) / len;
        *(dest + 1) = (y << W2V_SHIFT) / len;
        *(dest + 2) = (z << W2V_SHIFT) / len;
    } else {
        *(dest + 0) = 0;
        *(dest + 1) = 0;
        *(dest + 2) = 0;
    }
}

typedef struct phd_vbuf {
    float xv;
    float yv;
    float zv;
    float ooz;
    float xs;
    float ys;
    sint32 z;
    char clip;
    char fog;
    sint16 g;
    unsigned short u;
    unsigned short v;
    int dynamic;
} PHD_VBUF;

extern PHD_VBUF vbuf[1500];
extern int surfacenum;
extern int surfacenumbf;
extern int surfacenumfb;
extern sint16 *info3dptr;
extern sint32 *sort3dptr;
extern sint16 info3d_buffer[];
extern sint32 sort3d_buffer[];
extern sint16 *info3dptrbf;
extern sint16 *info3dptrfb;
extern sint32 *sort3dptrbf;
extern sint32 *sort3dptrfb;
extern sint16 info3d_bufferbf[];
extern sint16 info3d_bufferfb[];
extern sint32 sort3d_bufferbf[];
extern sint32 sort3d_bufferfb[];

void HWR_InitVertexList(void);

sint16 *calc_back_light(sint16 *objptr)
{
    int nNormals;
    int i;

    nNormals = *objptr++;
    if (nNormals > 0) {
        objptr += 3 * nNormals;
    } else if (nNormals < 0) {
        nNormals = -nNormals;
        objptr += nNormals;
    }
    for (i = 0; i < nNormals; i++)
        vbuf[i].g = 0x4210;
    return objptr;
}

extern float outsideBackgroundTop;
extern float phd_leftfloat, phd_topfloat, phd_rightfloat, phd_bottomfloat;
extern bool bFixSkyColour;
extern int BlackGouraudIndex;
extern int CurrentLevel;
extern sint32 nPolyType;
sint16 *calc_object_verticesback(sint16 *objptr);
void S_InsertBackPolygon(int x1, int y1, int x2, int y2, int col);

void S_InsertBackground(sint16 *objptr)
{
    sint16 *triPtr;
    sint16 nTris;

    nPolyType = 1;
    phd_leftfloat = float(phd_winxmin + phd_left);
    phd_topfloat = float(phd_winymin + phd_top);
    phd_rightfloat = float(phd_right + phd_winxmin + 1);
    phd_bottomfloat = float(phd_bottom + phd_winymin + 1);
    f_centerx = float(phd_winxmin + phd_centerx);
    f_centery = float(phd_winymin + phd_centery);
    objptr += 4;
    objptr = calc_object_verticesback(objptr);
    outsideBackgroundTop = 0;

    if (objptr) {
        objptr = calc_back_light(objptr);
        objptr = InsertObjectGT4(objptr + 1, objptr[0], 2);
        objptr = InsertObjectGT3(objptr + 1, objptr[0], 2);
        objptr = InsertObjectG4(objptr + 1, objptr[0], 2);

        if (bFixSkyColour) {
            nTris = *objptr;
            if (CurrentLevel != 19)
                triPtr = objptr + 65;
            else
                triPtr = objptr + 1;

            for (int i = 0; i < nTris - 16; i++) {
                triPtr[3] = (unsigned __int64)BlackGouraudIndex * 256;
                triPtr += 4;
            }
            bFixSkyColour = false;
        }

        InsertObjectG3(objptr + 1, objptr[0], 2);
    }

    if (CurrentLevel == 0)
        S_InsertBackPolygon(0, (int)outsideBackgroundTop, phd_right + phd_winxmin, phd_winymax, 0x800000);
}

void phd_InitPolyList(void)
{
    surfacenum = 0;
    surfacenumbf = 0;
    surfacenumfb = 0;
    sort3dptr = sort3d_buffer;
    info3dptr = info3d_buffer;
    info3dptrbf = info3d_bufferbf;
    sort3dptrbf = sort3d_bufferbf;
    info3dptrfb = info3d_bufferfb;
    sort3dptrfb = sort3d_bufferfb;
    HWR_InitVertexList();
}

extern sint16 phd_winxmin;
extern sint16 phd_winymin;
extern sint32 phd_left, phd_right, phd_top, phd_bottom;
extern sint32 phd_centerx, phd_centery;
extern float phd_leftfloat, phd_topfloat, phd_rightfloat, phd_bottomfloat;
extern float f_centerx, f_centery;
extern sint16 *(*InsertObjectGT4)(sint16 *pFaceInfo, int nFaces, int nSortType);
extern sint16 *(*InsertObjectGT3)(sint16 *pFaceInfo, int nFaces, int nSortType);
sint16 *calc_roomvert(sint16 *objptr, int far_clip);
sint16 *ins_room_sprite(sint16 *objptr, int num);

void S_InsertRoom(sint16 *objptr, int out)
{
    phd_leftfloat = (float)(phd_winxmin + phd_left);
    phd_topfloat = (float)(phd_winymin + phd_top);
    phd_rightfloat = (float)(phd_right + phd_winxmin + 1);
    phd_bottomfloat = (float)(phd_bottom + phd_winymin + 1);
    f_centerx = (float)(phd_winxmin + phd_centerx);
    f_centery = (float)(phd_winymin + phd_centery);
    objptr = calc_roomvert(objptr, out ? 16 : 0);
    objptr = InsertObjectGT4(objptr + 1, objptr[0], 1);
    objptr = InsertObjectGT3(objptr + 1, objptr[0], 1);
    ins_room_sprite(objptr + 1, objptr[0]);
}


sint16 *calc_roomvert(sint16 *objptr, int far_clip);

void phd_RotY(PHD_ANGLE ry)
{
    sint32 sy;
    sint32 *mptr;
    sint32 cy;
    sint32 r0;
    sint32 r1;

    if (ry) {
        mptr = phd_mxptr;
        sy = phd_sin(ry);
        cy = phd_cos(ry);

        r0 = *(mptr + M00) * cy - *(mptr + M02) * sy;
        r1 = *(mptr + M02) * cy + *(mptr + M00) * sy;
        *(mptr + M00) = r0 >> W2V_SHIFT;
        *(mptr + M02) = r1 >> W2V_SHIFT;

        r0 = *(mptr + M10) * cy - *(mptr + M12) * sy;
        r1 = *(mptr + M12) * cy + *(mptr + M10) * sy;
        *(mptr + M10) = r0 >> W2V_SHIFT;
        *(mptr + M12) = r1 >> W2V_SHIFT;

        r0 = *(mptr + M20) * cy - *(mptr + M22) * sy;
        r1 = *(mptr + M22) * cy + *(mptr + M20) * sy;
        *(mptr + M20) = r0 >> W2V_SHIFT;
        *(mptr + M22) = r1 >> W2V_SHIFT;
    }
}

void phd_LookAt(sint32 xsrc, sint32 ysrc, sint32 zsrc, sint32 xtar, sint32 ytar, sint32 ztar, PHD_ANGLE roll);
void phd_GetVectorAngles(sint32 x, sint32 y, sint32 z, PHD_ANGLE *dest);
void phd_NormaliseVector(sint32 x, sint32 y, sint32 z, sint32 *dest);

void phd_RotZ(PHD_ANGLE rz)
{
    sint32 sz, cz, r0, r1;
    sint32 *mptr;

    if (rz) {
        mptr = phd_mxptr;
        sz = phd_sin(rz);
        cz = phd_cos(rz);

        r0 = *(mptr + M00) * cz + *(mptr + M01) * sz;
        r1 = *(mptr + M01) * cz - *(mptr + M00) * sz;
        *(mptr + M00) = r0 >> W2V_SHIFT;
        *(mptr + M01) = r1 >> W2V_SHIFT;

        r0 = *(mptr + M10) * cz + *(mptr + M11) * sz;
        r1 = *(mptr + M11) * cz - *(mptr + M10) * sz;
        *(mptr + M10) = r0 >> W2V_SHIFT;
        *(mptr + M11) = r1 >> W2V_SHIFT;

        r0 = *(mptr + M20) * cz + *(mptr + M21) * sz;
        r1 = *(mptr + M21) * cz - *(mptr + M20) * sz;
        *(mptr + M20) = r0 >> W2V_SHIFT;
        *(mptr + M21) = r1 >> W2V_SHIFT;
    }
}


extern PHD_VECTOR CamPos;
extern PHD_VECTOR CamRot;
sint32 mGetAngle(sint32 x1, sint32 y1, sint32 x2, sint32 y2);

void phd_LookAt(sint32 xsrc, sint32 ysrc, sint32 zsrc, sint32 xtar, sint32 ytar, sint32 ztar, PHD_ANGLE roll)
{
    PHD_ANGLE angles[2];
    PHD_3DPOS viewer;
    sint32 dx, dy, dz;

    phd_GetVectorAngles(xtar - xsrc, ytar - ysrc, ztar - zsrc, angles);
    viewer.x_pos = xsrc;
    viewer.y_pos = ysrc;
    viewer.z_pos = zsrc;
    viewer.x_rot = angles[1];
    viewer.y_rot = angles[0];
    viewer.z_rot = roll;
    dx = xsrc - xtar;
    dy = ysrc - ytar;
    dz = zsrc - ztar;
    CamRot.x = ((uint32)mGetAngle(0, 0, (sint32)sqrt((double)(dx * dx + dz * dz)), dy) >> 4) & 4095;
    CamRot.y = ((uint32)mGetAngle(zsrc, xsrc, ztar, xtar) >> 4) & 4095;
    CamRot.z = 0;
    CamPos.x = xsrc;
    CamPos.y = ysrc;
    CamPos.z = zsrc;
    phd_GenerateW2V(&viewer);
}


void phd_GenerateW2V(PHD_3DPOS *viewpos)
{
    sint32 sx, sy, sz, cx, cy, cz;
    sint32 *mptr, *nptr;

    mptr = phd_mxptr = matrix_stack;
    nptr = w2v_matrix;
    sx = phd_sin(viewpos->x_rot);
    cx = phd_cos(viewpos->x_rot);
    sy = phd_sin(viewpos->y_rot);
    cy = phd_cos(viewpos->y_rot);
    sz = phd_sin(viewpos->z_rot);
    cz = phd_cos(viewpos->z_rot);
    nptr[M00] = mptr[M00] = (((sx * sy) >> W2V_SHIFT) * sz >> W2V_SHIFT) + ((cy * cz) >> W2V_SHIFT);
    nptr[M01] = mptr[M01] = (cx * sz) >> W2V_SHIFT;
    nptr[M02] = mptr[M02] = (((sx * cy) >> W2V_SHIFT) * sz >> W2V_SHIFT) - ((sy * cz) >> W2V_SHIFT);
    nptr[M10] = mptr[M10] = (((sx * sy) >> W2V_SHIFT) * cz >> W2V_SHIFT) - ((cy * sz) >> W2V_SHIFT);
    nptr[M11] = mptr[M11] = (cx * cz) >> W2V_SHIFT;
    nptr[M12] = mptr[M12] = (((sx * cy) >> W2V_SHIFT) * cz >> W2V_SHIFT) + ((sy * sz) >> W2V_SHIFT);
    nptr[M20] = mptr[M20] = (cx * sy) >> W2V_SHIFT;
    nptr[M21] = mptr[M21] = -sx;
    nptr[M22] = mptr[M22] = (cx * cy) >> W2V_SHIFT;
    nptr[M03] = mptr[M03] = viewpos->x_pos;
    nptr[M13] = mptr[M13] = viewpos->y_pos;
    nptr[M23] = mptr[M23] = viewpos->z_pos;
    nptr[M10] = mptr[M10] = (sint32)((float)mptr[M10] * LfAspectCorrection);
    nptr[M11] = mptr[M11] = (sint32)((float)mptr[M11] * LfAspectCorrection);
    nptr[M12] = mptr[M12] = (sint32)((float)mptr[M12] * LfAspectCorrection);
}


void phd_LookAt(sint32 xsrc, sint32 ysrc, sint32 zsrc, sint32 xtar, sint32 ytar, sint32 ztar, PHD_ANGLE roll);

void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz)
{
    sint32 sina, cosa, r0, r1;
    sint32 *mptr;

    mptr = phd_mxptr;
    if (ry) {
        sina = phd_sin(ry);
        cosa = phd_cos(ry);
        r0 = *(mptr + M00) * cosa - *(mptr + M02) * sina;
        r1 = *(mptr + M02) * cosa + *(mptr + M00) * sina;
        *(mptr + M00) = r0 >> W2V_SHIFT;
        *(mptr + M02) = r1 >> W2V_SHIFT;
        r0 = *(mptr + M10) * cosa - *(mptr + M12) * sina;
        r1 = *(mptr + M12) * cosa + *(mptr + M10) * sina;
        *(mptr + M10) = r0 >> W2V_SHIFT;
        *(mptr + M12) = r1 >> W2V_SHIFT;
        r0 = *(mptr + M20) * cosa - *(mptr + M22) * sina;
        r1 = *(mptr + M22) * cosa + *(mptr + M20) * sina;
        *(mptr + M20) = r0 >> W2V_SHIFT;
        *(mptr + M22) = r1 >> W2V_SHIFT;
    }
    if (rx) {
        sina = phd_sin(rx);
        cosa = phd_cos(rx);
        r0 = *(mptr + M01) * cosa + *(mptr + M02) * sina;
        r1 = *(mptr + M02) * cosa - *(mptr + M01) * sina;
        *(mptr + M01) = r0 >> W2V_SHIFT;
        *(mptr + M02) = r1 >> W2V_SHIFT;
        r0 = *(mptr + M11) * cosa + *(mptr + M12) * sina;
        r1 = *(mptr + M12) * cosa - *(mptr + M11) * sina;
        *(mptr + M11) = r0 >> W2V_SHIFT;
        *(mptr + M12) = r1 >> W2V_SHIFT;
        r0 = *(mptr + M21) * cosa + *(mptr + M22) * sina;
        r1 = *(mptr + M22) * cosa - *(mptr + M21) * sina;
        *(mptr + M21) = r0 >> W2V_SHIFT;
        *(mptr + M22) = r1 >> W2V_SHIFT;
    }
    if (rz) {
        sina = phd_sin(rz);
        cosa = phd_cos(rz);
        r0 = *(mptr + M00) * cosa + *(mptr + M01) * sina;
        r1 = *(mptr + M01) * cosa - *(mptr + M00) * sina;
        *(mptr + M00) = r0 >> W2V_SHIFT;
        *(mptr + M01) = r1 >> W2V_SHIFT;
        r0 = *(mptr + M10) * cosa + *(mptr + M11) * sina;
        r1 = *(mptr + M11) * cosa - *(mptr + M10) * sina;
        *(mptr + M10) = r0 >> W2V_SHIFT;
        *(mptr + M11) = r1 >> W2V_SHIFT;
        r0 = *(mptr + M20) * cosa + *(mptr + M21) * sina;
        r1 = *(mptr + M21) * cosa - *(mptr + M20) * sina;
        *(mptr + M20) = r0 >> W2V_SHIFT;
        *(mptr + M21) = r1 >> W2V_SHIFT;
    }
}


void phd_RotX(PHD_ANGLE rx);

void phd_RotYXZpack(sint32 rots)
{
    sint32 ang;
    sint32 sina, cosa, r0, r1;
    sint32 *mptr;

    mptr = phd_mxptr;
    ang = ((rots >> 10) & 1023) << 6;
    if (ang) {
        sina = phd_sin(ang);
        cosa = phd_cos(ang);
        r0 = *(mptr + M00) * cosa - *(mptr + M02) * sina;
        r1 = *(mptr + M02) * cosa + *(mptr + M00) * sina;
        *(mptr + M00) = r0 >> W2V_SHIFT;
        *(mptr + M02) = r1 >> W2V_SHIFT;
        r0 = *(mptr + M10) * cosa - *(mptr + M12) * sina;
        r1 = *(mptr + M12) * cosa + *(mptr + M10) * sina;
        *(mptr + M10) = r0 >> W2V_SHIFT;
        *(mptr + M12) = r1 >> W2V_SHIFT;
        r0 = *(mptr + M20) * cosa - *(mptr + M22) * sina;
        r1 = *(mptr + M22) * cosa + *(mptr + M20) * sina;
        *(mptr + M20) = r0 >> W2V_SHIFT;
        *(mptr + M22) = r1 >> W2V_SHIFT;
    }
    ang = ((rots >> 20) & 1023) << 6;
    if (ang) {
        sina = phd_sin(ang);
        cosa = phd_cos(ang);
        r0 = *(mptr + M01) * cosa + *(mptr + M02) * sina;
        r1 = *(mptr + M02) * cosa - *(mptr + M01) * sina;
        *(mptr + M01) = r0 >> W2V_SHIFT;
        *(mptr + M02) = r1 >> W2V_SHIFT;
        r0 = *(mptr + M11) * cosa + *(mptr + M12) * sina;
        r1 = *(mptr + M12) * cosa - *(mptr + M11) * sina;
        *(mptr + M11) = r0 >> W2V_SHIFT;
        *(mptr + M12) = r1 >> W2V_SHIFT;
        r0 = *(mptr + M21) * cosa + *(mptr + M22) * sina;
        r1 = *(mptr + M22) * cosa - *(mptr + M21) * sina;
        *(mptr + M21) = r0 >> W2V_SHIFT;
        *(mptr + M22) = r1 >> W2V_SHIFT;
    }
    ang = (rots & 1023) << 6;
    if (ang) {
        sina = phd_sin(ang);
        cosa = phd_cos(ang);
        r0 = *(mptr + M00) * cosa + *(mptr + M01) * sina;
        r1 = *(mptr + M01) * cosa - *(mptr + M00) * sina;
        *(mptr + M00) = r0 >> W2V_SHIFT;
        *(mptr + M01) = r1 >> W2V_SHIFT;
        r0 = *(mptr + M10) * cosa + *(mptr + M11) * sina;
        r1 = *(mptr + M11) * cosa - *(mptr + M10) * sina;
        *(mptr + M10) = r0 >> W2V_SHIFT;
        *(mptr + M11) = r1 >> W2V_SHIFT;
        r0 = *(mptr + M20) * cosa + *(mptr + M21) * sina;
        r1 = *(mptr + M21) * cosa - *(mptr + M20) * sina;
        *(mptr + M20) = r0 >> W2V_SHIFT;
        *(mptr + M21) = r1 >> W2V_SHIFT;
    }
}


sint16 *calc_object_vertices(sint16 *objptr);
sint16 *calc_vertice_light(sint16 *objptr, sint16 *objptr1);

void phd_PutPolygons(sint16 *objptr, int clipstatus)
{
    sint16 *objptr1;

    objptr += 4;
    phd_leftfloat = (float)phd_winxmin;
    phd_topfloat = (float)phd_winymin;
    phd_rightfloat = (float)(phd_winxmin + phd_winxmax + 1);
    phd_bottomfloat = (float)(phd_winymin + phd_winymax + 1);
    f_centerx = (float)(phd_winxmin + phd_centerx);
    f_centery = (float)(phd_winymin + phd_centery);
    objptr1 = objptr;
    objptr = calc_object_vertices(objptr);
    if (objptr) {
        objptr = calc_vertice_light(objptr, objptr1);
        objptr = InsertObjectGT4(objptr + 1, *objptr, 0);
        objptr = InsertObjectGT3(objptr + 1, *objptr, 0);
        objptr = InsertObjectG4(objptr + 1, *objptr, 0);
        objptr = InsertObjectG3(objptr + 1, *objptr, 0);
    }
}


void do_quickysorty(int left, int right, sint32 buffer[][3])
{
    sint32 i, j;
    sint32 compare, swap;

    i = left;
    j = right;
    compare = buffer[(left + right) / 2][1];
    do {
        while (buffer[i][1] > compare && i < right)
            ++i;
        while (compare > buffer[j][1] && j > left)
            --j;
        if (i <= j) {
            swap = buffer[i][1];
            buffer[i][1] = buffer[j][1];
            buffer[j][1] = swap;
            swap = buffer[i][0];
            buffer[i][0] = buffer[j][0];
            buffer[j][0] = swap;
            swap = buffer[i][2];
            buffer[i][2] = buffer[j][2];
            buffer[j][2] = swap;
            i++;
            j--;
        }
    } while (i <= j);
    if (left < j)
        do_quickysorty(left, j, buffer);
    if (i < right)
        do_quickysorty(i, right, buffer);
}


extern float f_a;
extern float f_b;
extern float f_boo;

void SetupZRange(void)
{
    f_b = ((0.995f - 0.005f) * f_znear * f_zfar) / (f_znear - f_zfar);
    f_a = 0.005f - (f_b / f_znear);
    f_b = -f_b;
    f_boo = f_b / one;
}

#if FEAT_C_ASM

sint32 phdtan2[8] = {
    0, -16384, -65535, 49152, -32768, 16384, 32768, -49152,
};

sint16 phdtantab[2050] = {
    0, 5, 10, 15, 20, 25, 31, 36, 41, 46, 51, 56,
    61, 66, 71, 76, 81, 87, 92, 97, 102, 107, 112, 117,
    122, 127, 132, 138, 143, 148, 153, 158, 163, 168, 173, 178,
    183, 188, 194, 199, 204, 209, 214, 219, 224, 229, 234, 239,
    244, 250, 255, 260, 265, 270, 275, 280, 285, 290, 295, 300,
    305, 311, 316, 321, 326, 331, 336, 341, 346, 351, 356, 361,
    367, 372, 377, 382, 387, 392, 397, 402, 407, 412, 417, 422,
    428, 433, 438, 443, 448, 453, 458, 463, 468, 473, 478, 483,
    489, 494, 499, 504, 509, 514, 519, 524, 529, 534, 539, 544,
    550, 555, 560, 565, 570, 575, 580, 585, 590, 595, 600, 605,
    610, 616, 621, 626, 631, 636, 641, 646, 651, 656, 661, 666,
    671, 676, 681, 687, 692, 697, 702, 707, 712, 717, 722, 727,
    732, 737, 742, 747, 752, 758, 763, 768, 773, 778, 783, 788,
    793, 798, 803, 808, 813, 818, 823, 828, 833, 839, 844, 849,
    854, 859, 864, 869, 874, 879, 884, 889, 894, 899, 904, 909,
    914, 919, 924, 930, 935, 940, 945, 950, 955, 960, 965, 970,
    975, 980, 985, 990, 995, 1000, 1005, 1010, 1015, 1020, 1025, 1031,
    1036, 1041, 1046, 1051, 1056, 1061, 1066, 1071, 1076, 1081, 1086, 1091,
    1096, 1101, 1106, 1111, 1116, 1121, 1126, 1131, 1136, 1141, 1146, 1151,
    1156, 1161, 1166, 1172, 1177, 1182, 1187, 1192, 1197, 1202, 1207, 1212,
    1217, 1222, 1227, 1232, 1237, 1242, 1247, 1252, 1257, 1262, 1267, 1272,
    1277, 1282, 1287, 1292, 1297, 1302, 1307, 1312, 1317, 1322, 1327, 1332,
    1337, 1342, 1347, 1352, 1357, 1362, 1367, 1372, 1377, 1382, 1387, 1392,
    1397, 1402, 1407, 1412, 1417, 1422, 1427, 1432, 1437, 1442, 1447, 1452,
    1457, 1462, 1467, 1472, 1477, 1482, 1487, 1492, 1497, 1502, 1507, 1512,
    1517, 1522, 1527, 1532, 1537, 1542, 1547, 1552, 1557, 1562, 1567, 1572,
    1577, 1582, 1587, 1592, 1597, 1602, 1607, 1612, 1617, 1622, 1627, 1632,
    1637, 1642, 1646, 1651, 1656, 1661, 1666, 1671, 1676, 1681, 1686, 1691,
    1696, 1701, 1706, 1711, 1716, 1721, 1726, 1731, 1736, 1741, 1746, 1751,
    1756, 1761, 1765, 1770, 1775, 1780, 1785, 1790, 1795, 1800, 1805, 1810,
    1815, 1820, 1825, 1830, 1835, 1840, 1845, 1849, 1854, 1859, 1864, 1869,
    1874, 1879, 1884, 1889, 1894, 1899, 1904, 1909, 1914, 1918, 1923, 1928,
    1933, 1938, 1943, 1948, 1953, 1958, 1963, 1968, 1973, 1977, 1982, 1987,
    1992, 1997, 2002, 2007, 2012, 2017, 2022, 2027, 2031, 2036, 2041, 2046,
    2051, 2056, 2061, 2066, 2071, 2076, 2080, 2085, 2090, 2095, 2100, 2105,
    2110, 2115, 2120, 2124, 2129, 2134, 2139, 2144, 2149, 2154, 2159, 2163,
    2168, 2173, 2178, 2183, 2188, 2193, 2198, 2202, 2207, 2212, 2217, 2222,
    2227, 2232, 2237, 2241, 2246, 2251, 2256, 2261, 2266, 2271, 2275, 2280,
    2285, 2290, 2295, 2300, 2305, 2309, 2314, 2319, 2324, 2329, 2334, 2338,
    2343, 2348, 2353, 2358, 2363, 2367, 2372, 2377, 2382, 2387, 2392, 2396,
    2401, 2406, 2411, 2416, 2421, 2425, 2430, 2435, 2440, 2445, 2450, 2454,
    2459, 2464, 2469, 2474, 2478, 2483, 2488, 2493, 2498, 2502, 2507, 2512,
    2517, 2522, 2526, 2531, 2536, 2541, 2546, 2550, 2555, 2560, 2565, 2570,
    2574, 2579, 2584, 2589, 2594, 2598, 2603, 2608, 2613, 2617, 2622, 2627,
    2632, 2637, 2641, 2646, 2651, 2656, 2660, 2665, 2670, 2675, 2679, 2684,
    2689, 2694, 2699, 2703, 2708, 2713, 2718, 2722, 2727, 2732, 2737, 2741,
    2746, 2751, 2756, 2760, 2765, 2770, 2775, 2779, 2784, 2789, 2793, 2798,
    2803, 2808, 2812, 2817, 2822, 2827, 2831, 2836, 2841, 2846, 2850, 2855,
    2860, 2864, 2869, 2874, 2879, 2883, 2888, 2893, 2897, 2902, 2907, 2912,
    2916, 2921, 2926, 2930, 2935, 2940, 2944, 2949, 2954, 2959, 2963, 2968,
    2973, 2977, 2982, 2987, 2991, 2996, 3001, 3005, 3010, 3015, 3019, 3024,
    3029, 3033, 3038, 3043, 3047, 3052, 3057, 3061, 3066, 3071, 3075, 3080,
    3085, 3089, 3094, 3099, 3103, 3108, 3113, 3117, 3122, 3127, 3131, 3136,
    3141, 3145, 3150, 3155, 3159, 3164, 3168, 3173, 3178, 3182, 3187, 3192,
    3196, 3201, 3206, 3210, 3215, 3219, 3224, 3229, 3233, 3238, 3243, 3247,
    3252, 3256, 3261, 3266, 3270, 3275, 3279, 3284, 3289, 3293, 3298, 3302,
    3307, 3312, 3316, 3321, 3325, 3330, 3335, 3339, 3344, 3348, 3353, 3358,
    3362, 3367, 3371, 3376, 3380, 3385, 3390, 3394, 3399, 3403, 3408, 3412,
    3417, 3422, 3426, 3431, 3435, 3440, 3444, 3449, 3453, 3458, 3463, 3467,
    3472, 3476, 3481, 3485, 3490, 3494, 3499, 3503, 3508, 3513, 3517, 3522,
    3526, 3531, 3535, 3540, 3544, 3549, 3553, 3558, 3562, 3567, 3571, 3576,
    3580, 3585, 3589, 3594, 3599, 3603, 3608, 3612, 3617, 3621, 3626, 3630,
    3635, 3639, 3644, 3648, 3653, 3657, 3662, 3666, 3670, 3675, 3679, 3684,
    3688, 3693, 3697, 3702, 3706, 3711, 3715, 3720, 3724, 3729, 3733, 3738,
    3742, 3747, 3751, 3756, 3760, 3764, 3769, 3773, 3778, 3782, 3787, 3791,
    3796, 3800, 3804, 3809, 3813, 3818, 3822, 3827, 3831, 3836, 3840, 3844,
    3849, 3853, 3858, 3862, 3867, 3871, 3875, 3880, 3884, 3889, 3893, 3898,
    3902, 3906, 3911, 3915, 3920, 3924, 3928, 3933, 3937, 3942, 3946, 3950,
    3955, 3959, 3964, 3968, 3972, 3977, 3981, 3985, 3990, 3994, 3999, 4003,
    4007, 4012, 4016, 4021, 4025, 4029, 4034, 4038, 4042, 4047, 4051, 4055,
    4060, 4064, 4069, 4073, 4077, 4082, 4086, 4090, 4095, 4099, 4103, 4108,
    4112, 4116, 4121, 4125, 4129, 4134, 4138, 4142, 4147, 4151, 4155, 4160,
    4164, 4168, 4173, 4177, 4181, 4186, 4190, 4194, 4199, 4203, 4207, 4211,
    4216, 4220, 4224, 4229, 4233, 4237, 4242, 4246, 4250, 4254, 4259, 4263,
    4267, 4272, 4276, 4280, 4284, 4289, 4293, 4297, 4302, 4306, 4310, 4314,
    4319, 4323, 4327, 4331, 4336, 4340, 4344, 4349, 4353, 4357, 4361, 4366,
    4370, 4374, 4378, 4383, 4387, 4391, 4395, 4400, 4404, 4408, 4412, 4416,
    4421, 4425, 4429, 4433, 4438, 4442, 4446, 4450, 4454, 4459, 4463, 4467,
    4471, 4476, 4480, 4484, 4488, 4492, 4497, 4501, 4505, 4509, 4513, 4518,
    4522, 4526, 4530, 4534, 4539, 4543, 4547, 4551, 4555, 4559, 4564, 4568,
    4572, 4576, 4580, 4585, 4589, 4593, 4597, 4601, 4605, 4610, 4614, 4618,
    4622, 4626, 4630, 4634, 4639, 4643, 4647, 4651, 4655, 4659, 4663, 4668,
    4672, 4676, 4680, 4684, 4688, 4692, 4697, 4701, 4705, 4709, 4713, 4717,
    4721, 4725, 4730, 4734, 4738, 4742, 4746, 4750, 4754, 4758, 4762, 4767,
    4771, 4775, 4779, 4783, 4787, 4791, 4795, 4799, 4803, 4807, 4812, 4816,
    4820, 4824, 4828, 4832, 4836, 4840, 4844, 4848, 4852, 4856, 4860, 4865,
    4869, 4873, 4877, 4881, 4885, 4889, 4893, 4897, 4901, 4905, 4909, 4913,
    4917, 4921, 4925, 4929, 4933, 4937, 4941, 4945, 4949, 4954, 4958, 4962,
    4966, 4970, 4974, 4978, 4982, 4986, 4990, 4994, 4998, 5002, 5006, 5010,
    5014, 5018, 5022, 5026, 5030, 5034, 5038, 5042, 5046, 5050, 5054, 5058,
    5062, 5066, 5070, 5074, 5078, 5082, 5086, 5090, 5094, 5097, 5101, 5105,
    5109, 5113, 5117, 5121, 5125, 5129, 5133, 5137, 5141, 5145, 5149, 5153,
    5157, 5161, 5165, 5169, 5173, 5177, 5181, 5184, 5188, 5192, 5196, 5200,
    5204, 5208, 5212, 5216, 5220, 5224, 5228, 5232, 5235, 5239, 5243, 5247,
    5251, 5255, 5259, 5263, 5267, 5271, 5275, 5278, 5282, 5286, 5290, 5294,
    5298, 5302, 5306, 5310, 5313, 5317, 5321, 5325, 5329, 5333, 5337, 5341,
    5344, 5348, 5352, 5356, 5360, 5364, 5368, 5371, 5375, 5379, 5383, 5387,
    5391, 5395, 5398, 5402, 5406, 5410, 5414, 5418, 5421, 5425, 5429, 5433,
    5437, 5441, 5444, 5448, 5452, 5456, 5460, 5464, 5467, 5471, 5475, 5479,
    5483, 5486, 5490, 5494, 5498, 5502, 5505, 5509, 5513, 5517, 5521, 5524,
    5528, 5532, 5536, 5540, 5543, 5547, 5551, 5555, 5559, 5562, 5566, 5570,
    5574, 5577, 5581, 5585, 5589, 5592, 5596, 5600, 5604, 5608, 5611, 5615,
    5619, 5623, 5626, 5630, 5634, 5638, 5641, 5645, 5649, 5652, 5656, 5660,
    5664, 5667, 5671, 5675, 5679, 5682, 5686, 5690, 5694, 5697, 5701, 5705,
    5708, 5712, 5716, 5720, 5723, 5727, 5731, 5734, 5738, 5742, 5745, 5749,
    5753, 5757, 5760, 5764, 5768, 5771, 5775, 5779, 5782, 5786, 5790, 5793,
    5797, 5801, 5804, 5808, 5812, 5815, 5819, 5823, 5826, 5830, 5834, 5837,
    5841, 5845, 5848, 5852, 5856, 5859, 5863, 5867, 5870, 5874, 5878, 5881,
    5885, 5888, 5892, 5896, 5899, 5903, 5907, 5910, 5914, 5917, 5921, 5925,
    5928, 5932, 5936, 5939, 5943, 5946, 5950, 5954, 5957, 5961, 5964, 5968,
    5972, 5975, 5979, 5982, 5986, 5990, 5993, 5997, 6000, 6004, 6008, 6011,
    6015, 6018, 6022, 6025, 6029, 6033, 6036, 6040, 6043, 6047, 6050, 6054,
    6058, 6061, 6065, 6068, 6072, 6075, 6079, 6082, 6086, 6089, 6093, 6097,
    6100, 6104, 6107, 6111, 6114, 6118, 6121, 6125, 6128, 6132, 6135, 6139,
    6142, 6146, 6150, 6153, 6157, 6160, 6164, 6167, 6171, 6174, 6178, 6181,
    6185, 6188, 6192, 6195, 6199, 6202, 6206, 6209, 6213, 6216, 6220, 6223,
    6227, 6230, 6234, 6237, 6240, 6244, 6247, 6251, 6254, 6258, 6261, 6265,
    6268, 6272, 6275, 6279, 6282, 6286, 6289, 6292, 6296, 6299, 6303, 6306,
    6310, 6313, 6317, 6320, 6323, 6327, 6330, 6334, 6337, 6341, 6344, 6348,
    6351, 6354, 6358, 6361, 6365, 6368, 6371, 6375, 6378, 6382, 6385, 6389,
    6392, 6395, 6399, 6402, 6406, 6409, 6412, 6416, 6419, 6423, 6426, 6429,
    6433, 6436, 6440, 6443, 6446, 6450, 6453, 6456, 6460, 6463, 6467, 6470,
    6473, 6477, 6480, 6483, 6487, 6490, 6493, 6497, 6500, 6504, 6507, 6510,
    6514, 6517, 6520, 6524, 6527, 6530, 6534, 6537, 6540, 6544, 6547, 6550,
    6554, 6557, 6560, 6564, 6567, 6570, 6574, 6577, 6580, 6584, 6587, 6590,
    6594, 6597, 6600, 6604, 6607, 6610, 6613, 6617, 6620, 6623, 6627, 6630,
    6633, 6637, 6640, 6643, 6646, 6650, 6653, 6656, 6660, 6663, 6666, 6669,
    6673, 6676, 6679, 6683, 6686, 6689, 6692, 6696, 6699, 6702, 6705, 6709,
    6712, 6715, 6718, 6722, 6725, 6728, 6731, 6735, 6738, 6741, 6744, 6748,
    6751, 6754, 6757, 6761, 6764, 6767, 6770, 6774, 6777, 6780, 6783, 6787,
    6790, 6793, 6796, 6799, 6803, 6806, 6809, 6812, 6815, 6819, 6822, 6825,
    6828, 6832, 6835, 6838, 6841, 6844, 6848, 6851, 6854, 6857, 6860, 6863,
    6867, 6870, 6873, 6876, 6879, 6883, 6886, 6889, 6892, 6895, 6898, 6902,
    6905, 6908, 6911, 6914, 6917, 6921, 6924, 6927, 6930, 6933, 6936, 6940,
    6943, 6946, 6949, 6952, 6955, 6958, 6962, 6965, 6968, 6971, 6974, 6977,
    6980, 6984, 6987, 6990, 6993, 6996, 6999, 7002, 7005, 7009, 7012, 7015,
    7018, 7021, 7024, 7027, 7030, 7033, 7037, 7040, 7043, 7046, 7049, 7052,
    7055, 7058, 7061, 7064, 7068, 7071, 7074, 7077, 7080, 7083, 7086, 7089,
    7092, 7095, 7098, 7101, 7105, 7108, 7111, 7114, 7117, 7120, 7123, 7126,
    7129, 7132, 7135, 7138, 7141, 7144, 7147, 7150, 7154, 7157, 7160, 7163,
    7166, 7169, 7172, 7175, 7178, 7181, 7184, 7187, 7190, 7193, 7196, 7199,
    7202, 7205, 7208, 7211, 7214, 7217, 7220, 7223, 7226, 7229, 7232, 7235,
    7238, 7241, 7244, 7247, 7250, 7253, 7256, 7259, 7262, 7265, 7268, 7271,
    7274, 7277, 7280, 7283, 7286, 7289, 7292, 7295, 7298, 7301, 7304, 7307,
    7310, 7313, 7316, 7319, 7322, 7325, 7328, 7331, 7334, 7337, 7340, 7343,
    7346, 7349, 7352, 7355, 7358, 7361, 7363, 7366, 7369, 7372, 7375, 7378,
    7381, 7384, 7387, 7390, 7393, 7396, 7399, 7402, 7405, 7408, 7411, 7413,
    7416, 7419, 7422, 7425, 7428, 7431, 7434, 7437, 7440, 7443, 7446, 7448,
    7451, 7454, 7457, 7460, 7463, 7466, 7469, 7472, 7475, 7477, 7480, 7483,
    7486, 7489, 7492, 7495, 7498, 7501, 7503, 7506, 7509, 7512, 7515, 7518,
    7521, 7524, 7526, 7529, 7532, 7535, 7538, 7541, 7544, 7547, 7549, 7552,
    7555, 7558, 7561, 7564, 7566, 7569, 7572, 7575, 7578, 7581, 7584, 7586,
    7589, 7592, 7595, 7598, 7601, 7603, 7606, 7609, 7612, 7615, 7618, 7620,
    7623, 7626, 7629, 7632, 7635, 7637, 7640, 7643, 7646, 7649, 7651, 7654,
    7657, 7660, 7663, 7665, 7668, 7671, 7674, 7677, 7679, 7682, 7685, 7688,
    7691, 7693, 7696, 7699, 7702, 7705, 7707, 7710, 7713, 7716, 7718, 7721,
    7724, 7727, 7730, 7732, 7735, 7738, 7741, 7743, 7746, 7749, 7752, 7754,
    7757, 7760, 7763, 7765, 7768, 7771, 7774, 7776, 7779, 7782, 7785, 7787,
    7790, 7793, 7796, 7798, 7801, 7804, 7807, 7809, 7812, 7815, 7818, 7820,
    7823, 7826, 7828, 7831, 7834, 7837, 7839, 7842, 7845, 7848, 7850, 7853,
    7856, 7858, 7861, 7864, 7866, 7869, 7872, 7875, 7877, 7880, 7883, 7885,
    7888, 7891, 7893, 7896, 7899, 7902, 7904, 7907, 7910, 7912, 7915, 7918,
    7920, 7923, 7926, 7928, 7931, 7934, 7936, 7939, 7942, 7944, 7947, 7950,
    7952, 7955, 7958, 7960, 7963, 7966, 7968, 7971, 7974, 7976, 7979, 7982,
    7984, 7987, 7990, 7992, 7995, 7997, 8000, 8003, 8005, 8008, 8011, 8013,
    8016, 8019, 8021, 8024, 8026, 8029, 8032, 8034, 8037, 8040, 8042, 8045,
    8047, 8050, 8053, 8055, 8058, 8060, 8063, 8066, 8068, 8071, 8074, 8076,
    8079, 8081, 8084, 8087, 8089, 8092, 8094, 8097, 8100, 8102, 8105, 8107,
    8110, 8112, 8115, 8118, 8120, 8123, 8125, 8128, 8131, 8133, 8136, 8138,
    8141, 8143, 8146, 8149, 8151, 8154, 8156, 8159, 8161, 8164, 8166, 8169,
    8172, 8174, 8177, 8179, 8182, 8184, 8187, 8189, 8192, 8192,
};

sint16 phdsintab[1026] = {
    0, 25, 50, 75, 101, 126, 151, 176, 201, 226, 251, 276,
    302, 327, 352, 377, 402, 427, 452, 477, 503, 528, 553, 578,
    603, 628, 653, 678, 704, 729, 754, 779, 804, 829, 854, 879,
    904, 929, 955, 980, 1005, 1030, 1055, 1080, 1105, 1130, 1155, 1180,
    1205, 1230, 1255, 1280, 1306, 1331, 1356, 1381, 1406, 1431, 1456, 1481,
    1506, 1531, 1556, 1581, 1606, 1631, 1656, 1681, 1706, 1731, 1756, 1781,
    1806, 1831, 1856, 1881, 1906, 1931, 1956, 1981, 2006, 2031, 2055, 2080,
    2105, 2130, 2155, 2180, 2205, 2230, 2255, 2280, 2305, 2329, 2354, 2379,
    2404, 2429, 2454, 2479, 2503, 2528, 2553, 2578, 2603, 2628, 2652, 2677,
    2702, 2727, 2752, 2776, 2801, 2826, 2851, 2875, 2900, 2925, 2949, 2974,
    2999, 3024, 3048, 3073, 3098, 3122, 3147, 3172, 3196, 3221, 3246, 3270,
    3295, 3320, 3344, 3369, 3393, 3418, 3442, 3467, 3492, 3516, 3541, 3565,
    3590, 3614, 3639, 3663, 3688, 3712, 3737, 3761, 3786, 3810, 3835, 3859,
    3883, 3908, 3932, 3957, 3981, 4005, 4030, 4054, 4078, 4103, 4127, 4151,
    4176, 4200, 4224, 4249, 4273, 4297, 4321, 4346, 4370, 4394, 4418, 4442,
    4467, 4491, 4515, 4539, 4563, 4587, 4612, 4636, 4660, 4684, 4708, 4732,
    4756, 4780, 4804, 4828, 4852, 4876, 4900, 4924, 4948, 4972, 4996, 5020,
    5044, 5068, 5092, 5115, 5139, 5163, 5187, 5211, 5235, 5259, 5282, 5306,
    5330, 5354, 5377, 5401, 5425, 5449, 5472, 5496, 5520, 5543, 5567, 5591,
    5614, 5638, 5661, 5685, 5708, 5732, 5756, 5779, 5803, 5826, 5850, 5873,
    5897, 5920, 5943, 5967, 5990, 6014, 6037, 6060, 6084, 6107, 6130, 6154,
    6177, 6200, 6223, 6247, 6270, 6293, 6316, 6339, 6363, 6386, 6409, 6432,
    6455, 6478, 6501, 6524, 6547, 6570, 6593, 6616, 6639, 6662, 6685, 6708,
    6731, 6754, 6777, 6800, 6823, 6846, 6868, 6891, 6914, 6937, 6960, 6982,
    7005, 7028, 7050, 7073, 7096, 7118, 7141, 7164, 7186, 7209, 7231, 7254,
    7276, 7299, 7321, 7344, 7366, 7389, 7411, 7434, 7456, 7478, 7501, 7523,
    7545, 7568, 7590, 7612, 7635, 7657, 7679, 7701, 7723, 7746, 7768, 7790,
    7812, 7834, 7856, 7878, 7900, 7922, 7944, 7966, 7988, 8010, 8032, 8054,
    8076, 8098, 8119, 8141, 8163, 8185, 8207, 8228, 8250, 8272, 8293, 8315,
    8337, 8358, 8380, 8401, 8423, 8445, 8466, 8488, 8509, 8531, 8552, 8573,
    8595, 8616, 8638, 8659, 8680, 8702, 8723, 8744, 8765, 8787, 8808, 8829,
    8850, 8871, 8892, 8914, 8935, 8956, 8977, 8998, 9019, 9040, 9061, 9082,
    9102, 9123, 9144, 9165, 9186, 9207, 9227, 9248, 9269, 9290, 9310, 9331,
    9352, 9372, 9393, 9413, 9434, 9455, 9475, 9496, 9516, 9537, 9557, 9577,
    9598, 9618, 9638, 9659, 9679, 9699, 9720, 9740, 9760, 9780, 9800, 9820,
    9841, 9861, 9881, 9901, 9921, 9941, 9961, 9981, 10001, 10020, 10040, 10060,
    10080, 10100, 10120, 10139, 10159, 10179, 10198, 10218, 10238, 10257, 10277, 10296,
    10316, 10336, 10355, 10374, 10394, 10413, 10433, 10452, 10471, 10491, 10510, 10529,
    10549, 10568, 10587, 10606, 10625, 10644, 10663, 10683, 10702, 10721, 10740, 10759,
    10778, 10796, 10815, 10834, 10853, 10872, 10891, 10909, 10928, 10947, 10966, 10984,
    11003, 11021, 11040, 11059, 11077, 11096, 11114, 11133, 11151, 11169, 11188, 11206,
    11224, 11243, 11261, 11279, 11297, 11316, 11334, 11352, 11370, 11388, 11406, 11424,
    11442, 11460, 11478, 11496, 11514, 11532, 11550, 11567, 11585, 11603, 11621, 11638,
    11656, 11674, 11691, 11709, 11727, 11744, 11762, 11779, 11797, 11814, 11831, 11849,
    11866, 11883, 11901, 11918, 11935, 11952, 11970, 11987, 12004, 12021, 12038, 12055,
    12072, 12089, 12106, 12123, 12140, 12157, 12173, 12190, 12207, 12224, 12240, 12257,
    12274, 12290, 12307, 12324, 12340, 12357, 12373, 12390, 12406, 12423, 12439, 12455,
    12472, 12488, 12504, 12520, 12537, 12553, 12569, 12585, 12601, 12617, 12633, 12649,
    12665, 12681, 12697, 12713, 12729, 12744, 12760, 12776, 12792, 12807, 12823, 12839,
    12854, 12870, 12885, 12901, 12916, 12932, 12947, 12963, 12978, 12993, 13008, 13024,
    13039, 13054, 13069, 13085, 13100, 13115, 13130, 13145, 13160, 13175, 13190, 13205,
    13219, 13234, 13249, 13264, 13279, 13293, 13308, 13323, 13337, 13352, 13366, 13381,
    13395, 13410, 13424, 13439, 13453, 13467, 13482, 13496, 13510, 13524, 13538, 13553,
    13567, 13581, 13595, 13609, 13623, 13637, 13651, 13665, 13678, 13692, 13706, 13720,
    13733, 13747, 13761, 13774, 13788, 13802, 13815, 13829, 13842, 13856, 13869, 13882,
    13896, 13909, 13922, 13935, 13949, 13962, 13975, 13988, 14001, 14014, 14027, 14040,
    14053, 14066, 14079, 14092, 14104, 14117, 14130, 14143, 14155, 14168, 14181, 14193,
    14206, 14218, 14231, 14243, 14256, 14268, 14280, 14293, 14305, 14317, 14329, 14341,
    14354, 14366, 14378, 14390, 14402, 14414, 14426, 14438, 14449, 14461, 14473, 14485,
    14497, 14508, 14520, 14531, 14543, 14555, 14566, 14578, 14589, 14601, 14612, 14623,
    14635, 14646, 14657, 14668, 14680, 14691, 14702, 14713, 14724, 14735, 14746, 14757,
    14768, 14779, 14789, 14800, 14811, 14822, 14832, 14843, 14854, 14864, 14875, 14885,
    14896, 14906, 14917, 14927, 14937, 14948, 14958, 14968, 14978, 14989, 14999, 15009,
    15019, 15029, 15039, 15049, 15059, 15069, 15078, 15088, 15098, 15108, 15118, 15127,
    15137, 15146, 15156, 15166, 15175, 15184, 15194, 15203, 15213, 15222, 15231, 15240,
    15250, 15259, 15268, 15277, 15286, 15295, 15304, 15313, 15322, 15331, 15340, 15349,
    15357, 15366, 15375, 15383, 15392, 15401, 15409, 15418, 15426, 15435, 15443, 15451,
    15460, 15468, 15476, 15485, 15493, 15501, 15509, 15517, 15525, 15533, 15541, 15549,
    15557, 15565, 15573, 15581, 15588, 15596, 15604, 15611, 15619, 15627, 15634, 15642,
    15649, 15656, 15664, 15671, 15679, 15686, 15693, 15700, 15707, 15715, 15722, 15729,
    15736, 15743, 15750, 15757, 15763, 15770, 15777, 15784, 15791, 15797, 15804, 15810,
    15817, 15824, 15830, 15837, 15843, 15849, 15856, 15862, 15868, 15875, 15881, 15887,
    15893, 15899, 15905, 15911, 15917, 15923, 15929, 15935, 15941, 15946, 15952, 15958,
    15964, 15969, 15975, 15980, 15986, 15991, 15997, 16002, 16008, 16013, 16018, 16024,
    16029, 16034, 16039, 16044, 16049, 16054, 16059, 16064, 16069, 16074, 16079, 16084,
    16088, 16093, 16098, 16103, 16107, 16112, 16116, 16121, 16125, 16130, 16134, 16138,
    16143, 16147, 16151, 16156, 16160, 16164, 16168, 16172, 16176, 16180, 16184, 16188,
    16192, 16195, 16199, 16203, 16207, 16210, 16214, 16218, 16221, 16225, 16228, 16232,
    16235, 16238, 16242, 16245, 16248, 16251, 16255, 16258, 16261, 16264, 16267, 16270,
    16273, 16276, 16279, 16281, 16284, 16287, 16290, 16292, 16295, 16298, 16300, 16303,
    16305, 16308, 16310, 16312, 16315, 16317, 16319, 16321, 16324, 16326, 16328, 16330,
    16332, 16334, 16336, 16338, 16340, 16341, 16343, 16345, 16347, 16348, 16350, 16352,
    16353, 16355, 16356, 16358, 16359, 16360, 16362, 16363, 16364, 16365, 16367, 16368,
    16369, 16370, 16371, 16372, 16373, 16374, 16375, 16375, 16376, 16377, 16378, 16378,
    16379, 16380, 16380, 16381, 16381, 16382, 16382, 16382, 16383, 16383, 16383, 16384,
    16384, 16384, 16384, 16384, 16384, 16384,
};

sint16 __fastcall phd_atan(sint32 x, sint32 y)
{
    sint32 octant, n, result;

    result = 0;
    if (x | y) {
        octant = 0;
        if (x < 0) {
            octant += 4;
            x = -x;
        }
        if (y < 0) {
            octant += 2;
            y = -y;
        }
        if (y > x) {
            octant++;
            n = x;
            x = y;
            y = n;
        }
        result = (uint16)phdtantab[(sint32)(((__int64)(uint32)y << 11) / x)] + phdtan2[octant];
        if (result < 0)
            result = -result;
    }
    return (sint16)result;
}

sint32 __fastcall phd_sin(sint32 angle)
{
    sint32 a;

    a = angle & 0xffff;
    if (a >= 0x8000) {
        a -= 0x8000;
        if (a > 0x4000)
            a = 0x8000 - a;
        return -(sint32)(uint16)phdsintab[(uint32)a >> 4];
    }
    if (a > 0x4000)
        a = 0x8000 - a;
    return (uint16)phdsintab[(uint32)a >> 4];
}

sint32 __fastcall phd_cos(sint32 angle)
{
    return phd_sin(angle + 0x4000);
}

uint32 __fastcall phd_sqrt(uint32 n)
{
    uint32 num, base, result, tmp;

    num = n;
    base = 0x40000000;
    result = 0;
    do {
        tmp = result >> 1;
        result += base;
        if (result > num) {
            result = tmp;
        } else {
            num -= result;
            result = base | tmp;
        }
        base >>= 2;
    } while (base);
    return result;
}

#endif

#if FEAT_C_ASM
void phd_PushMatrix(void)
{
    sint32 *mptr;
    int i;

    mptr = phd_mxptr;
    for (i = 0; i < 12; i++)
        mptr[12 + i] = mptr[i];
    phd_mxptr = mptr + 12;
}

void phd_PushUnitMatrix(void)
{
    sint32 *mptr;

    mptr = phd_mxptr += 12;
    mptr[M00] = 0x4000;
    mptr[M11] = 0x4000;
    mptr[M22] = 0x4000;
    mptr[M01] = 0;
    mptr[M02] = 0;
    mptr[M10] = 0;
    mptr[M12] = 0;
    mptr[M20] = 0;
    mptr[M21] = 0;
}
#endif

}
