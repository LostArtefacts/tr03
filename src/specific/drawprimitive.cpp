#include "standard.h"
#include <mmreg.h>
#include <stdio.h>
#include <commctrl.h>
#include <d3drm.h>
#include "specific.h"
#include "winmain.h"
#include "drawprimitive.h"

extern "C" {

extern LPDIRECT3DDEVICE2 D3DDev;

float WinFrameRate(void);
extern void (*pDrawLine)(D3DTLVERTEX *v1, D3DTLVERTEX *v2);
void DrawTriangle(D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
extern HGLOBAL hSpanBuffer;
extern unsigned char *SW_SpanBuffer;
extern unsigned long *SW_Surface;
extern long SW_SurfacePitch;
extern unsigned char *RGBTableRB;
extern long *YTable;
extern long bMMXPerspective;
extern float PerspThreshold;
extern void (*pMMXDrawTriangleP)(D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
extern void (*pMMXDrawTriangle)(D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
void MMXDrawLine(D3DTLVERTEX *v1, D3DTLVERTEX *v2);
extern LPDIRECTDRAWSURFACE3 lpSurface;
extern float *WidthTable;
extern long *WidthTablef;
extern unsigned char *RGBTableG;
extern unsigned char *RGBTableAlpha;

#define MAGICNUM float((1 << 26) * ((1 << 26) * 1.5))
#define FTOLFIX(x) ((fconv = x + MAGICNUM / 65536.0), *(int *)(&fconv))
#define FTOLFIX284(x) ((fconv = x + MAGICNUM / 16.0f), *(int *)(&fconv))
#define FIX284TOFLOAT(x) float(float(x) * 0.0625f)

inline void FloorDivMod(long Numerator, long Denominator, long &Floor, long &Modulus)
{
    __asm {
        mov ecx, dword ptr [Floor]
        mov ebx, dword ptr [Modulus]
        mov edx, dword ptr [Numerator]
        xor eax, eax
        cmp edx, 0
        jl Minus
        je Zero
        mov eax, edx
        cdq
        idiv dword ptr [Denominator]
    Zero:
        mov dword ptr [ecx], eax
        mov dword ptr [ebx], edx
        jmp End
    Minus:
        sub eax, edx
        cdq
        idiv dword ptr [Denominator]
        cmp edx, 0
        je Store
        inc eax
        mov dword ptr [ecx], 0
        sub dword ptr [ecx], eax
        mov eax, dword ptr [Denominator]
        mov dword ptr [ebx], eax
        sub dword ptr [ebx], edx
        jmp End
    Store:
        mov dword ptr [ecx], 0
        sub dword ptr [ecx], eax
        mov dword ptr [ebx], edx
    End:
    }
}

inline long ceilint(float v)
{
    long val;
    long frac;
    double fconv;

    val = FTOLFIX(v);
    if (val > 0) {
        frac = val & 0xffff;
        if (!frac)
            return val >> 16;
        else
            return (val >> 16) + 1;
    } else
        return val >> 16;
}

void CalculateEdgeTG(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom)
{
    float YPreStep;
    float XPreStep;
    long dn, dm, ndm, ndn;
    long inum;
    double fconv;

    Edge->y = ceilint(Top->sy);
    Edge->height = ceilint(Bottom->sy) - Edge->y;
    if (Edge->height) {
        long tsx, tsy;
        tsx = FTOLFIX284(Top->sx);
        tsy = FTOLFIX284(Top->sy);
        ndm = FTOLFIX284(Bottom->sx) - tsx;
        ndn = FTOLFIX284(Bottom->sy) - tsy;
        dn = ndn << 4;
        dm = ndm << 4;
        inum = dm * Edge->y - ndm * tsy + ndn * tsx - 1 + dn;
        FloorDivMod(inum, dn, Edge->x, Edge->ErrorTerm);
        FloorDivMod(dm, dn, Edge->xstep, Edge->Numerator);
        Edge->Denominator = dn;
        XPreStep = FIX284TOFLOAT((Edge->x << 4) - tsx);
        YPreStep = FIX284TOFLOAT((Edge->y << 4) - tsy);
        Edge->oozxtra = Gradients->dozx;
        Edge->uozxtra = Gradients->duzx;
        Edge->vozxtra = Gradients->dvzx;
        Edge->rxtraf = FTOLFIX(Gradients->drx);
        Edge->gxtraf = FTOLFIX(Gradients->dgx);
        Edge->bxtraf = FTOLFIX(Gradients->dbx);
        Edge->ooz = Top->rhw + YPreStep * Gradients->dozy + XPreStep * Gradients->dozx;
        Edge->oozstep = Edge->xstep * Gradients->dozx + Gradients->dozy;
        Edge->uoz = Gradients->uoz[nTop] + YPreStep * Gradients->duzy + XPreStep * Gradients->duzx;
        Edge->uozstep = Edge->xstep * Gradients->duzx + Gradients->duzy;
        Edge->voz = Gradients->voz[nTop] + YPreStep * Gradients->dvzy + XPreStep * Gradients->dvzx;
        Edge->vozstep = Edge->xstep * Gradients->dvzx + Gradients->dvzy;
        float r, g, b;
        float rstep, gstep, bstep;
        r = Gradients->r[nTop] + YPreStep * Gradients->dry + XPreStep * Gradients->drx;
        rstep = Edge->xstep * Gradients->drx + Gradients->dry;
        g = Gradients->g[nTop] + YPreStep * Gradients->dgy + XPreStep * Gradients->dgx;
        gstep = Edge->xstep * Gradients->dgx + Gradients->dgy;
        b = Gradients->b[nTop] + YPreStep * Gradients->dby + XPreStep * Gradients->dbx;
        bstep = Edge->xstep * Gradients->dbx + Gradients->dby;
        Edge->rf = FTOLFIX(r) + 0x8000;
        Edge->rstepf = FTOLFIX(rstep);
        Edge->gf = FTOLFIX(g) + 0x8000;
        Edge->gstepf = FTOLFIX(gstep);
        Edge->bf = FTOLFIX(b) + 0x8000;
        Edge->bstepf = FTOLFIX(bstep);
    }
}

void CalculateGradientsG(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3)
{
    float oodx, oody;
    float v1v3y, v2v3y, v1v3x, v2v3x;
    float c2c3, c1c3;

    v1v3y = v1->sy - v3->sy;
    v2v3y = v2->sy - v3->sy;
    v1v3x = v1->sx - v3->sx;
    v2v3x = v2->sx - v3->sx;
    oodx = 1.0f / ((v2v3x * v1v3y) - (v1v3x * v2v3y));
    oody = -oodx;
    Gradients->r[0] = float(RGBA_GETRED(v1->color));
    Gradients->g[0] = float(RGBA_GETGREEN(v1->color));
    Gradients->b[0] = float(RGBA_GETBLUE(v1->color));
    Gradients->ooz[0] = float(RGBA_GETALPHA(v1->color));
    Gradients->r[1] = float(RGBA_GETRED(v2->color));
    Gradients->g[1] = float(RGBA_GETGREEN(v2->color));
    Gradients->b[1] = float(RGBA_GETBLUE(v2->color));
    Gradients->ooz[1] = float(RGBA_GETALPHA(v3->color));
    Gradients->r[2] = float(RGBA_GETRED(v3->color));
    Gradients->g[2] = float(RGBA_GETGREEN(v3->color));
    Gradients->b[2] = float(RGBA_GETBLUE(v3->color));
    Gradients->ooz[2] = float(RGBA_GETALPHA(v3->color));
    c2c3 = Gradients->r[1] - Gradients->r[2];
    c1c3 = Gradients->r[0] - Gradients->r[2];
    Gradients->drx = oodx * ((c2c3 * v1v3y) - (c1c3 * v2v3y));
    Gradients->dry = oody * ((c2c3 * v1v3x) - (c1c3 * v2v3x));
    c2c3 = Gradients->g[1] - Gradients->g[2];
    c1c3 = Gradients->g[0] - Gradients->g[2];
    Gradients->dgx = oodx * ((c2c3 * v1v3y) - (c1c3 * v2v3y));
    Gradients->dgy = oody * ((c2c3 * v1v3x) - (c1c3 * v2v3x));
    c2c3 = Gradients->b[1] - Gradients->b[2];
    c1c3 = Gradients->b[0] - Gradients->b[2];
    Gradients->dbx = oodx * ((c2c3 * v1v3y) - (c1c3 * v2v3y));
    Gradients->dby = oody * ((c2c3 * v1v3x) - (c1c3 * v2v3x));
}

void CalculateGradientsTG(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3)
{
    float oodx, oody;
    float v1v3y, v2v3y, v1v3x, v2v3x;
    float u2u3, u1u3;
    float v2v3, v1v3;
    float z2z3, z1z3;
    float c2c3, c1c3;

    v1v3y = v1->sy - v3->sy;
    v2v3y = v2->sy - v3->sy;
    v1v3x = v1->sx - v3->sx;
    v2v3x = v2->sx - v3->sx;
    oodx = 1.0f / ((v2v3x * v1v3y) - (v1v3x * v2v3y));
    oody = -oodx;
    Gradients->r[0] = float(RGBA_GETRED(v1->color));
    Gradients->g[0] = float(RGBA_GETGREEN(v1->color));
    Gradients->b[0] = float(RGBA_GETBLUE(v1->color));
    Gradients->r[1] = float(RGBA_GETRED(v2->color));
    Gradients->g[1] = float(RGBA_GETGREEN(v2->color));
    Gradients->b[1] = float(RGBA_GETBLUE(v2->color));
    Gradients->r[2] = float(RGBA_GETRED(v3->color));
    Gradients->g[2] = float(RGBA_GETGREEN(v3->color));
    Gradients->b[2] = float(RGBA_GETBLUE(v3->color));
    Gradients->uoz[0] = v1->tu * v1->rhw * 256.0f;
    Gradients->voz[0] = v1->tv * v1->rhw * 256.0f;
    Gradients->uoz[1] = v2->tu * v2->rhw * 256.0f;
    Gradients->voz[1] = v2->tv * v2->rhw * 256.0f;
    Gradients->uoz[2] = v3->tu * v3->rhw * 256.0f;
    Gradients->voz[2] = v3->tv * v3->rhw * 256.0f;
    u2u3 = Gradients->uoz[1] - Gradients->uoz[2];
    u1u3 = Gradients->uoz[0] - Gradients->uoz[2];
    v2v3 = Gradients->voz[1] - Gradients->voz[2];
    v1v3 = Gradients->voz[0] - Gradients->voz[2];
    Gradients->duzx = oodx * ((u2u3 * v1v3y) - (u1u3 * v2v3y));
    Gradients->duzy = oody * ((u2u3 * v1v3x) - (u1u3 * v2v3x));
    Gradients->dvzx = oodx * ((v2v3 * v1v3y) - (v1v3 * v2v3y));
    Gradients->dvzy = oody * ((v2v3 * v1v3x) - (v1v3 * v2v3x));
    z1z3 = v1->rhw - v3->rhw;
    z2z3 = v2->rhw - v3->rhw;
    Gradients->dozx = oodx * ((z2z3 * v1v3y) - (z1z3 * v2v3y));
    Gradients->dozy = oody * ((z2z3 * v1v3x) - (z1z3 * v2v3x));
    c2c3 = Gradients->r[1] - Gradients->r[2];
    c1c3 = Gradients->r[0] - Gradients->r[2];
    Gradients->drx = oodx * ((c2c3 * v1v3y) - (c1c3 * v2v3y));
    Gradients->dry = oody * ((c2c3 * v1v3x) - (c1c3 * v2v3x));
    c2c3 = Gradients->g[1] - Gradients->g[2];
    c1c3 = Gradients->g[0] - Gradients->g[2];
    Gradients->dgx = oodx * ((c2c3 * v1v3y) - (c1c3 * v2v3y));
    Gradients->dgy = oody * ((c2c3 * v1v3x) - (c1c3 * v2v3x));
    c2c3 = Gradients->b[1] - Gradients->b[2];
    c1c3 = Gradients->b[0] - Gradients->b[2];
    Gradients->dbx = oodx * ((c2c3 * v1v3y) - (c1c3 * v2v3y));
    Gradients->dby = oody * ((c2c3 * v1v3x) - (c1c3 * v2v3x));
}

void CalculateEdgeT(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom)
{
    float YPreStep;
    float XPreStep;
    long dn, dm, ndm, ndn;
    long inum;
    double fconv;

    Edge->y = ceilint(Top->sy);
    Edge->height = ceilint(Bottom->sy) - Edge->y;
    if (Edge->height) {
        long tsx, tsy;
        tsx = FTOLFIX284(Top->sx);
        tsy = FTOLFIX284(Top->sy);
        ndm = FTOLFIX284(Bottom->sx - Top->sx);
        ndn = FTOLFIX284(Bottom->sy - Top->sy);
        dn = ndn << 4;
        dm = ndm << 4;
        Edge->Denominator = dn;
        inum = (((dm * Edge->y) - (ndm * tsy)) + ((ndn * tsx) - 1)) + dn;
        FloorDivMod(inum, dn, Edge->x, Edge->ErrorTerm);
        FloorDivMod(dm, dn, Edge->xstep, Edge->Numerator);
        XPreStep = FIX284TOFLOAT((Edge->x << 4) - tsx);
        YPreStep = FIX284TOFLOAT((Edge->y << 4) - tsy);
        Edge->oozxtra = Gradients->dozx;
        Edge->uozxtra = Gradients->duzx;
        Edge->vozxtra = Gradients->dvzx;
        Edge->ooz = Top->rhw + YPreStep * Gradients->dozy + XPreStep * Gradients->dozx;
        Edge->uoz = Gradients->uoz[nTop] + YPreStep * Gradients->duzy + XPreStep * Gradients->duzx;
        Edge->voz = Gradients->voz[nTop] + XPreStep * Gradients->dvzx + YPreStep * Gradients->dvzy;
        Edge->oozstep = Edge->xstep * Gradients->dozx + Gradients->dozy;
        Edge->uozstep = Edge->xstep * Gradients->duzx + Gradients->duzy;
        Edge->vozstep = Edge->xstep * Gradients->dvzx + Gradients->dvzy;
    }
}

void CalculateEdgeBlack(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom)
{
    long dn, dm, ndm, ndn;
    long inum;
    double fconv;

    Edge->y = ceilint(Top->sy);
    Edge->height = ceilint(Bottom->sy) - Edge->y;
    if (Edge->height) {
        long tsx, tsy;
        tsx = FTOLFIX284(Top->sx);
        tsy = FTOLFIX284(Top->sy);
        ndm = FTOLFIX284(Bottom->sx - Top->sx);
        ndn = FTOLFIX284(Bottom->sy - Top->sy);
        dn = ndn << 4;
        dm = ndm << 4;
        Edge->Denominator = dn;
        inum = (((dm * Edge->y) - (ndm * tsy)) + ((ndn * tsx) - 1)) + dn;
        FloorDivMod(inum, dn, Edge->x, Edge->ErrorTerm);
        FloorDivMod(dm, dn, Edge->xstep, Edge->Numerator);
    }
}

void CalculateGradientsTF(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3)
{
    float oodx, oody;
    float v1v3y, v2v3y, v1v3x, v2v3x;
    float u2u3, u1u3;
    float v2v3, v1v3;
    float z2z3, z1z3;

    v1v3y = v1->sy - v3->sy;
    v2v3y = v2->sy - v3->sy;
    v1v3x = v1->sx - v3->sx;
    v2v3x = v2->sx - v3->sx;
    oodx = 1.0f / ((v2v3x * v1v3y) - (v1v3x * v2v3y));
    oody = -oodx;
    Gradients->uoz[0] = v1->tu * v1->rhw * 256.0f;
    Gradients->voz[0] = v1->tv * v1->rhw * 256.0f;
    Gradients->uoz[1] = v2->tu * v2->rhw * 256.0f;
    Gradients->voz[1] = v2->tv * v2->rhw * 256.0f;
    Gradients->uoz[2] = v3->tu * v3->rhw * 256.0f;
    Gradients->voz[2] = v3->tv * v3->rhw * 256.0f;
    u2u3 = Gradients->uoz[1] - Gradients->uoz[2];
    u1u3 = Gradients->uoz[0] - Gradients->uoz[2];
    v2v3 = Gradients->voz[1] - Gradients->voz[2];
    v1v3 = Gradients->voz[0] - Gradients->voz[2];
    Gradients->duzx = oodx * ((u2u3 * v1v3y) - (u1u3 * v2v3y));
    Gradients->duzy = oody * ((u2u3 * v1v3x) - (u1u3 * v2v3x));
    Gradients->dvzx = oodx * ((v2v3 * v1v3y) - (v1v3 * v2v3y));
    Gradients->dvzy = oody * ((v2v3 * v1v3x) - (v1v3 * v2v3x));
    z1z3 = v1->rhw - v3->rhw;
    z2z3 = v2->rhw - v3->rhw;
    Gradients->dozx = oodx * ((z2z3 * v1v3y) - (z1z3 * v2v3y));
    Gradients->dozy = oody * ((z2z3 * v1v3x) - (z1z3 * v2v3x));
    Gradients->rf = short(RGBA_GETRED(v1->color));
    Gradients->gf = short(RGBA_GETGREEN(v1->color));
    Gradients->bf = short(RGBA_GETBLUE(v1->color));
}

void CalculateGradientsTFA(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3)
{
    float oodx, oody;
    float v1v3y, v2v3y, v1v3x, v2v3x;
    float u2u3, u1u3;
    float v2v3, v1v3;

    v1v3y = v1->sy - v3->sy;
    v2v3y = v2->sy - v3->sy;
    v1v3x = v1->sx - v3->sx;
    v2v3x = v2->sx - v3->sx;
    oodx = 1.0f / ((v2v3x * v1v3y) - (v1v3x * v2v3y));
    oody = -oodx;
    Gradients->uoz[0] = v1->tu * 256.0f;
    Gradients->voz[0] = v1->tv * 256.0f;
    Gradients->uoz[1] = v2->tu * 256.0f;
    Gradients->voz[1] = v2->tv * 256.0f;
    Gradients->uoz[2] = v3->tu * 256.0f;
    Gradients->voz[2] = v3->tv * 256.0f;
    u2u3 = Gradients->uoz[1] - Gradients->uoz[2];
    u1u3 = Gradients->uoz[0] - Gradients->uoz[2];
    v2v3 = Gradients->voz[1] - Gradients->voz[2];
    v1v3 = Gradients->voz[0] - Gradients->voz[2];
    Gradients->duzx = oodx * ((u2u3 * v1v3y) - (u1u3 * v2v3y));
    Gradients->duzy = oody * ((u2u3 * v1v3x) - (u1u3 * v2v3x));
    Gradients->dvzx = oodx * ((v2v3 * v1v3y) - (v1v3 * v2v3y));
    Gradients->dvzy = oody * ((v2v3 * v1v3x) - (v1v3 * v2v3x));
    Gradients->rf = short(RGBA_GETRED(v1->color));
    Gradients->gf = short(RGBA_GETGREEN(v1->color));
    Gradients->bf = short(RGBA_GETBLUE(v1->color));
}

void CalculateGradientsTGA(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3)
{
    float oodx, oody;
    float v1v3y, v2v3y, v1v3x, v2v3x;
    float u2u3, u1u3;
    float v2v3, v1v3;
    float c2c3, c1c3;

    v1v3y = v1->sy - v3->sy;
    v2v3y = v2->sy - v3->sy;
    v1v3x = v1->sx - v3->sx;
    v2v3x = v2->sx - v3->sx;
    oodx = 1.0f / ((v2v3x * v1v3y) - (v1v3x * v2v3y));
    oody = -oodx;
    Gradients->uoz[0] = v1->tu * 256.0f;
    Gradients->voz[0] = v1->tv * 256.0f;
    Gradients->uoz[1] = v2->tu * 256.0f;
    Gradients->voz[1] = v2->tv * 256.0f;
    Gradients->uoz[2] = v3->tu * 256.0f;
    Gradients->voz[2] = v3->tv * 256.0f;
    u2u3 = Gradients->uoz[1] - Gradients->uoz[2];
    u1u3 = Gradients->uoz[0] - Gradients->uoz[2];
    v2v3 = Gradients->voz[1] - Gradients->voz[2];
    v1v3 = Gradients->voz[0] - Gradients->voz[2];
    Gradients->duzx = oodx * ((u2u3 * v1v3y) - (u1u3 * v2v3y));
    Gradients->duzy = oody * ((u2u3 * v1v3x) - (u1u3 * v2v3x));
    Gradients->dvzx = oodx * ((v2v3 * v1v3y) - (v1v3 * v2v3y));
    Gradients->dvzy = oody * ((v2v3 * v1v3x) - (v1v3 * v2v3x));
    Gradients->r[0] = float(RGBA_GETRED(v1->color));
    Gradients->g[0] = float(RGBA_GETGREEN(v1->color));
    Gradients->b[0] = float(RGBA_GETBLUE(v1->color));
    Gradients->r[1] = float(RGBA_GETRED(v2->color));
    Gradients->g[1] = float(RGBA_GETGREEN(v2->color));
    Gradients->b[1] = float(RGBA_GETBLUE(v2->color));
    Gradients->r[2] = float(RGBA_GETRED(v3->color));
    Gradients->g[2] = float(RGBA_GETGREEN(v3->color));
    Gradients->b[2] = float(RGBA_GETBLUE(v3->color));
    c2c3 = Gradients->r[1] - Gradients->r[2];
    c1c3 = Gradients->r[0] - Gradients->r[2];
    Gradients->drx = oodx * ((c2c3 * v1v3y) - (c1c3 * v2v3y));
    Gradients->dry = oody * ((c2c3 * v1v3x) - (c1c3 * v2v3x));
    c2c3 = Gradients->g[1] - Gradients->g[2];
    c1c3 = Gradients->g[0] - Gradients->g[2];
    Gradients->dgx = oodx * ((c2c3 * v1v3y) - (c1c3 * v2v3y));
    Gradients->dgy = oody * ((c2c3 * v1v3x) - (c1c3 * v2v3x));
    c2c3 = Gradients->b[1] - Gradients->b[2];
    c1c3 = Gradients->b[0] - Gradients->b[2];
    Gradients->dbx = oodx * ((c2c3 * v1v3y) - (c1c3 * v2v3y));
    Gradients->dby = oody * ((c2c3 * v1v3x) - (c1c3 * v2v3x));
}

void MMXClose(void);

void CloseDrawPrimitive(void)
{
    lpSurface->Unlock(NULL);
    if (App.DXConfig.MMX) {
        MMXClose();
        return;
    }
    free(YTable);
    free(WidthTable);
    free(WidthTablef);
    free(RGBTableRB);
    free(RGBTableG);
    free(RGBTableAlpha);
}

void BuildXYTables(void)
{
    int n, w, h;
    double fconv;

    w = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].w;
    h = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].h;
    YTable = (long *)malloc((h + 1) * sizeof(long));
    for (n = 0; n < h + 1; n++)
        YTable[n] = n * w * 2;
    WidthTable = (float *)malloc((w + 1) * sizeof(float));
    WidthTable[0] = 0.0f;
    for (n = 1; n < w + 1; n++)
        WidthTable[n] = 1.0f / float(n);
    WidthTablef = (long *)malloc((w + 1) * sizeof(long));
    WidthTablef[0] = 0;
    for (n = 1; n < w + 1; n++)
        WidthTablef[n] = FTOLFIX((1.0f / float(n)));
}

void BuildRGBTable(void)
{
    unsigned int i, j;

    RGBTableRB = (unsigned char *)malloc(0x800);
    RGBTableG = (unsigned char *)malloc(0x800);
    RGBTableAlpha = (unsigned char *)malloc(0x2000);
    for (i = 0; i < 64; i++) {
        for (j = 0; j < 32; j++) {
            RGBTableRB[i + j * 64] = (unsigned char)((i * j) >> 6);
            RGBTableG[i + j * 64] = (unsigned char)((i * j) >> 5);
        }
    }
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 256; j++)
            RGBTableAlpha[i * 256 + j] = (unsigned char)(i * (j / 256.0f));
    }
}

HRESULT HWBeginScene(void)
{
    WinFrameRate();
    return D3DDev->BeginScene();
}

HRESULT HWEndScene(void)
{
    return D3DDev->EndScene();
}

extern unsigned short *Output;
extern long pitch;
extern long bytepitch;

HRESULT SWBeginScene(void)
{
    Output = (unsigned short *)App.unk;
    pitch = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].w;
    bytepitch = pitch << 1;
    return D3D_OK;
}

HRESULT HWSetRenderState(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState)
{
    return D3DDev->SetRenderState(dwRenderStateType, dwRenderState);
}

extern long bColorKey;
extern long bAlphaBlend;
extern long bSrcBlendOne;
extern int bForceAffineMap;
extern long tpitch;
extern unsigned short *TextureHandle;

HRESULT SWSetRenderState(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState)
{
    switch (dwRenderStateType) {
    case D3DRENDERSTATE_COLORKEYENABLE:
        bColorKey = dwRenderState;
        return D3D_OK;
    case D3DRENDERSTATE_ALPHABLENDENABLE:
        bAlphaBlend = dwRenderState;
        return D3D_OK;
    case D3DRENDERSTATE_SRCBLEND:
        bSrcBlendOne = dwRenderState == D3DBLEND_ONE;
        return D3D_OK;
    case D3DRENDERSTATE_TEXTUREPERSPECTIVE:
        bForceAffineMap = dwRenderState == 0;
        return D3D_OK;
    case D3DRENDERSTATE_TEXTUREHANDLE:
        tpitch = 512;
        TextureHandle = (unsigned short *)dwRenderState;
        break;
    }
    return D3D_OK;
}

HRESULT HWDrawPrimitive(D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags)
{
    return D3DDev->DrawPrimitive(dptPrimitiveType, dvtVertexType, lpvVertices, dwVertexCount, dwFlags);
}


HRESULT SWDrawPrimitive(D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags)
{
    return EmulateDrawPrimitive(dptPrimitiveType, dvtVertexType, lpvVertices, dwVertexCount, dwFlags);
}

void PrepareTriangleList(D3DTLVERTEX *v, DWORD vcnt)
{
    DWORD n;
    double fconv;

    for (n = 0; n < vcnt; n++, v++) {
        v->sx = FIX284TOFLOAT(FTOLFIX284(v->sx));
        v->sy = FIX284TOFLOAT(FTOLFIX284(v->sy));
    }
}


void DrawTriangleList(D3DTLVERTEX *v, DWORD vcnt)
{
    int n;
    D3DTLVERTEX *v1, *v2, *v3;

    for (n = vcnt; n > 0; n -= 3) {
        v1 = v;
        v++;
        v2 = v;
        v++;
        v3 = v;
        v++;
        DrawTriangle(v1, v2, v3);
    }
}

void DrawLineList(D3DTLVERTEX *v, DWORD vcnt)
{
    int n;
    D3DTLVERTEX *v1, *v2;

    for (n = vcnt; n > 0; n -= 2) {
        v1 = v;
        v++;
        v2 = v;
        v++;
        pDrawLine(v1, v2);
    }
}

void DrawTriangleFan(D3DTLVERTEX *v, DWORD vcnt)
{
    int n;
    D3DTLVERTEX *v1, *v2, *v3;

    v1 = v;
    v++;
    v2 = v;
    v++;
    v3 = v;
    v++;
    DrawTriangle(v1, v2, v3);
    vcnt -= 3;
    for (n = vcnt; n > 0; n--) {
        v2 = v3;
        v3 = v;
        v++;
        DrawTriangle(v1, v2, v3);
    }
}

void DrawTriangleStrip(D3DTLVERTEX *v, DWORD vcnt)
{
    int n;
    D3DTLVERTEX *v1, *v2, *v3;

    v1 = v;
    v++;
    v2 = v;
    v++;
    v3 = v;
    v++;
    DrawTriangle(v1, v2, v3);
    vcnt -= 3;
    for (n = vcnt; n > 0; n--) {
        v1 = v2;
        v2 = v;
        v++;
        DrawTriangle(v1, v2, v3);
    }
}

void DrawLineStrip(D3DTLVERTEX *v, DWORD vcnt)
{
    int n;
    D3DTLVERTEX *v1, *v2;

    v1 = v;
    v++;
    v2 = v;
    v++;
    pDrawLine(v1, v2);
    vcnt -= 2;
    for (n = vcnt; n > 0; n--) {
        v1 = v2;
        v2 = v;
        v++;
        pDrawLine(v1, v2);
    }
}


void MMXClose(void)
{
    GlobalFree(hSpanBuffer);
    SW_SpanBuffer = 0;
}

HRESULT MMXBeginScene(void)
{
    SW_Surface = App.unk;
    SW_SurfacePitch = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].w << 2;
    return 0;
}


HRESULT MMXDrawPrimitive(D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags)
{
    D3DTLVERTEX *v, *v2, *v3;
    int n;

    if (!SW_Surface)
        return 0;
    switch (dptPrimitiveType) {
    case D3DPT_TRIANGLELIST:
        v = (D3DTLVERTEX *)lpvVertices;
        n = dwVertexCount;
        do {
            if (bMMXPerspective && v->rhw > PerspThreshold)
                pMMXDrawTriangleP(v, v + 1, v + 2);
            else
                pMMXDrawTriangle(v, v + 1, v + 2);
            n -= 3;
            v += 3;
        } while (n > 0);
        break;
    case D3DPT_TRIANGLEFAN:
        v = (D3DTLVERTEX *)lpvVertices;
        v2 = v + 1;
        v3 = v + 2;
        n = dwVertexCount;
        do {
            if (bMMXPerspective && v->rhw > PerspThreshold)
                pMMXDrawTriangleP(v, v2, v3);
            else
                pMMXDrawTriangle(v, v2, v3);
            v2++;
            v3++;
            n--;
        } while (n > 2);
        break;
    case D3DPT_LINELIST:
        n = dwVertexCount;
        v = (D3DTLVERTEX *)lpvVertices;
        do {
            MMXDrawLine(v, v + 1);
            n -= 2;
            v += 2;
        } while (n > 0);
        break;
    case D3DPT_LINESTRIP:
        n = dwVertexCount;
        v = (D3DTLVERTEX *)lpvVertices;
        do {
            MMXDrawLine(v, v + 1);
            n--;
            v++;
        } while (n > 1);
        break;
    }
    return 0;
}



void NullCalculateGradient(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
void NullCalculateEdge(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);
void NullScanLines(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);

void CalculateEdgeG(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom)
{
    float YPreStep;
    float XPreStep;
    long dn, dm, ndm, ndn;
    long inum;
    double fconv;

    Edge->y = ceilint(Top->sy);
    Edge->height = ceilint(Bottom->sy) - Edge->y;
    if (Edge->height) {
        long tsx, tsy;
        tsx = FTOLFIX284(Top->sx);
        tsy = FTOLFIX284(Top->sy);
        ndm = FTOLFIX284(Bottom->sx) - tsx;
        ndn = FTOLFIX284(Bottom->sy) - tsy;
        dn = ndn << 4;
        dm = ndm << 4;
        inum = dm * Edge->y - ndm * tsy + ndn * tsx - 1 + dn;
        FloorDivMod(inum, dn, Edge->x, Edge->ErrorTerm);
        FloorDivMod(dm, dn, Edge->xstep, Edge->Numerator);
        Edge->Denominator = dn;
        XPreStep = FIX284TOFLOAT((Edge->x << 4) - tsx);
        YPreStep = FIX284TOFLOAT((Edge->y << 4) - tsy);
        Edge->rxtraf = FTOLFIX(Gradients->drx);
        Edge->gxtraf = FTOLFIX(Gradients->dgx);
        Edge->bxtraf = FTOLFIX(Gradients->dbx);
        float r, g, b;
        float rstep, gstep, bstep;
        r = Gradients->r[nTop] + YPreStep * Gradients->dry + XPreStep * Gradients->drx;
        rstep = Edge->xstep * Gradients->drx + Gradients->dry;
        g = Gradients->g[nTop] + YPreStep * Gradients->dgy + XPreStep * Gradients->dgx;
        gstep = Edge->xstep * Gradients->dgx + Gradients->dgy;
        b = Gradients->b[nTop] + YPreStep * Gradients->dby + XPreStep * Gradients->dbx;
        bstep = Edge->xstep * Gradients->dbx + Gradients->dby;
        Edge->rf = FTOLFIX(r) + 0x8000;
        Edge->rstepf = FTOLFIX(rstep);
        Edge->gf = FTOLFIX(g) + 0x8000;
        Edge->gstepf = FTOLFIX(gstep);
        Edge->bf = FTOLFIX(b) + 0x8000;
        Edge->bstepf = FTOLFIX(bstep);
    }
}

void CalculateEdgeTGA(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);

void CalculateEdgeTA(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom)
{
    float YPreStep;
    float XPreStep;
    long dn, dm, ndm, ndn;
    long inum;
    double fconv;

    Edge->y = ceilint(Top->sy);
    Edge->height = ceilint(Bottom->sy) - Edge->y;
    if (Edge->height) {
        long tsx, tsy;
        tsx = FTOLFIX284(Top->sx);
        tsy = FTOLFIX284(Top->sy);
        ndm = FTOLFIX284(Bottom->sx - Top->sx);
        ndn = FTOLFIX284(Bottom->sy - Top->sy);
        dn = ndn << 4;
        dm = ndm << 4;
        Edge->Denominator = dn;
        inum = (((dm * Edge->y) - (ndm * tsy)) + ((ndn * tsx) - 1)) + dn;
        FloorDivMod(inum, dn, Edge->x, Edge->ErrorTerm);
        FloorDivMod(dm, dn, Edge->xstep, Edge->Numerator);
        XPreStep = FIX284TOFLOAT((Edge->x << 4) - tsx);
        YPreStep = FIX284TOFLOAT((Edge->y << 4) - tsy);
        float u, v, us, vs;
        u = Gradients->uoz[nTop] + YPreStep * Gradients->duzy + XPreStep * Gradients->duzx;
        v = Gradients->voz[nTop] + YPreStep * Gradients->dvzy + XPreStep * Gradients->dvzx;
        us = Edge->xstep * Gradients->duzx + Gradients->duzy;
        vs = Edge->xstep * Gradients->dvzx + Gradients->dvzy;
        Edge->uozf = FTOLFIX(u);
        Edge->vozf = FTOLFIX(v);
        Edge->uozstepf = FTOLFIX(us);
        Edge->vozstepf = FTOLFIX(vs);
        Edge->uozxtraf = FTOLFIX(Gradients->duzx);
        Edge->vozxtraf = FTOLFIX(Gradients->dvzx);
    }
}

void CalculateEdgeTG(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);
void CalculateEdgeT(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);

void CalculateEdgeTGA(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom)
{
    float YPreStep;
    float XPreStep;
    long dn, dm, ndm, ndn;
    long inum;
    double fconv;

    Edge->y = ceilint(Top->sy);
    Edge->height = ceilint(Bottom->sy) - Edge->y;
    if (Edge->height) {
        long tsx, tsy;
        tsx = FTOLFIX284(Top->sx);
        tsy = FTOLFIX284(Top->sy);
        ndm = FTOLFIX284(Bottom->sx - Top->sx);
        ndn = FTOLFIX284(Bottom->sy - Top->sy);
        dn = ndn << 4;
        dm = ndm << 4;
        Edge->Denominator = dn;
        inum = (((dm * Edge->y) - (ndm * tsy)) + ((ndn * tsx) - 1)) + dn;
        FloorDivMod(inum, dn, Edge->x, Edge->ErrorTerm);
        FloorDivMod(dm, dn, Edge->xstep, Edge->Numerator);
        XPreStep = FIX284TOFLOAT((Edge->x << 4) - tsx);
        YPreStep = FIX284TOFLOAT((Edge->y << 4) - tsy);
        float u, v, us, vs;
        u = Gradients->uoz[nTop] + YPreStep * Gradients->duzy + XPreStep * Gradients->duzx;
        v = Gradients->voz[nTop] + YPreStep * Gradients->dvzy + XPreStep * Gradients->dvzx;
        us = Edge->xstep * Gradients->duzx + Gradients->duzy;
        vs = Edge->xstep * Gradients->dvzx + Gradients->dvzy;
        Edge->uozf = FTOLFIX(u);
        Edge->vozf = FTOLFIX(v);
        Edge->uozstepf = FTOLFIX(us);
        Edge->vozstepf = FTOLFIX(vs);
        Edge->uozxtraf = FTOLFIX(Gradients->duzx);
        Edge->vozxtraf = FTOLFIX(Gradients->dvzx);
        float r, g, b, rstep, gstep, bstep;
        r = Gradients->r[nTop] + YPreStep * Gradients->dry + XPreStep * Gradients->drx;
        g = Gradients->g[nTop] + YPreStep * Gradients->dgy + XPreStep * Gradients->dgx;
        b = Gradients->b[nTop] + YPreStep * Gradients->dby + XPreStep * Gradients->dbx;
        rstep = Edge->xstep * Gradients->drx + Gradients->dry;
        gstep = Edge->xstep * Gradients->dgx + Gradients->dgy;
        bstep = Edge->xstep * Gradients->dbx + Gradients->dby;
        Edge->rf = FTOLFIX(r) + 0x8000;
        Edge->rstepf = FTOLFIX(rstep);
        Edge->gf = FTOLFIX(g) + 0x8000;
        Edge->gstepf = FTOLFIX(gstep);
        Edge->bf = FTOLFIX(b) + 0x8000;
        Edge->bstepf = FTOLFIX(bstep);
        Edge->rxtraf = FTOLFIX(Gradients->drx);
        Edge->gxtraf = FTOLFIX(Gradients->dgx);
        Edge->bxtraf = FTOLFIX(Gradients->dbx);
    }
}

extern void (*CalcGradientsPersp)(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
extern void (*CalcEdgePersp)(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);
extern void (*CalcGradientsAffine)(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
extern void (*CalcEdgeAffine)(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);
extern void (*DrawScanLinesAffine)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*CalcGradientsBlack)(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
extern void (*CalcEdgeBlack)(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);
extern void (*DrawScanLinesBlack)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*CalcGradientsFlat)(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
extern void (*CalcEdgeFlat)(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);
extern void (*DrawScanLinesFlat)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*CalcGradientsFlatAffine)(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
extern void (*CalcEdgeFlatAffine)(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);
extern void (*DrawScanLinesFlatAffine)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);

void DrawTriangle(D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3)
{
    D3DTLVERTEX *Top, *Middle, *Bottom;
    GRADIENT Gradients;
    EDGE TopToBottom;
    EDGE TopToMiddle;
    EDGE MiddleToBottom;
    EDGE *pLeft, *pRight;
    float y0, y1, y2;
    int nTop, nMiddle, nBottom;
    int MiddleIsLeft;

    y0 = v1->sy;
    y1 = v2->sy;
    y2 = v3->sy;
    if (y0 < y1) {
        if (y2 < y0) {
            Top = v3;
            Middle = v1;
            Bottom = v2;
            nTop = 2;
            nMiddle = 0;
            nBottom = 1;
            MiddleIsLeft = 0;
        } else {
            Top = v1;
            nTop = 0;
            if (y1 < y2) {
                Middle = v2;
                Bottom = v3;
                nMiddle = 1;
                nBottom = 2;
                MiddleIsLeft = 0;
            } else {
                Middle = v3;
                Bottom = v2;
                nMiddle = 2;
                nBottom = 1;
                MiddleIsLeft = 1;
            }
        }
    } else {
        if (y2 < y1) {
            Top = v3;
            Middle = v2;
            Bottom = v1;
            nTop = 2;
            nMiddle = 1;
            nBottom = 0;
            MiddleIsLeft = 1;
        } else {
            Top = v2;
            nTop = 1;
            if (y0 < y2) {
                Middle = v1;
                Bottom = v3;
                nMiddle = 0;
                nBottom = 2;
                MiddleIsLeft = 1;
            } else {
                Middle = v3;
                Bottom = v1;
                nMiddle = 2;
                nBottom = 0;
                MiddleIsLeft = 0;
            }
        }
    }
    if (!(v1->color & 0xffffff) && !(v2->color & 0xffffff) && !(v3->color & 0xffffff)) {
        CalcGradientsBlack(&Gradients, v1, v2, v3);
        CalcEdgeBlack(&TopToBottom, &Gradients, Top, Bottom, nTop, nBottom);
        CalcEdgeBlack(&TopToMiddle, &Gradients, Top, Middle, nTop, nMiddle);
        CalcEdgeBlack(&MiddleToBottom, &Gradients, Middle, Bottom, nMiddle, nBottom);
        if (MiddleIsLeft) {
            pLeft = &TopToMiddle;
            pRight = &TopToBottom;
        } else {
            pLeft = &TopToBottom;
            pRight = &TopToMiddle;
        }
        DrawScanLinesBlack(&Gradients, pLeft, pRight, &TopToMiddle, &TopToBottom);
        if (MiddleIsLeft) {
            pLeft = &MiddleToBottom;
            pRight = &TopToBottom;
        } else {
            pLeft = &TopToBottom;
            pRight = &MiddleToBottom;
        }
        DrawScanLinesBlack(&Gradients, pLeft, pRight, &MiddleToBottom, &TopToBottom);
        return;
    }
    if (bForceAffineMap == 0) {
        if (v1->rhw > 0.4f) {
            if (!((v1->color ^ v2->color) & 0xffffff)) {
                if (!((v2->color ^ v3->color) & 0xffffff)) {
                    CalcGradientsFlat(&Gradients, v1, v2, v3);
                    CalcEdgeFlat(&TopToBottom, &Gradients, Top, Bottom, nTop, nBottom);
                    CalcEdgeFlat(&TopToMiddle, &Gradients, Top, Middle, nTop, nMiddle);
                    CalcEdgeFlat(&MiddleToBottom, &Gradients, Middle, Bottom, nMiddle, nBottom);
                    if (MiddleIsLeft) {
                        pLeft = &TopToMiddle;
                        pRight = &TopToBottom;
                    } else {
                        pLeft = &TopToBottom;
                        pRight = &TopToMiddle;
                    }
                    DrawScanLinesFlat(&Gradients, pLeft, pRight, &TopToMiddle, &TopToBottom);
                    if (MiddleIsLeft) {
                        pLeft = &MiddleToBottom;
                        pRight = &TopToBottom;
                    } else {
                        pLeft = &TopToBottom;
                        pRight = &MiddleToBottom;
                    }
                    DrawScanLinesFlat(&Gradients, pLeft, pRight, &MiddleToBottom, &TopToBottom);
                    return;
                }
            }
            CalcGradientsPersp(&Gradients, v1, v2, v3);
            CalcEdgePersp(&TopToBottom, &Gradients, Top, Bottom, nTop, nBottom);
            CalcEdgePersp(&TopToMiddle, &Gradients, Top, Middle, nTop, nMiddle);
            CalcEdgePersp(&MiddleToBottom, &Gradients, Middle, Bottom, nMiddle, nBottom);
            if (MiddleIsLeft) {
                pLeft = &TopToMiddle;
                pRight = &TopToBottom;
            } else {
                pLeft = &TopToBottom;
                pRight = &TopToMiddle;
            }
            DrawScanLines(&Gradients, pLeft, pRight, &TopToMiddle, &TopToBottom);
            if (MiddleIsLeft) {
                pLeft = &MiddleToBottom;
                pRight = &TopToBottom;
            } else {
                pLeft = &TopToBottom;
                pRight = &MiddleToBottom;
            }
            DrawScanLines(&Gradients, pLeft, pRight, &MiddleToBottom, &TopToBottom);
            return;
        }
    }
    if (!((v1->color ^ v2->color) & 0xffffff)) {
        if (!((v2->color ^ v3->color) & 0xffffff)) {
            CalcGradientsFlatAffine(&Gradients, v1, v2, v3);
            CalcEdgeFlatAffine(&TopToBottom, &Gradients, Top, Bottom, nTop, nBottom);
            CalcEdgeFlatAffine(&TopToMiddle, &Gradients, Top, Middle, nTop, nMiddle);
            CalcEdgeFlatAffine(&MiddleToBottom, &Gradients, Middle, Bottom, nMiddle, nBottom);
            if (MiddleIsLeft) {
                pLeft = &TopToMiddle;
                pRight = &TopToBottom;
            } else {
                pLeft = &TopToBottom;
                pRight = &TopToMiddle;
            }
            DrawScanLinesFlatAffine(&Gradients, pLeft, pRight, &TopToMiddle, &TopToBottom);
            if (MiddleIsLeft) {
                pLeft = &MiddleToBottom;
                pRight = &TopToBottom;
            } else {
                pLeft = &TopToBottom;
                pRight = &MiddleToBottom;
            }
            DrawScanLinesFlatAffine(&Gradients, pLeft, pRight, &MiddleToBottom, &TopToBottom);
            return;
        }
    }
    CalcGradientsAffine(&Gradients, v1, v2, v3);
    CalcEdgeAffine(&TopToBottom, &Gradients, Top, Bottom, nTop, nBottom);
    CalcEdgeAffine(&TopToMiddle, &Gradients, Top, Middle, nTop, nMiddle);
    CalcEdgeAffine(&MiddleToBottom, &Gradients, Middle, Bottom, nMiddle, nBottom);
    if (MiddleIsLeft) {
        pLeft = &TopToMiddle;
        pRight = &TopToBottom;
    } else {
        pLeft = &TopToBottom;
        pRight = &TopToMiddle;
    }
    DrawScanLinesAffine(&Gradients, pLeft, pRight, &TopToMiddle, &TopToBottom);
    if (MiddleIsLeft) {
        pLeft = &MiddleToBottom;
        pRight = &TopToBottom;
    } else {
        pLeft = &TopToBottom;
        pRight = &MiddleToBottom;
    }
    DrawScanLinesAffine(&Gradients, pLeft, pRight, &MiddleToBottom, &TopToBottom);
}

extern long SW_SpanEnd;
extern long SW_SpanY;
extern long SpansTexturedMask;
extern long SW_SpanDU;
extern long SW_SpanDV;
extern long SW_TextureBits;
extern long SW_TexShift;
extern long MipOffsets[];
extern __int64 SW_SpanGrad;

__declspec(naked) void SW_SpanGT(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [SW_SpanBuffer]
        cmp esi, dword ptr [SW_SpanEnd]
        je Done
        push ebp
        mov edi, dword ptr [SW_Surface]
        mov eax, dword ptr [SW_SpanY]
        imul eax, dword ptr [SW_SurfacePitch]
        add edi, eax
        mov ecx, dword ptr [SW_TexShift]
        mov eax, 0xff0000
        shl eax, cl
        and eax, 0xff0000
        mov dword ptr [SpansTexturedMask], eax
        mov dword ptr [P1 + 2], eax
        mov dword ptr [P4 + 2], eax
        mov al, cl
        add al, 8
        add al, cl
        mov byte ptr [P5 + 2], al
        mov byte ptr [P3 + 2], al
        mov al, cl
        add al, 0x10
        mov byte ptr [P6 + 2], al
        mov byte ptr [P7 + 2], al
        mov ebx, dword ptr [SW_SpanDU]
        mov eax, dword ptr [SW_SpanDV]
        mov edx, dword ptr [SW_TextureBits]
        add edx, dword ptr [MipOffsets + ecx * 4]
        mov dword ptr [P8 + 2], ebx
        mov dword ptr [P2 + 2], eax
        mov dword ptr [P0 + 4], edx
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    LineLoop:
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [esi + 4]
        push edi
        lea edi, [edi + ebx * 4]
        or eax, eax
        jle NextLine
        mov ebx, dword ptr [esi + 8]
        mov ecx, dword ptr [esi + 0x10]
        mov edx, ecx
        mov ebp, ebx
        and edx, dword ptr [SpansTexturedMask]
        and ebp, dword ptr [SpansTexturedMask]
    P5:
        shr edx, 8
        pxor mm5, mm5
    P6:
        shr ebp, 0x10
        movq mm1, qword ptr [SW_SpanGrad]
        or edx, ebp
        movq mm0, qword ptr [esi + 0x18]
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        add ecx, dword ptr [SW_SpanDV]
        add ebx, dword ptr [SW_SpanDU]
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
    PixelLoop:
    P0:
        movd mm7, dword ptr [edx * 4 + 0x12345678]
        psrlw mm6, 8
        mov edx, ecx
        punpcklbw mm7, mm5
        pmullw mm7, mm6
        mov ebp, ebx
    P1:
        and edx, 0xff0000
    P2:
        add ecx, 0x12345678
    P3:
        shr edx, 8
    P4:
        and ebp, 0xff0000
    P7:
        shr ebp, 0x10
        psrlw mm7, 8
        packuswb mm7, mm7
        add edi, 4
    P8:
        add ebx, 0x12345678
        or edx, ebp
        movd dword ptr [edi - 4], mm7
        paddw mm0, mm1
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        dec eax
        jne PixelLoop
    NextLine:
        pop edi
        add esi, 0x20
        add edi, dword ptr [SW_SurfacePitch]
        cmp dword ptr [SW_SpanEnd], esi
        jne LineLoop
        emms
        pop ebp
    Done:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

__declspec(naked) void SW_SpanG(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [SW_SpanBuffer]
        cmp esi, dword ptr [SW_SpanEnd]
        je L6
        push ebp
        mov edi, dword ptr [SW_Surface]
        mov eax, dword ptr [SW_SpanY]
        imul eax, dword ptr [SW_SurfacePitch]
        add edi, eax
    L1:
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [esi+0x4]
        shl ebx, 0x2
        push edi
        add edi, ebx
        or eax, eax
        jle L5
        movq mm0, qword ptr [esi+0x18]
        movq mm1, qword ptr [SW_SpanGrad]
        test edi, 0x7
        je L2
        _emit 0x0f
        _emit 0x7f
        _emit 0xc7
        add edi, 0x4
        psrlw mm7, 0x8
        paddw mm0, mm1
        packuswb mm7, mm7
        dec eax
        movd dword ptr [edi-0x4], mm7
        je L5
    L2:
        cmp eax, 0x2
        jb L4
        push eax
        _emit 0x0f
        _emit 0x7f
        _emit 0xc2
        paddw mm2, mm1
        shr eax, 1
        psllw mm1, 0x1
    L3:
        add edi, 0x8
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        psrlw mm6, 0x8
        _emit 0x0f
        _emit 0x7f
        _emit 0xd7
        psrlw mm7, 0x8
        packuswb mm6, mm7
        paddw mm2, mm1
        paddw mm0, mm1
        movq qword ptr [edi-0x8], mm6
        dec eax
        jne L3
        pop eax
        and eax, 0x1
        je L5
    L4:
        movq mm2, qword ptr [SW_SpanGrad]
        _emit 0x0f
        _emit 0x7f
        _emit 0xc1
        psrlw mm1, 0x8
        packuswb mm1, mm1
        movd dword ptr [edi], mm1
    L5:
        pop edi
        add edi, dword ptr [SW_SurfacePitch]
        add esi, 0x20
        cmp dword ptr [SW_SpanEnd], esi
        jne L1
        _emit 0x0f
        _emit 0x31
        emms
        pop ebp
    L6:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

extern long SpansAdditiveMask;

__declspec(naked) void SW_SpanGTAdd(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [SW_SpanBuffer]
        cmp esi, dword ptr [SW_SpanEnd]
        je L13
        push ebp
        mov edi, dword ptr [SW_Surface]
        mov eax, dword ptr [SW_SpanY]
        imul eax, dword ptr [SW_SurfacePitch]
        add edi, eax
        mov ecx, dword ptr [SW_TexShift]
        mov eax, 0xff0000
        shl eax, cl
        and eax, 0xff0000
        mov dword ptr [SpansAdditiveMask], eax
        mov dword ptr [L7 + 2], eax
        mov dword ptr [L8 + 2], eax
        mov al, cl
        add al, 0x8
        add al, cl
        mov byte ptr [L10 + 2], al
        mov byte ptr [L2 + 2], al
        mov al, cl
        add al, 0x10
        mov byte ptr [L11 + 2], al
        mov byte ptr [L3 + 2], al
        mov ebx, dword ptr [SW_SpanDU]
        mov eax, dword ptr [SW_SpanDV]
        mov edx, dword ptr [SW_TextureBits]
        add edx, dword ptr [ecx*4+MipOffsets]
        mov dword ptr [L5 + 2], ebx
        mov dword ptr [L9 + 2], eax
        mov dword ptr [L6 + 4], edx
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L1:
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [esi+0x4]
        shl ebx, 0x2
        push edi
        add edi, ebx
        or eax, eax
        jle L12
        movq mm0, qword ptr [esi+0x18]
        movq mm1, qword ptr [SW_SpanGrad]
        push ebx
        push ecx
        push esi
        mov ebx, dword ptr [esi+0x8]
        mov ecx, dword ptr [esi+0x10]
        mov edx, ecx
        mov ebp, ebx
        and edx, dword ptr [SpansAdditiveMask]
        and ebp, dword ptr [SpansAdditiveMask]
        pxor mm5, mm5
    L2:
        shr edx, 0x8
    L3:
        shr ebp, 0x10
        add ecx, dword ptr [SW_SpanDV]
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L4:
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        or edx, ebp
        psrlw mm6, 0x8
    L5:
        add ebx, 0x12345678
    L6:
        movd mm7, dword ptr [edx*4+0x12345678]
        punpcklbw mm7, mm5
        mov ebp, ebx
        pmullw mm7, mm6
        mov edx, ecx
        movd mm4, dword ptr [edi]
    L7:
        and edx, 0xff0000
    L8:
        and ebp, 0xff0000
        psrlw mm7, 0x8
    L9:
        add ecx, 0x12345678
    L10:
        shr edx, 0x8
        packuswb mm7, mm7
    L11:
        shr ebp, 0x10
        paddusb mm7, mm4
        paddw mm0, mm1
        dec eax
        movd dword ptr [edi], mm7
        lea edi, [edi+0x4]
        jne L4
        pop esi
        pop ebx
        pop ecx
    L12:
        pop edi
        add edi, dword ptr [SW_SurfacePitch]
        add esi, 0x20
        cmp dword ptr [SW_SpanEnd], esi
        jne L1
        emms
        pop ebp
    L13:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

extern long SpansColorKeyMask;
extern __int64 SpansColorKeyInvert;

__declspec(naked) void SW_SpanGTKey(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [SW_SpanBuffer]
        cmp esi, dword ptr [SW_SpanEnd]
        je L12
        push ebp
        mov edi, dword ptr [SW_Surface]
        mov eax, dword ptr [SW_SpanY]
        imul eax, dword ptr [SW_SurfacePitch]
        add edi, eax
        mov ecx, dword ptr [SW_TexShift]
        mov eax, 0xff0000
        shl eax, cl
        and eax, 0xff0000
        mov dword ptr [SpansColorKeyMask], eax
        mov dword ptr [L5 + 2], eax
        mov dword ptr [L8 + 2], eax
        mov al, cl
        add al, 0x8
        add al, cl
        mov byte ptr [L2 + 2], al
        mov byte ptr [L7 + 2], al
        mov al, cl
        add al, 0x10
        mov byte ptr [L3 + 2], al
        mov byte ptr [L9 + 2], al
        mov edx, dword ptr [SW_TextureBits]
        add edx, dword ptr [ecx*4+MipOffsets]
        mov dword ptr [L4 + 4], edx
        mov eax, dword ptr [SW_SpanDU]
        mov ebx, dword ptr [SW_SpanDV]
        mov dword ptr [L10 + 2], eax
        mov dword ptr [L6 + 2], ebx
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L1:
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [esi+0x4]
        push edi
        lea edi, [edi+ebx*4]
        add esi, 0x20
        or eax, eax
        jle L11
        mov ebx, dword ptr [esi-0x18]
        mov ecx, dword ptr [esi-0x10]
        mov edx, ecx
        mov ebp, ebx
        and edx, dword ptr [SpansColorKeyMask]
        and ebp, dword ptr [SpansColorKeyMask]
    L2:
        shr edx, 0x8
        pxor mm5, mm5
    L3:
        shr ebp, 0x10
        movq mm1, qword ptr [SW_SpanGrad]
        or edx, ebp
        movq mm0, qword ptr [esi-0x8]
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        add ecx, dword ptr [SW_SpanDV]
        add ebx, dword ptr [SW_SpanDU]
        nop
    L4:
        movd mm7, dword ptr [edx*4+0x12345678]
        psrlw mm6, 0x8
        mov edx, ecx
        mov ebp, ebx
        _emit 0x0f
        _emit 0x7f
        _emit 0xfc
        punpcklbw mm7, mm5
        pcmpeqd mm4, mm5
        pmullw mm7, mm6
        movq mm6, qword ptr [edi]
        pand mm6, mm4
        pxor mm4, qword ptr [SpansColorKeyInvert]
    L5:
        and edx, 0xff0000
    L6:
        add ecx, 0x12345678
    L7:
        shr edx, 0x8
    L8:
        and ebp, 0xff0000
    L9:
        shr ebp, 0x10
        psrlw mm7, 0x8
        packuswb mm7, mm7
        add edi, 0x4
        pand mm7, mm4
        por mm7, mm6
        or edx, ebp
    L10:
        add ebx, 0x12345678
        movd dword ptr [edi-0x4], mm7
        paddw mm0, mm1
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        dec eax
        jne L4
    L11:
        pop edi
        add edi, dword ptr [SW_SurfacePitch]
        cmp dword ptr [SW_SpanEnd], esi
        jne L1
        emms
        pop ebp
    L12:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

extern long SpansTexturedPMask;

__declspec(naked) void SW_SpanGTP(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [SW_SpanBuffer]
        cmp esi, dword ptr [SW_SpanEnd]
        je L11
        push ebp
        mov edi, dword ptr [SW_Surface]
        mov eax, dword ptr [SW_SpanY]
        imul eax, dword ptr [SW_SurfacePitch]
        add edi, eax
        mov ecx, dword ptr [SW_TexShift]
        mov eax, 0xff0000
        shl eax, cl
        and eax, 0xff0000
        mov dword ptr [SpansTexturedPMask], eax
        mov dword ptr [L6 + 2], eax
        mov dword ptr [L8 + 2], eax
        mov al, cl
        add al, 0x8
        add al, cl
        mov byte ptr [L2 + 2], al
        mov byte ptr [L7 + 2], al
        mov al, cl
        add al, 0x10
        mov byte ptr [L3 + 2], al
        mov byte ptr [L9 + 2], al
        mov edx, dword ptr [SW_TextureBits]
        add edx, dword ptr [ecx*4+MipOffsets]
        mov dword ptr [L5 + 4], edx
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
    L1:
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [esi+0x4]
        push edi
        lea edi, [edi+ebx*4]
        add esi, 0x20
        or eax, eax
        jle L10
        mov ebx, dword ptr [esi-0x18]
        mov ecx, dword ptr [esi-0x10]
        mov edx, ecx
        mov ebp, ebx
        and edx, dword ptr [SpansTexturedPMask]
        and ebp, dword ptr [SpansTexturedPMask]
    L2:
        shr edx, 0x8
        pxor mm5, mm5
    L3:
        shr ebp, 0x10
        movq mm1, qword ptr [SW_SpanGrad]
        or edx, ebp
        movq mm0, qword ptr [esi-0x8]
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        add ebx, dword ptr [esi]
        add ecx, dword ptr [esi+0x4]
        nop
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L4:
        push eax
        cmp eax, 0x10
        jle L5
        mov eax, 0x10
        nop
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
    L5:
        movd mm7, dword ptr [edx*4+0x12345678]
        psrlw mm6, 0x8
        mov edx, ecx
        punpcklbw mm7, mm5
        pmullw mm7, mm6
        mov ebp, ebx
    L6:
        and edx, 0xff0000
        add ecx, dword ptr [esi+0x4]
    L7:
        shr edx, 0x8
    L8:
        and ebp, 0xff0000
    L9:
        shr ebp, 0x10
        psrlw mm7, 0x8
        packuswb mm7, mm7
        add edi, 0x4
        or edx, ebp
        add ebx, dword ptr [esi]
        movd dword ptr [edi-0x4], mm7
        paddw mm0, mm1
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        dec eax
        jne L5
        pop eax
        add esi, 0x8
        sub eax, 0x10
        jg L4
        sub esi, 0x8
    L10:
        add esi, 0x8
        pop edi
        add edi, dword ptr [SW_SurfacePitch]
        cmp dword ptr [SW_SpanEnd], esi
        jne L1
        emms
        pop ebp
    L11:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

extern long SpansColorKeyPMask;
extern __int64 SpansColorKeyPInvert;

__declspec(naked) void SW_SpanGTPKey(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [SW_SpanBuffer]
        cmp esi, dword ptr [SW_SpanEnd]
        je L11
        push ebp
        mov edi, dword ptr [SW_Surface]
        mov eax, dword ptr [SW_SpanY]
        imul eax, dword ptr [SW_SurfacePitch]
        add edi, eax
        mov ecx, dword ptr [SW_TexShift]
        mov eax, 0xff0000
        shl eax, cl
        and eax, 0xff0000
        mov dword ptr [SpansColorKeyPMask], eax
        mov dword ptr [L6 + 2], eax
        mov dword ptr [L8 + 2], eax
        mov al, cl
        add al, 0x8
        add al, cl
        mov byte ptr [L2 + 2], al
        mov byte ptr [L7 + 2], al
        mov al, cl
        add al, 0x10
        mov byte ptr [L3 + 2], al
        mov byte ptr [L9 + 2], al
        mov edx, dword ptr [SW_TextureBits]
        add edx, dword ptr [ecx*4+MipOffsets]
        mov dword ptr [L5 + 4], edx
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
    L1:
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [esi+0x4]
        push edi
        lea edi, [edi+ebx*4]
        add esi, 0x20
        or eax, eax
        jle L10
        mov ebx, dword ptr [esi-0x18]
        mov ecx, dword ptr [esi-0x10]
        mov edx, ecx
        mov ebp, ebx
        and edx, dword ptr [SpansColorKeyPMask]
        and ebp, dword ptr [SpansColorKeyPMask]
    L2:
        shr edx, 0x8
        pxor mm5, mm5
    L3:
        shr ebp, 0x10
        movq mm1, qword ptr [SW_SpanGrad]
        or edx, ebp
        movq mm0, qword ptr [esi-0x8]
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        add ebx, dword ptr [esi]
        add ecx, dword ptr [esi+0x4]
        nop
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L4:
        push eax
        cmp eax, 0x10
        jle L5
        mov eax, 0x10
        nop
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
    L5:
        movd mm7, dword ptr [edx*4+0x12345678]
        psrlw mm6, 0x8
        mov edx, ecx
        mov ebp, ebx
        _emit 0x0f
        _emit 0x7f
        _emit 0xfc
        punpcklbw mm7, mm5
        pmullw mm7, mm6
    L6:
        and edx, 0xff0000
        movq mm6, qword ptr [edi]
        pcmpeqd mm4, mm5
        pand mm6, mm4
        pxor mm4, qword ptr [SpansColorKeyPInvert]
    L7:
        shr edx, 0x8
    L8:
        and ebp, 0xff0000
    L9:
        shr ebp, 0x10
        add ecx, dword ptr [esi+0x4]
        psrlw mm7, 0x8
        add edi, 0x4
        packuswb mm7, mm7
        or edx, ebp
        pand mm7, mm4
        paddw mm0, mm1
        por mm7, mm6
        add ebx, dword ptr [esi]
        movd dword ptr [edi-0x4], mm7
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        dec eax
        jne L5
        pop eax
        add esi, 0x8
        sub eax, 0x10
        jg L4
        sub esi, 0x8
    L10:
        add esi, 0x8
        pop edi
        add edi, dword ptr [SW_SurfacePitch]
        cmp dword ptr [SW_SpanEnd], esi
        jne L1
        emms
        pop ebp
    L11:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

extern long SpansAdditivePMask;

__declspec(naked) void SW_SpanGTPAdd(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [SW_SpanBuffer]
        cmp esi, dword ptr [SW_SpanEnd]
        je L11
        push ebp
        mov edi, dword ptr [SW_Surface]
        mov eax, dword ptr [SW_SpanY]
        imul eax, dword ptr [SW_SurfacePitch]
        add edi, eax
        mov ecx, dword ptr [SW_TexShift]
        mov eax, 0xff0000
        shl eax, cl
        and eax, 0xff0000
        mov dword ptr [SpansAdditivePMask], eax
        mov dword ptr [L6 + 2], eax
        mov dword ptr [L8 + 2], eax
        mov al, cl
        add al, 0x8
        add al, cl
        mov byte ptr [L2 + 2], al
        mov byte ptr [L7 + 2], al
        mov al, cl
        add al, 0x10
        mov byte ptr [L3 + 2], al
        mov byte ptr [L9 + 2], al
        mov edx, dword ptr [SW_TextureBits]
        add edx, dword ptr [ecx*4+MipOffsets]
        mov dword ptr [L5 + 4], edx
        pxor mm5, mm5
        nop
    L1:
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [esi+0x4]
        push edi
        lea edi, [edi+ebx*4]
        add esi, 0x20
        or eax, eax
        jle L10
        mov ebx, dword ptr [esi-0x18]
        mov ecx, dword ptr [esi-0x10]
        mov edx, ecx
        mov ebp, ebx
        and edx, dword ptr [SpansAdditivePMask]
        and ebp, dword ptr [SpansAdditivePMask]
    L2:
        shr edx, 0x8
        pxor mm5, mm5
    L3:
        shr ebp, 0x10
        movq mm1, qword ptr [SW_SpanGrad]
        or edx, ebp
        movq mm0, qword ptr [esi-0x8]
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        add ebx, dword ptr [esi]
        add ecx, dword ptr [esi+0x4]
        nop
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L4:
        push eax
        cmp eax, 0x10
        jle L5
        mov eax, 0x10
        nop
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
    L5:
        movd mm7, dword ptr [edx*4+0x12345678]
        psrlw mm6, 0x8
        movd mm4, dword ptr [edi]
        punpcklbw mm7, mm5
        pmullw mm7, mm6
        mov edx, ecx
        mov ebp, ebx
    L6:
        and edx, 0xff0000
    L7:
        shr edx, 0x8
        add ecx, dword ptr [esi+0x4]
    L8:
        and ebp, 0xff0000
        psrlw mm7, 0x8
        add edi, 0x4
    L9:
        shr ebp, 0x10
        packuswb mm7, mm7
        paddusb mm7, mm4
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        or edx, ebp
        add ebx, dword ptr [esi]
        movd dword ptr [edi-0x4], mm7
        paddw mm0, mm1
        dec eax
        jne L5
        pop eax
        add esi, 0x8
        sub eax, 0x10
        jg L4
        sub esi, 0x8
    L10:
        add esi, 0x8
        pop edi
        add edi, dword ptr [SW_SurfacePitch]
        cmp dword ptr [SW_SpanEnd], esi
        jne L1
        emms
        pop ebp
    L11:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

extern long SpansTexturedFilteredV;
extern long SpansTexturedFilteredU;
extern __int64 SpansTexturedFilteredFrac;

__declspec(naked) void SW_SpanGTBilinear(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [SW_SpanBuffer]
        cmp esi, dword ptr [SW_SpanEnd]
        je L9
        push ebp
        mov edi, dword ptr [SW_Surface]
        mov eax, dword ptr [SW_SpanY]
        imul eax, dword ptr [SW_SurfacePitch]
        add edi, eax
        mov ebx, dword ptr [SW_SpanDU]
        mov eax, dword ptr [SW_SpanDV]
        mov edx, dword ptr [SW_TextureBits]
        mov dword ptr [L7 + 2], ebx
        mov dword ptr [L6 + 2], eax
        mov dword ptr [L2 + 4], edx
        add edx, 0x4
        mov dword ptr [L3 + 4], edx
        add edx, 0x3fc
        mov dword ptr [L4 + 4], edx
        add edx, 0x4
        mov dword ptr [L5 + 4], edx
        pxor mm5, mm5
        _emit 0x8d
        _emit 0x49
        _emit 0x00
    L1:
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [esi+0x4]
        push edi
        lea edi, [edi+ebx*4]
        or eax, eax
        jle L8
        mov ebx, dword ptr [esi+0x8]
        mov ecx, dword ptr [esi+0x10]
        movq mm1, qword ptr [SW_SpanGrad]
        movq mm0, qword ptr [esi+0x18]
        mov edx, ecx
        mov ebp, ebx
        mov dword ptr [SpansTexturedFilteredV], edx
        mov dword ptr [SpansTexturedFilteredU], ebp
        movd mm2, dword ptr [SpansTexturedFilteredU]
        and edx, 0xff0000
        pand mm2, qword ptr [SpansTexturedFilteredFrac]
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        and ebp, 0xff0000
        psllq mm3, 0x10
        shr edx, 0x8
        shr ebp, 0x10
        por mm2, mm3
        or edx, ebp
        mov edi, edi
    L2:
        movd mm7, dword ptr [edx*4+0x12345678]
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
    L3:
        movd mm4, dword ptr [edx*4+0x12345678]
        punpcklbw mm7, mm5
    L4:
        movd mm6, dword ptr [edx*4+0x12345678]
        punpcklbw mm4, mm5
        psubw mm4, mm7
        psllq mm2, 0x20
        punpcklbw mm6, mm5
        por mm2, mm3
    L5:
        movd mm3, dword ptr [edx*4+0x12345678]
        psrlq mm2, 0x9
        punpcklbw mm3, mm5
        pmullw mm4, mm2
        psubw mm3, mm6
    L6:
        add ecx, 0x12345678
        pmullw mm3, mm2
        movd mm2, dword ptr [SpansTexturedFilteredV]
        pand mm2, qword ptr [SpansTexturedFilteredFrac]
        psraw mm4, 0x7
        psrlq mm2, 0x9
        psraw mm3, 0x7
        paddw mm6, mm3
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        paddw mm7, mm4
        psllq mm3, 0x10
        por mm2, mm3
        psubw mm6, mm7
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
    L7:
        add ebx, 0x12345678
        psllq mm3, 0x20
        mov edx, ecx
        por mm2, mm3
        mov ebp, ebx
        pmullw mm6, mm2
        mov dword ptr [SpansTexturedFilteredV], edx
        mov dword ptr [SpansTexturedFilteredU], ebp
        movd mm2, dword ptr [SpansTexturedFilteredU]
        psraw mm6, 0x7
        and edx, 0xff0000
        paddw mm7, mm6
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        psrlw mm6, 0x8
        pand mm2, qword ptr [SpansTexturedFilteredFrac]
        pmullw mm7, mm6
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        and ebp, 0xff0000
        shr edx, 0x8
        psllq mm3, 0x10
        psrlw mm7, 0x8
        add edi, 0x4
        shr ebp, 0x10
        packuswb mm7, mm7
        por mm2, mm3
        or edx, ebp
        movd dword ptr [edi-0x4], mm7
        paddw mm0, mm1
        dec eax
        jne L2
    L8:
        pop edi
        add esi, 0x20
        add edi, dword ptr [SW_SurfacePitch]
        cmp dword ptr [SW_SpanEnd], esi
        jne L1
        emms
        pop ebp
    L9:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

extern long SpansAdditiveFilteredV;
extern long SpansAdditiveFilteredU;
extern __int64 SpansAdditiveFilteredFrac;

__declspec(naked) void SW_SpanGTAddBilinear(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [SW_SpanBuffer]
        cmp esi, dword ptr [SW_SpanEnd]
        je L9
        push ebp
        mov edi, dword ptr [SW_Surface]
        mov eax, dword ptr [SW_SpanY]
        imul eax, dword ptr [SW_SurfacePitch]
        add edi, eax
        mov ebx, dword ptr [SW_SpanDU]
        mov eax, dword ptr [SW_SpanDV]
        mov edx, dword ptr [SW_TextureBits]
        mov dword ptr [L7 + 2], ebx
        mov dword ptr [L6 + 2], eax
        mov dword ptr [L2 + 4], edx
        add edx, 0x4
        mov dword ptr [L3 + 4], edx
        add edx, 0x3fc
        mov dword ptr [L4 + 4], edx
        add edx, 0x4
        mov dword ptr [L5 + 4], edx
        pxor mm5, mm5
        _emit 0x8d
        _emit 0x49
        _emit 0x00
    L1:
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [esi+0x4]
        push edi
        lea edi, [edi+ebx*4]
        or eax, eax
        jle L8
        mov ebx, dword ptr [esi+0x8]
        mov ecx, dword ptr [esi+0x10]
        movq mm1, qword ptr [SW_SpanGrad]
        movq mm0, qword ptr [esi+0x18]
        mov edx, ecx
        mov ebp, ebx
        mov dword ptr [SpansAdditiveFilteredV], edx
        mov dword ptr [SpansAdditiveFilteredU], ebp
        movd mm2, dword ptr [SpansAdditiveFilteredU]
        and edx, 0xff0000
        pand mm2, qword ptr [SpansAdditiveFilteredFrac]
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        and ebp, 0xff0000
        psllq mm3, 0x10
        shr edx, 0x8
        shr ebp, 0x10
        por mm2, mm3
        or edx, ebp
        mov edi, edi
    L2:
        movd mm7, dword ptr [edx*4+0x12345678]
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
    L3:
        movd mm4, dword ptr [edx*4+0x12345678]
        punpcklbw mm7, mm5
    L4:
        movd mm6, dword ptr [edx*4+0x12345678]
        punpcklbw mm4, mm5
        psubw mm4, mm7
        psllq mm2, 0x20
        punpcklbw mm6, mm5
        por mm2, mm3
    L5:
        movd mm3, dword ptr [edx*4+0x12345678]
        psrlq mm2, 0x9
        punpcklbw mm3, mm5
        pmullw mm4, mm2
        psubw mm3, mm6
    L6:
        add ecx, 0x12345678
        pmullw mm3, mm2
        movd mm2, dword ptr [SpansAdditiveFilteredV]
        pand mm2, qword ptr [SpansAdditiveFilteredFrac]
        psraw mm4, 0x7
        psrlq mm2, 0x9
        psraw mm3, 0x7
        paddw mm6, mm3
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        paddw mm7, mm4
        psllq mm3, 0x10
        por mm2, mm3
        psubw mm6, mm7
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
    L7:
        add ebx, 0x12345678
        psllq mm3, 0x20
        mov edx, ecx
        por mm2, mm3
        mov ebp, ebx
        pmullw mm6, mm2
        mov dword ptr [SpansAdditiveFilteredV], edx
        mov dword ptr [SpansAdditiveFilteredU], ebp
        movd mm2, dword ptr [SpansAdditiveFilteredU]
        psraw mm6, 0x7
        and edx, 0xff0000
        paddw mm7, mm6
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        psrlw mm6, 0x8
        pand mm2, qword ptr [SpansAdditiveFilteredFrac]
        pmullw mm7, mm6
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        and ebp, 0xff0000
        shr edx, 0x8
        psllq mm3, 0x10
        movd mm6, dword ptr [edi]
        psrlw mm7, 0x8
        shr ebp, 0x10
        packuswb mm7, mm7
        paddusb mm7, mm6
        por mm2, mm3
        or edx, ebp
        dec eax
        movd dword ptr [edi], mm7
        paddw mm0, mm1
        lea edi, [edi+0x4]
        jne L2
    L8:
        pop edi
        add esi, 0x20
        add edi, dword ptr [SW_SurfacePitch]
        cmp dword ptr [SW_SpanEnd], esi
        jne L1
        emms
        pop ebp
    L9:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

extern long SpansTexturedFilteredPV;
extern long SpansTexturedFilteredPU;
extern __int64 SpansTexturedFilteredPFrac;

__declspec(naked) void SW_SpanGTPBilinear(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [SW_SpanBuffer]
        cmp esi, dword ptr [SW_SpanEnd]
        je L8
        push ebp
        mov edi, dword ptr [SW_Surface]
        mov eax, dword ptr [SW_SpanY]
        imul eax, dword ptr [SW_SurfacePitch]
        add edi, eax
        mov edx, dword ptr [SW_TextureBits]
        mov dword ptr [L3 + 4], edx
        add edx, 0x4
        mov dword ptr [L4 + 4], edx
        add edx, 0x3fc
        mov dword ptr [L5 + 4], edx
        add edx, 0x4
        mov dword ptr [L6 + 4], edx
        pxor mm5, mm5
        mov edi, edi
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L1:
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [esi+0x4]
        push edi
        lea edi, [edi+ebx*4]
        add esi, 0x20
        or eax, eax
        jle L7
        mov ebx, dword ptr [esi-0x18]
        mov ecx, dword ptr [esi-0x10]
        movq mm1, qword ptr [SW_SpanGrad]
        movq mm0, qword ptr [esi-0x8]
        mov edx, ecx
        mov ebp, ebx
        mov dword ptr [SpansTexturedFilteredPV], edx
        mov dword ptr [SpansTexturedFilteredPU], ebp
        movd mm2, dword ptr [SpansTexturedFilteredPU]
        and edx, 0xff0000
        pand mm2, qword ptr [SpansTexturedFilteredPFrac]
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        and ebp, 0xff0000
        psllq mm3, 0x10
        shr edx, 0x8
        shr ebp, 0x10
        por mm2, mm3
        or edx, ebp
        nop
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L2:
        push eax
        cmp eax, 0x10
        jle L3
        mov eax, 0x10
        nop
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
    L3:
        movd mm7, dword ptr [edx*4+0x12345678]
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
    L4:
        movd mm4, dword ptr [edx*4+0x12345678]
        punpcklbw mm7, mm5
    L5:
        movd mm6, dword ptr [edx*4+0x12345678]
        punpcklbw mm4, mm5
        psubw mm4, mm7
        psllq mm2, 0x20
        punpcklbw mm6, mm5
        por mm2, mm3
    L6:
        movd mm3, dword ptr [edx*4+0x12345678]
        psrlq mm2, 0x9
        punpcklbw mm3, mm5
        pmullw mm4, mm2
        psubw mm3, mm6
        add ecx, dword ptr [esi+0x4]
        pmullw mm3, mm2
        movd mm2, dword ptr [SpansTexturedFilteredPV]
        pand mm2, qword ptr [SpansTexturedFilteredPFrac]
        psraw mm4, 0x7
        psrlq mm2, 0x9
        psraw mm3, 0x7
        paddw mm6, mm3
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        paddw mm7, mm4
        psllq mm3, 0x10
        por mm2, mm3
        psubw mm6, mm7
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        add ebx, dword ptr [esi]
        psllq mm3, 0x20
        mov edx, ecx
        por mm2, mm3
        mov ebp, ebx
        pmullw mm6, mm2
        mov dword ptr [SpansTexturedFilteredPV], edx
        mov dword ptr [SpansTexturedFilteredPU], ebp
        movd mm2, dword ptr [SpansTexturedFilteredPU]
        psraw mm6, 0x7
        and edx, 0xff0000
        paddw mm7, mm6
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        psrlw mm6, 0x8
        pand mm2, qword ptr [SpansTexturedFilteredPFrac]
        pmullw mm7, mm6
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        and ebp, 0xff0000
        shr edx, 0x8
        psllq mm3, 0x10
        psrlw mm7, 0x8
        add edi, 0x4
        shr ebp, 0x10
        packuswb mm7, mm7
        por mm2, mm3
        or edx, ebp
        movd dword ptr [edi-0x4], mm7
        paddw mm0, mm1
        dec eax
        jne L3
        pop eax
        add esi, 0x8
        sub eax, 0x10
        jg L2
        sub esi, 0x8
    L7:
        add esi, 0x8
        pop edi
        add edi, dword ptr [SW_SurfacePitch]
        cmp dword ptr [SW_SpanEnd], esi
        jne L1
        emms
        pop ebp
    L8:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

extern long SpansAdditiveFilteredPV;
extern long SpansAdditiveFilteredPU;
extern __int64 SpansAdditiveFilteredPFrac;

__declspec(naked) void SW_SpanGTPAddBilinear(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [SW_SpanBuffer]
        cmp esi, dword ptr [SW_SpanEnd]
        je L8
        push ebp
        mov edi, dword ptr [SW_Surface]
        mov eax, dword ptr [SW_SpanY]
        imul eax, dword ptr [SW_SurfacePitch]
        add edi, eax
        mov edx, dword ptr [SW_TextureBits]
        mov dword ptr [L3 + 4], edx
        add edx, 0x4
        mov dword ptr [L4 + 4], edx
        add edx, 0x3fc
        mov dword ptr [L5 + 4], edx
        add edx, 0x4
        mov dword ptr [L6 + 4], edx
        pxor mm5, mm5
        mov edi, edi
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L1:
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [esi+0x4]
        push edi
        lea edi, [edi+ebx*4]
        add esi, 0x20
        or eax, eax
        jle L7
        mov ebx, dword ptr [esi-0x18]
        mov ecx, dword ptr [esi-0x10]
        movq mm1, qword ptr [SW_SpanGrad]
        movq mm0, qword ptr [esi-0x8]
        mov edx, ecx
        mov ebp, ebx
        mov dword ptr [SpansAdditiveFilteredPV], edx
        mov dword ptr [SpansAdditiveFilteredPU], ebp
        movd mm2, dword ptr [SpansAdditiveFilteredPU]
        and edx, 0xff0000
        pand mm2, qword ptr [SpansAdditiveFilteredPFrac]
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        and ebp, 0xff0000
        psllq mm3, 0x10
        shr edx, 0x8
        shr ebp, 0x10
        por mm2, mm3
        or edx, ebp
        nop
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L2:
        push eax
        cmp eax, 0x10
        jle L3
        mov eax, 0x10
        nop
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
    L3:
        movd mm7, dword ptr [edx*4+0x12345678]
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
    L4:
        movd mm4, dword ptr [edx*4+0x12345678]
        punpcklbw mm7, mm5
    L5:
        movd mm6, dword ptr [edx*4+0x12345678]
        punpcklbw mm4, mm5
        psubw mm4, mm7
        psllq mm2, 0x20
        punpcklbw mm6, mm5
        por mm2, mm3
    L6:
        movd mm3, dword ptr [edx*4+0x12345678]
        psrlq mm2, 0x9
        punpcklbw mm3, mm5
        pmullw mm4, mm2
        psubw mm3, mm6
        add ecx, dword ptr [esi+0x4]
        pmullw mm3, mm2
        movd mm2, dword ptr [SpansAdditiveFilteredPV]
        pand mm2, qword ptr [SpansAdditiveFilteredPFrac]
        psraw mm4, 0x7
        psrlq mm2, 0x9
        psraw mm3, 0x7
        paddw mm6, mm3
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        paddw mm7, mm4
        psllq mm3, 0x10
        por mm2, mm3
        psubw mm6, mm7
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        add ebx, dword ptr [esi]
        psllq mm3, 0x20
        mov edx, ecx
        por mm2, mm3
        mov ebp, ebx
        pmullw mm6, mm2
        mov dword ptr [SpansAdditiveFilteredPV], edx
        mov dword ptr [SpansAdditiveFilteredPU], ebp
        movd mm2, dword ptr [SpansAdditiveFilteredPU]
        psraw mm6, 0x7
        and edx, 0xff0000
        paddw mm7, mm6
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        psrlw mm6, 0x8
        pand mm2, qword ptr [SpansAdditiveFilteredPFrac]
        pmullw mm7, mm6
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        and ebp, 0xff0000
        shr edx, 0x8
        psllq mm3, 0x10
        movd mm6, dword ptr [edi]
        psrlw mm7, 0x8
        shr ebp, 0x10
        packuswb mm7, mm7
        por mm2, mm3
        paddusb mm7, mm6
        or edx, ebp
        dec eax
        movd dword ptr [edi], mm7
        paddw mm0, mm1
        lea edi, [edi+0x4]
        jne L3
        pop eax
        add esi, 0x8
        sub eax, 0x10
        jg L2
        sub esi, 0x8
    L7:
        add esi, 0x8
        pop edi
        add edi, dword ptr [SW_SurfacePitch]
        cmp dword ptr [SW_SpanEnd], esi
        jne L1
        emms
        pop ebp
    L8:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

extern long SpansColorKeyFilteredV;
extern long SpansColorKeyFilteredU;
extern __int64 SpansColorKeyFilteredFrac;
extern __int64 SpansColorKeyFilteredKeyMask;
extern __int64 SpansColorKeyFilteredInvert;

__declspec(naked) void DrawSpansColorKeyFiltered(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [SW_SpanBuffer]
        cmp esi, dword ptr [SW_SpanEnd]
        je L10
        push ebp
        mov edi, dword ptr [SW_Surface]
        mov eax, dword ptr [SW_SpanY]
        imul eax, dword ptr [SW_SurfacePitch]
        add edi, eax
        mov ebx, dword ptr [SW_SpanDU]
        mov eax, dword ptr [SW_SpanDV]
        mov edx, dword ptr [SW_TextureBits]
        mov dword ptr [L8 + 2], ebx
        mov dword ptr [L3 + 2], eax
        mov dword ptr [L4 + 4], edx
        add edx, 0x4
        mov dword ptr [L5 + 4], edx
        add edx, 0x3fc
        mov dword ptr [L6 + 4], edx
        add edx, 0x4
        mov dword ptr [L7 + 4], edx
        pxor mm5, mm5
        _emit 0x8d
        _emit 0x49
        _emit 0x00
    L1:
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [esi+0x4]
        push edi
        lea edi, [edi+ebx*4]
        or eax, eax
        jle L9
        mov ebx, dword ptr [esi+0x8]
        mov ecx, dword ptr [esi+0x10]
        movq mm1, qword ptr [SW_SpanGrad]
        movq mm0, qword ptr [esi+0x18]
        mov edx, ecx
        mov ebp, ebx
        mov dword ptr [SpansColorKeyFilteredV], edx
        mov dword ptr [SpansColorKeyFilteredU], ebp
        and edx, 0xff0000
        and ebp, 0xff0000
        shr edx, 0x8
        shr ebp, 0x10
        or edx, ebp
        movd mm2, dword ptr [SpansColorKeyFilteredU]
        pand mm2, qword ptr [SpansColorKeyFilteredFrac]
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        psllq mm3, 0x10
        por mm2, mm3
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        nop
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L2:
        psllq mm2, 0x20
    L3:
        add ecx, 0x12345678
        por mm2, mm3
    L4:
        movd mm7, dword ptr [edx*4+0x12345678]
        psrlq mm2, 0x9
    L5:
        movd mm4, dword ptr [edx*4+0x12345678]
        punpcklbw mm7, mm5
    L6:
        movd mm6, dword ptr [edx*4+0x12345678]
        punpcklbw mm4, mm5
    L7:
        movd mm3, dword ptr [edx*4+0x12345678]
        punpcklbw mm6, mm5
        punpcklbw mm3, mm5
        psubw mm4, mm7
        psubw mm3, mm6
        pmullw mm4, mm2
        pmullw mm3, mm2
        movd mm2, dword ptr [SpansColorKeyFilteredV]
        pand mm2, qword ptr [SpansColorKeyFilteredFrac]
        psraw mm4, 0x7
        psrlq mm2, 0x9
        psraw mm3, 0x7
        paddw mm6, mm3
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        paddw mm7, mm4
        psllq mm3, 0x10
        por mm2, mm3
        psubw mm6, mm7
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
    L8:
        add ebx, 0x12345678
        psllq mm3, 0x20
        mov edx, ecx
        por mm2, mm3
        mov ebp, ebx
        pmullw mm6, mm2
        mov dword ptr [SpansColorKeyFilteredV], edx
        mov dword ptr [SpansColorKeyFilteredU], ebp
        and edx, 0xff0000
        shr edx, 0x8
        and ebp, 0xff0000
        shr ebp, 0x10
        psraw mm6, 0x7
        paddw mm7, mm6
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        _emit 0x0f
        _emit 0x7f
        _emit 0xfa
        psrlw mm6, 0x8
        pand mm2, qword ptr [SpansColorKeyFilteredKeyMask]
        pmullw mm7, mm6
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        _emit 0x0f
        _emit 0x7f
        _emit 0xd4
        psrlq mm2, 0x10
        psrlq mm3, 0x20
        movq mm6, qword ptr [edi]
        psrlq mm4, 0x30
        por mm4, mm3
        punpcklbw mm6, mm5
        por mm4, mm2
        psrlw mm7, 0x8
        pmullw mm7, mm4
        pxor mm4, qword ptr [SpansColorKeyFilteredInvert]
        movd mm2, dword ptr [SpansColorKeyFilteredU]
        pmullw mm6, mm4
        pand mm2, qword ptr [SpansColorKeyFilteredFrac]
        or edx, ebp
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        paddw mm7, mm6
        psllq mm3, 0x10
        psrlw mm7, 0x8
        packuswb mm7, mm7
        por mm2, mm3
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        dec eax
        movd dword ptr [edi], mm7
        paddw mm0, mm1
        lea edi, [edi+0x4]
        jne L2
    L9:
        add esi, 0x20
        pop edi
        add edi, dword ptr [SW_SurfacePitch]
        cmp dword ptr [SW_SpanEnd], esi
        jne L1
        emms
        pop ebp
    L10:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

extern long SpansColorKeyFilteredPV;
extern long SpansColorKeyFilteredPU;
extern __int64 SpansColorKeyFilteredPFrac;
extern __int64 SpansColorKeyFilteredPKeyMask;
extern __int64 SpansColorKeyFilteredPInvert;

__declspec(naked) void DrawSpansColorKeyFilteredP(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [SW_SpanBuffer]
        cmp esi, dword ptr [SW_SpanEnd]
        je L9
        push ebp
        mov edi, dword ptr [SW_Surface]
        mov eax, dword ptr [SW_SpanY]
        imul eax, dword ptr [SW_SurfacePitch]
        add edi, eax
        mov edx, dword ptr [SW_TextureBits]
        mov dword ptr [L4 + 4], edx
        add edx, 0x4
        mov dword ptr [L5 + 4], edx
        add edx, 0x3fc
        mov dword ptr [L6 + 4], edx
        add edx, 0x4
        mov dword ptr [L7 + 4], edx
        pxor mm5, mm5
        mov edi, edi
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L1:
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [esi+0x4]
        push edi
        lea edi, [edi+ebx*4]
        add esi, 0x20
        test eax, eax
        jle L8
        mov ebx, dword ptr [esi-0x18]
        mov ecx, dword ptr [esi-0x10]
        movq mm1, qword ptr [SW_SpanGrad]
        movq mm0, qword ptr [esi-0x8]
        mov edx, ecx
        mov ebp, ebx
        mov dword ptr [SpansColorKeyFilteredPV], edx
        mov dword ptr [SpansColorKeyFilteredPU], ebp
        and edx, 0xff0000
        and ebp, 0xff0000
        shr edx, 0x8
        shr ebp, 0x10
        or edx, ebp
        movd mm2, dword ptr [SpansColorKeyFilteredPU]
        pand mm2, qword ptr [SpansColorKeyFilteredPFrac]
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        psllq mm3, 0x10
        por mm2, mm3
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L2:
        push eax
        cmp eax, 0x10
        jle L3
        mov eax, 0x10
        nop
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
    L3:
        psllq mm2, 0x20
        add ecx, dword ptr [esi+0x4]
        por mm2, mm3
    L4:
        movd mm7, dword ptr [edx*4+0x12345678]
        psrlq mm2, 0x9
    L5:
        movd mm4, dword ptr [edx*4+0x12345678]
        punpcklbw mm7, mm5
    L6:
        movd mm6, dword ptr [edx*4+0x12345678]
        punpcklbw mm4, mm5
    L7:
        movd mm3, dword ptr [edx*4+0x12345678]
        punpcklbw mm6, mm5
        punpcklbw mm3, mm5
        psubw mm4, mm7
        psubw mm3, mm6
        pmullw mm4, mm2
        pmullw mm3, mm2
        movd mm2, dword ptr [SpansColorKeyFilteredPV]
        pand mm2, qword ptr [SpansColorKeyFilteredPFrac]
        psraw mm4, 0x7
        psrlq mm2, 0x9
        psraw mm3, 0x7
        paddw mm6, mm3
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        paddw mm7, mm4
        psllq mm3, 0x10
        por mm2, mm3
        psubw mm6, mm7
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        add ebx, dword ptr [esi]
        psllq mm3, 0x20
        mov edx, ecx
        por mm2, mm3
        mov ebp, ebx
        pmullw mm6, mm2
        mov dword ptr [SpansColorKeyFilteredPV], edx
        mov dword ptr [SpansColorKeyFilteredPU], ebp
        and edx, 0xff0000
        shr edx, 0x8
        and ebp, 0xff0000
        shr ebp, 0x10
        psraw mm6, 0x7
        paddw mm7, mm6
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        _emit 0x0f
        _emit 0x7f
        _emit 0xfa
        psrlw mm6, 0x8
        pand mm2, qword ptr [SpansColorKeyFilteredPKeyMask]
        pmullw mm7, mm6
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        _emit 0x0f
        _emit 0x7f
        _emit 0xd4
        psrlq mm2, 0x10
        psrlq mm3, 0x20
        movq mm6, qword ptr [edi]
        psrlq mm4, 0x30
        por mm4, mm3
        punpcklbw mm6, mm5
        por mm4, mm2
        psrlw mm7, 0x8
        pmullw mm7, mm4
        pxor mm4, qword ptr [SpansColorKeyFilteredPInvert]
        movd mm2, dword ptr [SpansColorKeyFilteredPU]
        pmullw mm6, mm4
        pand mm2, qword ptr [SpansColorKeyFilteredPFrac]
        or edx, ebp
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        paddw mm7, mm6
        psllq mm3, 0x10
        psrlw mm7, 0x8
        packuswb mm7, mm7
        por mm2, mm3
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        dec eax
        movd dword ptr [edi], mm7
        paddw mm0, mm1
        lea edi, [edi+0x4]
        jne L3
        pop eax
        add esi, 0x8
        sub eax, 0x10
        jg L2
        sub esi, 0x8
    L8:
        add esi, 0x8
        pop edi
        add edi, dword ptr [SW_SurfacePitch]
        cmp dword ptr [SW_SpanEnd], esi
        jne L1
        emms
        pop ebp
    L9:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

extern long AlphaVertex;
extern long SpansAlphaDest;
extern long SpansAlphaDestHi;
extern __int64 SpansAlphaInvert;
extern __int64 SpansAlphaSrc;

__declspec(naked) void DrawSpansAlpha(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [SW_SpanBuffer]
        cmp esi, dword ptr [SW_SpanEnd]
        je L4
        push ebp
        mov edi, dword ptr [SW_Surface]
        mov eax, dword ptr [SW_SpanY]
        imul eax, dword ptr [SW_SurfacePitch]
        add edi, eax
        mov ebx, dword ptr [AlphaVertex]
        mov eax, dword ptr [ebx+0x10]
        and eax, 0xff000000
        xor eax, 0xff000000
        mov ecx, eax
        shr eax, 0x8
        shr ecx, 0x18
        or eax, ecx
        mov dword ptr [SpansAlphaDest], eax
        mov dword ptr [SpansAlphaDestHi], eax
        movq mm0, qword ptr [SpansAlphaDest]
        pxor mm0, qword ptr [SpansAlphaInvert]
        movq qword ptr [SpansAlphaSrc], mm0
        pxor mm4, mm4
        movq mm5, qword ptr [SpansAlphaDest]
        movq mm7, qword ptr [SpansAlphaSrc]
    L1:
        mov ebx, dword ptr [esi]
        mov eax, dword ptr [esi+0x4]
        shl ebx, 0x2
        push edi
        add edi, ebx
        or eax, eax
        jle L3
        movq mm0, qword ptr [esi+0x18]
        movq mm1, qword ptr [SW_SpanGrad]
    L2:
        movd mm6, dword ptr [edi]
        _emit 0x0f
        _emit 0x7f
        _emit 0xc3
        punpcklbw mm6, mm4
        psrlw mm3, 0x8
        pmullw mm6, mm5
        add edi, 0x4
        pmullw mm3, mm7
        paddw mm0, mm1
        paddsw mm6, mm3
        psrlw mm6, 0x8
        packuswb mm6, mm6
        dec eax
        movd dword ptr [edi-0x4], mm6
        jne L2
    L3:
        pop edi
        add edi, dword ptr [SW_SurfacePitch]
        add esi, 0x20
        cmp dword ptr [SW_SpanEnd], esi
        jne L1
        emms
        pop ebp
    L4:
        pop edi
        pop esi
        pop ebx
        ret
    }
}

extern double MagicFix16;
extern long RoundBias;
extern double MagicInt;

#if FEAT_C_ASM
extern long RecipTable[];

#define SW_F(x) (*(float *)&(x))
#define SW_VF(x) (*(volatile float *)&(x))
#define SW_L(x) (*(long *)&(x))

static long SW_RoundL(double d)
{
    double t;

    if (d >= -2147483648.5 && d < 2147483647.5) {
        t = d + 6755399441055744.0;
        return *(long *)&t;
    }
    return 0x80000000;
}

static short SW_RoundW(double d)
{
    double t;

    if (d >= -32768.5 && d < 32767.5) {
        t = d + 6755399441055744.0;
        return *(short *)&t;
    }
    return -32768;
}

static void SW_SetF(long *p, double d)
{
    *(float *)p = (float)d;
}

static float SW_ToF(double d)
{
    float f;

    SW_SetF((long *)&f, d);
    return f;
}

static long SW_Recip(long w)
{
    return ((w ^ 0x7f800000) & 0xff800000) + RecipTable[((unsigned long)w >> 11) & 0xfff];
}
#endif

#ifndef FEAT_C_ASM
__declspec(naked) void MMXDrawLine(D3DTLVERTEX *v1, D3DTLVERTEX *v2)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0x70
        push ebx
        push esi
        push edi
        mov ebx, dword ptr [ebp+0x8]
        mov ecx, dword ptr [ebp+0xc]
        fld dword ptr [ebx]
        fsub dword ptr [ecx]
        fstp dword ptr [ebp-0x30]
        fld dword ptr [ebx+0x4]
        fsub dword ptr [ecx+0x4]
        fstp dword ptr [ebp-0x2c]
        mov eax, dword ptr [ebp-0x30]
        mov edx, dword ptr [ebp-0x2c]
        and eax, 0x7fffffff
        and edx, 0x7fffffff
        cmp eax, edx
        jb L3
        mov eax, dword ptr [ebp-0x30]
        test eax, eax
        jns L1
        xor dword ptr [ebp-0x30], 0x80000000
        xor dword ptr [ebp-0x2c], 0x80000000
        xchg ebx, ecx
    L1:
        fld1
        fdiv dword ptr [ebp-0x30]
        mov esi, dword ptr [ebx+0x10]
        mov edi, esi
        mov eax, esi
        shl esi, 0x8
        and edi, 0xff00
        shr eax, 0x8
        and esi, 0xff00
        and eax, 0xff00
        add edi, 0x80
        add esi, 0x80
        add eax, 0x80
        mov dword ptr [ebp-0x28], edi
        mov dword ptr [ebp-0x24], esi
        mov dword ptr [ebp-0x20], eax
        mov esi, dword ptr [ecx+0x10]
        mov edi, esi
        mov eax, esi
        shl esi, 0x8
        and edi, 0xff00
        shr eax, 0x8
        and esi, 0xff00
        and eax, 0xff00
        add edi, 0x80
        add esi, 0x80
        add eax, 0x80
        mov dword ptr [ebp-0x1c], edi
        mov dword ptr [ebp-0x18], esi
        mov dword ptr [ebp-0x14], eax
        fstp dword ptr [ebp-0x10]
        fld dword ptr [ebp-0x2c]
        fmul dword ptr [ebp-0x10]
        fst dword ptr [ebp-0xc]
        fadd qword ptr [MagicFix16]
        fstp qword ptr [ebp-0x70]
        fld dword ptr [ecx]
        fsub dword ptr [RoundBias]
        fadd qword ptr [MagicInt]
        fstp qword ptr [ebp-0x68]
        inc dword ptr [ebp-0x68]
        fld dword ptr [ebx]
        fsub dword ptr [RoundBias]
        fadd qword ptr [MagicInt]
        fstp qword ptr [ebp-0x60]
        inc dword ptr [ebp-0x60]
        fild dword ptr [ebp-0x68]
        fsub dword ptr [ecx]
        fmul dword ptr [ebp-0xc]
        fadd dword ptr [ecx+0x4]
        fadd qword ptr [MagicFix16]
        fstp qword ptr [ebp-0x58]
        fild dword ptr [ebp-0x24]
        fisub dword ptr [ebp-0x18]
        fmul dword ptr [ebp-0x10]
        fst dword ptr [ebp-0x8]
        fistp dword ptr [ebp-0x50]
        fild dword ptr [ebp-0x68]
        fsub dword ptr [ecx]
        fmul dword ptr [ebp+0xc]
        fiadd dword ptr [ebp-0x18]
        fistp dword ptr [ebp-0x48]
        fild dword ptr [ebp-0x28]
        fisub dword ptr [ebp-0x1c]
        fmul dword ptr [ebp-0x10]
        fst dword ptr [ebp-0x4]
        fistp dword ptr [ebp-0x4e]
        fild dword ptr [ebp-0x68]
        fsub dword ptr [ecx]
        fmul dword ptr [ebp-0x4]
        fiadd dword ptr [ebp-0x1c]
        fistp dword ptr [ebp-0x46]
        fild dword ptr [ebp-0x20]
        fisub dword ptr [ebp-0x14]
        fmul dword ptr [ebp-0x10]
        fst dword ptr [ebp-0x8]
        fistp dword ptr [ebp-0x4c]
        fild dword ptr [ebp-0x68]
        fsub dword ptr [ecx]
        fmul dword ptr [ebp-0x8]
        fiadd dword ptr [ebp-0x14]
        fistp dword ptr [ebp-0x44]
        mov edi, dword ptr [SW_Surface]
        mov eax, dword ptr [ebp-0x68]
        lea edi, [edi+eax*4]
        mov edx, dword ptr [ebp-0x60]
        sub edx, dword ptr [ebp-0x68]
        dec edx
        jle L6
        mov ecx, dword ptr [ebp-0x70]
        mov ebx, dword ptr [ebp-0x58]
        movq mm0, qword ptr [ebp-0x48]
        movq mm1, qword ptr [ebp-0x50]
        pxor mm3, mm3
    L2:
        _emit 0x0f
        _emit 0x7f
        _emit 0xc2
        paddw mm0, mm1
        mov eax, ebx
        psrlw mm2, 0x8
        packuswb mm2, mm3
        sar eax, 0x10
        push edx
        imul eax, dword ptr [SW_SurfacePitch]
        pop edx
        movd dword ptr [edi+eax*1], mm2
        add edi, 0x4
        add ebx, ecx
        dec edx
        jge L2
        jmp L6
    L3:
        mov eax, dword ptr [ebp-0x2c]
        test eax, eax
        jns L4
        xor dword ptr [ebp-0x30], 0x80000000
        xor dword ptr [ebp-0x2c], 0x80000000
        xchg ebx, ecx
    L4:
        fld1
        fdiv dword ptr [ebp-0x2c]
        mov esi, dword ptr [ebx+0x10]
        mov edi, esi
        mov eax, esi
        shl esi, 0x8
        and edi, 0xff00
        shr eax, 0x8
        and esi, 0xff00
        and eax, 0xff00
        add edi, 0x80
        add esi, 0x80
        add eax, 0x80
        mov dword ptr [ebp-0x28], edi
        mov dword ptr [ebp-0x24], esi
        mov dword ptr [ebp-0x20], eax
        mov esi, dword ptr [ecx+0x10]
        mov edi, esi
        mov eax, esi
        shl esi, 0x8
        and edi, 0xff00
        shr eax, 0x8
        and esi, 0xff00
        and eax, 0xff00
        add edi, 0x80
        add esi, 0x80
        add eax, 0x80
        mov dword ptr [ebp-0x1c], edi
        mov dword ptr [ebp-0x18], esi
        mov dword ptr [ebp-0x14], eax
        fstp dword ptr [ebp-0x10]
        fld dword ptr [ebp-0x30]
        fmul dword ptr [ebp-0x10]
        fst dword ptr [ebp-0xc]
        fadd qword ptr [MagicFix16]
        fstp qword ptr [ebp-0x70]
        fld dword ptr [ecx]
        fadd qword ptr [MagicFix16]
        fstp qword ptr [ebp-0x58]
        fld dword ptr [ecx+0x4]
        fsub dword ptr [RoundBias]
        fadd qword ptr [MagicInt]
        fstp qword ptr [ebp-0x40]
        inc dword ptr [ebp-0x40]
        fld dword ptr [ebx+0x4]
        fsub dword ptr [RoundBias]
        fadd qword ptr [MagicInt]
        fstp qword ptr [ebp-0x38]
        inc dword ptr [ebp-0x38]
        fild dword ptr [ebp-0x24]
        fisub dword ptr [ebp-0x18]
        fmul dword ptr [ebp-0x10]
        fst dword ptr [ebp-0x8]
        fistp dword ptr [ebp-0x50]
        fild dword ptr [ebp-0x40]
        fsub dword ptr [ecx+0x4]
        fmul dword ptr [ebp+0xc]
        fiadd dword ptr [ebp-0x18]
        fistp dword ptr [ebp-0x48]
        fild dword ptr [ebp-0x28]
        fisub dword ptr [ebp-0x1c]
        fmul dword ptr [ebp-0x10]
        fst dword ptr [ebp-0x4]
        fistp dword ptr [ebp-0x4e]
        fild dword ptr [ebp-0x40]
        fsub dword ptr [ecx+0x4]
        fmul dword ptr [ebp-0x4]
        fiadd dword ptr [ebp-0x1c]
        fistp dword ptr [ebp-0x46]
        fild dword ptr [ebp-0x20]
        fisub dword ptr [ebp-0x14]
        fmul dword ptr [ebp-0x10]
        fst dword ptr [ebp-0x8]
        fistp dword ptr [ebp-0x4c]
        fild dword ptr [ebp-0x40]
        fsub dword ptr [ecx+0x4]
        fmul dword ptr [ebp-0x8]
        fiadd dword ptr [ebp-0x14]
        fistp dword ptr [ebp-0x44]
        mov edi, dword ptr [SW_Surface]
        mov eax, dword ptr [ebp-0x40]
        imul eax, dword ptr [SW_SurfacePitch]
        add edi, eax
        mov edx, dword ptr [ebp-0x38]
        sub edx, dword ptr [ebp-0x40]
        dec edx
        jle L6
        mov ecx, dword ptr [ebp-0x70]
        mov ebx, dword ptr [ebp-0x58]
        movq mm0, qword ptr [ebp-0x48]
        movq mm1, qword ptr [ebp-0x50]
        pxor mm3, mm3
    L5:
        mov eax, ebx
        _emit 0x0f
        _emit 0x7f
        _emit 0xc2
        paddw mm0, mm1
        shr eax, 0x10
        psrlw mm2, 0x8
        packuswb mm2, mm3
        movd dword ptr [edi+eax*4], mm2
        add edi, dword ptr [SW_SurfacePitch]
        add ebx, ecx
        dec edx
        jge L5
    L6:
        emms
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}
#else
void MMXDrawLine(D3DTLVERTEX *v1, D3DTLVERTEX *v2)
{
    D3DTLVERTEX *a, *b, *t;
    unsigned char *p;
    double d, mag;
    float dx, dy, inv, slope, gg, gr;
    unsigned long ca, cb, w[4], s[4];
    long ba, ga, ra, bb, gg2, rb, i0, i1, n, pos, step, ds[3], st[3];

    a = v1;
    b = v2;
    dx = SW_ToF((double)a->sx - b->sx);
    dy = SW_ToF((double)a->sy - b->sy);
    if ((*(unsigned long *)&dx & 0x7fffffff) >= (*(unsigned long *)&dy & 0x7fffffff)) {
        if (*(long *)&dx < 0) {
            *(unsigned long *)&dx ^= 0x80000000;
            *(unsigned long *)&dy ^= 0x80000000;
            t = a;
            a = b;
            b = t;
        }
        inv = SW_ToF(1.0 / dx);
    } else {
        if (*(long *)&dy < 0) {
            *(unsigned long *)&dx ^= 0x80000000;
            *(unsigned long *)&dy ^= 0x80000000;
            t = a;
            a = b;
            b = t;
        }
        inv = SW_ToF(1.0 / dy);
    }
    ca = a->color;
    ga = (ca & 0xff00) + 0x80;
    ba = ((ca << 8) & 0xff00) + 0x80;
    ra = ((ca >> 8) & 0xff00) + 0x80;
    cb = b->color;
    gg2 = (cb & 0xff00) + 0x80;
    bb = ((cb << 8) & 0xff00) + 0x80;
    rb = ((cb >> 8) & 0xff00) + 0x80;
    if ((*(unsigned long *)&dx & 0x7fffffff) >= (*(unsigned long *)&dy & 0x7fffffff)) {
        d = (double)dy * inv;
        slope = SW_ToF(d);
        d = d + MagicFix16;
        step = SW_L(d);
        d = ((double)b->sx - SW_VF(RoundBias)) + MagicInt;
        i0 = SW_L(d) + 1;
        d = ((double)a->sx - SW_VF(RoundBias)) + MagicInt;
        i1 = SW_L(d) + 1;
        mag = (double)i0 - b->sx;
        d = (mag * slope + b->sy) + MagicFix16;
        pos = SW_L(d);
    } else {
        d = (double)dx * inv;
        slope = SW_ToF(d);
        d = d + MagicFix16;
        step = SW_L(d);
        d = (double)b->sx + MagicFix16;
        pos = SW_L(d);
        d = ((double)b->sy - SW_VF(RoundBias)) + MagicInt;
        i0 = SW_L(d) + 1;
        d = ((double)a->sy - SW_VF(RoundBias)) + MagicInt;
        i1 = SW_L(d) + 1;
        mag = (double)i0 - b->sy;
    }
    d = ((double)ba - bb) * inv;
    ds[0] = SW_RoundL(d);
    st[0] = SW_RoundL(mag * *(float *)&v2 + bb);
    d = ((double)ga - gg2) * inv;
    gg = SW_ToF(d);
    ds[1] = SW_RoundL(d);
    st[1] = SW_RoundL(mag * gg + gg2);
    d = ((double)ra - rb) * inv;
    gr = SW_ToF(d);
    ds[2] = SW_RoundL(d);
    st[2] = SW_RoundL(mag * gr + rb);
    s[0] = ds[0] & 0xffff;
    s[1] = ds[1] & 0xffff;
    s[2] = ds[2] & 0xffff;
    s[3] = (unsigned long)ds[2] >> 16;
    w[0] = st[0] & 0xffff;
    w[1] = st[1] & 0xffff;
    w[2] = st[2] & 0xffff;
    w[3] = (unsigned long)st[2] >> 16;
    n = i1 - i0 - 1;
    if (n <= 0)
        return;
    if ((*(unsigned long *)&dx & 0x7fffffff) >= (*(unsigned long *)&dy & 0x7fffffff)) {
        p = (unsigned char *)SW_Surface + i0 * 4;
        do {
            *(unsigned long *)(p + (pos >> 16) * SW_SurfacePitch) = (w[0] >> 8) | (w[1] >> 8) << 8 | (w[2] >> 8) << 16 | (w[3] >> 8) << 24;
            w[0] = (w[0] + s[0]) & 0xffff;
            w[1] = (w[1] + s[1]) & 0xffff;
            w[2] = (w[2] + s[2]) & 0xffff;
            w[3] = (w[3] + s[3]) & 0xffff;
            p += 4;
            pos += step;
        } while (--n >= 0);
    } else {
        p = (unsigned char *)SW_Surface + i0 * SW_SurfacePitch;
        do {
            *(unsigned long *)(p + ((unsigned long)pos >> 16) * 4) = (w[0] >> 8) | (w[1] >> 8) << 8 | (w[2] >> 8) << 16 | (w[3] >> 8) << 24;
            w[0] = (w[0] + s[0]) & 0xffff;
            w[1] = (w[1] + s[1]) & 0xffff;
            w[2] = (w[2] + s[2]) & 0xffff;
            w[3] = (w[3] + s[3]) & 0xffff;
            p += SW_SurfacePitch;
            pos += step;
        } while (--n >= 0);
    }
}
#endif

extern long TriColor;
extern double MagicFloat;
extern void (*pDrawSpans)(void);
extern long Fix16Scale;

#ifndef FEAT_C_ASM
__declspec(naked) void MMXDrawTriangleFog(D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3)
{
    __asm {
        push ebp
        mov ebp, esp
        sub esp, 0xa4
        push ebx
        push esi
        push edi
        mov ebx, dword ptr [ebp+0x8]
        mov ecx, dword ptr [ebp+0xc]
        mov edx, dword ptr [ebp+0x10]
        mov dword ptr [AlphaVertex], ebx
        fld dword ptr [ebx]
        fsub dword ptr [ecx]
        fld dword ptr [ebx+0x4]
        fsub dword ptr [edx+0x4]
        fxch st(1)
        fst dword ptr [ebp-0x68]
        fxch st(1)
        fst dword ptr [ebp-0x9c]
        fmulp st(1), st
        fld dword ptr [ebx]
        fsub dword ptr [edx]
        fld dword ptr [ebx+0x4]
        fsub dword ptr [ecx+0x4]
        fxch st(1)
        fst dword ptr [ebp-0x80]
        fxch st(1)
        fst dword ptr [ebp-0x94]
        fmulp st(1), st
        fsubp st(1), st
        fst dword ptr [ebp-0x64]
        fld1
        test dword ptr [ebp-0x64], 0x80000000
        jne L5
        fdivrp st(1), st
        mov dword ptr [ebx+0x8], ecx
        mov dword ptr [ecx+0x8], edx
        mov dword ptr [edx+0x8], ebx
        mov dword ptr [ebx+0x14], edx
        mov dword ptr [ecx+0x14], ebx
        mov dword ptr [edx+0x14], ecx
        mov esi, ebx
        mov eax, dword ptr [ebx+0x4]
        cmp eax, dword ptr [ecx+0x4]
        jb L1
        mov eax, dword ptr [ecx+0x4]
        mov esi, ecx
    L1:
        cmp eax, dword ptr [edx+0x4]
        jb L2
        mov esi, edx
    L2:
        mov dword ptr [ebp-0x3c], esi
        mov dword ptr [ebp-0x1c], esi
        mov eax, dword ptr [ebx+0x10]
        mov dword ptr [TriColor], eax
        mov esi, dword ptr [ebx+0x10]
        mov edi, esi
        mov eax, esi
        shl esi, 0x8
        and edi, 0xff00
        shr eax, 0x8
        and esi, 0xff00
        and eax, 0xff00
        add edi, 0x80
        add esi, 0x80
        add eax, 0x80
        mov dword ptr [ebp-0x2c], edi
        mov dword ptr [ebp-0xa4], esi
        mov dword ptr [ebp-0x8], eax
        mov esi, dword ptr [ecx+0x10]
        mov edi, esi
        mov eax, esi
        shl esi, 0x8
        and edi, 0xff00
        shr eax, 0x8
        and esi, 0xff00
        and eax, 0xff00
        add edi, 0x80
        add esi, 0x80
        add eax, 0x80
        mov dword ptr [ebp-0x34], edi
        mov dword ptr [ebp-0x4], esi
        mov dword ptr [ebp-0x18], eax
        mov esi, dword ptr [edx+0x10]
        mov edi, esi
        mov eax, esi
        shl esi, 0x8
        and edi, 0xff00
        shr eax, 0x8
        and esi, 0xff00
        and eax, 0xff00
        add edi, 0x80
        add esi, 0x80
        add eax, 0x80
        mov dword ptr [ebp-0x4c], edi
        mov dword ptr [ebp-0x14], esi
        mov dword ptr [ebp-0x28], eax
        fild dword ptr [ebp-0x8]
        fild dword ptr [ebp-0x18]
        fild dword ptr [ebp-0x28]
        fxch st(1)
        fsubr st, st(2)
        fxch st(2)
        fsubrp st(1), st
        fxch st(1)
        fst dword ptr [ebp-0x38]
        fmul dword ptr [ebp-0x9c]
        fxch st(1)
        fst dword ptr [ebp-0x50]
        fmul dword ptr [ebp-0x94]
        fld dword ptr [ebp-0x38]
        fmul dword ptr [ebp-0x80]
        fxch st(1)
        fsubp st(2), st
        fld dword ptr [ebp-0x50]
        fmul dword ptr [ebp-0x68]
        fxch st(2)
        fmul st, st(3)
        fist word ptr [SW_SpanGrad + 4]
        fxch st(1)
        fsubp st(2), st
        fstp dword ptr [ebp-0x30]
        fmul st, st(1)
        fld dword ptr [ebx]
        fmul dword ptr [ebp-0x30]
        fld dword ptr [ebx+0x4]
        fmul st, st(2)
        fxch st(2)
        fstp dword ptr [ebp-0x48]
        fild dword ptr [ebp-0x8]
        fxch st(1)
        faddp st(2), st
        fadd qword ptr [MagicFloat]
        fsubrp st(1), st
        fstp qword ptr [ebp-0x58]
        fild dword ptr [ebp-0x2c]
        fild dword ptr [ebp-0x34]
        fild dword ptr [ebp-0x4c]
        fxch st(1)
        fsubr st, st(2)
        fxch st(2)
        fsubrp st(1), st
        fxch st(1)
        fst dword ptr [ebp-0x38]
        fmul dword ptr [ebp-0x9c]
        fxch st(1)
        fst dword ptr [ebp-0x50]
        fmul dword ptr [ebp-0x94]
        fld dword ptr [ebp-0x38]
        fmul dword ptr [ebp-0x80]
        fxch st(1)
        fsubp st(2), st
        fld dword ptr [ebp-0x50]
        fmul dword ptr [ebp-0x68]
        fxch st(2)
        fmul st, st(3)
        fist word ptr [SW_SpanGrad + 2]
        fxch st(1)
        fsubp st(2), st
        fstp dword ptr [ebp-0x90]
        fmul st, st(1)
        fld dword ptr [ebx]
        fmul dword ptr [ebp-0x90]
        fld dword ptr [ebx+0x4]
        fmul st, st(2)
        fxch st(2)
        fstp dword ptr [ebp-0x98]
        fild dword ptr [ebp-0x2c]
        fxch st(1)
        faddp st(2), st
        fadd qword ptr [MagicFloat]
        fsubrp st(1), st
        fstp qword ptr [ebp-0x24]
        fild dword ptr [ebp-0xa4]
        fild dword ptr [ebp-0x4]
        fild dword ptr [ebp-0x14]
        fxch st(1)
        fsubr st, st(2)
        fxch st(2)
        fsubrp st(1), st
        fxch st(1)
        fst dword ptr [ebp-0x38]
        fmul dword ptr [ebp-0x9c]
        fxch st(1)
        fst dword ptr [ebp-0x50]
        fmul dword ptr [ebp-0x94]
        fld dword ptr [ebp-0x38]
        fmul dword ptr [ebp-0x80]
        fxch st(1)
        fsubp st(2), st
        fld dword ptr [ebp-0x50]
        fmul dword ptr [ebp-0x68]
        fxch st(2)
        fmul st, st(3)
        fist word ptr [SW_SpanGrad]
        fxch st(1)
        fsubp st(2), st
        fstp dword ptr [ebp-0x88]
        fmul st, st(1)
        fld dword ptr [ebx]
        fmul dword ptr [ebp-0x88]
        fld dword ptr [ebx+0x4]
        fmul st, st(2)
        fxch st(2)
        fstp dword ptr [ebp-0x8c]
        fild dword ptr [ebp-0xa4]
        fxch st(1)
        faddp st(2), st
        fadd qword ptr [MagicFloat]
        fsubrp st(1), st
        fstp qword ptr [ebp-0x7c]
        faddp st(0), st
        mov esi, dword ptr [ebp-0x3c]
        mov eax, 0x3
        mov dword ptr [ebp-0x84], eax
        mov edi, dword ptr [SW_SpanBuffer]
        fld dword ptr [esi+0x4]
        fsub dword ptr [RoundBias]
        fadd qword ptr [MagicInt]
        fstp qword ptr [ebp-0x70]
        inc dword ptr [ebp-0x70]
        mov eax, dword ptr [ebp-0x70]
        xor edx, edx
        mov dword ptr [SW_SpanY], eax
        jmp L6
        mov edi, edi
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L3:
        mov eax, dword ptr [edi]
        push ebx
        shr ebx, 0x10
        mov eax, esi
        shr eax, 0x10
        mov dword ptr [ebp-0x64], ebx
        sub eax, ebx
        fild dword ptr [ebp-0x70]
        fild dword ptr [ebp-0x64]
        mov dword ptr [edi+0x4], eax
        mov dword ptr [edi], ebx
        fld dword ptr [ebp-0x88]
        fmul st, st(1)
        fld dword ptr [ebp-0x8c]
        fmul st, st(3)
        fld dword ptr [ebp-0x90]
        fmul st, st(3)
        fld dword ptr [ebp-0x98]
        fmul st, st(5)
        fxch st(2)
        faddp st(3), st
        fld dword ptr [ebp-0x30]
        fmul st, st(4)
        fld dword ptr [ebp-0x48]
        fmul st, st(6)
        fxch st(2)
        faddp st(3), st
        fxch st(3)
        fadd qword ptr [ebp-0x7c]
        fxch st(3)
        faddp st(1), st
        fxch st(1)
        fadd qword ptr [ebp-0x24]
        fxch st(3)
        faddp st(0), st
        fxch st(1)
        fstp dword ptr [edi+0x18]
        fadd qword ptr [ebp-0x58]
        fxch st(1)
        fstp dword ptr [edi+0x1a]
        fxch st(1)
        faddp st(0), st
        fstp dword ptr [edi+0x1c]
        mov eax, dword ptr [ebp-0x70]
        add edi, 0x20
        inc eax
        pop ebx
        mov dword ptr [ebp-0x70], eax
        mov eax, dword ptr [ebp-0x10]
        add ebx, eax
        mov eax, dword ptr [ebp-0x74]
        add esi, eax
        dec edx
        dec ecx
        je L6
        test edx, edx
        je L7
        jmp L3
    L4:
        mov dword ptr [SW_SpanEnd], edi
        call dword ptr [pDrawSpans]
        jmp L8
    L5:
        faddp st(0), st
        faddp st(0), st
        jmp L8
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L6:
        mov esi, dword ptr [ebp-0x3c]
        mov eax, dword ptr [ebp-0x84]
        mov ecx, esi
        dec eax
        js L4
        mov ecx, dword ptr [ecx+0x8]
        mov dword ptr [ebp-0x84], eax
        mov dword ptr [ebp-0x3c], ecx
        fld dword ptr [esi]
        fsub dword ptr [ecx]
        fld dword ptr [esi+0x4]
        fsub dword ptr [ecx+0x4]
        fdivp st(1), st
        fstp dword ptr [ebp-0xc]
        fild dword ptr [ebp-0x70]
        fsub dword ptr [esi+0x4]
        fmul dword ptr [ebp-0xc]
        fadd dword ptr [esi]
        fstp dword ptr [ebp-0x5c]
        fld dword ptr [ecx+0x4]
        fsub dword ptr [RoundBias]
        fadd qword ptr [MagicInt]
        fstp qword ptr [ebp-0x44]
        mov ecx, dword ptr [ebp-0x44]
        sub ecx, dword ptr [ebp-0x70]
        inc ecx
        je L6
        fld dword ptr [ebp-0xc]
        fmul dword ptr [Fix16Scale]
        fistp dword ptr [ebp-0x74]
        fld dword ptr [ebp-0x5c]
        fmul dword ptr [Fix16Scale]
        fistp dword ptr [ebp-0x64]
        mov esi, dword ptr [ebp-0x64]
        add esi, 0xffff
        test edx, edx
        _emit 0x0f
        _emit 0x85
        _emit 0x7d
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L7:
        mov edx, dword ptr [ebp-0x1c]
        mov eax, dword ptr [ebp-0x84]
        mov ebx, edx
        dec eax
        js L4
        mov ebx, dword ptr [ebx+0x14]
        mov dword ptr [ebp-0x84], eax
        mov dword ptr [ebp-0x1c], ebx
        fld dword ptr [edx]
        fsub dword ptr [ebx]
        fld dword ptr [edx+0x4]
        fsub dword ptr [ebx+0x4]
        fdivp st(1), st
        fstp dword ptr [ebp-0x60]
        fild dword ptr [ebp-0x70]
        fsub dword ptr [edx+0x4]
        fmul dword ptr [ebp-0x60]
        fadd dword ptr [edx]
        fstp dword ptr [ebp-0xa0]
        fld dword ptr [ebx+0x4]
        fsub dword ptr [RoundBias]
        fadd qword ptr [MagicInt]
        fstp qword ptr [ebp-0x44]
        mov edx, dword ptr [ebp-0x44]
        sub edx, dword ptr [ebp-0x70]
        inc edx
        je L7
        fld dword ptr [ebp-0x60]
        fmul dword ptr [Fix16Scale]
        fistp dword ptr [ebp-0x10]
        fld dword ptr [ebp-0xa0]
        fmul dword ptr [Fix16Scale]
        fistp dword ptr [ebp-0x64]
        mov ebx, dword ptr [ebp-0x64]
        add ebx, 0xffff
        jmp L3
    L8:
        pop edi
        pop esi
        pop ebx
        mov esp, ebp
        pop ebp
        ret
    }
}
#else
void MMXDrawTriangleFog(D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3)
{
    D3DTLVERTEX *t, *n, *ea, *eb;
    unsigned char *span;
    double a, b, g, h, inv, x, y, ty, endy, r0, g0, b0;
    float dx01, dy02, dx02, dy01, d01, d02, rdx, drdy, gdx, dgdy, bdx, dbdy, slopea, slopeb, xr, xl;
    unsigned long sy, c, xa, xb;
    long na, nb, edges, stepa, stepb, xi, r1, r2, r3, g1, g2, g3, b1, b2, b3;

    AlphaVertex = (long)v1;
    a = (double)v1->sx - v2->sx;
    b = (double)v1->sy - v3->sy;
    dx01 = SW_ToF(a);
    dy02 = SW_ToF(b);
    g = a * b;
    a = (double)v1->sx - v3->sx;
    b = (double)v1->sy - v2->sy;
    dx02 = SW_ToF(a);
    dy01 = SW_ToF(b);
    g = g - a * b;
    xr = SW_ToF(g);
    if (*(unsigned long *)&xr & 0x80000000)
        return;
    inv = 1.0 / g;
    *(D3DTLVERTEX **)&v1->sz = v2;
    *(D3DTLVERTEX **)&v2->sz = v3;
    *(D3DTLVERTEX **)&v3->sz = v1;
    *(D3DTLVERTEX **)&v1->specular = v3;
    *(D3DTLVERTEX **)&v2->specular = v1;
    *(D3DTLVERTEX **)&v3->specular = v2;
    t = v1;
    sy = *(unsigned long *)&v1->sy;
    if (sy >= *(unsigned long *)&v2->sy) {
        sy = *(unsigned long *)&v2->sy;
        t = v2;
    }
    if (sy >= *(unsigned long *)&v3->sy)
        t = v3;
    ea = t;
    eb = t;
    TriColor = v1->color;
    c = v1->color;
    g1 = (c & 0xff00) + 0x80;
    b1 = ((c << 8) & 0xff00) + 0x80;
    r1 = ((c >> 8) & 0xff00) + 0x80;
    c = v2->color;
    g2 = (c & 0xff00) + 0x80;
    b2 = ((c << 8) & 0xff00) + 0x80;
    r2 = ((c >> 8) & 0xff00) + 0x80;
    c = v3->color;
    g3 = (c & 0xff00) + 0x80;
    b3 = ((c << 8) & 0xff00) + 0x80;
    r3 = ((c >> 8) & 0xff00) + 0x80;

    a = (double)r1 - r2;
    b = (double)r1 - r3;
    d01 = SW_ToF(a);
    d02 = SW_ToF(b);
    g = (a * dy02 - b * dy01) * inv;
    ((short *)&SW_SpanGrad)[2] = SW_RoundW(g);
    rdx = SW_ToF(g);
    h = ((double)d02 * dx01 - (double)d01 * dx02) * inv;
    drdy = SW_ToF(h);
    r0 = ((double)r1 + MagicFloat) - (v1->sy * h + v1->sx * rdx);

    a = (double)g1 - g2;
    b = (double)g1 - g3;
    d01 = SW_ToF(a);
    d02 = SW_ToF(b);
    g = (a * dy02 - b * dy01) * inv;
    ((short *)&SW_SpanGrad)[1] = SW_RoundW(g);
    gdx = SW_ToF(g);
    h = ((double)d02 * dx01 - (double)d01 * dx02) * inv;
    dgdy = SW_ToF(h);
    g0 = ((double)g1 + MagicFloat) - (v1->sy * h + v1->sx * gdx);

    a = (double)b1 - b2;
    b = (double)b1 - b3;
    d01 = SW_ToF(a);
    d02 = SW_ToF(b);
    g = (a * dy02 - b * dy01) * inv;
    ((short *)&SW_SpanGrad)[0] = SW_RoundW(g);
    bdx = SW_ToF(g);
    h = ((double)d02 * dx01 - (double)d01 * dx02) * inv;
    dbdy = SW_ToF(h);
    b0 = ((double)b1 + MagicFloat) - (v1->sy * h + v1->sx * bdx);

    edges = 3;
    span = SW_SpanBuffer;
    ty = ((double)ea->sy - SW_VF(RoundBias)) + MagicInt;
    SW_L(ty)++;
    SW_SpanY = SW_L(ty);
    nb = 0;
    na = 0;
    xa = 0;
    xb = 0;
    stepa = 0;
    stepb = 0;

edge_a:
    t = ea;
    if (edges - 1 < 0)
        goto done;
    n = *(D3DTLVERTEX **)&t->sz;
    edges--;
    ea = n;
    slopea = SW_ToF(((double)t->sx - n->sx) / ((double)t->sy - n->sy));
    xr = SW_ToF(((double)SW_L(ty) - t->sy) * slopea + t->sx);
    endy = ((double)n->sy - SW_VF(RoundBias)) + MagicInt;
    na = SW_L(endy) - SW_L(ty) + 1;
    if (!na)
        goto edge_a;
    stepa = SW_RoundL(slopea * SW_VF(Fix16Scale));
    xa = SW_RoundL(xr * SW_VF(Fix16Scale)) + 0xffff;
    if (nb)
        goto spans;

edge_b:
    t = eb;
    if (edges - 1 < 0)
        goto done;
    n = *(D3DTLVERTEX **)&t->specular;
    edges--;
    eb = n;
    slopeb = SW_ToF(((double)t->sx - n->sx) / ((double)t->sy - n->sy));
    xl = SW_ToF(((double)SW_L(ty) - t->sy) * slopeb + t->sx);
    endy = ((double)n->sy - SW_VF(RoundBias)) + MagicInt;
    nb = SW_L(endy) - SW_L(ty) + 1;
    if (!nb)
        goto edge_b;
    stepb = SW_RoundL(slopeb * SW_VF(Fix16Scale));
    xb = SW_RoundL(xl * SW_VF(Fix16Scale)) + 0xffff;

spans:
    xi = xb >> 16;
    ((long *)span)[1] = (xa >> 16) - xi;
    ((long *)span)[0] = xi;
    y = (double)SW_L(ty);
    x = (double)xi;
    *(float *)(span + 24) = (float)((bdx * x + dbdy * y) + b0);
    *(float *)(span + 26) = (float)((gdx * x + dgdy * y) + g0);
    *(float *)(span + 28) = (float)((rdx * x + drdy * y) + r0);
    span += 32;
    SW_L(ty)++;
    xb += stepb;
    xa += stepa;
    nb--;
    na--;
    if (!na)
        goto edge_a;
    if (!nb)
        goto edge_b;
    goto spans;

done:
    SW_SpanEnd = (long)span;
    pDrawSpans();
}
#endif

extern long SW_TriTDX01;
extern long SW_TriTDY02;
extern long SW_TriTDX02;
extern long SW_TriTDY01;
extern long SW_TriTTemp;
extern long SW_TriTEdgeA;
extern long SW_TriTEdgeB;
extern long SW_TriTG0;
extern long SW_TriTB0;
extern long SW_TriTR0;
extern long TriG2;
extern long TriB2;
extern long TriR2;
extern long TriG3;
extern long TriB3;
extern long TriR3;
extern long SW_TriTD01;
extern long SW_TriTD02;
extern long SW_TriTRDX;
extern long TriDrDy;
extern double TriR0;
extern long SW_TriTGDX;
extern long TriDgDy;
extern double TriG0;
extern long SW_TriTBDX;
extern long TriDbDy;
extern double TriB0;
extern long SW_TriTUDX;
extern double MagicFix24;
extern long SW_TriTUDY;
extern double TriU0;
extern long SW_TriTVDX;
extern long SW_TriTVDY;
extern double TriV0;
extern long SW_TriTEdges;
extern double SW_TriTY;
extern long TriLeftXStep;
extern long TriRightXStep;
extern void (*pDrawSpansFiltered)(void);
extern long bMMXMipMap;
extern long SW_TriTSlopeA;
extern long TriRightX;
extern double SW_TriTEndY;
extern long SW_TriTSlopeB;
extern long TriLeftX;

#ifndef FEAT_C_ASM
__declspec(naked) void SW_TriSetupGT(D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3)
{
    __asm {
        push edi
        push esi
        push ebx
        mov ebx, dword ptr [esp+0x10]
        mov ecx, dword ptr [esp+0x14]
        mov edx, dword ptr [esp+0x18]
        fld dword ptr [ebx]
        fsub dword ptr [ecx]
        fld dword ptr [ebx+0x4]
        fsub dword ptr [edx+0x4]
        fxch st(1)
        fst dword ptr [SW_TriTDX01]
        fxch st(1)
        fst dword ptr [SW_TriTDY02]
        fmulp st(1), st
        fld dword ptr [ebx]
        fsub dword ptr [edx]
        fld dword ptr [ebx+0x4]
        fsub dword ptr [ecx+0x4]
        fxch st(1)
        fst dword ptr [SW_TriTDX02]
        fxch st(1)
        fst dword ptr [SW_TriTDY01]
        fmulp st(1), st
        fsubp st(1), st
        fst dword ptr [SW_TriTTemp]
        fld1
        test dword ptr [SW_TriTTemp], 0x80000000
        jne L9
        fdivrp st(1), st
        mov dword ptr [ebx+0x8], ecx
        mov dword ptr [ecx+0x8], edx
        mov dword ptr [edx+0x8], ebx
        mov dword ptr [ebx+0x14], edx
        mov dword ptr [ecx+0x14], ebx
        mov dword ptr [edx+0x14], ecx
        mov esi, ebx
        mov eax, dword ptr [ebx+0x4]
        cmp eax, dword ptr [ecx+0x4]
        jb L1
        mov eax, dword ptr [ecx+0x4]
        mov esi, ecx
    L1:
        cmp eax, dword ptr [edx+0x4]
        jb L2
        mov esi, edx
    L2:
        mov dword ptr [SW_TriTEdgeA], esi
        mov dword ptr [SW_TriTEdgeB], esi
        mov eax, dword ptr [ebx+0x10]
        mov dword ptr [TriColor], eax
        mov esi, dword ptr [ebx+0x10]
        mov edi, esi
        mov eax, esi
        shl esi, 0x8
        and edi, 0xff00
        shr eax, 0x8
        and esi, 0xff00
        and eax, 0xff00
        add edi, 0x80
        add esi, 0x80
        add eax, 0x80
        mov dword ptr [SW_TriTG0], edi
        mov dword ptr [SW_TriTB0], esi
        mov dword ptr [SW_TriTR0], eax
        mov esi, dword ptr [ecx+0x10]
        mov edi, esi
        mov eax, esi
        shl esi, 0x8
        and edi, 0xff00
        shr eax, 0x8
        and esi, 0xff00
        and eax, 0xff00
        add edi, 0x80
        add esi, 0x80
        add eax, 0x80
        mov dword ptr [TriG2], edi
        mov dword ptr [TriB2], esi
        mov dword ptr [TriR2], eax
        mov esi, dword ptr [edx+0x10]
        mov edi, esi
        mov eax, esi
        shl esi, 0x8
        and edi, 0xff00
        shr eax, 0x8
        and esi, 0xff00
        and eax, 0xff00
        add edi, 0x80
        add esi, 0x80
        add eax, 0x80
        mov dword ptr [TriG3], edi
        mov dword ptr [TriB3], esi
        mov dword ptr [TriR3], eax
        fild dword ptr [SW_TriTR0]
        fild dword ptr [TriR2]
        fild dword ptr [TriR3]
        fxch st(1)
        fsubr st, st(2)
        fxch st(2)
        fsubrp st(1), st
        fxch st(1)
        fst dword ptr [SW_TriTD01]
        fmul dword ptr [SW_TriTDY02]
        fxch st(1)
        fst dword ptr [SW_TriTD02]
        fmul dword ptr [SW_TriTDY01]
        fld dword ptr [SW_TriTD01]
        fmul dword ptr [SW_TriTDX02]
        fxch st(1)
        fsubp st(2), st
        fld dword ptr [SW_TriTD02]
        fmul dword ptr [SW_TriTDX01]
        fxch st(2)
        fmul st, st(3)
        fist word ptr [SW_SpanGrad + 4]
        fxch st(1)
        fsubp st(2), st
        fstp dword ptr [SW_TriTRDX]
        fmul st, st(1)
        fld dword ptr [ebx]
        fmul dword ptr [SW_TriTRDX]
        fld dword ptr [ebx+0x4]
        fmul st, st(2)
        fxch st(2)
        fstp dword ptr [TriDrDy]
        fild dword ptr [SW_TriTR0]
        fxch st(1)
        faddp st(2), st
        fadd qword ptr [MagicFloat]
        fsubrp st(1), st
        fstp qword ptr [TriR0]
        fild dword ptr [SW_TriTG0]
        fild dword ptr [TriG2]
        fild dword ptr [TriG3]
        fxch st(1)
        fsubr st, st(2)
        fxch st(2)
        fsubrp st(1), st
        fxch st(1)
        fst dword ptr [SW_TriTD01]
        fmul dword ptr [SW_TriTDY02]
        fxch st(1)
        fst dword ptr [SW_TriTD02]
        fmul dword ptr [SW_TriTDY01]
        fld dword ptr [SW_TriTD01]
        fmul dword ptr [SW_TriTDX02]
        fxch st(1)
        fsubp st(2), st
        fld dword ptr [SW_TriTD02]
        fmul dword ptr [SW_TriTDX01]
        fxch st(2)
        fmul st, st(3)
        fist word ptr [SW_SpanGrad + 2]
        fxch st(1)
        fsubp st(2), st
        fstp dword ptr [SW_TriTGDX]
        fmul st, st(1)
        fld dword ptr [ebx]
        fmul dword ptr [SW_TriTGDX]
        fld dword ptr [ebx+0x4]
        fmul st, st(2)
        fxch st(2)
        fstp dword ptr [TriDgDy]
        fild dword ptr [SW_TriTG0]
        fxch st(1)
        faddp st(2), st
        fadd qword ptr [MagicFloat]
        fsubrp st(1), st
        fstp qword ptr [TriG0]
        fild dword ptr [SW_TriTB0]
        fild dword ptr [TriB2]
        fild dword ptr [TriB3]
        fxch st(1)
        fsubr st, st(2)
        fxch st(2)
        fsubrp st(1), st
        fxch st(1)
        fst dword ptr [SW_TriTD01]
        fmul dword ptr [SW_TriTDY02]
        fxch st(1)
        fst dword ptr [SW_TriTD02]
        fmul dword ptr [SW_TriTDY01]
        fld dword ptr [SW_TriTD01]
        fmul dword ptr [SW_TriTDX02]
        fxch st(1)
        fsubp st(2), st
        fld dword ptr [SW_TriTD02]
        fmul dword ptr [SW_TriTDX01]
        fxch st(2)
        fmul st, st(3)
        fist word ptr [SW_SpanGrad]
        fxch st(1)
        fsubp st(2), st
        fstp dword ptr [SW_TriTBDX]
        fmul st, st(1)
        fld dword ptr [ebx]
        fmul dword ptr [SW_TriTBDX]
        fld dword ptr [ebx+0x4]
        fmul st, st(2)
        fxch st(2)
        fstp dword ptr [TriDbDy]
        fild dword ptr [SW_TriTB0]
        fxch st(1)
        faddp st(2), st
        fadd qword ptr [MagicFloat]
        fsubrp st(1), st
        fstp qword ptr [TriB0]
        fld dword ptr [ebx+0x18]
        fsub dword ptr [ecx+0x18]
        fld dword ptr [ebx+0x18]
        fsub dword ptr [edx+0x18]
        fxch st(1)
        fst dword ptr [SW_TriTD01]
        fmul dword ptr [SW_TriTDY02]
        fxch st(1)
        fst dword ptr [SW_TriTD02]
        fmul dword ptr [SW_TriTDY01]
        fld dword ptr [SW_TriTD01]
        fmul dword ptr [SW_TriTDX02]
        fxch st(1)
        fsubp st(2), st
        fld dword ptr [SW_TriTD02]
        fmul dword ptr [SW_TriTDX01]
        fxch st(2)
        fmul st, st(3)
        fxch st(1)
        fsubp st(2), st
        fst dword ptr [SW_TriTUDX]
        fxch st(1)
        fmul st, st(2)
        fxch st(1)
        fadd qword ptr [MagicFix24]
        fxch st(1)
        fld dword ptr [ebx]
        fmul dword ptr [SW_TriTUDX]
        fxch st(1)
        fstp dword ptr [SW_TriTUDY]
        fld dword ptr [ebx+0x4]
        fmul dword ptr [SW_TriTUDY]
        fld dword ptr [ebx+0x18]
        fadd qword ptr [MagicFix24]
        fxch st(3)
        fstp qword ptr [SW_SpanDU]
        faddp st(1), st
        fsubp st(1), st
        fstp qword ptr [TriU0]
        fld dword ptr [ebx+0x1c]
        fsub dword ptr [ecx+0x1c]
        fld dword ptr [ebx+0x1c]
        fsub dword ptr [edx+0x1c]
        fxch st(1)
        fst dword ptr [SW_TriTD01]
        fmul dword ptr [SW_TriTDY02]
        fxch st(1)
        fst dword ptr [SW_TriTD02]
        fmul dword ptr [SW_TriTDY01]
        fld dword ptr [SW_TriTD01]
        fmul dword ptr [SW_TriTDX02]
        fxch st(1)
        fsubp st(2), st
        fld dword ptr [SW_TriTD02]
        fmul dword ptr [SW_TriTDX01]
        fxch st(2)
        fmul st, st(3)
        fxch st(1)
        fsubp st(2), st
        fst dword ptr [SW_TriTVDX]
        fxch st(1)
        fmul st, st(2)
        fxch st(1)
        fadd qword ptr [MagicFix24]
        fxch st(1)
        fld dword ptr [ebx]
        fmul dword ptr [SW_TriTVDX]
        fxch st(1)
        fstp dword ptr [SW_TriTVDY]
        fld dword ptr [ebx+0x4]
        fmul dword ptr [SW_TriTVDY]
        fld dword ptr [ebx+0x1c]
        fadd qword ptr [MagicFix24]
        fxch st(3)
        fstp qword ptr [SW_SpanDV]
        faddp st(1), st
        fsubp st(1), st
        fstp qword ptr [TriV0]
        fld dword ptr [ebx+0x18]
        fsub dword ptr [ecx+0x18]
        fld dword ptr [ebx+0x1c]
        fsub dword ptr [edx+0x1c]
        fmulp st(1), st
        fld dword ptr [ebx+0x18]
        fsub dword ptr [edx+0x18]
        fld dword ptr [ebx+0x1c]
        fsub dword ptr [ecx+0x1c]
        fmulp st(1), st
        fsubp st(1), st
        fmul st, st(1)
        fstp dword ptr [SW_TriTTemp]
        mov eax, dword ptr [SW_TriTTemp]
        sub eax, 0x400000
        shr eax, 0x17
        and eax, 0xff
        sub eax, 0x6f
        inc eax
        sar eax, 1
        test eax, eax
        jns L3
        mov eax, 0x0
    L3:
        cmp eax, 0x4
        jb L4
        mov eax, 0x4
    L4:
        ffree st(0)
        mov dword ptr [SW_TexShift], eax
        mov esi, dword ptr [SW_TriTEdgeA]
        fld dword ptr [esi+0x4]
        fsub dword ptr [RoundBias]
        mov eax, 0x3
        mov edi, dword ptr [SW_SpanBuffer]
        xor edx, edx
        mov dword ptr [SW_TriTEdges], eax
        fadd qword ptr [MagicInt]
        fstp qword ptr [SW_TriTY]
        mov eax, dword ptr [SW_TriTY]
        inc eax
        mov dword ptr [SW_TriTY], eax
        mov dword ptr [SW_SpanY], eax
        jmp L10
    L5:
        push ebx
        mov eax, dword ptr [edi]
        mov eax, dword ptr [edi+0x20]
        shr ebx, 0x10
        mov eax, esi
        shr eax, 0x10
        mov dword ptr [SW_TriTTemp], ebx
        sub eax, ebx
        fild dword ptr [SW_TriTY]
        fild dword ptr [SW_TriTTemp]
        mov dword ptr [edi+0x4], eax
        mov dword ptr [edi], ebx
        fld dword ptr [SW_TriTUDX]
        fmul st, st(1)
        fld dword ptr [SW_TriTUDY]
        fmul st, st(3)
        fld dword ptr [SW_TriTVDX]
        fmul st, st(3)
        fld dword ptr [SW_TriTVDY]
        fmul st, st(5)
        fxch st(2)
        faddp st(3), st
        mov eax, dword ptr [SW_TriTY]
        faddp st(1), st
        fxch st(1)
        fadd qword ptr [TriU0]
        fxch st(1)
        fld dword ptr [SW_TriTBDX]
        fmul st, st(3)
        fxch st(1)
        fadd qword ptr [TriV0]
        fxch st(2)
        fstp qword ptr [edi+0x8]
        fld dword ptr [TriDbDy]
        fmul st, st(4)
        fxch st(2)
        fstp qword ptr [edi+0x10]
        inc eax
        mov dword ptr [SW_TriTY], eax
        fld dword ptr [SW_TriTGDX]
        fmul st, st(3)
        fld dword ptr [TriDgDy]
        fmul st, st(5)
        fxch st(2)
        faddp st(3), st
        fld dword ptr [SW_TriTRDX]
        fmul st, st(4)
        fld dword ptr [TriDrDy]
        fmul st, st(6)
        fxch st(2)
        faddp st(3), st
        fxch st(3)
        fadd qword ptr [TriB0]
        fxch st(3)
        faddp st(1), st
        fxch st(1)
        fadd qword ptr [TriG0]
        fxch st(3)
        faddp st(0), st
        fxch st(1)
        fstp dword ptr [edi+0x18]
        fadd qword ptr [TriR0]
        fxch st(1)
        fstp dword ptr [SW_TriTTemp]
        fxch st(1)
        faddp st(0), st
        fstp dword ptr [edi+0x1c]
        mov eax, dword ptr [SW_TriTTemp]
        pop ebx
        mov word ptr [edi+0x1a], ax
        mov eax, dword ptr [TriLeftXStep]
        add edi, 0x20
        add ebx, eax
        mov eax, dword ptr [TriRightXStep]
        add esi, eax
        dec edx
        dec ecx
        je L10
        test edx, edx
        je L11
        jmp L5
    L6:
        mov dword ptr [SW_SpanEnd], edi
        cmp dword ptr [SW_TexShift], 0x0
        jne L7
        call dword ptr [pDrawSpansFiltered]
        pop ebx
        pop esi
        pop edi
        ret
    L7:
        cmp dword ptr [bMMXMipMap], 0x1
        je L8
        mov dword ptr [SW_TexShift], 0x0
    L8:
        call dword ptr [pDrawSpans]
        pop ebx
        pop esi
        pop edi
        ret
    L9:
        faddp st(0), st
        faddp st(0), st
        pop ebx
        pop esi
        pop edi
        ret
        nop
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L10:
        mov esi, dword ptr [SW_TriTEdgeA]
        mov eax, dword ptr [SW_TriTEdges]
        mov ecx, esi
        dec eax
        js L6
        mov ecx, dword ptr [ecx+0x8]
        mov dword ptr [SW_TriTEdges], eax
        mov dword ptr [SW_TriTEdgeA], ecx
        fld dword ptr [esi]
        fsub dword ptr [ecx]
        fld dword ptr [esi+0x4]
        fsub dword ptr [ecx+0x4]
        fdivp st(1), st
        fstp dword ptr [SW_TriTSlopeA]
        fild dword ptr [SW_TriTY]
        fsub dword ptr [esi+0x4]
        fmul dword ptr [SW_TriTSlopeA]
        fadd dword ptr [esi]
        fstp dword ptr [TriRightX]
        fld dword ptr [ecx+0x4]
        fsub dword ptr [RoundBias]
        fadd qword ptr [MagicInt]
        fstp qword ptr [SW_TriTEndY]
        mov ecx, dword ptr [SW_TriTEndY]
        inc ecx
        sub ecx, dword ptr [SW_TriTY]
        je L10
        fld dword ptr [SW_TriTSlopeA]
        fmul dword ptr [Fix16Scale]
        fistp dword ptr [TriRightXStep]
        fld dword ptr [TriRightX]
        fmul dword ptr [Fix16Scale]
        fistp dword ptr [SW_TriTTemp]
        mov esi, dword ptr [SW_TriTTemp]
        add esi, 0xffff
        test edx, edx
        jne L12
    L11:
        mov edx, dword ptr [SW_TriTEdgeB]
        mov eax, dword ptr [SW_TriTEdges]
        mov ebx, edx
        dec eax
        js L6
        mov ebx, dword ptr [ebx+0x14]
        mov dword ptr [SW_TriTEdges], eax
        mov dword ptr [SW_TriTEdgeB], ebx
        fld dword ptr [edx]
        fsub dword ptr [ebx]
        fld dword ptr [edx+0x4]
        fsub dword ptr [ebx+0x4]
        fdivp st(1), st
        fstp dword ptr [SW_TriTSlopeB]
        fild dword ptr [SW_TriTY]
        fsub dword ptr [edx+0x4]
        fmul dword ptr [SW_TriTSlopeB]
        fadd dword ptr [edx]
        fstp dword ptr [TriLeftX]
        fld dword ptr [ebx+0x4]
        fsub dword ptr [RoundBias]
        fadd qword ptr [MagicInt]
        fstp qword ptr [SW_TriTEndY]
        mov edx, dword ptr [SW_TriTEndY]
        inc edx
        sub edx, dword ptr [SW_TriTY]
        je L11
        fld dword ptr [SW_TriTSlopeB]
        fmul dword ptr [Fix16Scale]
        fistp dword ptr [TriLeftXStep]
        fld dword ptr [TriLeftX]
        fmul dword ptr [Fix16Scale]
        fistp dword ptr [SW_TriTTemp]
        mov ebx, dword ptr [SW_TriTTemp]
        add ebx, 0xffff
    L12:
        jmp L5
    }
}
#else
void SW_TriSetupGT(D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3)
{
    D3DTLVERTEX *t, *n;
    unsigned char *span;
    double a, b, g, h, inv, d01, d02, x, y;
    float f;
    unsigned long sy, c, xa, xb;
    long na, nb, e;

    a = (double)v1->sx - v2->sx;
    b = (double)v1->sy - v3->sy;
    SW_SetF(&SW_TriTDX01, a);
    SW_SetF(&SW_TriTDY02, b);
    g = a * b;
    a = (double)v1->sx - v3->sx;
    b = (double)v1->sy - v2->sy;
    SW_SetF(&SW_TriTDX02, a);
    SW_SetF(&SW_TriTDY01, b);
    g = g - a * b;
    SW_SetF(&SW_TriTTemp, g);
    if (SW_TriTTemp & 0x80000000)
        return;
    inv = 1.0 / g;
    *(D3DTLVERTEX **)&v1->sz = v2;
    *(D3DTLVERTEX **)&v2->sz = v3;
    *(D3DTLVERTEX **)&v3->sz = v1;
    *(D3DTLVERTEX **)&v1->specular = v3;
    *(D3DTLVERTEX **)&v2->specular = v1;
    *(D3DTLVERTEX **)&v3->specular = v2;
    t = v1;
    sy = *(unsigned long *)&v1->sy;
    if (sy >= *(unsigned long *)&v2->sy) {
        sy = *(unsigned long *)&v2->sy;
        t = v2;
    }
    if (sy >= *(unsigned long *)&v3->sy)
        t = v3;
    SW_TriTEdgeA = (long)t;
    SW_TriTEdgeB = (long)t;
    TriColor = v1->color;
    c = v1->color;
    SW_TriTG0 = (c & 0xff00) + 0x80;
    SW_TriTB0 = ((c << 8) & 0xff00) + 0x80;
    SW_TriTR0 = ((c >> 8) & 0xff00) + 0x80;
    c = v2->color;
    TriG2 = (c & 0xff00) + 0x80;
    TriB2 = ((c << 8) & 0xff00) + 0x80;
    TriR2 = ((c >> 8) & 0xff00) + 0x80;
    c = v3->color;
    TriG3 = (c & 0xff00) + 0x80;
    TriB3 = ((c << 8) & 0xff00) + 0x80;
    TriR3 = ((c >> 8) & 0xff00) + 0x80;

    d01 = (double)SW_TriTR0 - TriR2;
    d02 = (double)SW_TriTR0 - TriR3;
    SW_SetF(&SW_TriTD01, d01);
    SW_SetF(&SW_TriTD02, d02);
    g = (d01 * SW_VF(SW_TriTDY02) - d02 * SW_VF(SW_TriTDY01)) * inv;
    ((short *)&SW_SpanGrad)[2] = SW_RoundW(g);
    SW_SetF(&SW_TriTRDX, g);
    h = (SW_VF(SW_TriTD02) * SW_VF(SW_TriTDX01) - SW_VF(SW_TriTD01) * SW_VF(SW_TriTDX02)) * inv;
    SW_SetF(&TriDrDy, h);
    TriR0 = ((double)SW_TriTR0 + MagicFloat) - (v1->sy * h + v1->sx * SW_VF(SW_TriTRDX));

    d01 = (double)SW_TriTG0 - TriG2;
    d02 = (double)SW_TriTG0 - TriG3;
    SW_SetF(&SW_TriTD01, d01);
    SW_SetF(&SW_TriTD02, d02);
    g = (d01 * SW_VF(SW_TriTDY02) - d02 * SW_VF(SW_TriTDY01)) * inv;
    ((short *)&SW_SpanGrad)[1] = SW_RoundW(g);
    SW_SetF(&SW_TriTGDX, g);
    h = (SW_VF(SW_TriTD02) * SW_VF(SW_TriTDX01) - SW_VF(SW_TriTD01) * SW_VF(SW_TriTDX02)) * inv;
    SW_SetF(&TriDgDy, h);
    TriG0 = ((double)SW_TriTG0 + MagicFloat) - (v1->sy * h + v1->sx * SW_VF(SW_TriTGDX));

    d01 = (double)SW_TriTB0 - TriB2;
    d02 = (double)SW_TriTB0 - TriB3;
    SW_SetF(&SW_TriTD01, d01);
    SW_SetF(&SW_TriTD02, d02);
    g = (d01 * SW_VF(SW_TriTDY02) - d02 * SW_VF(SW_TriTDY01)) * inv;
    ((short *)&SW_SpanGrad)[0] = SW_RoundW(g);
    SW_SetF(&SW_TriTBDX, g);
    h = (SW_VF(SW_TriTD02) * SW_VF(SW_TriTDX01) - SW_VF(SW_TriTD01) * SW_VF(SW_TriTDX02)) * inv;
    SW_SetF(&TriDbDy, h);
    TriB0 = ((double)SW_TriTB0 + MagicFloat) - (v1->sy * h + v1->sx * SW_VF(SW_TriTBDX));

    d01 = (double)v1->tu - v2->tu;
    d02 = (double)v1->tu - v3->tu;
    SW_SetF(&SW_TriTD01, d01);
    SW_SetF(&SW_TriTD02, d02);
    g = (d01 * SW_VF(SW_TriTDY02) - d02 * SW_VF(SW_TriTDY01)) * inv;
    SW_SetF(&SW_TriTUDX, g);
    h = (SW_VF(SW_TriTD02) * SW_VF(SW_TriTDX01) - SW_VF(SW_TriTD01) * SW_VF(SW_TriTDX02)) * inv;
    SW_SetF(&SW_TriTUDY, h);
    *(double *)&SW_SpanDU = g + MagicFix24;
    TriU0 = ((double)v1->tu + MagicFix24) - (v1->sx * SW_VF(SW_TriTUDX) + v1->sy * SW_VF(SW_TriTUDY));

    d01 = (double)v1->tv - v2->tv;
    d02 = (double)v1->tv - v3->tv;
    SW_SetF(&SW_TriTD01, d01);
    SW_SetF(&SW_TriTD02, d02);
    g = (d01 * SW_VF(SW_TriTDY02) - d02 * SW_VF(SW_TriTDY01)) * inv;
    SW_SetF(&SW_TriTVDX, g);
    h = (SW_VF(SW_TriTD02) * SW_VF(SW_TriTDX01) - SW_VF(SW_TriTD01) * SW_VF(SW_TriTDX02)) * inv;
    SW_SetF(&SW_TriTVDY, h);
    *(double *)&SW_SpanDV = g + MagicFix24;
    TriV0 = ((double)v1->tv + MagicFix24) - (v1->sx * SW_VF(SW_TriTVDX) + v1->sy * SW_VF(SW_TriTVDY));

    a = ((double)v1->tu - v2->tu) * ((double)v1->tv - v3->tv);
    b = ((double)v1->tu - v3->tu) * ((double)v1->tv - v2->tv);
    SW_SetF(&SW_TriTTemp, ((a - b) * inv));
    e = ((unsigned long)(SW_TriTTemp - 0x400000) >> 23) & 0xff;
    e = (e - 0x6f + 1) >> 1;
    if (e < 0)
        e = 0;
    if ((unsigned long)e >= 4)
        e = 4;
    SW_TexShift = e;

    t = (D3DTLVERTEX *)SW_TriTEdgeA;
    span = SW_SpanBuffer;
    nb = 0;
    SW_TriTEdges = 3;
    SW_TriTY = ((double)t->sy - SW_VF(RoundBias)) + MagicInt;
    SW_L(SW_TriTY)++;
    SW_SpanY = SW_L(SW_TriTY);
    xa = 0;
    xb = 0;
    na = 0;

edge_a:
    t = (D3DTLVERTEX *)SW_TriTEdgeA;
    if (SW_TriTEdges - 1 < 0)
        goto done;
    n = *(D3DTLVERTEX **)&t->sz;
    SW_TriTEdges--;
    SW_TriTEdgeA = (long)n;
    SW_SetF(&SW_TriTSlopeA, (((double)t->sx - n->sx) / ((double)t->sy - n->sy)));
    SW_SetF(&TriRightX, (((double)SW_L(SW_TriTY) - t->sy) * SW_VF(SW_TriTSlopeA) + t->sx));
    SW_TriTEndY = ((double)n->sy - SW_VF(RoundBias)) + MagicInt;
    na = SW_L(SW_TriTEndY) + 1 - SW_L(SW_TriTY);
    if (!na)
        goto edge_a;
    TriRightXStep = SW_RoundL(SW_VF(SW_TriTSlopeA) * SW_VF(Fix16Scale));
    SW_TriTTemp = SW_RoundL(SW_VF(TriRightX) * SW_VF(Fix16Scale));
    xa = SW_TriTTemp + 0xffff;
    if (nb)
        goto spans;

edge_b:
    t = (D3DTLVERTEX *)SW_TriTEdgeB;
    if (SW_TriTEdges - 1 < 0)
        goto done;
    n = *(D3DTLVERTEX **)&t->specular;
    SW_TriTEdges--;
    SW_TriTEdgeB = (long)n;
    SW_SetF(&SW_TriTSlopeB, (((double)t->sx - n->sx) / ((double)t->sy - n->sy)));
    SW_SetF(&TriLeftX, (((double)SW_L(SW_TriTY) - t->sy) * SW_VF(SW_TriTSlopeB) + t->sx));
    SW_TriTEndY = ((double)n->sy - SW_VF(RoundBias)) + MagicInt;
    nb = SW_L(SW_TriTEndY) + 1 - SW_L(SW_TriTY);
    if (!nb)
        goto edge_b;
    TriLeftXStep = SW_RoundL(SW_VF(SW_TriTSlopeB) * SW_VF(Fix16Scale));
    SW_TriTTemp = SW_RoundL(SW_VF(TriLeftX) * SW_VF(Fix16Scale));
    xb = SW_TriTTemp + 0xffff;

spans:
    SW_TriTTemp = xb >> 16;
    ((long *)span)[1] = (xa >> 16) - (xb >> 16);
    ((long *)span)[0] = xb >> 16;
    y = (double)SW_L(SW_TriTY);
    x = (double)SW_TriTTemp;
    *(double *)(span + 8) = (SW_VF(SW_TriTUDX) * x + SW_VF(SW_TriTUDY) * y) + TriU0;
    *(double *)(span + 16) = (SW_VF(SW_TriTVDX) * x + SW_VF(SW_TriTVDY) * y) + TriV0;
    SW_L(SW_TriTY)++;
    *(float *)(span + 24) = (float)((SW_VF(SW_TriTBDX) * x + SW_VF(TriDbDy) * y) + TriB0);
    SW_SetF(&SW_TriTTemp, ((SW_VF(SW_TriTGDX) * x + SW_VF(TriDgDy) * y) + TriG0));
    f = (float)((SW_VF(SW_TriTRDX) * x + SW_VF(TriDrDy) * y) + TriR0);
    *(float *)(span + 28) = f;
    *(unsigned short *)(span + 26) = (unsigned short)SW_TriTTemp;
    span += 32;
    xb += TriLeftXStep;
    xa += TriRightXStep;
    nb--;
    na--;
    if (!na)
        goto edge_a;
    if (!nb)
        goto edge_b;
    goto spans;

done:
    SW_SpanEnd = (long)span;
    if (!SW_TexShift) {
        pDrawSpansFiltered();
        return;
    }
    if (bMMXMipMap != 1)
        SW_TexShift = 0;
    pDrawSpans();
}
#endif

extern long TriVz3P;
extern long TriUz3P;
extern long TriVz2P;
extern long TriUz2P;
extern long SW_TriPV0;
extern long SW_TriPU0;
extern long SW_TriPDX01;
extern long SW_TriPDY02;
extern long SW_TriPDX02;
extern long SW_TriPDY01;
extern long SW_TriPTemp;
extern long SW_TriPEdgeA;
extern long SW_TriPEdgeB;
extern long SW_TriPG0;
extern long SW_TriPB0;
extern long SW_TriPR0;
extern long TriG2P;
extern long TriB2P;
extern long TriR2P;
extern long TriG3P;
extern long TriB3P;
extern long TriR3P;
extern long SW_TriPD01;
extern long SW_TriPD02;
extern long SW_TriPRDX;
extern long TriDrDyP;
extern double TriR0P;
extern long SW_TriPGDX;
extern long TriDgDyP;
extern double TriG0P;
extern long SW_TriPBDX;
extern long TriDbDyP;
extern double TriB0P;
extern long SW_TriPUDX;
extern long SubSpanLength;
extern long SW_TriPUDY;
extern long TriDuzDx16P;
extern long TriUz0P;
extern long SW_TriPVDX;
extern long SW_TriPVDY;
extern long TriDvzDx16P;
extern long TriVz0P;
extern long SW_TriPWDX;
extern long SW_TriPWDY;
extern long TriDozDx16P;
extern long TriOz0P;
extern long SW_TriPEdges;
extern double SW_TriPY;
extern long SW_TriPW;
extern long RecipTable[];
extern long SW_TriPRecip;
extern long SW_TriPU;
extern long SW_TriPV;
extern double SW_TriPTmp;
extern double TriLeftXStepP;
extern double TriRightXStepP;
extern void (*pDrawSpansFilteredP)(void);
extern void (*pDrawSpansP)(void);

#ifndef FEAT_C_ASM
__declspec(naked) void SW_TriSetupGTP(D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3)
{
    __asm {
        push edi
        push esi
        push ebx
        mov ebx, dword ptr [esp+0x10]
        mov ecx, dword ptr [esp+0x14]
        mov edx, dword ptr [esp+0x18]
        fld dword ptr [ebx+0x18]
        fmul dword ptr [ebx+0xc]
        fld dword ptr [ebx+0x1c]
        fmul dword ptr [ebx+0xc]
        fld dword ptr [ecx+0x18]
        fmul dword ptr [ecx+0xc]
        fld dword ptr [ecx+0x1c]
        fmul dword ptr [ecx+0xc]
        fld dword ptr [edx+0x18]
        fmul dword ptr [edx+0xc]
        fld dword ptr [edx+0x1c]
        fmul dword ptr [edx+0xc]
        fstp dword ptr [TriVz3P]
        fstp dword ptr [TriUz3P]
        fstp dword ptr [TriVz2P]
        fstp dword ptr [TriUz2P]
        fstp dword ptr [SW_TriPV0]
        fstp dword ptr [SW_TriPU0]
        fld dword ptr [ebx]
        fsub dword ptr [ecx]
        fld dword ptr [ebx+0x4]
        fsub dword ptr [edx+0x4]
        fxch st(1)
        fst dword ptr [SW_TriPDX01]
        fxch st(1)
        fst dword ptr [SW_TriPDY02]
        fmulp st(1), st
        fld dword ptr [ebx]
        fsub dword ptr [edx]
        fld dword ptr [ebx+0x4]
        fsub dword ptr [ecx+0x4]
        fxch st(1)
        fst dword ptr [SW_TriPDX02]
        fxch st(1)
        fst dword ptr [SW_TriPDY01]
        fmulp st(1), st
        fsubp st(1), st
        fst dword ptr [SW_TriPTemp]
        fld1
        test dword ptr [SW_TriPTemp], 0x80000000
        jne L11
        fdivrp st(1), st
        mov dword ptr [ebx+0x8], ecx
        mov dword ptr [ecx+0x8], edx
        mov dword ptr [edx+0x8], ebx
        mov dword ptr [ebx+0x14], edx
        mov dword ptr [ecx+0x14], ebx
        mov dword ptr [edx+0x14], ecx
        mov esi, ebx
        mov eax, dword ptr [ebx+0x4]
        cmp eax, dword ptr [ecx+0x4]
        jb L1
        mov eax, dword ptr [ecx+0x4]
        mov esi, ecx
    L1:
        cmp eax, dword ptr [edx+0x4]
        jb L2
        mov esi, edx
    L2:
        mov dword ptr [SW_TriPEdgeA], esi
        mov dword ptr [SW_TriPEdgeB], esi
        mov eax, dword ptr [ebx+0x10]
        mov dword ptr [TriColor], eax
        mov esi, dword ptr [ebx+0x10]
        mov edi, esi
        mov eax, esi
        shl esi, 0x8
        and edi, 0xff00
        shr eax, 0x8
        and esi, 0xff00
        and eax, 0xff00
        add edi, 0x80
        add esi, 0x80
        add eax, 0x80
        mov dword ptr [SW_TriPG0], edi
        mov dword ptr [SW_TriPB0], esi
        mov dword ptr [SW_TriPR0], eax
        mov esi, dword ptr [ecx+0x10]
        mov edi, esi
        mov eax, esi
        shl esi, 0x8
        and edi, 0xff00
        shr eax, 0x8
        and esi, 0xff00
        and eax, 0xff00
        add edi, 0x80
        add esi, 0x80
        add eax, 0x80
        mov dword ptr [TriG2P], edi
        mov dword ptr [TriB2P], esi
        mov dword ptr [TriR2P], eax
        mov esi, dword ptr [edx+0x10]
        mov edi, esi
        mov eax, esi
        shl esi, 0x8
        and edi, 0xff00
        shr eax, 0x8
        and esi, 0xff00
        and eax, 0xff00
        add edi, 0x80
        add esi, 0x80
        add eax, 0x80
        mov dword ptr [TriG3P], edi
        mov dword ptr [TriB3P], esi
        mov dword ptr [TriR3P], eax
        fild dword ptr [SW_TriPR0]
        fild dword ptr [TriR2P]
        fild dword ptr [TriR3P]
        fxch st(1)
        fsubr st, st(2)
        fxch st(2)
        fsubrp st(1), st
        fxch st(1)
        fst dword ptr [SW_TriPD01]
        fmul dword ptr [SW_TriPDY02]
        fxch st(1)
        fst dword ptr [SW_TriPD02]
        fmul dword ptr [SW_TriPDY01]
        fld dword ptr [SW_TriPD01]
        fmul dword ptr [SW_TriPDX02]
        fxch st(1)
        fsubp st(2), st
        fld dword ptr [SW_TriPD02]
        fmul dword ptr [SW_TriPDX01]
        fxch st(2)
        fmul st, st(3)
        fist word ptr [SW_SpanGrad + 4]
        fxch st(1)
        fsubp st(2), st
        fstp dword ptr [SW_TriPRDX]
        fmul st, st(1)
        fld dword ptr [ebx]
        fmul dword ptr [SW_TriPRDX]
        fld dword ptr [ebx+0x4]
        fmul st, st(2)
        fxch st(2)
        fstp dword ptr [TriDrDyP]
        fild dword ptr [SW_TriPR0]
        fxch st(1)
        faddp st(2), st
        fadd qword ptr [MagicFloat]
        fsubrp st(1), st
        fstp qword ptr [TriR0P]
        fild dword ptr [SW_TriPG0]
        fild dword ptr [TriG2P]
        fild dword ptr [TriG3P]
        fxch st(1)
        fsubr st, st(2)
        fxch st(2)
        fsubrp st(1), st
        fxch st(1)
        fst dword ptr [SW_TriPD01]
        fmul dword ptr [SW_TriPDY02]
        fxch st(1)
        fst dword ptr [SW_TriPD02]
        fmul dword ptr [SW_TriPDY01]
        fld dword ptr [SW_TriPD01]
        fmul dword ptr [SW_TriPDX02]
        fxch st(1)
        fsubp st(2), st
        fld dword ptr [SW_TriPD02]
        fmul dword ptr [SW_TriPDX01]
        fxch st(2)
        fmul st, st(3)
        fist word ptr [SW_SpanGrad + 2]
        fxch st(1)
        fsubp st(2), st
        fstp dword ptr [SW_TriPGDX]
        fmul st, st(1)
        fld dword ptr [ebx]
        fmul dword ptr [SW_TriPGDX]
        fld dword ptr [ebx+0x4]
        fmul st, st(2)
        fxch st(2)
        fstp dword ptr [TriDgDyP]
        fild dword ptr [SW_TriPG0]
        fxch st(1)
        faddp st(2), st
        fadd qword ptr [MagicFloat]
        fsubrp st(1), st
        fstp qword ptr [TriG0P]
        fild dword ptr [SW_TriPB0]
        fild dword ptr [TriB2P]
        fild dword ptr [TriB3P]
        fxch st(1)
        fsubr st, st(2)
        fxch st(2)
        fsubrp st(1), st
        fxch st(1)
        fst dword ptr [SW_TriPD01]
        fmul dword ptr [SW_TriPDY02]
        fxch st(1)
        fst dword ptr [SW_TriPD02]
        fmul dword ptr [SW_TriPDY01]
        fld dword ptr [SW_TriPD01]
        fmul dword ptr [SW_TriPDX02]
        fxch st(1)
        fsubp st(2), st
        fld dword ptr [SW_TriPD02]
        fmul dword ptr [SW_TriPDX01]
        fxch st(2)
        fmul st, st(3)
        fist word ptr [SW_SpanGrad]
        fxch st(1)
        fsubp st(2), st
        fstp dword ptr [SW_TriPBDX]
        fmul st, st(1)
        fld dword ptr [ebx]
        fmul dword ptr [SW_TriPBDX]
        fld dword ptr [ebx+0x4]
        fmul st, st(2)
        fxch st(2)
        fstp dword ptr [TriDbDyP]
        fild dword ptr [SW_TriPB0]
        fxch st(1)
        faddp st(2), st
        fadd qword ptr [MagicFloat]
        fsubrp st(1), st
        fstp qword ptr [TriB0P]
        fld dword ptr [SW_TriPU0]
        fsub dword ptr [TriUz2P]
        fld dword ptr [SW_TriPU0]
        fsub dword ptr [TriUz3P]
        fxch st(1)
        fst dword ptr [SW_TriPD01]
        fmul dword ptr [SW_TriPDY02]
        fxch st(1)
        fst dword ptr [SW_TriPD02]
        fmul dword ptr [SW_TriPDY01]
        fld dword ptr [SW_TriPD01]
        fmul dword ptr [SW_TriPDX02]
        fxch st(1)
        fsubp st(2), st
        fld dword ptr [SW_TriPD02]
        fmul dword ptr [SW_TriPDX01]
        fxch st(2)
        fmul st, st(3)
        fxch st(1)
        fsubp st(2), st
        fst dword ptr [SW_TriPUDX]
        fxch st(1)
        fmul st, st(2)
        fxch st(1)
        fmul dword ptr [SubSpanLength]
        fxch st(1)
        fld dword ptr [ebx]
        fmul dword ptr [SW_TriPUDX]
        fxch st(1)
        fstp dword ptr [SW_TriPUDY]
        fld dword ptr [ebx+0x4]
        fmul dword ptr [SW_TriPUDY]
        fld dword ptr [SW_TriPU0]
        fxch st(3)
        fstp dword ptr [TriDuzDx16P]
        faddp st(1), st
        fsubp st(1), st
        fstp dword ptr [TriUz0P]
        fld dword ptr [SW_TriPV0]
        fsub dword ptr [TriVz2P]
        fld dword ptr [SW_TriPV0]
        fsub dword ptr [TriVz3P]
        fxch st(1)
        fst dword ptr [SW_TriPD01]
        fmul dword ptr [SW_TriPDY02]
        fxch st(1)
        fst dword ptr [SW_TriPD02]
        fmul dword ptr [SW_TriPDY01]
        fld dword ptr [SW_TriPD01]
        fmul dword ptr [SW_TriPDX02]
        fxch st(1)
        fsubp st(2), st
        fld dword ptr [SW_TriPD02]
        fmul dword ptr [SW_TriPDX01]
        fxch st(2)
        fmul st, st(3)
        fxch st(1)
        fsubp st(2), st
        fst dword ptr [SW_TriPVDX]
        fxch st(1)
        fmul st, st(2)
        fxch st(1)
        fmul dword ptr [SubSpanLength]
        fxch st(1)
        fld dword ptr [ebx]
        fmul dword ptr [SW_TriPVDX]
        fxch st(1)
        fstp dword ptr [SW_TriPVDY]
        fld dword ptr [ebx+0x4]
        fmul dword ptr [SW_TriPVDY]
        fld dword ptr [SW_TriPV0]
        fxch st(3)
        fstp dword ptr [TriDvzDx16P]
        faddp st(1), st
        fsubp st(1), st
        fstp dword ptr [TriVz0P]
        fld dword ptr [ebx+0xc]
        fsub dword ptr [ecx+0xc]
        fld dword ptr [ebx+0xc]
        fsub dword ptr [edx+0xc]
        fxch st(1)
        fst dword ptr [SW_TriPD01]
        fmul dword ptr [SW_TriPDY02]
        fxch st(1)
        fst dword ptr [SW_TriPD02]
        fmul dword ptr [SW_TriPDY01]
        fld dword ptr [SW_TriPD01]
        fmul dword ptr [SW_TriPDX02]
        fxch st(1)
        fsubp st(2), st
        fld dword ptr [SW_TriPD02]
        fmul dword ptr [SW_TriPDX01]
        fxch st(2)
        fmul st, st(3)
        fxch st(1)
        fsubp st(2), st
        fst dword ptr [SW_TriPWDX]
        fxch st(1)
        fmul st, st(2)
        fxch st(1)
        fmul dword ptr [SubSpanLength]
        fxch st(1)
        fld dword ptr [ebx]
        fmul dword ptr [SW_TriPWDX]
        fxch st(1)
        fstp dword ptr [SW_TriPWDY]
        fld dword ptr [ebx+0x4]
        fmul dword ptr [SW_TriPWDY]
        fld dword ptr [ebx+0xc]
        fxch st(3)
        fstp dword ptr [TriDozDx16P]
        faddp st(1), st
        fsubp st(1), st
        fstp dword ptr [TriOz0P]
        fld dword ptr [ebx+0x18]
        fsub dword ptr [ecx+0x18]
        fld dword ptr [ebx+0x1c]
        fsub dword ptr [edx+0x1c]
        fmulp st(1), st
        fld dword ptr [ebx+0x18]
        fsub dword ptr [edx+0x18]
        fld dword ptr [ebx+0x1c]
        fsub dword ptr [ecx+0x1c]
        fmulp st(1), st
        fsubp st(1), st
        fmul st, st(1)
        fstp dword ptr [SW_TriPTemp]
        mov eax, dword ptr [SW_TriPTemp]
        sub eax, 0x400000
        shr eax, 0x17
        and eax, 0xff
        sub eax, 0x6f
        inc eax
        sar eax, 1
        test eax, eax
        jns L3
        mov eax, 0x0
    L3:
        cmp eax, 0x4
        jb L4
        mov eax, 0x4
    L4:
        mov dword ptr [SW_TexShift], eax
        faddp st(0), st
        mov esi, dword ptr [SW_TriPEdgeA]
        mov eax, 0x3
        mov dword ptr [SW_TriPEdges], eax
        mov edi, dword ptr [SW_SpanBuffer]
        fld dword ptr [esi+0x4]
        fsub dword ptr [RoundBias]
        fadd qword ptr [MagicInt]
        fstp qword ptr [SW_TriPY]
        inc dword ptr [SW_TriPY]
        mov eax, dword ptr [SW_TriPY]
        xor edx, edx
        mov dword ptr [SW_SpanY], eax
        jmp L12
        nop
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L5:
        push ecx
        push ebx
        shr ebx, 0x10
        mov eax, esi
        mov ecx, dword ptr [edi]
        shr eax, 0x10
        mov dword ptr [SW_TriPTemp], ebx
        sub eax, ebx
        mov ecx, dword ptr [edi]
        fild dword ptr [SW_TriPY]
        fild dword ptr [SW_TriPTemp]
        mov dword ptr [edi+0x4], eax
        mov dword ptr [edi], ebx
        fld dword ptr [SW_TriPBDX]
        fmul st, st(1)
        fld dword ptr [TriDbDyP]
        fmul st, st(3)
        fld dword ptr [SW_TriPGDX]
        fmul st, st(3)
        fld dword ptr [TriDgDyP]
        fmul st, st(5)
        fxch st(2)
        faddp st(3), st
        fld dword ptr [SW_TriPRDX]
        fmul st, st(4)
        fld dword ptr [TriDrDyP]
        fmul st, st(6)
        fxch st(2)
        faddp st(3), st
        fxch st(3)
        fadd qword ptr [TriB0P]
        fxch st(3)
        faddp st(1), st
        fxch st(1)
        fadd qword ptr [TriG0P]
        fld dword ptr [SW_TriPWDX]
        fmul st, st(4)
        fxch st(3)
        fstp dword ptr [edi+0x18]
        fxch st(1)
        fadd qword ptr [TriR0P]
        fxch st(1)
        fstp dword ptr [SW_TriPTemp]
        fld dword ptr [SW_TriPWDY]
        fmul st, st(4)
        fxch st(1)
        fstp dword ptr [edi+0x1c]
        fld dword ptr [SW_TriPUDX]
        fmul st, st(3)
        fxch st(2)
        faddp st(1), st
        fld dword ptr [SW_TriPUDY]
        fmul st, st(4)
        fxch st(1)
        fadd dword ptr [TriOz0P]
        fld dword ptr [SW_TriPVDX]
        fmul st, st(4)
        fxch st(2)
        faddp st(3), st
        fld dword ptr [SW_TriPVDY]
        fmul st, st(5)
        fxch st(1)
        fstp dword ptr [SW_TriPW]
        mov ebx, dword ptr [SW_TriPW]
        mov ecx, ebx
        shr ebx, 0x9
        xor ecx, 0x7f800000
        and ebx, 0x3ffc
        and ecx, 0xff800000
        faddp st(1), st
        fxch st(1)
        fadd dword ptr [TriUz0P]
        mov ebx, dword ptr [ebx+RecipTable]
        add ecx, ebx
        mov dword ptr [SW_TriPRecip], ecx
        fld st(0)
        fmul dword ptr [SW_TriPRecip]
        fxch st(2)
        fadd dword ptr [TriVz0P]
        fxch st(4)
        faddp st(0), st
        fxch st(2)
        faddp st(0), st
        mov ecx, dword ptr [SW_TriPTemp]
        mov word ptr [edi+0x1a], cx
        fld st(2)
        fmul dword ptr [SW_TriPRecip]
        fxch st(1)
        fst dword ptr [SW_TriPU]
        fadd qword ptr [MagicFix24]
        fxch st(1)
        fst dword ptr [SW_TriPV]
        fld dword ptr [SW_TriPW]
        fxch st(1)
        fadd qword ptr [MagicFix24]
        fxch st(2)
        fstp qword ptr [edi+0x8]
        fxch st(1)
        fstp qword ptr [edi+0x10]
        sub eax, 0x10
        jle L7
    L6:
        fadd dword ptr [TriDozDx16P]
        fxch st(1)
        fadd dword ptr [TriDuzDx16P]
        fxch st(1)
        fxch st(2)
        fadd dword ptr [TriDvzDx16P]
        fxch st(2)
        fst dword ptr [SW_TriPTemp]
        mov ebx, dword ptr [SW_TriPTemp]
        mov ecx, ebx
        shr ebx, 0x9
        xor ecx, 0x7f800000
        and ebx, 0x3ffc
        and ecx, 0xff800000
        fld st(1)
        mov ebx, dword ptr [ebx+RecipTable]
        add ecx, ebx
        mov dword ptr [SW_TriPRecip], ecx
        fmul dword ptr [SW_TriPRecip]
        fld st(3)
        fmul dword ptr [SW_TriPRecip]
        fxch st(1)
        fld st(0)
        fsub dword ptr [SW_TriPU]
        fxch st(1)
        fstp dword ptr [SW_TriPU]
        fmul dword ptr [MipOffsets + 88]
        fxch st(1)
        fld st(0)
        fsub dword ptr [SW_TriPV]
        fxch st(2)
        fadd qword ptr [MagicFix24]
        fxch st(1)
        fstp dword ptr [SW_TriPV]
        fxch st(1)
        fmul dword ptr [MipOffsets + 88]
        fxch st(1)
        fstp qword ptr [edi+0x20]
        fadd qword ptr [MagicFix24]
        add edi, 0x8
        sub eax, 0x10
        fstp qword ptr [SW_TriPTmp]
        mov ebx, dword ptr [SW_TriPTmp]
        mov dword ptr [edi+0x1c], ebx
        jg L6
    L7:
        and eax, 0xf
        fld dword ptr [SW_TriPWDX]
        fmul dword ptr [eax*4+MipOffsets + 96]
        fld dword ptr [SW_TriPUDX]
        fmul dword ptr [eax*4+MipOffsets + 96]
        fxch st(1)
        faddp st(2), st
        fld dword ptr [SW_TriPVDX]
        fmul dword ptr [eax*4+MipOffsets + 96]
        fxch st(2)
        fstp dword ptr [SW_TriPW]
        mov ebx, dword ptr [SW_TriPW]
        mov ecx, ebx
        faddp st(2), st
        shr ebx, 0x9
        xor ecx, 0x7f800000
        and ebx, 0x3ffc
        and ecx, 0xff800000
        faddp st(2), st
        mov ebx, dword ptr [ebx+RecipTable]
        add ecx, ebx
        mov dword ptr [SW_TriPRecip], ecx
        fmul dword ptr [SW_TriPRecip]
        fld dword ptr [eax*4+MipOffsets + 24]
        fxch st(1)
        fsub dword ptr [SW_TriPU]
        fxch st(2)
        fmul dword ptr [SW_TriPRecip]
        add edi, 0x28
        mov eax, dword ptr [TriLeftXStepP]
        mov ecx, dword ptr [TriRightXStepP]
        pop ebx
        fsub dword ptr [SW_TriPV]
        fxch st(2)
        fmul st, st(1)
        add ebx, eax
        mov eax, dword ptr [SW_TriPY]
        add esi, ecx
        inc eax
        fadd qword ptr [MagicFix24]
        fxch st(2)
        fmulp st(1), st
        mov dword ptr [SW_TriPY], eax
        pop ecx
        dec edx
        dec ecx
        fadd qword ptr [MagicFix24]
        fxch st(1)
        fstp qword ptr [edi-0x8]
        fstp qword ptr [SW_TriPTmp]
        mov eax, dword ptr [SW_TriPTmp]
        mov dword ptr [edi-0x4], eax
        je L12
        test edx, edx
        je L13
        jmp L5
    L8:
        mov dword ptr [SW_SpanEnd], edi
        cmp dword ptr [SW_TexShift], 0x0
        jne L9
        call dword ptr [pDrawSpansFilteredP]
        pop ebx
        pop esi
        pop edi
        ret
    L9:
        cmp dword ptr [bMMXMipMap], 0x1
        je L10
        mov dword ptr [SW_TexShift], 0x0
    L10:
        call dword ptr [pDrawSpansP]
        pop ebx
        pop esi
        pop edi
        ret
        nop
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
    L11:
        faddp st(0), st
        faddp st(0), st
        pop ebx
        pop esi
        pop edi
        ret
        nop
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L12:
        mov esi, dword ptr [SW_TriPEdgeA]
        mov eax, dword ptr [SW_TriPEdges]
        mov ecx, esi
        dec eax
        js L8
        mov ecx, dword ptr [ecx+0x8]
        mov dword ptr [SW_TriPEdges], eax
        mov dword ptr [SW_TriPEdgeA], ecx
        fld dword ptr [esi]
        fsub dword ptr [ecx]
        fld dword ptr [esi+0x4]
        fsub dword ptr [ecx+0x4]
        fdivp st(1), st
        fld st(0)
        fadd qword ptr [MagicFix16]
        fstp qword ptr [TriRightXStepP]
        fild dword ptr [SW_TriPY]
        fsub dword ptr [esi+0x4]
        fmulp st(1), st
        fadd dword ptr [esi]
        fadd qword ptr [MagicFix16]
        fstp qword ptr [SW_TriPTmp]
        fld dword ptr [ecx+0x4]
        fsub dword ptr [RoundBias]
        mov esi, dword ptr [SW_TriPTmp]
        fadd qword ptr [MagicInt]
        fstp qword ptr [SW_TriPTmp]
        mov ecx, dword ptr [SW_TriPTmp]
        add esi, 0xffff
        sub ecx, dword ptr [SW_TriPY]
        inc ecx
        je L12
        test edx, edx
        jne L14
    L13:
        mov edx, dword ptr [SW_TriPEdgeB]
        mov eax, dword ptr [SW_TriPEdges]
        mov ebx, edx
        dec eax
        js L8
        mov ebx, dword ptr [ebx+0x14]
        mov dword ptr [SW_TriPEdges], eax
        mov dword ptr [SW_TriPEdgeB], ebx
        fld dword ptr [edx]
        fsub dword ptr [ebx]
        fld dword ptr [edx+0x4]
        fsub dword ptr [ebx+0x4]
        fdivp st(1), st
        fld st(0)
        fadd qword ptr [MagicFix16]
        fstp qword ptr [TriLeftXStepP]
        fild dword ptr [SW_TriPY]
        fsub dword ptr [edx+0x4]
        fmulp st(1), st
        fadd dword ptr [edx]
        fadd qword ptr [MagicFix16]
        fstp qword ptr [SW_TriPTmp]
        fld dword ptr [ebx+0x4]
        fsub dword ptr [RoundBias]
        mov ebx, dword ptr [SW_TriPTmp]
        fadd qword ptr [MagicInt]
        add ebx, 0xffff
        fstp qword ptr [SW_TriPTmp]
        mov edx, dword ptr [SW_TriPTmp]
        sub edx, dword ptr [SW_TriPY]
        inc edx
        je L13
    L14:
        jmp L5
    }
}
#else
void SW_TriSetupGTP(D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3)
{
    D3DTLVERTEX *t, *n;
    unsigned char *span;
    double a, b, g, h, inv, d01, d02, x, y, s, uz, vz, w, u, v, du, dv;
    unsigned long sy, c, xa, xb;
    long na, nb, e, cnt;

    SW_SetF(&TriVz3P, (double)v3->tv * v3->rhw);
    SW_SetF(&TriUz3P, (double)v3->tu * v3->rhw);
    SW_SetF(&TriVz2P, (double)v2->tv * v2->rhw);
    SW_SetF(&TriUz2P, (double)v2->tu * v2->rhw);
    SW_SetF(&SW_TriPV0, (double)v1->tv * v1->rhw);
    SW_SetF(&SW_TriPU0, (double)v1->tu * v1->rhw);
    a = (double)v1->sx - v2->sx;
    b = (double)v1->sy - v3->sy;
    SW_SetF(&SW_TriPDX01, a);
    SW_SetF(&SW_TriPDY02, b);
    g = a * b;
    a = (double)v1->sx - v3->sx;
    b = (double)v1->sy - v2->sy;
    SW_SetF(&SW_TriPDX02, a);
    SW_SetF(&SW_TriPDY01, b);
    g = g - a * b;
    SW_SetF(&SW_TriPTemp, g);
    if (SW_TriPTemp & 0x80000000)
        return;
    inv = 1.0 / g;
    *(D3DTLVERTEX **)&v1->sz = v2;
    *(D3DTLVERTEX **)&v2->sz = v3;
    *(D3DTLVERTEX **)&v3->sz = v1;
    *(D3DTLVERTEX **)&v1->specular = v3;
    *(D3DTLVERTEX **)&v2->specular = v1;
    *(D3DTLVERTEX **)&v3->specular = v2;
    t = v1;
    sy = *(unsigned long *)&v1->sy;
    if (sy >= *(unsigned long *)&v2->sy) {
        sy = *(unsigned long *)&v2->sy;
        t = v2;
    }
    if (sy >= *(unsigned long *)&v3->sy)
        t = v3;
    SW_TriPEdgeA = (long)t;
    SW_TriPEdgeB = (long)t;
    TriColor = v1->color;
    c = v1->color;
    SW_TriPG0 = (c & 0xff00) + 0x80;
    SW_TriPB0 = ((c << 8) & 0xff00) + 0x80;
    SW_TriPR0 = ((c >> 8) & 0xff00) + 0x80;
    c = v2->color;
    TriG2P = (c & 0xff00) + 0x80;
    TriB2P = ((c << 8) & 0xff00) + 0x80;
    TriR2P = ((c >> 8) & 0xff00) + 0x80;
    c = v3->color;
    TriG3P = (c & 0xff00) + 0x80;
    TriB3P = ((c << 8) & 0xff00) + 0x80;
    TriR3P = ((c >> 8) & 0xff00) + 0x80;

    d01 = (double)SW_TriPR0 - TriR2P;
    d02 = (double)SW_TriPR0 - TriR3P;
    SW_SetF(&SW_TriPD01, d01);
    SW_SetF(&SW_TriPD02, d02);
    g = (d01 * SW_VF(SW_TriPDY02) - d02 * SW_VF(SW_TriPDY01)) * inv;
    ((short *)&SW_SpanGrad)[2] = SW_RoundW(g);
    SW_SetF(&SW_TriPRDX, g);
    h = (SW_VF(SW_TriPD02) * SW_VF(SW_TriPDX01) - SW_VF(SW_TriPD01) * SW_VF(SW_TriPDX02)) * inv;
    SW_SetF(&TriDrDyP, h);
    TriR0P = ((double)SW_TriPR0 + MagicFloat) - (v1->sy * h + v1->sx * SW_VF(SW_TriPRDX));

    d01 = (double)SW_TriPG0 - TriG2P;
    d02 = (double)SW_TriPG0 - TriG3P;
    SW_SetF(&SW_TriPD01, d01);
    SW_SetF(&SW_TriPD02, d02);
    g = (d01 * SW_VF(SW_TriPDY02) - d02 * SW_VF(SW_TriPDY01)) * inv;
    ((short *)&SW_SpanGrad)[1] = SW_RoundW(g);
    SW_SetF(&SW_TriPGDX, g);
    h = (SW_VF(SW_TriPD02) * SW_VF(SW_TriPDX01) - SW_VF(SW_TriPD01) * SW_VF(SW_TriPDX02)) * inv;
    SW_SetF(&TriDgDyP, h);
    TriG0P = ((double)SW_TriPG0 + MagicFloat) - (v1->sy * h + v1->sx * SW_VF(SW_TriPGDX));

    d01 = (double)SW_TriPB0 - TriB2P;
    d02 = (double)SW_TriPB0 - TriB3P;
    SW_SetF(&SW_TriPD01, d01);
    SW_SetF(&SW_TriPD02, d02);
    g = (d01 * SW_VF(SW_TriPDY02) - d02 * SW_VF(SW_TriPDY01)) * inv;
    ((short *)&SW_SpanGrad)[0] = SW_RoundW(g);
    SW_SetF(&SW_TriPBDX, g);
    h = (SW_VF(SW_TriPD02) * SW_VF(SW_TriPDX01) - SW_VF(SW_TriPD01) * SW_VF(SW_TriPDX02)) * inv;
    SW_SetF(&TriDbDyP, h);
    TriB0P = ((double)SW_TriPB0 + MagicFloat) - (v1->sy * h + v1->sx * SW_VF(SW_TriPBDX));

    d01 = (double)SW_F(SW_TriPU0) - SW_F(TriUz2P);
    d02 = (double)SW_F(SW_TriPU0) - SW_F(TriUz3P);
    SW_SetF(&SW_TriPD01, d01);
    SW_SetF(&SW_TriPD02, d02);
    g = (d01 * SW_F(SW_TriPDY02) - d02 * SW_F(SW_TriPDY01)) * inv;
    SW_SetF(&SW_TriPUDX, g);
    h = (SW_F(SW_TriPD02) * SW_F(SW_TriPDX01) - SW_F(SW_TriPD01) * SW_F(SW_TriPDX02)) * inv;
    SW_SetF(&SW_TriPUDY, h);
    SW_SetF(&TriDuzDx16P, g * SW_F(SubSpanLength));
    SW_SetF(&TriUz0P, (double)SW_F(SW_TriPU0) - (v1->sx * SW_F(SW_TriPUDX) + v1->sy * SW_F(SW_TriPUDY)));
    d01 = (double)SW_F(SW_TriPV0) - SW_F(TriVz2P);
    d02 = (double)SW_F(SW_TriPV0) - SW_F(TriVz3P);
    SW_SetF(&SW_TriPD01, d01);
    SW_SetF(&SW_TriPD02, d02);
    g = (d01 * SW_F(SW_TriPDY02) - d02 * SW_F(SW_TriPDY01)) * inv;
    SW_SetF(&SW_TriPVDX, g);
    h = (SW_F(SW_TriPD02) * SW_F(SW_TriPDX01) - SW_F(SW_TriPD01) * SW_F(SW_TriPDX02)) * inv;
    SW_SetF(&SW_TriPVDY, h);
    SW_SetF(&TriDvzDx16P, g * SW_F(SubSpanLength));
    SW_SetF(&TriVz0P, (double)SW_F(SW_TriPV0) - (v1->sx * SW_F(SW_TriPVDX) + v1->sy * SW_F(SW_TriPVDY)));
    d01 = (double)v1->rhw - v2->rhw;
    d02 = (double)v1->rhw - v3->rhw;
    SW_SetF(&SW_TriPD01, d01);
    SW_SetF(&SW_TriPD02, d02);
    g = (d01 * SW_F(SW_TriPDY02) - d02 * SW_F(SW_TriPDY01)) * inv;
    SW_SetF(&SW_TriPWDX, g);
    h = (SW_F(SW_TriPD02) * SW_F(SW_TriPDX01) - SW_F(SW_TriPD01) * SW_F(SW_TriPDX02)) * inv;
    SW_SetF(&SW_TriPWDY, h);
    SW_SetF(&TriDozDx16P, g * SW_F(SubSpanLength));
    SW_SetF(&TriOz0P, (double)v1->rhw - (v1->sx * SW_F(SW_TriPWDX) + v1->sy * SW_F(SW_TriPWDY)));
    a = ((double)v1->tu - v2->tu) * ((double)v1->tv - v3->tv);
    b = ((double)v1->tu - v3->tu) * ((double)v1->tv - v2->tv);
    SW_SetF(&SW_TriPTemp, (a - b) * inv);
    e = ((unsigned long)(SW_TriPTemp - 0x400000) >> 23) & 0xff;
    e = (e - 0x6f + 1) >> 1;
    if (e < 0)
        e = 0;
    if ((unsigned long)e >= 4)
        e = 4;
    SW_TexShift = e;

    t = (D3DTLVERTEX *)SW_TriPEdgeA;
    SW_TriPEdges = 3;
    span = SW_SpanBuffer;
    SW_TriPY = ((double)t->sy - SW_F(RoundBias)) + MagicInt;
    SW_L(SW_TriPY)++;
    SW_SpanY = SW_L(SW_TriPY);
    nb = 0;
    na = 0;
    xa = 0;
    xb = 0;

edge_a:
    t = (D3DTLVERTEX *)SW_TriPEdgeA;
    if (SW_TriPEdges - 1 < 0)
        goto done;
    n = *(D3DTLVERTEX **)&t->sz;
    SW_TriPEdges--;
    SW_TriPEdgeA = (long)n;
    s = ((double)t->sx - n->sx) / ((double)t->sy - n->sy);
    TriRightXStepP = s + MagicFix16;
    SW_TriPTmp = (((double)SW_L(SW_TriPY) - t->sy) * s + t->sx) + MagicFix16;
    xa = SW_L(SW_TriPTmp) + 0xffff;
    SW_TriPTmp = ((double)n->sy - SW_F(RoundBias)) + MagicInt;
    na = SW_L(SW_TriPTmp) - SW_L(SW_TriPY) + 1;
    if (!na)
        goto edge_a;
    if (nb)
        goto spans;

edge_b:
    t = (D3DTLVERTEX *)SW_TriPEdgeB;
    if (SW_TriPEdges - 1 < 0)
        goto done;
    n = *(D3DTLVERTEX **)&t->specular;
    SW_TriPEdges--;
    SW_TriPEdgeB = (long)n;
    s = ((double)t->sx - n->sx) / ((double)t->sy - n->sy);
    TriLeftXStepP = s + MagicFix16;
    SW_TriPTmp = (((double)SW_L(SW_TriPY) - t->sy) * s + t->sx) + MagicFix16;
    xb = SW_L(SW_TriPTmp) + 0xffff;
    SW_TriPTmp = ((double)n->sy - SW_F(RoundBias)) + MagicInt;
    nb = SW_L(SW_TriPTmp) - SW_L(SW_TriPY) + 1;
    if (!nb)
        goto edge_b;

spans:
    SW_TriPTemp = xb >> 16;
    cnt = (xa >> 16) - (xb >> 16);
    ((long *)span)[1] = cnt;
    ((long *)span)[0] = xb >> 16;
    y = (double)SW_L(SW_TriPY);
    x = (double)SW_TriPTemp;
    *(float *)(span + 24) = (float)((SW_F(SW_TriPBDX) * x + SW_F(TriDbDyP) * y) + TriB0P);
    SW_SetF(&SW_TriPTemp, (SW_F(SW_TriPGDX) * x + SW_F(TriDgDyP) * y) + TriG0P);
    *(float *)(span + 28) = (float)((SW_F(SW_TriPRDX) * x + SW_F(TriDrDyP) * y) + TriR0P);
    SW_SetF(&SW_TriPW, (SW_F(SW_TriPWDX) * x + SW_F(SW_TriPWDY) * y) + SW_F(TriOz0P));
    uz = (SW_F(SW_TriPUDX) * x + SW_F(SW_TriPUDY) * y) + SW_F(TriUz0P);
    vz = (SW_F(SW_TriPVDX) * x + SW_F(SW_TriPVDY) * y) + SW_F(TriVz0P);
    SW_TriPRecip = SW_Recip(SW_TriPW);
    u = uz * SW_F(SW_TriPRecip);
    v = vz * SW_F(SW_TriPRecip);
    *(unsigned short *)(span + 26) = (unsigned short)SW_TriPTemp;
    SW_SetF(&SW_TriPU, u);
    SW_SetF(&SW_TriPV, v);
    w = SW_F(SW_TriPW);
    *(double *)(span + 8) = u + MagicFix24;
    *(double *)(span + 16) = v + MagicFix24;
    cnt -= 16;
    while (cnt > 0) {
        w += SW_F(TriDozDx16P);
        uz += SW_F(TriDuzDx16P);
        vz += SW_F(TriDvzDx16P);
        SW_SetF(&SW_TriPTemp, w);
        SW_TriPRecip = SW_Recip(SW_TriPTemp);
        u = uz * SW_F(SW_TriPRecip);
        v = vz * SW_F(SW_TriPRecip);
        du = u - SW_F(SW_TriPU);
        SW_SetF(&SW_TriPU, u);
        dv = v - SW_F(SW_TriPV);
        SW_SetF(&SW_TriPV, v);
        *(double *)(span + 32) = du * SW_F(MipOffsets[22]) + MagicFix24;
        span += 8;
        SW_TriPTmp = dv * SW_F(MipOffsets[22]) + MagicFix24;
        *(long *)(span + 28) = SW_L(SW_TriPTmp);
        cnt -= 16;
    }
    cnt &= 0xf;
    w += SW_F(SW_TriPWDX) * SW_F(MipOffsets[24 + cnt]);
    uz += SW_F(SW_TriPUDX) * SW_F(MipOffsets[24 + cnt]);
    vz += SW_F(SW_TriPVDX) * SW_F(MipOffsets[24 + cnt]);
    SW_SetF(&SW_TriPW, w);
    SW_TriPRecip = SW_Recip(SW_TriPW);
    du = uz * SW_F(SW_TriPRecip) - SW_F(SW_TriPU);
    dv = vz * SW_F(SW_TriPRecip) - SW_F(SW_TriPV);
    span += 40;
    xb += SW_L(TriLeftXStepP);
    xa += SW_L(TriRightXStepP);
    SW_L(SW_TriPY)++;
    *(double *)(span - 8) = du * SW_F(MipOffsets[6 + cnt]) + MagicFix24;
    SW_TriPTmp = dv * SW_F(MipOffsets[6 + cnt]) + MagicFix24;
    *(long *)(span - 4) = SW_L(SW_TriPTmp);
    nb--;
    na--;
    if (!na)
        goto edge_a;
    if (!nb)
        goto edge_b;
    goto spans;

done:
    SW_SpanEnd = (long)span;
    if (!SW_TexShift) {
        pDrawSpansFilteredP();
        return;
    }
    if (bMMXMipMap != 1)
        SW_TexShift = 0;
    pDrawSpansP();
}
#endif

void MMXBlit32to16(void *dest, void *src, int w)
{
    unsigned long rmask[2];
    unsigned long gmask[2];
    unsigned long bmask[2];

    rmask[0] = 0xf80000;
    rmask[1] = 0xf80000;
    gmask[0] = 0xfc00;
    gmask[1] = 0xfc00;
    bmask[0] = 0xf8;
    bmask[1] = 0xf8;
    __asm {
        mov eax, dword ptr [w]
        shr eax, 0x2
        mov esi, dword ptr [src]
        mov edi, dword ptr [dest]
        movq mm3, qword ptr [rmask]
        movq mm7, qword ptr [gmask]
        _emit 0x8d
        _emit 0x49
        _emit 0x00
    L1:
        movq mm0, qword ptr [esi]
        movq mm4, qword ptr [esi+0x8]
        _emit 0x0f
        _emit 0x7f
        _emit 0xc1
        _emit 0x0f
        _emit 0x7f
        _emit 0xc2
        _emit 0x0f
        _emit 0x7f
        _emit 0xe5
        _emit 0x0f
        _emit 0x7f
        _emit 0xe6
        pand mm0, mm3
        pand mm1, mm7
        pand mm2, qword ptr [bmask]
        pand mm4, mm3
        pand mm5, mm7
        pand mm6, qword ptr [bmask]
        pslld mm0, 0x8
        pslld mm4, 0x8
        psrad mm0, 0x10
        psrld mm1, 0x5
        psrld mm2, 0x3
        psrad mm4, 0x10
        psrld mm5, 0x5
        psrld mm6, 0x3
        por mm0, mm1
        por mm4, mm5
        por mm0, mm2
        por mm4, mm6
        packssdw mm0, mm4
        movq qword ptr [edi], mm0
        add edi, 0x8
        add esi, 0x10
        dec eax
        jne L1
        emms
    }
}

void MMXBlit32to24(void *dest, void *src, int w)
{
    unsigned long mask1[2];
    unsigned long mask2[2];
    unsigned long mask3[2];
    unsigned long mask4[2];

    mask1[0] = 0xffffff;
    mask1[1] = 0;
    mask3[0] = 0;
    mask3[1] = 0xffffff;
    mask2[0] = 0xffff;
    mask2[1] = 0;
    mask4[0] = 0xff0000;
    mask4[1] = 0;
    __asm {
        mov esi, dword ptr [src]
        mov edi, dword ptr [dest]
        mov eax, dword ptr [w]
    L1:
        movq mm0, qword ptr [esi]
        movq mm1, qword ptr [esi+0x8]
        _emit 0x0f
        _emit 0x7f
        _emit 0xc2
        _emit 0x0f
        _emit 0x7f
        _emit 0xcb
        sub eax, 0x4
        _emit 0x0f
        _emit 0x7f
        _emit 0xcc
        pand mm0, qword ptr [mask1]
        pand mm1, qword ptr [mask2]
        pand mm2, qword ptr [mask3]
        psrlq mm2, 0x8
        psllq mm1, 0x30
        por mm1, mm2
        por mm0, mm1
        pand mm3, qword ptr [mask4]
        pand mm4, qword ptr [mask3]
        psrlq mm3, 0x10
        psrlq mm4, 0x18
        por mm3, mm4
        lea esi, [esi+0x10]
        movq qword ptr [edi], mm0
        movd dword ptr [edi+0x8], mm3
        lea edi, [edi+0xc]
        jne L1
        emms
    }
}

void MMXBlit32to15(void *dest, void *src, int w)
{
    unsigned long mult[2];
    unsigned long gmask[2];
    unsigned long rbmask[2];

    mult[0] = 0x20000008;
    mult[1] = 0x20000008;
    rbmask[0] = 0xf800f8;
    rbmask[1] = 0xf800f8;
    gmask[0] = 0xf800;
    gmask[1] = 0xf800;
    __asm {
        mov eax, dword ptr [w]
        mov ebx, dword ptr [src]
        sub eax, 0x1
        mov edx, dword ptr [dest]
        and eax, 0xfffffff8
        movq mm7, qword ptr [mult]
        movq mm6, qword ptr [gmask]
        movq mm2, qword ptr [ebx+eax*4+0x8]
        movq mm0, qword ptr [ebx+eax*4]
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        pand mm3, qword ptr [rbmask]
        _emit 0x0f
        _emit 0x7f
        _emit 0xc1
        pand mm1, qword ptr [rbmask]
        pmaddwd mm3, mm7
        pmaddwd mm1, mm7
        pand mm2, mm6
    L1:
        movq mm4, qword ptr [ebx+eax*4+0x18]
        pand mm0, mm6
        movq mm5, qword ptr [ebx+eax*4+0x10]
        por mm3, mm2
        psrld mm3, 0x6
        por mm1, mm0
        _emit 0x0f
        _emit 0x7f
        _emit 0xe0
        psrld mm1, 0x6
        pand mm0, qword ptr [rbmask]
        packssdw mm1, mm3
        _emit 0x0f
        _emit 0x7f
        _emit 0xeb
        pmaddwd mm0, mm7
        pand mm3, qword ptr [rbmask]
        pand mm4, mm6
        movq qword ptr [edx+eax*2], mm1
        pmaddwd mm3, mm7
        sub eax, 0x8
        por mm4, mm0
        pand mm5, mm6
        psrld mm4, 0x6
        movq mm2, qword ptr [ebx+eax*4+0x8]
        por mm5, mm3
        movq mm0, qword ptr [ebx+eax*4]
        psrld mm5, 0x6
        _emit 0x0f
        _emit 0x7f
        _emit 0xd3
        _emit 0x0f
        _emit 0x7f
        _emit 0xc1
        pand mm3, qword ptr [rbmask]
        packssdw mm5, mm4
        pand mm1, qword ptr [rbmask]
        pand mm2, mm6
        movq qword ptr [edx+eax*2+0x18], mm5
        pmaddwd mm3, mm7
        pmaddwd mm1, mm7
        jg L1
        movq mm4, qword ptr [ebx+eax*4+0x18]
        pand mm0, mm6
        movq mm5, qword ptr [ebx+eax*4+0x10]
        por mm3, mm2
        psrld mm3, 0x6
        por mm1, mm0
        _emit 0x0f
        _emit 0x7f
        _emit 0xe0
        psrld mm1, 0x6
        pand mm0, qword ptr [rbmask]
        packssdw mm1, mm3
        _emit 0x0f
        _emit 0x7f
        _emit 0xeb
        pmaddwd mm0, mm7
        pand mm3, qword ptr [rbmask]
        pand mm4, mm6
        movq qword ptr [edx+eax*2], mm1
        pmaddwd mm3, mm7
        sub eax, 0x8
        por mm4, mm0
        pand mm5, mm6
        psrld mm4, 0x6
        por mm5, mm3
        psrld mm5, 0x6
        packssdw mm5, mm4
        movq qword ptr [edx+eax*2+0x18], mm5
        emms
    }
}

extern long bMMXAlphaBlend;
extern long bMMXColorKey;
extern long bMMXDestBlendOne;
extern long bMMXDestBlendAlpha;
extern long bMMXBilinear;

HRESULT MMXSetRenderState(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState)
{
    switch (dwRenderStateType) {
    case D3DRENDERSTATE_COLORKEYENABLE:
        bMMXColorKey = dwRenderState;
        break;
    case D3DRENDERSTATE_TEXTUREMIN:
        if (dwRenderState == D3DFILTER_LINEAR || dwRenderState == D3DFILTER_NEAREST)
            bMMXMipMap = 0;
        else
            bMMXMipMap = 1;
        break;
    case D3DRENDERSTATE_TEXTUREMAG:
        if (dwRenderState == D3DFILTER_LINEAR || dwRenderState == D3DFILTER_MIPLINEAR)
            bMMXBilinear = 1;
        else
            bMMXBilinear = 0;
        break;
    case D3DRENDERSTATE_DESTBLEND:
        if (dwRenderState == D3DBLEND_ONE) {
            bMMXDestBlendOne = 1;
            bMMXDestBlendAlpha = 0;
        } else if (dwRenderState == D3DBLEND_SRCALPHA || dwRenderState == D3DBLEND_INVSRCALPHA) {
            bMMXDestBlendOne = 0;
            bMMXDestBlendAlpha = 1;
        } else {
            bMMXDestBlendOne = 0;
            bMMXDestBlendAlpha = 0;
        }
        break;
    case D3DRENDERSTATE_ALPHABLENDENABLE:
        bMMXAlphaBlend = dwRenderState == TRUE;
        break;
    case D3DRENDERSTATE_TEXTUREPERSPECTIVE:
        bMMXPerspective = dwRenderState;
        break;
    case D3DRENDERSTATE_TEXTUREHANDLE:
        SW_TextureBits = dwRenderState;
        break;
    }
    if (bMMXDestBlendAlpha && bMMXAlphaBlend && !SW_TextureBits) {
        pMMXDrawTriangleP = MMXDrawTriangleFog;
        pDrawSpansP = DrawSpansAlpha;
        pMMXDrawTriangle = MMXDrawTriangleFog;
        pDrawSpans = DrawSpansAlpha;
        return D3D_OK;
    }
    if (!bMMXDestBlendOne && bMMXColorKey && SW_TextureBits && bMMXAlphaBlend) {
        pMMXDrawTriangleP = SW_TriSetupGTP;
        pDrawSpansP = SW_SpanGTPKey;
        pMMXDrawTriangle = SW_TriSetupGT;
        pDrawSpans = SW_SpanGTKey;
        if (bMMXBilinear) {
            pDrawSpansFiltered = DrawSpansColorKeyFiltered;
            pDrawSpansFilteredP = DrawSpansColorKeyFilteredP;
        } else {
            pDrawSpansFiltered = SW_SpanGTKey;
            pDrawSpansFilteredP = SW_SpanGTPKey;
        }
        return D3D_OK;
    }
    if (SW_TextureBits && bMMXAlphaBlend && bMMXDestBlendOne) {
        pMMXDrawTriangleP = SW_TriSetupGTP;
        pDrawSpansP = SW_SpanGTPAdd;
        pMMXDrawTriangle = SW_TriSetupGT;
        pDrawSpans = SW_SpanGTAdd;
        if (bMMXBilinear) {
            pDrawSpansFiltered = SW_SpanGTAddBilinear;
            pDrawSpansFilteredP = SW_SpanGTPAddBilinear;
        } else {
            pDrawSpansFiltered = SW_SpanGTAdd;
            pDrawSpansFilteredP = SW_SpanGTPAdd;
        }
        return D3D_OK;
    }
    if (SW_TextureBits) {
        pMMXDrawTriangleP = SW_TriSetupGTP;
        pDrawSpansP = SW_SpanGTP;
        pMMXDrawTriangle = SW_TriSetupGT;
        pDrawSpans = SW_SpanGT;
        if (bMMXBilinear) {
            pDrawSpansFiltered = SW_SpanGTBilinear;
            pDrawSpansFilteredP = SW_SpanGTPBilinear;
        } else {
            pDrawSpansFiltered = SW_SpanGT;
            pDrawSpansFilteredP = SW_SpanGTP;
        }
        return D3D_OK;
    }
    pMMXDrawTriangleP = MMXDrawTriangleFog;
    pDrawSpansP = SW_SpanG;
    pMMXDrawTriangle = MMXDrawTriangleFog;
    pDrawSpans = SW_SpanG;
    return D3D_OK;
}

extern void (*pScanLinesTGAffine)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*pScanLinesTGDecalAffine)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*pScanLinesTGAlphaDecalAffine)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*pScanLinesTGAlphaDecal)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*pScanLinesTG)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*pScanLinesTGDecal)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*pScanLinesBlack)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*pScanLinesTF)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*pScanLinesTFAffine)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*pScanLinesTFDecal)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*pScanLinesTFDecalAffine)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*pScanLinesTFAlphaDecalAffine)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*pScanLinesTFAlphaDecal)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*pScanLinesG)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*pScanLinesGAlpha)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*pScanLinesBlackAlpha)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*pScanLinesNull)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);

HRESULT EmulateDrawPrimitive(D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags)
{
    DDSURFACEDESC ddsd;

    memset(&ddsd, 0, sizeof(ddsd));
    PrepareTriangleList((LPD3DTLVERTEX)lpvVertices, dwVertexCount);
    if (TextureHandle != NULL) {
        DrawScanLines = pScanLinesTG;
        CalcGradientsPersp = CalculateGradientsTG;
        CalcEdgePersp = CalculateEdgeTG;
        DrawScanLinesAffine = pScanLinesTGAffine;
        CalcGradientsAffine = CalculateGradientsTGA;
        CalcEdgeAffine = CalculateEdgeTGA;
        DrawScanLinesBlack = pScanLinesBlack;
        CalcGradientsBlack = NullCalculateGradient;
        CalcEdgeBlack = CalculateEdgeBlack;
        DrawScanLinesFlat = pScanLinesTF;
        CalcGradientsFlat = CalculateGradientsTF;
        CalcEdgeFlat = CalculateEdgeT;
        DrawScanLinesFlatAffine = pScanLinesTFAffine;
        CalcGradientsFlatAffine = CalculateGradientsTFA;
        CalcEdgeFlatAffine = CalculateEdgeTA;
        if (bColorKey) {
            DrawScanLines = pScanLinesTGDecal;
            CalcGradientsPersp = CalculateGradientsTG;
            CalcEdgePersp = CalculateEdgeTG;
            DrawScanLinesAffine = pScanLinesTGDecalAffine;
            CalcGradientsAffine = CalculateGradientsTGA;
            CalcEdgeAffine = CalculateEdgeTGA;
            DrawScanLinesBlack = pScanLinesTGDecalAffine;
            CalcGradientsBlack = CalculateGradientsTGA;
            CalcEdgeBlack = CalculateEdgeTGA;
            DrawScanLinesFlatAffine = pScanLinesTFDecalAffine;
            CalcGradientsFlatAffine = CalculateGradientsTFA;
            CalcEdgeFlatAffine = CalculateEdgeTA;
            DrawScanLinesFlat = pScanLinesTFDecal;
            CalcGradientsFlat = CalculateGradientsTF;
            CalcEdgeFlat = CalculateEdgeT;
        }
        if (bAlphaBlend && bSrcBlendOne) {
            DrawScanLines = pScanLinesTGAlphaDecal;
            CalcGradientsPersp = CalculateGradientsTG;
            CalcEdgePersp = CalculateEdgeTG;
            DrawScanLinesAffine = pScanLinesTGAlphaDecalAffine;
            CalcGradientsAffine = CalculateGradientsTGA;
            CalcEdgeAffine = CalculateEdgeTGA;
            DrawScanLinesBlack = pScanLinesNull;
            CalcGradientsBlack = NullCalculateGradient;
            CalcEdgeBlack = NullCalculateEdge;
            DrawScanLinesFlatAffine = pScanLinesTFAlphaDecalAffine;
            CalcGradientsFlatAffine = CalculateGradientsTFA;
            CalcEdgeFlatAffine = CalculateEdgeTA;
            DrawScanLinesFlat = pScanLinesTFAlphaDecal;
            CalcGradientsFlat = CalculateGradientsTF;
            CalcEdgeFlat = CalculateEdgeT;
        }
    } else {
        DrawScanLines = pScanLinesG;
        CalcGradientsPersp = CalculateGradientsG;
        CalcEdgePersp = CalculateEdgeG;
        DrawScanLinesAffine = pScanLinesG;
        CalcGradientsAffine = CalculateGradientsG;
        CalcEdgeAffine = CalculateEdgeG;
        DrawScanLinesBlack = pScanLinesBlack;
        CalcGradientsBlack = NullCalculateGradient;
        CalcEdgeBlack = CalculateEdgeBlack;
        DrawScanLinesFlat = pScanLinesG;
        CalcGradientsFlat = CalculateGradientsG;
        CalcEdgeFlat = CalculateEdgeG;
        DrawScanLinesFlatAffine = pScanLinesG;
        CalcGradientsFlatAffine = CalculateGradientsG;
        CalcEdgeFlatAffine = CalculateEdgeG;
        if (bAlphaBlend) {
            DrawScanLines = pScanLinesGAlpha;
            CalcGradientsPersp = CalculateGradientsG;
            CalcEdgePersp = CalculateEdgeG;
            DrawScanLinesAffine = pScanLinesGAlpha;
            CalcGradientsAffine = CalculateGradientsG;
            CalcEdgeAffine = CalculateEdgeG;
            DrawScanLinesBlack = pScanLinesBlackAlpha;
            CalcGradientsBlack = CalculateGradientsG;
            CalcEdgeBlack = CalculateEdgeG;
            DrawScanLinesFlat = pScanLinesGAlpha;
            CalcGradientsFlat = CalculateGradientsG;
            CalcEdgeFlat = CalculateEdgeG;
            DrawScanLinesFlatAffine = pScanLinesGAlpha;
            CalcGradientsFlatAffine = CalculateGradientsG;
            CalcEdgeFlatAffine = CalculateEdgeG;
        }
    }
    switch (dptPrimitiveType) {
    case D3DPT_TRIANGLELIST:
        DrawTriangleList((LPD3DTLVERTEX)lpvVertices, dwVertexCount);
        break;
    case D3DPT_TRIANGLEFAN:
        DrawTriangleFan((LPD3DTLVERTEX)lpvVertices, dwVertexCount);
        break;
    case D3DPT_TRIANGLESTRIP:
        DrawTriangleStrip((LPD3DTLVERTEX)lpvVertices, dwVertexCount);
        break;
    case D3DPT_LINELIST:
        DrawLineList((LPD3DTLVERTEX)lpvVertices, dwVertexCount);
        break;
    case D3DPT_LINESTRIP:
        DrawLineStrip((LPD3DTLVERTEX)lpvVertices, dwVertexCount);
        break;
    }
    return D3D_OK;
}

extern void (*pScanLinesTAffine)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*pScanLinesTDecalAffine)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*pScanLinesT)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern void (*pScanLinesTDecal)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
extern long lpTexture;
extern int bForceAffineMap;
void MMXInit(void);
HRESULT MMXSetRenderState(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState);
void DrawLineG(D3DTLVERTEX *v1, D3DTLVERTEX *v2);
void ScanLinesTAffine(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTDecalAffine(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesT(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTDecal(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTGAffine(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTF(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTFAffine(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTGDecalAffine(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTFDecal(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTFDecalAffine(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesG(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesGAlpha(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesBlackAlpha(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTFAlphaDecal(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTFAlphaDecalAffine(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTGAlphaDecal(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTGAlphaDecalAffine(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesBlackStipple(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesGStipple(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTFStippleDecalAffine(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTFStippleDecal(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTGStippleDecalAffine(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTGStippleDecal(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);

void InitDrawPrimitive(LPDIRECT3DDEVICE2 lpD3D, LPDIRECTDRAWSURFACE3 lpSurf, bool HW)
{
    int i;

    D3DDev = lpD3D;
    lpTexture = 0;
    if (HW) {
        DrawPrimitive = HWDrawPrimitive;
        BeginScene = HWBeginScene;
        EndScene = HWEndScene;
        SetRenderState = HWSetRenderState;
    } else if (App.DXConfig.MMX) {
        DrawPrimitive = MMXDrawPrimitive;
        lpSurface = lpSurf;
        BeginScene = MMXBeginScene;
        EndScene = SWEndScene;
        SetRenderState = MMXSetRenderState;
        MMXInit();
    } else {
        HWConfig.nFilter = 0;
        lpSurface = lpSurf;
        DrawPrimitive = SWDrawPrimitive;
        BeginScene = SWBeginScene;
        EndScene = SWEndScene;
        SetRenderState = SWSetRenderState;
        pScanLinesTAffine = ScanLinesTAffine;
        pScanLinesTDecalAffine = ScanLinesTDecalAffine;
        pScanLinesT = ScanLinesT;
        pScanLinesTDecal = ScanLinesTDecal;
        pScanLinesBlack = ScanLinesBlack;
        pScanLinesNull = NullScanLines;
        for (i = 0; i < App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].nTexture; i++) {
            if (App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].Texture[i].rbpp == 5
                && App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].Texture[i].gbpp == 6
                && App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].Texture[i].bbpp == 5) {
                App.DXConfigPtr->D3DTF = i;
                break;
            }
        }
        pScanLinesTG = ScanLinesTG;
        pScanLinesTGAffine = ScanLinesTGAffine;
        pScanLinesTF = ScanLinesTF;
        pScanLinesTFAffine = ScanLinesTFAffine;
        pScanLinesTGDecalAffine = ScanLinesTGDecalAffine;
        pScanLinesTGDecal = ScanLinesTGDecal;
        pScanLinesTFDecal = ScanLinesTFDecal;
        pScanLinesTFDecalAffine = ScanLinesTFDecalAffine;
        pScanLinesG = ScanLinesG;
        pScanLinesGAlpha = ScanLinesGAlpha;
        pScanLinesBlackAlpha = ScanLinesBlackAlpha;
        pScanLinesTFAlphaDecal = ScanLinesTFAlphaDecal;
        pScanLinesTFAlphaDecalAffine = ScanLinesTFAlphaDecalAffine;
        pScanLinesTGAlphaDecal = ScanLinesTGAlphaDecal;
        pScanLinesTGAlphaDecalAffine = ScanLinesTGAlphaDecalAffine;
        pDrawLine = DrawLineG;
        if (HWConfig.TrueAlpha) {
            pScanLinesBlackAlpha = ScanLinesBlackStipple;
            pScanLinesGAlpha = ScanLinesGStipple;
            pScanLinesTFAlphaDecalAffine = ScanLinesTFStippleDecalAffine;
            pScanLinesTFAlphaDecal = ScanLinesTFStippleDecal;
            pScanLinesTGAlphaDecalAffine = ScanLinesTGStippleDecalAffine;
            pScanLinesTGAlphaDecal = ScanLinesTGStippleDecal;
        }
        DrawScanLines = ScanLinesTG;
        CalcGradientsPersp = CalculateGradientsTG;
        CalcEdgePersp = CalculateEdgeTG;
        DrawScanLinesAffine = ScanLinesTGAffine;
        CalcGradientsAffine = CalculateGradientsTGA;
        CalcEdgeAffine = CalculateEdgeTGA;
        DrawScanLinesBlack = pScanLinesBlack;
        CalcGradientsBlack = NullCalculateGradient;
        CalcEdgeBlack = CalculateEdgeBlack;
        DrawScanLinesFlat = ScanLinesTF;
        CalcGradientsFlat = CalculateGradientsTF;
        CalcEdgeFlat = CalculateEdgeT;
        DrawScanLinesFlatAffine = ScanLinesTFAffine;
        CalcGradientsFlatAffine = CalculateGradientsTFA;
        CalcEdgeFlatAffine = CalculateEdgeTA;
        BuildXYTables();
        BuildRGBTable();
        bForceAffineMap = 0;
    }
}

void MMXInit(void)
{
    long i;
    long n;
    float f;

    RecipTable[0] = 0xff800000;
    for (i = 1; i < 4096; i++) {
        n = (i + 0x7f000) << 11;
        f = 1.0f / *(float *)&n;
        RecipTable[i] = (*(long *)&f & 0x7fffff) - 0x1000000;
    }
    if (!SW_SpanBuffer) {
        hSpanBuffer = GlobalAlloc(GMEM_FIXED, (float)(((App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].w >> 1) + 32) * App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].h));
        SW_SpanBuffer = (unsigned char *)(((long)hSpanBuffer + 32) & ~31);
    }
}

void DrawLineG(D3DTLVERTEX *v1, D3DTLVERTEX *v2)
{
    double fconv;
    long x1, y1, x2, y2, dx, dy, xinc, yinc, err, step, i, xcount, ycount;
    long r1, g1, b1, r2, g2, b2;
    long dr1 = 0, dg1 = 0, db1 = 0;
    long dr2 = 0, dg2 = 0, db2 = 0;
    unsigned short *p;

    x1 = FTOL(v1->sx);
    y1 = FTOL(v1->sy);
    r1 = RGBA_GETRED(v1->color) << 16;
    g1 = RGBA_GETGREEN(v1->color) << 16;
    b1 = RGBA_GETBLUE(v1->color) << 16;
    x2 = FTOL(v2->sx);
    y2 = FTOL(v2->sy);
    dx = x2 - x1;
    dy = y2 - y1;
    r2 = RGBA_GETRED(v2->color) << 16;
    g2 = RGBA_GETGREEN(v2->color) << 16;
    b2 = RGBA_GETBLUE(v2->color) << 16;
    p = Output + (YTable[y1] >> 1) + x1;
    if (dx == 0 && dy == 0) {
        *p = (unsigned short)((((r1 >> 19) * 64) | ((g1 >> 18) & 0x3f)) * 32 | ((b1 >> 19) & 0x1f));
        return;
    }
    if (dx >= 0) {
        xinc = 1;
    } else {
        dx = -dx;
        xinc = -1;
    }
    if (dy >= 0) {
        yinc = pitch;
    } else {
        yinc = -pitch;
        dy = -dy;
    }
    xcount = dx + 1;
    ycount = dy + 1;
    err = 0;
    if (dx < dy) {
        if (ycount) {
            dr1 = (r2 - r1) / ycount;
            dg1 = (g2 - g1) / ycount;
            db1 = (b2 - b1) / ycount;
        }
        step = (xcount << 16) / ycount;
        for (i = 0; i < ycount; i++) {
            *p = (unsigned short)((((r1 >> 19) * 64) | ((g1 >> 18) & 0x3f)) * 32 | ((b1 >> 19) & 0x1f));
            p += yinc;
            err += step;
            r1 += dr1;
            g1 += dg1;
            b1 += db1;
            if (err >= 0x10000) {
                p += xinc;
                err -= 0x10000;
            }
        }
    } else {
        step = (ycount << 16) / xcount;
        if (xcount) {
            dr2 = (r2 - r1) / xcount;
            dg2 = (g2 - g1) / xcount;
            db2 = (b2 - b1) / xcount;
        }
        for (i = 0; i < xcount; i++) {
            *p = (unsigned short)((((r1 >> 19) * 64) | ((g1 >> 18) & 0x3f)) * 32 | ((b1 >> 19) & 0x1f));
            p += xinc;
            err += step;
            r1 += dr2;
            g1 += dg2;
            b1 += db2;
            if (err >= 0x10000) {
                p += yinc;
                err -= 0x10000;
            }
        }
    }
}
}

#ifdef TR3_EXE
extern "C" void NullCalculateGradient(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3)
{
}

extern "C" void NullCalculateEdge(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom)
{
}

extern "C" void NullScanLines(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight)
{
}

extern "C" HRESULT SWEndScene(void)
{
    return 0;
}
#endif

#ifdef TR3_EXE
extern "C" {

double MagicFloat = 12582912.0;
double MagicInt = 6755399441055744.0;
double MagicFix16 = 103079215104.0;
double MagicFix24 = 402653184.0;
long RoundBias = 0x3F00068E;
long Fix16Scale = 0x47800000;
long SubSpanLength = 0x41800000;
long MipOffsets[40] = {
    0, 0x40000, 0x50000, 0x54000, 0x55000, 0,
    0x3D800000, 0x3F800000, 0x3F000000, 0x3EAAAAAB, 0x3E800000, 0x3E4CCCCD,
    0x3E2AAAAB, 0x3E124925, 0x3E000000, 0x3DE38E39, 0x3DCCCCCD, 0x3DBA2E8C,
    0x3DAAAAAB, 0x3D9D89D9, 0x3D924925, 0x3D888889, 0x3D800000, 0,
    0x41800000, 0x3F800000, 0x40000000, 0x40400000, 0x40800000, 0x40A00000,
    0x40C00000, 0x40E00000, 0x41000000, 0x41100000, 0x41200000, 0x41300000,
    0x41400000, 0x41500000, 0x41600000, 0x41700000,
};
float PerspThreshold = 0.4F;
__int64 SpansAlphaInvert = 0x00FF00FF00FF00FFi64;
__int64 SpansColorKeyInvert = 0xFFFFFFFFFFFFFFFFi64;
__int64 SpansColorKeyPInvert = 0xFFFFFFFFFFFFFFFFi64;
__int64 SpansTexturedFilteredFrac = 0x000000000000FF00i64;
__int64 SpansAdditiveFilteredFrac = 0x000000000000FF00i64;
__int64 SpansTexturedFilteredPFrac = 0x000000000000FF00i64;
__int64 SpansAdditiveFilteredPFrac = 0x000000000000FF00i64;
__int64 SpansColorKeyFilteredInvert = 0x00FF00FF00FF00FFi64;
__int64 SpansColorKeyFilteredFrac = 0x000000000000FF00i64;
__int64 SpansColorKeyFilteredKeyMask = 0x00FF000000000000i64;
__int64 SpansColorKeyFilteredPInvert = 0x00FF00FF00FF00FFi64;
__int64 SpansColorKeyFilteredPFrac = 0x000000000000FF00i64;
__int64 SpansColorKeyFilteredPKeyMask = 0x00FF000000000000i64;

}
#endif
