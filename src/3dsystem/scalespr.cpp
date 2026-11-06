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
