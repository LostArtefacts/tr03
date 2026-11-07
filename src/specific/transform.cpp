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


#include "../game.h"
#include "../effect2.h"

extern int CurrentRoom;
extern float f_znear;
extern float f_oneozfar;
extern int farz;
extern int distanceFogValue;
extern int wibble;
extern int water_effect;
extern float wibble_table[32];
extern int RColorTable[33][33][33];
extern int GColorTable[33][33][33];
extern int BColorTable[33][33][33];

typedef struct watertab {
    char shimmer;
    char choppy;
    unsigned char random;
    unsigned char abs;
} WATERTAB;

extern WATERTAB WaterTable[22][64];

sint32 __fastcall phd_sqrt(sint32 n);

short *calc_roomvert(short *sobjptr, int far_clip)
{
    PHD_VBUF *dest;
    long *mptr;
    int numvert;
    char clipflag;
    int n;
    float zv;
    float zv2;
    long z;
    int dist;
    unsigned long mod;
    int x1, y1, z1;
    unsigned int r, g, b;
    ROOM_INFO *this_room;
    double fconv;

    this_room = &room[CurrentRoom];
    mptr = phd_mxptr;
    float f_mid_sort;
    f_mid_sort = App.lpZBuffer ? 0.0f : (float)(mid_sort << 22);
    short *objptr;
    int nRoomVerts = (int)*(sobjptr++);
    objptr = sobjptr;
    dest = vbuf;
    MatrixToFloat(mptr);
    for (numvert = nRoomVerts; numvert > 0; numvert--, dest++, objptr += 6) {
        dest->g = objptr[5];
        dest->xv = m00 * float(objptr[0]) + m01 * float(objptr[1]) + m02 * float(objptr[2]) + m03;
        dest->yv = m10 * float(objptr[0]) + m11 * float(objptr[1]) + m12 * float(objptr[2]) + m13;
        zv = m20 * float(objptr[0]) + m21 * float(objptr[1]) + m22 * float(objptr[2]) + m23;
        dest->zv = zv;
        z = FTOL(zv);
        if (zv >= f_znear) {
            clipflag = 0;
            zv2 = f_persp / zv;
            dest->zv += f_mid_sort;
            if (z > farz << 14) {
                dest->zv = f_zfar;
                dest->ooz = zv2 * f_oneopersp;
            } else {
                dest->ooz = zv2 * f_oneopersp;
            }
            r = dest->g & 0x7c00;
            g = dest->g & 0x03e0;
            b = dest->g & 0x001f;
            for (n = 0; n < number_dynamics; n++) {
                if (dynamics[n].on) {
                    x1 = (dynamics[n].x - this_room->x - objptr[0]) >> 7;
                    y1 = (dynamics[n].y - this_room->y - objptr[1]) >> 7;
                    z1 = (dynamics[n].z - this_room->z - objptr[2]) >> 7;
                    mod = (x1 * x1) + (y1 * y1) + (z1 * z1);
                    if (mod < 1024) {
                        dist = phd_sqrt(mod);
                        int falloff;
                        falloff = dynamics[n].falloff >> 8;
                        if (dist < falloff) {
                            r += RColorTable[falloff][dist + 1][dynamics[n].r];
                            g += GColorTable[falloff][dist + 1][dynamics[n].g];
                            b += BColorTable[falloff][dist + 1][dynamics[n].b];
                        }
                    }
                }
            }
            if (r > 0x7c00)
                r = 0x7c00;
            if (g > 0x03e0)
                g = 0x03e0;
            if (b > 0x001f)
                b = 0x001f;
            dest->g = r | g | b;
            int off;
            off = ((objptr[0] + this_room->x) >> 6) + ((objptr[1] + this_room->y) >> 6) + ((objptr[2] + this_room->z) >> 7);
            off = off & 0xfc;
            if (objptr[4] & (1 << 13)) {
                dest->yv += float(WaterTable[this_room->MeshEffect][((WaterTable[this_room->MeshEffect][off & 63].random + (wibble >> 2)) & 63)].choppy << 14);
                int r, g, b;
                int shade;
                r = ((dest->g >> 10) & 0x1f);
                g = ((dest->g >> 5) & 0x1f);
                b = ((dest->g) & 0x1f);
                shade = -WaterTable[this_room->MeshEffect][((WaterTable[this_room->MeshEffect][off & 63].random + (wibble >> 2)) & 63)].choppy >> 4;
                r += shade;
                g += shade;
                b += shade;
                if (r > 0x1f)
                    r = 0x1f;
                if (g > 0x1f)
                    g = 0x1f;
                if (b > 0x1f)
                    b = 0x1f;
                if (r < 0)
                    r = 0;
                if (g < 0)
                    g = 0;
                if (b < 0)
                    b = 0;
                dest->g = r << 10 | g << 5 | b;
            }
            dest->xs = zv2 * dest->xv + f_centerx;
            dest->ys = zv2 * dest->yv + f_centery;
            if (objptr[4] & (1 << 14)) {
                int r, g, b;
                int shade;
                r = ((dest->g >> 10) & 0x1f);
                g = ((dest->g >> 5) & 0x1f);
                b = ((dest->g) & 0x1f);
                shade = WaterTable[this_room->MeshEffect][((WaterTable[this_room->MeshEffect][off & 63].random + (wibble >> 2)) & 63)].shimmer;
                shade += WaterTable[this_room->MeshEffect][((WaterTable[this_room->MeshEffect][off & 63].random + (wibble >> 2)) & 63)].abs;
                r += shade;
                g += shade;
                b += shade;
                if (r > 0x1f)
                    r = 0x1f;
                if (g > 0x1f)
                    g = 0x1f;
                if (b > 0x1f)
                    b = 0x1f;
                if (r < 0)
                    r = 0;
                if (g < 0)
                    g = 0;
                if (b < 0)
                    b = 0;
                dest->g = r << 10 | g << 5 | b;
            }
            if (water_effect && objptr[4] >= 0)
                dest->ys += wibble_table[(FTOL(dest->xs) + wibble >> 3) & 31];
            if (dest->xs < phd_leftfloat)
                clipflag++;
            else if (dest->xs > phd_rightfloat)
                clipflag += 2;
            if (dest->ys < phd_topfloat)
                clipflag += 4;
            else if (dest->ys > phd_bottomfloat)
                clipflag += 8;
            dest->clip = clipflag;
        } else
            dest->clip = -128;
        if (z > distanceFogValue << 14) {
            int r, g, b;
            int v;
            if (z > farz << 14)
                z = farz << 14;
            r = ((dest->g >> 10) & 0x1f) << 3;
            g = ((dest->g >> 5) & 0x1f) << 3;
            b = (dest->g & 0x1f) << 3;
            r += 7;
            g += 7;
            b += 7;
            v = 2048 - ((z - (distanceFogValue << 14)) >> 16);
            r *= v;
            g *= v;
            b *= v;
            r >>= 14;
            g >>= 14;
            b >>= 14;
            dest->g = r << 10 | g << 5 | b;
        }
    }
    return objptr;
}

}
