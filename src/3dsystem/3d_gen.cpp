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

void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz);
void phd_RotYXZpack(sint32 rpack);

}
