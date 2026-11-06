#include "../specific/standard.h"

extern "C" {

#include "../3dglodef.h"

extern sint32 xgen_ymin;
extern sint32 xgen_ymax;
extern char *texture_page_ptrs[32];

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

}
