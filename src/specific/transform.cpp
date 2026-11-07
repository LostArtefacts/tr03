#include <io.h>
#include "standard.h"
#include "global.h"
#include "winmain.h"

extern "C" {

extern float ZTable[40960];
extern float f_persp;
extern float f_oneopersp;

void InitZTable(void)
{
    int z;
    int n;

    z = 0;
    for (n = 0; n < 20480; n++) {
        ZTable[n * 2 + 0] = (float)(f_persp / (float)(z << 14));
        ZTable[n * 2 + 1] = (float)(f_persp / (float)(z << 14)) * f_oneopersp;
        z++;
    }
}

typedef struct phd_vbuf {
    float xv;
    float yv;
    float zv;
    float ooz;
    float xs;
    float ys;
    long z;
    char clip;
    char fog;
    short g;
    unsigned short u;
    unsigned short v;
    int dynamic;
} PHD_VBUF;

extern PHD_VBUF vbuf[];
extern long *phd_mxptr;
extern long mid_sort;
extern int phd_znear, phd_zfar;
extern float f_zfar;
extern float f_centerx, f_centery;
extern float phd_leftfloat, phd_topfloat, phd_rightfloat, phd_bottomfloat;
extern float m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23;

#define MAGICNUM float((1 << 26) * ((1 << 26) * 1.5))
#define FTOL(x) ((fconv = (x) + (MAGICNUM)), *(int *)(&fconv))

inline void MatrixToFloat(long *mptr)
{
    m00 = float(mptr[0]);
    m01 = float(mptr[1]);
    m02 = float(mptr[2]);
    m03 = float(mptr[3]);
    m10 = float(mptr[4]);
    m11 = float(mptr[5]);
    m12 = float(mptr[6]);
    m13 = float(mptr[7]);
    m20 = float(mptr[8]);
    m21 = float(mptr[9]);
    m22 = float(mptr[10]);
    m23 = float(mptr[11]);
}

short *calc_object_vertices(short *objptr)
{
    PHD_VBUF *dest;
    long *mptr;
    float zv;
    int numvert;
    char clipflag, totclip;
    float f_mid_sort;
    int z;
    float lz;
    double fconv;

    f_mid_sort = App.lpZBuffer ? 0.0f : (float)(mid_sort << 22);
    mptr = phd_mxptr;
    dest = vbuf;
    totclip = -1;
    objptr++;
    MatrixToFloat(mptr);
    for (numvert = (int)*(objptr++); numvert > 0; numvert--, dest++, objptr += 3) {
        dest->xv = m00 * (float)objptr[0] + m01 * (float)objptr[1] + m02 * (float)objptr[2] + m03;
        dest->yv = m10 * (float)objptr[0] + m11 * (float)objptr[1] + m12 * (float)objptr[2] + m13;
        zv = m20 * (float)objptr[0] + m21 * (float)objptr[1] + m22 * (float)objptr[2] + m23;
        dest->z = z = FTOL(zv);
        if (z >= phd_znear) {
            if (z >= phd_zfar) {
                dest->zv = f_zfar;
                zv = f_persp / zv;
                dest->xs = dest->xv * zv + f_centerx;
                dest->ys = dest->yv * zv + f_centery;
                dest->ooz = zv * f_oneopersp;
            } else {
                dest->zv = zv + f_mid_sort;
                lz = ZTable[((z >> 14) << 1) + 0];
                dest->ooz = ZTable[((z >> 14) << 1) + 1];
                dest->xs = dest->xv * lz + f_centerx;
                dest->ys = dest->yv * lz + f_centery;
            }
            clipflag = 0;
            if (dest->xs < phd_leftfloat)
                clipflag++;
            else if (dest->xs > phd_rightfloat)
                clipflag += 2;
            if (dest->ys < phd_topfloat)
                clipflag += 4;
            else if (dest->ys > phd_bottomfloat)
                clipflag += 8;
        } else {
            dest->zv = zv;
            clipflag = -128;
        }
        dest->clip = clipflag;
        totclip &= clipflag;
    }
    if (totclip)
        return NULL;
    return objptr;
}

}
