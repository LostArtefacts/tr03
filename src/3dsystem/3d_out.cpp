#include "../specific/standard.h"

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

}
