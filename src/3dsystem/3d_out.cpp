// cflags: /Op /Oa
#include "../specific/standard.h"
#include <io.h>
#include <direct.h>
#include <commctrl.h>
#include <mmreg.h>

extern "C" {

#include "../3dglodef.h"

extern sint32 xgen_ymin;
extern sint32 xgen_ymax;
extern char *texture_page_ptrs[32];
extern sint32 xbuffer[];

typedef struct x_info {
    sint16 xs;
    sint16 ys;
} X_INFO;

typedef struct xbuf_x {
    sint32 Xleft;
    sint32 Xright;
} XBUF_X;

typedef struct xg_info {
    sint16 xs;
    sint16 ys;
    sint16 shade;
} XG_INFO;

typedef struct xbuf_xg {
    sint32 Xleft;
    sint32 Gleft;
    sint32 Xright;
    sint32 Gright;
} XBUF_XG;

typedef struct uvg_info {
    sint16 xs;
    sint16 ys;
    sint16 shade;
    uint16 u;
    uint16 v;
} UVG_INFO;

typedef struct xbuf_xguv {
    sint32 Xleft;
    sint32 Gleft;
    sint32 Uleft;
    sint32 Vleft;
    sint32 Xright;
    sint32 Gright;
    sint32 Uright;
    sint32 Vright;
} XBUF_XGUV;

typedef struct uvgp_info {
    sint16 xs;
    sint16 ys;
    sint16 shade;
    float ooz;
    float uoz;
    float voz;
} UVGP_INFO;

typedef struct xbuf_persp_fp {
    sint32 Xleft;
    sint32 Gleft;
    float UOZleft;
    float VOZleft;
    float OOZleft;
    sint32 Xright;
    sint32 Gright;
    float UOZright;
    float VOZright;
    float OOZright;
} XBUF_PERSP_FP;

int xgen_x(sint16 *iptr);
int xgen_xg(sint16 *iptr);
int xgen_xguv(sint16 *iptr);
int xgen_xguvpersp_fp(sint16 *iptr);
void gtmap_persp32_fp(int ymin, int ymax, unsigned char *tptr);
void wgtmap_persp16_fp(int ymin, int ymax, char *tptr);
#ifndef FEAT_C_ASM
void flatA(void);
void transA(void);
void gourA(void);
void gtmapA(void);
void wgtmapA(void);
#else
void flatA(int ymin, int ymax, int colour);
void transA(int ymin, int ymax, int colour);
void gourA(int ymin, int ymax, int colour);
void gtmapA(int ymin, int ymax, char *texture);
void wgtmapA(int ymin, int ymax, char *texture);
#endif

#if FEAT_C_ASM
extern char depthq_table[33][256];
extern char gouraud_table[256][32];
extern sint32 phd_scrwidth;
extern char *phd_winptr;
extern int phd_winwidth;

void flatA(int ymin, int ymax, int colour)
{
    sint16 *xptr;
    char *line;
    int x, w;

    if (ymax - ymin <= 0)
        return;
    xptr = (sint16 *)(xbuffer + ymin * 2) + 1;
    line = phd_winptr + ymin * phd_scrwidth;
    for (ymax -= ymin; ymax > 0; ymax--, xptr += 4, line += phd_scrwidth) {
        x = xptr[0];
        w = xptr[2] - x;
        for (; w > 0; w--, x++)
            line[x] = (char)colour;
    }
}

void transA(int ymin, int ymax, int colour)
{
    sint16 *xptr;
    unsigned char *line, *pix, *table;
    int x, xr, w;

    if (ymax - ymin <= 0 || (unsigned int)colour > 32)
        return;
    table = (unsigned char *)depthq_table[colour];
    xptr = (sint16 *)(xbuffer + ymin * 2) + 1;
    line = (unsigned char *)phd_winptr + ymin * phd_scrwidth;
    for (ymax -= ymin; ymax > 0; ymax--, xptr += 4, line += phd_scrwidth) {
        x = xptr[0];
        if (x < 0)
            x = 0;
        xr = xptr[2];
        if (xr > phd_winwidth)
            xr = phd_winwidth;
        pix = line + x;
        for (w = xr - x; w > 0; w--, pix++)
            *pix = table[*pix];
    }
}

void gourA(int ymin, int ymax, int colour)
{
    sint32 *xptr;
    char *line, *pix, *table;
    int x, xr, w, g, dg;
    sint32 hi, dhi;
    uint32 lo, dlo, sum;

    if (ymax - ymin <= 0)
        return;
    table = gouraud_table[0] + ((colour & 0xffff) << 5);
    xptr = xbuffer + ymin * 4;
    line = phd_winptr + ymin * phd_scrwidth;
    for (ymax -= ymin; ymax > 0; ymax--, xptr += 4, line += phd_scrwidth) {
        x = (sint16)(xptr[0] >> 16);
        xr = (sint16)(xptr[2] >> 16);
        if (x >= xr)
            continue;
        g = xptr[1];
        dg = (xptr[3] - g) / (xr - x);
        if (x < 0) {
            g -= x * dg;
            x = 0;
        }
        pix = line + x;
        hi = g >> 16;
        lo = (uint32)g << 16;
        dhi = dg >> 16;
        dlo = (uint32)dg << 16;
        w = xr - x;
        if (w <= 0) {
            if ((uint32)pix & 1)
                *pix = table[hi];
            continue;
        }
        for (; w > 0; w--) {
            *pix++ = table[hi];
            sum = lo + dlo;
            hi += dhi + (sum < lo);
            lo = sum;
        }
    }
}

void gtmapA(int ymin, int ymax, char *texture)
{
    sint32 *xptr;
    unsigned char *line, *pix, *tex, *table;
    int x, xr, w;
    sint32 g, u, v, dg, du, dv;

    if (ymax - ymin <= 0)
        return;
    tex = (unsigned char *)texture;
    table = (unsigned char *)depthq_table;
    xptr = xbuffer + ymin * 8;
    line = (unsigned char *)phd_winptr + ymin * phd_scrwidth;
    for (ymax -= ymin; ymax > 0; ymax--, xptr += 8, line += phd_scrwidth) {
        x = xptr[0] >> 16;
        xr = xptr[4] >> 16;
        if (xr <= x)
            continue;
        w = xr - x;
        g = xptr[1];
        u = xptr[2];
        v = xptr[3];
        dg = (xptr[5] - g) / w;
        du = (xptr[6] - u) / w;
        dv = (xptr[7] - v) / w;
        for (pix = line + x; w > 0; w--) {
            *pix++ = table[((g >> 8) & 0xff00) | tex[((v >> 8) & 0xff00) | ((u >> 16) & 0xff)]];
            u += du;
            v += dv;
            g += dg;
        }
    }
}

void wgtmapA(int ymin, int ymax, char *texture)
{
    sint32 *xptr;
    unsigned char *line, *pix, *tex, *table;
    int x, xr, w;
    unsigned int texel;
    sint32 g, u, v, dg, du, dv;

    if (ymax - ymin <= 0)
        return;
    tex = (unsigned char *)texture;
    table = (unsigned char *)depthq_table;
    xptr = xbuffer + ymin * 8;
    line = (unsigned char *)phd_winptr + ymin * phd_scrwidth;
    for (ymax -= ymin; ymax > 0; ymax--, xptr += 8, line += phd_scrwidth) {
        x = xptr[0] >> 16;
        xr = xptr[4] >> 16;
        if (xr <= x)
            continue;
        w = xr - x;
        g = xptr[1];
        u = xptr[2];
        v = xptr[3];
        dg = (xptr[5] - g) / w;
        du = (xptr[6] - u) / w;
        dv = (xptr[7] - v) / w;
        for (pix = line + x; w > 0; w--, pix++) {
            texel = tex[((v >> 8) & 0xff00) | ((u >> 16) & 0xff)];
            if (texel)
                *pix = table[((g >> 8) & 0xff00) | texel];
            u += du;
            v += dv;
            g += dg;
        }
    }
}
#endif

void draw_poly_flat(sint16 *iptr)
{
    int colour;

    if (xgen_x(iptr + 1)) {
        colour = *iptr;
#ifndef FEAT_C_ASM
        __asm {
            mov eax, xgen_ymin
            mov edx, xgen_ymax
            mov ebx, colour
            call flatA
        }
#else
        flatA(xgen_ymin, xgen_ymax, colour);
#endif
    }
}

void draw_poly_trans(sint16 *iptr)
{
    int colour;

    if (xgen_x(iptr + 1)) {
        colour = *iptr;
#ifndef FEAT_C_ASM
        __asm {
            mov eax, xgen_ymin
            mov edx, xgen_ymax
            mov ebx, colour
            call transA
        }
#else
        transA(xgen_ymin, xgen_ymax, colour);
#endif
    }
}

void draw_poly_gouraud(sint16 *iptr)
{
    int colour;

    if (xgen_xg(iptr + 1)) {
        colour = *iptr;
#ifndef FEAT_C_ASM
        __asm {
            mov eax, xgen_ymin
            mov edx, xgen_ymax
            mov ebx, colour
            call gourA
        }
#else
        gourA(xgen_ymin, xgen_ymax, colour);
#endif
    }
}

void draw_poly_gtmap(sint16 *iptr)
{
    char *texture;

    if (xgen_xguv(iptr + 1)) {
        texture = texture_page_ptrs[*iptr];
#ifndef FEAT_C_ASM
        __asm {
            mov eax, xgen_ymin
            mov edx, xgen_ymax
            mov ebx, texture
            call gtmapA
        }
#else
        gtmapA(xgen_ymin, xgen_ymax, texture);
#endif
    }
}

void draw_poly_wgtmap(sint16 *iptr)
{
    char *texture;

    if (xgen_xguv(iptr + 1)) {
        texture = texture_page_ptrs[*iptr];
#ifndef FEAT_C_ASM
        __asm {
            mov eax, xgen_ymin
            mov edx, xgen_ymax
            mov ebx, texture
            call wgtmapA
        }
#else
        wgtmapA(xgen_ymin, xgen_ymax, texture);
#endif
    }
}

int xgen_x(sint16 *iptr)
{
    X_INFO *ptr1, *ptr2;
    XBUF_X *xptr;
    int x, x_add;
    int y1, y2, ydif;
    int min_y, max_y;
    int numcoords;

    numcoords = *iptr++;
    ptr1 = (X_INFO *)iptr;
    ptr2 = ptr1;
    ptr1 += numcoords - 1;
    min_y = max_y = ptr1->ys;
    for (; numcoords > 0; numcoords--, ptr1 = ptr2, ptr2++) {
        y1 = ptr1->ys;
        y2 = ptr2->ys;
        if (y1 < y2) {
            if (y1 < min_y)
                min_y = y1;
            ydif = y2 - y1;
            x = ptr1->xs;
            x_add = ((ptr2->xs - x) << 16) / ydif;
            x = (x << 16) + 0xffff;
            xptr = (XBUF_X *)xbuffer + y1;
            for (; y1 < y2; y1++, xptr++) {
                x += x_add;
                xptr->Xright = x;
            }
        } else if (y2 < y1) {
            if (y1 > max_y)
                max_y = y1;
            ydif = y1 - y2;
            x = ptr2->xs;
            x_add = ((ptr1->xs - x) << 16) / ydif;
            x = (x << 16) + 1;
            xptr = (XBUF_X *)xbuffer + y2;
            for (; y2 < y1; y2++, xptr++) {
                x += x_add;
                xptr->Xleft = x;
            }
        }
    }
    if (min_y == max_y)
        return 0;
    xgen_ymin = min_y;
    xgen_ymax = max_y;
    return 1;
}

int xgen_xg(sint16 *iptr)
{
    XG_INFO *ptr1, *ptr2;
    XBUF_XG *xptr;
    int g, g_add;
    int x, x_add;
    int y1, y2, ydif;
    int min_y, max_y;
    int numcoords;

    numcoords = *iptr++;
    ptr1 = (XG_INFO *)iptr;
    ptr2 = ptr1;
    ptr1 += numcoords - 1;
    min_y = max_y = ptr1->ys;
    for (; numcoords > 0; numcoords--, ptr1 = ptr2, ptr2++) {
        y1 = ptr1->ys;
        y2 = ptr2->ys;
        if (y1 < y2) {
            if (y1 < min_y)
                min_y = y1;
            ydif = y2 - y1;
            x = ptr1->xs;
            x_add = ((ptr2->xs - x) << 16) / ydif;
            x = (x << 16) + 0xffff;
            g = ptr1->shade << 8;
            g_add = ((ptr2->shade << 8) - g) / ydif;
            xptr = (XBUF_XG *)xbuffer + y1;
            for (; y1 < y2; y1++, xptr++) {
                x += x_add;
                g += g_add;
                xptr->Xright = x;
                xptr->Gright = g;
            }
        } else if (y2 < y1) {
            if (y1 > max_y)
                max_y = y1;
            ydif = y1 - y2;
            x = ptr2->xs;
            x_add = ((ptr1->xs - x) << 16) / ydif;
            x = (x << 16) + 1;
            g = ptr2->shade << 8;
            g_add = ((ptr1->shade << 8) - g) / ydif;
            xptr = (XBUF_XG *)xbuffer + y2;
            for (; y2 < y1; y2++, xptr++) {
                x += x_add;
                g += g_add;
                xptr->Xleft = x;
                xptr->Gleft = g;
            }
        }
    }
    if (min_y == max_y)
        return 0;
    xgen_ymin = min_y;
    xgen_ymax = max_y;
    return 1;
}

int xgen_xguv(sint16 *iptr)
{
    UVG_INFO *ptr1, *ptr2;
    XBUF_XGUV *xptr;
    int g, g_add;
    int x, x_add;
    int u, u_add;
    int v, v_add;
    int y1, y2, ydif;
    int min_y, max_y;
    int numcoords;

    numcoords = *iptr++;
    ptr1 = (UVG_INFO *)iptr;
    ptr2 = ptr1;
    ptr1 += numcoords - 1;
    min_y = max_y = ptr1->ys;
    for (; numcoords > 0; numcoords--, ptr1 = ptr2, ptr2++) {
        y1 = ptr1->ys;
        y2 = ptr2->ys;
        if (y1 < y2) {
            if (y1 < min_y)
                min_y = y1;
            ydif = y2 - y1;
            x = ptr1->xs;
            x_add = ((ptr2->xs - x) << 16) / ydif;
            x = (x << 16) + 0xffff;
            g = ptr1->shade << 8;
            g_add = ((ptr2->shade << 8) - g) / ydif;
            u = ptr1->u << 8;
            u_add = ((ptr2->u << 8) - u) / ydif;
            v = ptr1->v << 8;
            v_add = ((ptr2->v << 8) - v) / ydif;
            xptr = (XBUF_XGUV *)xbuffer + y1;
            for (; y1 < y2; y1++, xptr++) {
                x += x_add;
                g += g_add;
                u += u_add;
                v += v_add;
                xptr->Xright = x;
                xptr->Gright = g;
                xptr->Uright = u;
                xptr->Vright = v;
            }
        } else if (y2 < y1) {
            if (y1 > max_y)
                max_y = y1;
            ydif = y1 - y2;
            x = ptr2->xs;
            x_add = ((ptr1->xs - x) << 16) / ydif;
            x = (x << 16) + 1;
            g = ptr2->shade << 8;
            g_add = ((ptr1->shade << 8) - g) / ydif;
            u = ptr2->u << 8;
            u_add = ((ptr1->u << 8) - u) / ydif;
            v = ptr2->v << 8;
            v_add = ((ptr1->v << 8) - v) / ydif;
            xptr = (XBUF_XGUV *)xbuffer + y2;
            for (; y2 < y1; y2++, xptr++) {
                x += x_add;
                g += g_add;
                u += u_add;
                v += v_add;
                xptr->Xleft = x;
                xptr->Gleft = g;
                xptr->Uleft = u;
                xptr->Vleft = v;
            }
        }
    }
    if (min_y == max_y)
        return 0;
    xgen_ymin = min_y;
    xgen_ymax = max_y;
    return 1;
}


int xgen_xguvpersp_fp(sint16 *iptr)
{
    UVGP_INFO *ptr1, *ptr2;
    XBUF_PERSP_FP *xptr;
    int g, g_add;
    int x, x_add;
    float scale;
    float uoz, uoz_add;
    float voz, voz_add;
    float ooz, ooz_add;
    int y1, y2, ydif;
    int min_y, max_y;
    int numcoords;

    numcoords = (int)*(iptr++);
    ptr2 = (UVGP_INFO *)iptr;
    ptr1 = ptr2 + numcoords - 1;
    min_y = max_y = ptr1->ys;
    for (; numcoords > 0; numcoords--, ptr1 = ptr2, ptr2++) {
        y1 = ptr1->ys;
        y2 = ptr2->ys;
        if (y1 < y2) {
            if (y1 < min_y)
                min_y = y1;
            ydif = y2 - y1;
            x = ptr1->xs;
            x_add = ((ptr2->xs - x) << 16) / ydif;
            x = (x << 16) + 0x0ffff;
            g = ptr1->shade << 8;
            scale = 1.0f / ydif;
            g_add = ((ptr2->shade << 8) - g) / ydif;
            uoz = ptr1->uoz;
            uoz_add = (ptr2->uoz - uoz) * scale;
            voz = ptr1->voz;
            voz_add = (ptr2->voz - voz) * scale;
            ooz = ptr1->ooz;
            ooz_add = (ptr2->ooz - ooz) * scale;
            xptr = ((XBUF_PERSP_FP *)xbuffer) + y1;
            for (; y1 < y2; y1++, xptr++) {
                x += x_add;
                g += g_add;
                uoz += uoz_add;
                voz += voz_add;
                ooz += ooz_add;
                xptr->Xright = x;
                xptr->Gright = g;
                xptr->UOZright = uoz;
                xptr->VOZright = voz;
                xptr->OOZright = ooz;
            }
        } else if (y2 < y1) {
            if (y1 > max_y)
                max_y = y1;
            ydif = y1 - y2;
            x = ptr2->xs;
            x_add = ((ptr1->xs - x) << 16) / ydif;
            x = (x << 16) + 0x0001;
            g = ptr2->shade << 8;
            scale = 1.0f / ydif;
            g_add = ((ptr1->shade << 8) - g) / ydif;
            uoz = ptr2->uoz;
            uoz_add = (ptr1->uoz - uoz) * scale;
            voz = ptr2->voz;
            voz_add = (ptr1->voz - voz) * scale;
            ooz = ptr2->ooz;
            ooz_add = (ptr1->ooz - ooz) * scale;
            xptr = ((XBUF_PERSP_FP *)xbuffer) + y2;
            for (; y2 < y1; y2++, xptr++) {
                x += x_add;
                g += g_add;
                uoz += uoz_add;
                voz += voz_add;
                ooz += ooz_add;
                xptr->Xleft = x;
                xptr->Gleft = g;
                xptr->UOZleft = uoz;
                xptr->VOZleft = voz;
                xptr->OOZleft = ooz;
            }
        }
    }
    if (min_y == max_y)
        return (0);
    xgen_ymin = min_y;
    xgen_ymax = max_y;
    return (1);
}

#define SINGLE_MASK 0xffff8000

extern char depthq_table[33][256];
extern sint32 phd_scrwidth;
extern char *phd_winptr;

void gtmap_persp32_fp(int ymin, int ymax, unsigned char *tptr)
{
    XBUF_PERSP_FP *dataptr;
    unsigned char *lineptr, *pixptr;
    float ooz, uoz, voz, scale;
    float oozadd32, uozadd32, vozadd32;
    int start_g, add_g, add_g2;
    int end_u, end_v;
    int start_u, start_v;
    int add_u, add_v;
    int start_x, end_x;
    int x;

    dataptr = ((XBUF_PERSP_FP *)xbuffer) + ymin;
    lineptr = (unsigned char *)phd_winptr + (ymin * phd_scrwidth);
    for (ymax -= ymin; ymax > 0; ymax--, dataptr++, lineptr += phd_scrwidth) {
        start_x = dataptr->Xleft >> 16;
        end_x = dataptr->Xright >> 16;
        x = end_x - start_x;
        if (x <= 0)
            continue;
        if (x > 32) {
            start_g = dataptr->Gleft;
            add_g = (dataptr->Gright - start_g) / x;
            add_g2 = add_g << 1;
            scale = 32.0f / x;
            uoz = dataptr->UOZleft;
            uozadd32 = (dataptr->UOZright - uoz) * scale;
            voz = dataptr->VOZleft;
            vozadd32 = (dataptr->VOZright - voz) * scale;
            ooz = dataptr->OOZleft;
            oozadd32 = (dataptr->OOZright - ooz) * scale;
            scale = 256.0f / ooz;
            end_u = (int)(uoz * scale);
            end_v = (int)(voz * scale);
            pixptr = lineptr + start_x;
            ooz += oozadd32;
            scale = 256.0f / ooz;
            for (; x >= 32; x -= 32) {
                start_u = end_u;
                uoz += uozadd32;
                end_u = (int)(uoz * scale);
                add_u = (end_u - start_u) >> 5;
                start_v = end_v;
                voz += vozadd32;
                end_v = (int)(voz * scale);
                add_v = (end_v - start_v) >> 5;
                ooz += oozadd32;
                scale = 256.0f / ooz;
                if ((ABS(add_u) + ABS(add_v)) & SINGLE_MASK) {
                    *(pixptr) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 1) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 2) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 3) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 4) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 5) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 6) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 7) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 8) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 9) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 10) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 11) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 12) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 13) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 14) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 15) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_g += add_g;
                    start_u += add_u;
                    start_v += add_v;
                    *(pixptr + 16) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 17) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 18) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 19) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 20) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 21) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 22) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 23) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 24) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 25) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 26) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 27) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 28) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 29) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 30) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    *(pixptr + 31) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                } else {
                    add_u <<= 1;
                    add_v <<= 1;
                    *(pixptr + 0) = *(pixptr + 1) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 2) = *(pixptr + 3) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 4) = *(pixptr + 5) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 6) = *(pixptr + 7) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 8) = *(pixptr + 9) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 10) = *(pixptr + 11) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 12) = *(pixptr + 13) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 14) = *(pixptr + 15) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 16) = *(pixptr + 17) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 18) = *(pixptr + 19) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 20) = *(pixptr + 21) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 22) = *(pixptr + 23) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 24) = *(pixptr + 25) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 26) = *(pixptr + 27) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 28) = *(pixptr + 29) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    *(pixptr + 30) = *(pixptr + 31) = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                }
                pixptr += 32;
            }
            if (!x)
                continue;
            scale = 256.0f / dataptr->OOZright;
            start_u = end_u;
            end_u = (int)(dataptr->UOZright * scale);
            add_u = (end_u - start_u) / x;
            start_v = end_v;
            end_v = (int)(dataptr->VOZright * scale);
            add_v = (end_v - start_v) / x;
        } else {
            start_g = dataptr->Gleft;
            add_g = (dataptr->Gright - start_g) / x;
            scale = 256.0f / dataptr->OOZleft;
            start_u = (int)(dataptr->UOZleft * scale);
            start_v = (int)(dataptr->VOZleft * scale);
            scale = 256.0f / dataptr->OOZright;
            end_u = (int)(dataptr->UOZright * scale);
            end_v = (int)(dataptr->VOZright * scale);
            add_u = (end_u - start_u) / x;
            add_v = (end_v - start_v) / x;
            pixptr = lineptr + start_x;
        }
        if ((ABS(add_u) + ABS(add_v)) & SINGLE_MASK) {
            for (; x > 1; x -= 2, pixptr += 2) {
                pixptr[0] = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                start_u += add_u;
                start_v += add_v;
                start_g += add_g;
                pixptr[1] = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                start_u += add_u;
                start_v += add_v;
                start_g += add_g;
            }
        } else {
            add_u <<= 1;
            add_v <<= 1;
            add_g2 = add_g << 1;
            for (; x > 1; x -= 2, pixptr += 2) {
                pixptr[0] = pixptr[1] = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
                start_u += add_u;
                start_v += add_v;
                start_g += add_g2;
            }
        }
        if (x)
            pixptr[0] = depthq_table[start_g >> 16][*(tptr + ((start_v >> 16) << 8) + (start_u >> 16))];
    }
}


void wgtmap_persp16_fp(int ymin, int ymax, char *tptr)
{
    XBUF_PERSP_FP *dataptr;
    unsigned char *lineptr, *pixptr, texel;
    float ooz, uoz, voz, scale;
    float oozadd32, uozadd32, vozadd32;
    int start_g, add_g, add_g2;
    int end_u, end_v;
    int start_u, start_v;
    int add_u, add_v;
    int start_x, end_x;
    int x;

    dataptr = ((XBUF_PERSP_FP *)xbuffer) + ymin;
    lineptr = (unsigned char *)phd_winptr + (ymin * phd_scrwidth);
    for (ymax -= ymin; ymax > 0; ymax--, dataptr++, lineptr += phd_scrwidth) {
        start_x = dataptr->Xleft >> 16;
        end_x = dataptr->Xright >> 16;
        x = end_x - start_x;
        if (x > 32) {
            start_g = dataptr->Gleft;
            add_g = (dataptr->Gright - start_g) / x;
            add_g2 = add_g << 1;
            scale = 32.0f / x;
            uoz = dataptr->UOZleft;
            uozadd32 = (dataptr->UOZright - uoz) * scale;
            voz = dataptr->VOZleft;
            vozadd32 = (dataptr->VOZright - voz) * scale;
            ooz = dataptr->OOZleft;
            oozadd32 = (dataptr->OOZright - ooz) * scale;
            scale = 256.0f / ooz;
            end_u = (int)(uoz * scale);
            end_v = (int)(voz * scale);
            pixptr = lineptr + start_x;
            ooz += oozadd32;
            scale = 256.0f / ooz;
            for (; x >= 32; x -= 32) {
                start_u = end_u;
                uoz += uozadd32;
                end_u = (int)(uoz * scale);
                add_u = (end_u - start_u) >> 5;
                start_v = end_v;
                voz += vozadd32;
                end_v = (int)(voz * scale);
                add_v = (end_v - start_v) >> 5;
                if ((ABS(add_u) + ABS(add_v)) & SINGLE_MASK) {
                    ooz += oozadd32;
                    scale = 256.0f / ooz;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 1) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 2) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 3) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 4) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 5) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 6) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 7) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 8) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 9) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 10) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 11) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 12) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 13) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 14) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 15) = depthq_table[(start_g >> 16)][texel];
                    start_g += add_g;
                    start_u += add_u;
                    start_v += add_v;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 16) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 17) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 18) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 19) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 20) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 21) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 22) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 23) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 24) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 25) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 26) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 27) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 28) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 29) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 30) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 31) = depthq_table[(start_g >> 16)][texel];
                    start_g += add_g;
                } else {
                    ooz += oozadd32;
                    scale = 256.0f / ooz;
                    add_u <<= 1;
                    add_v <<= 1;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr) = *(pixptr + 1) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 2) = *(pixptr + 3) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 4) = *(pixptr + 5) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 6) = *(pixptr + 7) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 8) = *(pixptr + 9) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 10) = *(pixptr + 11) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 12) = *(pixptr + 13) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 14) = *(pixptr + 15) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 16) = *(pixptr + 17) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 18) = *(pixptr + 19) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 20) = *(pixptr + 21) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 22) = *(pixptr + 23) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 24) = *(pixptr + 25) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 26) = *(pixptr + 27) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 28) = *(pixptr + 29) = depthq_table[(start_g >> 16)][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        *(pixptr + 30) = *(pixptr + 31) = depthq_table[(start_g >> 16)][texel];
                    start_g += add_g2;
                }
                pixptr += 32;
            }
            if (x) {
                scale = 256.0f / dataptr->OOZright;
                start_u = end_u;
                end_u = (int)(dataptr->UOZright * scale);
                add_u = (end_u - start_u) / x;
                start_v = end_v;
                end_v = (int)(dataptr->VOZright * scale);
                add_v = (end_v - start_v) / x;
                if ((ABS(add_u) + ABS(add_v)) & SINGLE_MASK) {
                    for (; x > 1; x -= 2, pixptr += 2) {
                        if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                            pixptr[0] = depthq_table[start_g >> 16][texel];
                        start_u += add_u;
                        start_v += add_v;
                        start_g += add_g;
                        if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                            pixptr[1] = depthq_table[start_g >> 16][texel];
                        start_u += add_u;
                        start_v += add_v;
                        start_g += add_g;
                    }
                } else {
                    add_u <<= 1;
                    add_v <<= 1;
                    for (; x > 1; x -= 2, pixptr += 2) {
                        if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                            pixptr[0] = pixptr[1] = depthq_table[start_g >> 16][texel];
                        start_u += add_u;
                        start_v += add_v;
                        start_g += add_g2;
                    }
                }
                if (x && (texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                    pixptr[0] = depthq_table[start_g >> 16][texel];
            }
        } else if (x > 0) {
            start_g = dataptr->Gleft;
            add_g = (dataptr->Gright - start_g) / x;
            scale = 256.0f / dataptr->OOZleft;
            start_u = (int)(dataptr->UOZleft * scale);
            start_v = (int)(dataptr->VOZleft * scale);
            scale = 256.0f / dataptr->OOZright;
            end_u = (int)(dataptr->UOZright * scale);
            end_v = (int)(dataptr->VOZright * scale);
            add_u = (end_u - start_u) / x;
            add_v = (end_v - start_v) / x;
            pixptr = lineptr + start_x;
            if ((ABS(add_u) + ABS(add_v)) & SINGLE_MASK) {
                for (; x > 1; x -= 2, pixptr += 2) {
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        pixptr[0] = depthq_table[start_g >> 16][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        pixptr[1] = depthq_table[start_g >> 16][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g;
                }
            } else {
                add_u <<= 1;
                add_v <<= 1;
                add_g2 = add_g << 1;
                for (; x > 1; x -= 2, pixptr += 2) {
                    if ((texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                        pixptr[0] = pixptr[1] = depthq_table[start_g >> 16][texel];
                    start_u += add_u;
                    start_v += add_v;
                    start_g += add_g2;
                }
            }
            if (x && (texel = *(tptr + ((start_v >> 16) << 8) + (start_u >> 16))))
                pixptr[0] = depthq_table[start_g >> 16][texel];
        }
    }
}


void draw_poly_gtmap_persp(sint16 *iptr)
{
    if (xgen_xguvpersp_fp(iptr + 1))
        gtmap_persp32_fp(xgen_ymin, xgen_ymax, (unsigned char *)texture_page_ptrs[*iptr]);
}

void draw_poly_wgtmap_persp(sint16 *iptr)
{
    if (xgen_xguvpersp_fp(iptr + 1))
        wgtmap_persp16_fp(xgen_ymin, xgen_ymax, texture_page_ptrs[*iptr]);
}


}
