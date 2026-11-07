// cflags: /Op /Oa
#include "../specific/standard.h"
#include <io.h>
#include <direct.h>
#include <commctrl.h>

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

int gen_x(sint16 *iptr);
int gen_xg(sint16 *iptr);
int gen_xguv(sint16 *iptr);
int gen_xguvpersp_fp(sint16 *iptr);
void gtmap_persp32_fp(int ymin, int ymax, unsigned char *tptr);
void wgtmap_persp16_fp(int ymin, int ymax, char *tptr);
void flatA(void);
void transA(void);
void gourA(void);
void gtmapA(void);
void wgtmapA(void);

void draw_poly_flat(sint16 *iptr)
{
    int colour;

    if (gen_x(iptr + 1)) {
        colour = *iptr;
        __asm {
            mov eax, xgen_ymin
            mov edx, xgen_ymax
            mov ebx, colour
            call flatA
        }
    }
}

void draw_poly_trans(sint16 *iptr)
{
    int colour;

    if (gen_x(iptr + 1)) {
        colour = *iptr;
        __asm {
            mov eax, xgen_ymin
            mov edx, xgen_ymax
            mov ebx, colour
            call transA
        }
    }
}

void draw_poly_gouraud(sint16 *iptr)
{
    int colour;

    if (gen_xg(iptr + 1)) {
        colour = *iptr;
        __asm {
            mov eax, xgen_ymin
            mov edx, xgen_ymax
            mov ebx, colour
            call gourA
        }
    }
}

void draw_poly_gtmap(sint16 *iptr)
{
    char *texture;

    if (gen_xguv(iptr + 1)) {
        texture = texture_page_ptrs[*iptr];
        __asm {
            mov eax, xgen_ymin
            mov edx, xgen_ymax
            mov ebx, texture
            call gtmapA
        }
    }
}

void draw_poly_wgtmap(sint16 *iptr)
{
    char *texture;

    if (gen_xguv(iptr + 1)) {
        texture = texture_page_ptrs[*iptr];
        __asm {
            mov eax, xgen_ymin
            mov edx, xgen_ymax
            mov ebx, texture
            call wgtmapA
        }
    }
}

int gen_x(sint16 *iptr)
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

int gen_xg(sint16 *iptr)
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

int gen_xguv(sint16 *iptr)
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


int gen_xguvpersp_fp(sint16 *iptr)
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

void draw_poly_gtmap_persp(sint16 *iptr)
{
    if (gen_xguvpersp_fp(iptr + 1))
        gtmap_persp32_fp(xgen_ymin, xgen_ymax, (unsigned char *)texture_page_ptrs[*iptr]);
}

void draw_poly_wgtmap_persp(sint16 *iptr)
{
    if (gen_xguvpersp_fp(iptr + 1))
        wgtmap_persp16_fp(xgen_ymin, xgen_ymax, texture_page_ptrs[*iptr]);
}


#define SINGLE_MASK 0xffff8000

extern char depthq_table[33][256];
extern sint32 phd_scrwidth;
extern char *phd_winptr;


}
