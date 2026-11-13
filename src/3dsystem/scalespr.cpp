#include "../specific/standard.h"

extern "C" {

#include "../3dglodef.h"

typedef struct phdspritestruct {
    unsigned short tpage;
    unsigned short offset;
    unsigned short width;
    unsigned short height;
    sint16 x1;
    sint16 y1;
    sint16 x2;
    sint16 y2;
} PHDSPRITESTRUCT;

extern PHDSPRITESTRUCT phdspriteinfo[];
extern int phd_winwidth;
extern int phd_winheight;
extern int phd_znear;
extern unsigned long TextLight[14];
extern unsigned long TextDark[14];
extern void (*InsertSprite)(int zdepth, int x1, int y1, int x2, int y2, int nSprite, int shade, int shade1, int nDrawType, int offset);

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

extern PHD_VBUF vbuf[];
extern sint32 phd_persp, phd_centerx, phd_centery;
extern sint32 phd_left, phd_right, phd_top, phd_bottom;

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
extern sint32 phd_viewdist;
extern sint32 phd_zfar;
extern int distanceFogValue;

void S_DrawScreenSprite2d(sint32 sx, sint32 sy, sint32 z, sint32 scaleH, sint32 scaleV, sint16 sprnum, sint16 shade, uint16 flags);
void S_DrawScreenSprite(sint32 sx, sint32 sy, sint32 z, sint32 scaleH, sint32 scaleV, sint16 sprnum, sint16 shade, uint16 flags);
sint16 *ins_room_sprite(sint16 *objptr, int num);
extern void (*InsertSprite)(int zdepth, int x1, int y1, int x2, int y2, int nSprite, int shade, int shade1, int nDrawType, int offset);

void S_DrawSprite(uint32 dwFlags, sint32 nX, sint32 nY, sint32 nZ, sint16 nSprite, sint16 nShade, sint16 nScale);
int phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz);

void S_DrawSprite(uint32 dwFlags, sint32 nX, sint32 nY, sint32 nZ, sint16 nSprite, sint16 nShade, sint16 nScale)
{
    sint32 xv, yv, zv, zp;
    sint32 x1, y1, x2, y2;
    PHDSPRITESTRUCT *pSpriteInfo;
    int r, g, b;

    if (dwFlags & 0x1000000) {
        nX -= w2v_matrix[M03];
        nY -= w2v_matrix[M13];
        nZ -= w2v_matrix[M23];
        if (nX < -phd_viewdist || nX > phd_viewdist || nY < -phd_viewdist || nY > phd_viewdist || nZ < -phd_viewdist || nZ > phd_viewdist)
            return;
        zv = w2v_matrix[M20] * nX + w2v_matrix[M21] * nY + w2v_matrix[M22] * nZ;
        if (zv < phd_znear || zv >= phd_zfar)
            return;
        xv = w2v_matrix[M00] * nX + w2v_matrix[M01] * nY + w2v_matrix[M02] * nZ;
        yv = w2v_matrix[M10] * nX + w2v_matrix[M11] * nY + w2v_matrix[M12] * nZ;
    } else {
        if ((nX | nY | nZ) == 0) {
            zv = phd_mxptr[M23];
            if (zv < phd_znear || zv > phd_zfar)
                return;
            xv = phd_mxptr[M03];
            yv = phd_mxptr[M13];
        } else {
            zv = phd_mxptr[M20] * nX + phd_mxptr[M21] * nY + phd_mxptr[M22] * nZ + phd_mxptr[M23];
            if (zv < phd_znear || zv > phd_zfar)
                return;
            xv = phd_mxptr[M00] * nX + phd_mxptr[M01] * nY + phd_mxptr[M02] * nZ + phd_mxptr[M03];
            yv = phd_mxptr[M10] * nX + phd_mxptr[M11] * nY + phd_mxptr[M12] * nZ + phd_mxptr[M13];
        }
    }
    zp = zv / phd_persp;
    pSpriteInfo = phdspriteinfo + nSprite;
    x1 = pSpriteInfo->x1;
    y1 = pSpriteInfo->y1;
    x2 = pSpriteInfo->x2;
    y2 = pSpriteInfo->y2;
    if (dwFlags & 0x4000000) {
        x1 = (x1 * nScale) << (W2V_SHIFT - 8);
        y1 = (y1 * nScale) << (W2V_SHIFT - 8);
        x2 = (x2 * nScale) << (W2V_SHIFT - 8);
        y2 = (y2 * nScale) << (W2V_SHIFT - 8);
    } else {
        x1 <<= W2V_SHIFT;
        y1 <<= W2V_SHIFT;
        x2 <<= W2V_SHIFT;
        y2 <<= W2V_SHIFT;
    }
    x1 = (xv + x1) / zp + phd_centerx;
    if (x1 >= phd_winwidth)
        return;
    y1 = (yv + y1) / zp + phd_centery;
    if (y1 >= phd_winheight)
        return;
    x2 = (xv + x2) / zp + phd_centerx;
    if (x2 < 0)
        return;
    y2 = (yv + y2) / zp + phd_centery;
    if (y2 < 0)
        return;
    if (dwFlags & 0x8000000) {
        if (zv > distanceFogValue << W2V_SHIFT) {
            nShade += (zv >> W2V_SHIFT) - distanceFogValue;
            if (nShade > 0x1fff)
                return;
        }
    } else
        nShade = 0x1000;
    if (dwFlags & 0xffffff) {
        nShade = dwFlags & 0xffffff;
        r = (nShade & 0xff) >> 3;
        g = ((nShade >> 8) & 0xff) >> 3;
        b = ((nShade >> 16) & 0xff) >> 3;
        nShade = r << 10 | g << 5 | b;
        InsertSprite(zv, x1, y1, x2, y2, nSprite, nShade, -1, 14, 0);
    } else
        InsertSprite(zv, x1, y1, x2, y2, nSprite, nShade, -1, 10, 0);
}

sint16 *ins_room_sprite(sint16 *objptr, int num)
{
    PHD_VBUF *vn;
    sint32 x1, y1, x2, y2, zv;
    PHDSPRITESTRUCT *sptr;

    for (; num > 0; num--, objptr += 2) {
        vn = &vbuf[*objptr];
        if (vn->clip < 0)
            continue;
        zv = (sint32)vn->zv / phd_persp;
        sptr = &phdspriteinfo[objptr[1]];
        x1 = (sint32)((vn->xv + (sptr->x1 << W2V_SHIFT)) / zv + phd_centerx);
        y1 = (sint32)((vn->yv + (sptr->y1 << W2V_SHIFT)) / zv + phd_centery);
        x2 = (sint32)((vn->xv + (sptr->x2 << W2V_SHIFT)) / zv + phd_centerx);
        y2 = (sint32)((vn->yv + (sptr->y2 << W2V_SHIFT)) / zv + phd_centery);
        if (x2 < phd_left || y2 < phd_top || x1 >= phd_right || y1 >= phd_bottom)
            continue;
        InsertSprite((int)vn->zv, x1, y1, x2, y2, objptr[1], vn->g, -1, 10, 0);
    }
    return objptr;
}

void S_DrawScreenSprite2d(sint32 sx, sint32 sy, sint32 z, sint32 scaleH, sint32 scaleV, sint16 sprnum, sint16 shade, uint16 flags)
{
    PHDSPRITESTRUCT *sptr;
    sint32 x1;
    sint32 y1;
    sint32 x2;
    sint32 y2;
    int r;
    int g;
    int b;
    int nShade1;
    int nShade2;

    sptr = &phdspriteinfo[sprnum];
    x1 = ((sptr->x1 * scaleH) >> 16) + sx;
    y1 = ((sptr->y1 * scaleV) >> 16) + sy;
    x2 = ((sptr->x2 * scaleH) >> 16) + sx;
    y2 = ((sptr->y2 * scaleV) >> 16) + sy;
    if (x2 < 0 || y2 < 0 || x1 >= phd_winwidth || y1 >= phd_winheight)
        return;
    b = (TextLight[shade] >> 19) & 31;
    g = (TextLight[shade] >> 11) & 31;
    r = (TextLight[shade] >> 3) & 31;
    nShade1 = r << 10 | g << 5 | b;
    b = (TextDark[shade] >> 19) & 31;
    g = (TextDark[shade] >> 11) & 31;
    r = (TextDark[shade] >> 3) & 31;
    nShade2 = r << 10 | g << 5 | b;
    if (flags != 0xffff) {
        InsertSprite(phd_znear + 0x28000, x1, y1, x2, y2, sprnum, nShade1, nShade2, 10, 0);
        InsertSprite(phd_znear + 0x3c000, x1 + 1, y1 + 1, x2 + 1, y2 + 1, sprnum, 0, 0, 10, 0);
    } else {
        InsertSprite(phd_znear, x1, y1, x2, y2, sprnum, nShade1, nShade2, 10, 0);
    }
}

void S_DrawScreenSprite(sint32 sx, sint32 sy, sint32 z, sint32 scaleH, sint32 scaleV, sint16 sprnum, sint16 shade, uint16 flags)
{
    PHDSPRITESTRUCT *sptr;
    sint32 x1;
    sint32 y1;
    sint32 x2;
    sint32 y2;

    sptr = &phdspriteinfo[sprnum];
    x1 = (((sptr->x1 >> 3) * scaleH) >> 16) + sx;
    x2 = (((sptr->x2 >> 3) * scaleH) >> 16) + sx;
    y1 = (((sptr->y1 >> 3) * scaleV) >> 16) + sy;
    y2 = (((sptr->y2 >> 3) * scaleV) >> 16) + sy;
    if (x2 < 0 || y2 < 0 || x1 >= phd_winwidth || y1 >= phd_winheight)
        return;
    InsertSprite(z << 3, x1, y1, x2, y2, sprnum, shade, -1, 10, 0);
}

}

#ifdef TR3_EXE
extern "C" {

unsigned long TextLight[14] = {
    0xFFFFFF, 0xB0B0, 0xA0A0A0, 0x6060FF,
    0xFF8080, 0x4080C0, 0x64D1B6, 0xC0FFC0,
    0xFFFFFF, 0xFF00FF, 0xFF00FF, 0xFF00FF,
    0xFF00FF, 0,
};
unsigned long TextDark[14] = {
    0x808080, 0x5050, 0x181818, 0x18,
    0x180000, 0x1040, 0x1320B6, 0xC0FFC0,
    0xFFFFFF, 0x3F003F, 0x3F003F, 0x3F003F,
    0x3F003F, 0,
};

}
#endif
