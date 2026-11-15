#include <conio.h>
#include <direct.h>
#include <io.h>
#include <signal.h>
#include "standard.h"
#include "global.h"
#include "winmain.h"

extern "C" {

extern float ZTable[40960];
extern float f_persp;
extern float f_oneopersp;



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
extern float m11, m23, m01, m00, m21, m20, m03, m12, m13, m22, m02, m10;

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

extern float outsideBackgroundTop;
extern short phd_winxmax, phd_winxmin, phd_winymin, phd_winymax;

short *calc_object_verticesback(short *objptr)
{
    PHD_VBUF *buf;
    float zv, fMidSort, zT;
    long y, z, x;
    long nVtx;
    long zi;
    char totalClip, clipFlag;
    double fconv;

    if (App.lpZBuffer)
        fMidSort = 0;
    else
        fMidSort = float(mid_sort << 22);
    outsideBackgroundTop = 0;
    totalClip = -1;
    buf = vbuf;
    objptr++;
    MatrixToFloat(phd_mxptr);
    for (nVtx = *(objptr++); nVtx > 0; nVtx--, buf++, objptr += 3) {
        x = objptr[0] << 5;
        y = objptr[1] << 5;
        z = objptr[2] << 5;
        buf->xv = (*(float volatile *)&m00) * x + m01 * y + (*(float volatile *)&m02) * z + m03;
        buf->yv = m10 * x + m11 * y + m12 * z + m13;
        zv = m20 * x + m21 * y + m22 * z + m23;
        buf->z = zi = FTOL(zv);
        if (zi >= phd_znear) {
            if (zi >= phd_zfar) {
                buf->zv = f_zfar;
                zv = f_persp / zv;
                buf->xs = buf->xv * zv + f_centerx;
                buf->ys = buf->yv * zv + f_centery;
                buf->ooz = zv * f_oneopersp;
            } else {
                buf->zv = zv + fMidSort;
                zT = ZTable[((zi >> 14) << 1)];
                buf->xs = buf->xv * zT + f_centerx;
                buf->ys = buf->yv * zT + f_centery;
                buf->ooz = ZTable[((zi >> 14) << 1) + 1];
            }
            if (buf->ys > outsideBackgroundTop && buf->xs > 0 && buf->xs < phd_winxmax + phd_winxmin && buf->ys > 0 && buf->ys < phd_winymax + phd_winymin) {
                outsideBackgroundTop = buf->ys;
                if (outsideBackgroundTop > phd_winymax + phd_winymin)
                    outsideBackgroundTop = float(phd_winymax + phd_winymin - 1);
                if (outsideBackgroundTop < 0)
                    outsideBackgroundTop = 0;
            }
            clipFlag = 0;
            if (buf->xs < phd_leftfloat)
                clipFlag++;
            else if (buf->xs > phd_rightfloat)
                clipFlag += 2;
            if (buf->ys < phd_topfloat)
                clipFlag += 4;
            else if (buf->ys > phd_bottomfloat)
                clipFlag += 8;
        } else {
            buf->zv = zv;
            clipFlag = -128;
        }
        buf->clip = clipFlag;
        totalClip &= clipFlag;
    }
    if (totalClip)
        return 0;
    return objptr;
}


}
