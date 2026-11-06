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

}
