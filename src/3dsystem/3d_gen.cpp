#include "../specific/standard.h"

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
extern int phd_znear;
extern int phd_persp;
extern int phd_oopersp;
extern int phd_winwidth;
extern int phd_winheight;
extern float f_zfar;
extern float f_znear;
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
    float fAspect;
    float fDesiredAspect;

    fov /= 2;
    phd_persp = ((phd_winwidth / 2) * phd_cos(fov)) / phd_sin(fov);
    phd_oopersp = sint32(one) / phd_persp;
    f_persp = float(phd_persp);
    f_oneopersp = one / f_persp;
    f_perspoznear = f_persp / f_znear;
    fAspect = float(phd_winwidth) / float(phd_winheight);
    fDesiredAspect = 4.0f / 3.0f;
    LfAspectCorrection = fDesiredAspect / fAspect;
    InitZTable();
}

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
extern sint32 phd_left, phd_right, phd_top, phd_bottom;
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
    int nTris;

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
            nTris = *objptr - 16;
            if (CurrentLevel != 19)
                triPtr = objptr + 65;
            else
                triPtr = objptr + 1;

            for (; nTris > 0; nTris--) {
                triPtr[3] = BlackGouraudIndex << 8;
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
    sint32 sy, cy, r0, r1;
    sint32 *mptr;

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

}
