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
extern void (*Var6ca198)(D3DTLVERTEX *v1, D3DTLVERTEX *v2);
void DrawTriangle(D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
extern HGLOBAL Var6ca3ac;
extern unsigned char *Var6ca4d0;
extern unsigned long *Var6ca2d0;
extern long Var6ca20c;
extern unsigned char *Var6ca12c;
extern long *YTable;
extern long Var6ce4e4;
extern float Var4cec20;
extern void (*Var6ca408)(D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
extern void (*Var6ca24c)(D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
void MMXDrawLine(D3DTLVERTEX *v1, D3DTLVERTEX *v2);
extern LPDIRECTDRAWSURFACE3 lpSurface;
extern float *WidthTable;
extern long *WidthTablef;
extern unsigned char *Var6ca1a8;
extern unsigned char *Var6ca1f0;

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

void Func491400(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3)
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

void Func4934b0(void);

void CloseDrawPrimitive(void)
{
    lpSurface->Unlock(NULL);
    if (App.DXConfig.MMX) {
        Func4934b0();
        return;
    }
    free(YTable);
    free(WidthTable);
    free(WidthTablef);
    free(Var6ca12c);
    free(Var6ca1a8);
    free(Var6ca1f0);
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

    Var6ca12c = (unsigned char *)malloc(0x800);
    Var6ca1a8 = (unsigned char *)malloc(0x800);
    Var6ca1f0 = (unsigned char *)malloc(0x2000);
    for (i = 0; i < 64; i++) {
        for (j = 0; j < 32; j++) {
            Var6ca12c[i + j * 64] = (unsigned char)((i * j) >> 6);
            Var6ca1a8[i + j * 64] = (unsigned char)((i * j) >> 5);
        }
    }
    for (i = 0; i < 32; i++) {
        for (j = 0; j < 256; j++)
            Var6ca1f0[i * 256 + j] = (unsigned char)(i * (j / 256.0f));
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

extern long Var6ca1f8;
extern long Var6ca1f4;
extern long Var6ca1fc;
extern int bForceAffineMap;
extern long tpitch;
extern unsigned short *TextureHandle;

HRESULT SWSetRenderState(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState)
{
    switch (dwRenderStateType) {
    case D3DRENDERSTATE_COLORKEYENABLE:
        Var6ca1f8 = dwRenderState;
        return D3D_OK;
    case D3DRENDERSTATE_ALPHABLENDENABLE:
        Var6ca1f4 = dwRenderState;
        return D3D_OK;
    case D3DRENDERSTATE_SRCBLEND:
        Var6ca1fc = dwRenderState == D3DBLEND_ONE;
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

void Func490f50(D3DTLVERTEX *v, DWORD vcnt)
{
    int n;
    D3DTLVERTEX *v1, *v2;

    for (n = vcnt; n > 0; n -= 2) {
        v1 = v;
        v++;
        v2 = v;
        v++;
        Var6ca198(v1, v2);
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

void Func491020(D3DTLVERTEX *v, DWORD vcnt)
{
    int n;
    D3DTLVERTEX *v1, *v2;

    v1 = v;
    v++;
    v2 = v;
    v++;
    Var6ca198(v1, v2);
    vcnt -= 2;
    for (n = vcnt; n > 0; n--) {
        v1 = v2;
        v2 = v;
        v++;
        Var6ca198(v1, v2);
    }
}


void Func4934b0(void)
{
    GlobalFree(Var6ca3ac);
    Var6ca4d0 = 0;
}

HRESULT MMXBeginScene(void)
{
    Var6ca2d0 = App.unk;
    Var6ca20c = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].w << 2;
    return 0;
}


HRESULT MMXDrawPrimitive(D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags)
{
    D3DTLVERTEX *v, *v2, *v3;
    int n;

    if (!Var6ca2d0)
        return 0;
    switch (dptPrimitiveType) {
    case D3DPT_TRIANGLELIST:
        v = (D3DTLVERTEX *)lpvVertices;
        n = dwVertexCount;
        do {
            if (Var6ce4e4 && v->rhw > Var4cec20)
                Var6ca408(v, v + 1, v + 2);
            else
                Var6ca24c(v, v + 1, v + 2);
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
            if (Var6ce4e4 && v->rhw > Var4cec20)
                Var6ca408(v, v2, v3);
            else
                Var6ca24c(v, v2, v3);
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

void Func491680(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom)
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

extern long Var6ca248;
extern long Var6ca460;
extern long Var6ca2c8;
extern long Var6ca4c8;
extern long Var6ca2b0;
extern long Var6ce4d8;
extern long Var6ce4e0;
extern long Var4ceb80[];
extern __int64 Var6ca338;

__declspec(naked) void Func495490(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [Var6ca4d0]
        cmp esi, dword ptr [Var6ca248]
        je Done
        push ebp
        mov edi, dword ptr [Var6ca2d0]
        mov eax, dword ptr [Var6ca460]
        imul eax, dword ptr [Var6ca20c]
        add edi, eax
        mov ecx, dword ptr [Var6ce4e0]
        mov eax, 0xff0000
        shl eax, cl
        and eax, 0xff0000
        mov dword ptr [Var6ca2c8], eax
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
        mov ebx, dword ptr [Var6ca4c8]
        mov eax, dword ptr [Var6ca2b0]
        mov edx, dword ptr [Var6ce4d8]
        add edx, dword ptr [Var4ceb80 + ecx * 4]
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
        and edx, dword ptr [Var6ca2c8]
        and ebp, dword ptr [Var6ca2c8]
    P5:
        shr edx, 8
        pxor mm5, mm5
    P6:
        shr ebp, 0x10
        movq mm1, qword ptr [Var6ca338]
        or edx, ebp
        movq mm0, qword ptr [esi + 0x18]
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        add ecx, dword ptr [Var6ca2b0]
        add ebx, dword ptr [Var6ca4c8]
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
        add edi, dword ptr [Var6ca20c]
        cmp dword ptr [Var6ca248], esi
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

__declspec(naked) void DrawSpansGouraud(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [Var6ca4d0]
        cmp esi, dword ptr [Var6ca248]
        je L6
        push ebp
        mov edi, dword ptr [Var6ca2d0]
        mov eax, dword ptr [Var6ca460]
        imul eax, dword ptr [Var6ca20c]
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
        movq mm1, qword ptr [Var6ca338]
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
        movq mm2, qword ptr [Var6ca338]
        _emit 0x0f
        _emit 0x7f
        _emit 0xc1
        psrlw mm1, 0x8
        packuswb mm1, mm1
        movd dword ptr [edi], mm1
    L5:
        pop edi
        add edi, dword ptr [Var6ca20c]
        add esi, 0x20
        cmp dword ptr [Var6ca248], esi
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

extern long Var6ca300;

__declspec(naked) void Func4955f0(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [Var6ca4d0]
        cmp esi, dword ptr [Var6ca248]
        je L13
        push ebp
        mov edi, dword ptr [Var6ca2d0]
        mov eax, dword ptr [Var6ca460]
        imul eax, dword ptr [Var6ca20c]
        add edi, eax
        mov ecx, dword ptr [Var6ce4e0]
        mov eax, 0xff0000
        shl eax, cl
        and eax, 0xff0000
        mov dword ptr [Var6ca300], eax
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
        mov ebx, dword ptr [Var6ca4c8]
        mov eax, dword ptr [Var6ca2b0]
        mov edx, dword ptr [Var6ce4d8]
        add edx, dword ptr [ecx*4+Var4ceb80]
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
        movq mm1, qword ptr [Var6ca338]
        push ebx
        push ecx
        push esi
        mov ebx, dword ptr [esi+0x8]
        mov ecx, dword ptr [esi+0x10]
        mov edx, ecx
        mov ebp, ebx
        and edx, dword ptr [Var6ca300]
        and ebp, dword ptr [Var6ca300]
        pxor mm5, mm5
    L2:
        shr edx, 0x8
    L3:
        shr ebp, 0x10
        add ecx, dword ptr [Var6ca2b0]
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
        add edi, dword ptr [Var6ca20c]
        add esi, 0x20
        cmp dword ptr [Var6ca248], esi
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

extern long Var6ca394;
extern __int64 Var4cec30;

__declspec(naked) void Func495760(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [Var6ca4d0]
        cmp esi, dword ptr [Var6ca248]
        je L12
        push ebp
        mov edi, dword ptr [Var6ca2d0]
        mov eax, dword ptr [Var6ca460]
        imul eax, dword ptr [Var6ca20c]
        add edi, eax
        mov ecx, dword ptr [Var6ce4e0]
        mov eax, 0xff0000
        shl eax, cl
        and eax, 0xff0000
        mov dword ptr [Var6ca394], eax
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
        mov edx, dword ptr [Var6ce4d8]
        add edx, dword ptr [ecx*4+Var4ceb80]
        mov dword ptr [L4 + 4], edx
        mov eax, dword ptr [Var6ca4c8]
        mov ebx, dword ptr [Var6ca2b0]
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
        and edx, dword ptr [Var6ca394]
        and ebp, dword ptr [Var6ca394]
    L2:
        shr edx, 0x8
        pxor mm5, mm5
    L3:
        shr ebp, 0x10
        movq mm1, qword ptr [Var6ca338]
        or edx, ebp
        movq mm0, qword ptr [esi-0x8]
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        add ecx, dword ptr [Var6ca2b0]
        add ebx, dword ptr [Var6ca4c8]
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
        pxor mm4, qword ptr [Var4cec30]
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
        add edi, dword ptr [Var6ca20c]
        cmp dword ptr [Var6ca248], esi
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

extern long Var6ca228;

__declspec(naked) void Func4958d0(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [Var6ca4d0]
        cmp esi, dword ptr [Var6ca248]
        je L11
        push ebp
        mov edi, dword ptr [Var6ca2d0]
        mov eax, dword ptr [Var6ca460]
        imul eax, dword ptr [Var6ca20c]
        add edi, eax
        mov ecx, dword ptr [Var6ce4e0]
        mov eax, 0xff0000
        shl eax, cl
        and eax, 0xff0000
        mov dword ptr [Var6ca228], eax
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
        mov edx, dword ptr [Var6ce4d8]
        add edx, dword ptr [ecx*4+Var4ceb80]
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
        and edx, dword ptr [Var6ca228]
        and ebp, dword ptr [Var6ca228]
    L2:
        shr edx, 0x8
        pxor mm5, mm5
    L3:
        shr ebp, 0x10
        movq mm1, qword ptr [Var6ca338]
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
        add edi, dword ptr [Var6ca20c]
        cmp dword ptr [Var6ca248], esi
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

extern long Var6ca28c;
extern __int64 Var4cec38;

__declspec(naked) void Func495a20(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [Var6ca4d0]
        cmp esi, dword ptr [Var6ca248]
        je L11
        push ebp
        mov edi, dword ptr [Var6ca2d0]
        mov eax, dword ptr [Var6ca460]
        imul eax, dword ptr [Var6ca20c]
        add edi, eax
        mov ecx, dword ptr [Var6ce4e0]
        mov eax, 0xff0000
        shl eax, cl
        and eax, 0xff0000
        mov dword ptr [Var6ca28c], eax
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
        mov edx, dword ptr [Var6ce4d8]
        add edx, dword ptr [ecx*4+Var4ceb80]
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
        and edx, dword ptr [Var6ca28c]
        and ebp, dword ptr [Var6ca28c]
    L2:
        shr edx, 0x8
        pxor mm5, mm5
    L3:
        shr ebp, 0x10
        movq mm1, qword ptr [Var6ca338]
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
        pxor mm4, qword ptr [Var4cec38]
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
        add edi, dword ptr [Var6ca20c]
        cmp dword ptr [Var6ca248], esi
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

extern long Var6ca2f8;

__declspec(naked) void Func495b90(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [Var6ca4d0]
        cmp esi, dword ptr [Var6ca248]
        je L11
        push ebp
        mov edi, dword ptr [Var6ca2d0]
        mov eax, dword ptr [Var6ca460]
        imul eax, dword ptr [Var6ca20c]
        add edi, eax
        mov ecx, dword ptr [Var6ce4e0]
        mov eax, 0xff0000
        shl eax, cl
        and eax, 0xff0000
        mov dword ptr [Var6ca2f8], eax
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
        mov edx, dword ptr [Var6ce4d8]
        add edx, dword ptr [ecx*4+Var4ceb80]
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
        and edx, dword ptr [Var6ca2f8]
        and ebp, dword ptr [Var6ca2f8]
    L2:
        shr edx, 0x8
        pxor mm5, mm5
    L3:
        shr ebp, 0x10
        movq mm1, qword ptr [Var6ca338]
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
        add edi, dword ptr [Var6ca20c]
        cmp dword ptr [Var6ca248], esi
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

extern long Var6ca43c;
extern long Var6ca2bc;
extern __int64 Var4cec40;

__declspec(naked) void Func495cf0(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [Var6ca4d0]
        cmp esi, dword ptr [Var6ca248]
        je L9
        push ebp
        mov edi, dword ptr [Var6ca2d0]
        mov eax, dword ptr [Var6ca460]
        imul eax, dword ptr [Var6ca20c]
        add edi, eax
        mov ebx, dword ptr [Var6ca4c8]
        mov eax, dword ptr [Var6ca2b0]
        mov edx, dword ptr [Var6ce4d8]
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
        movq mm1, qword ptr [Var6ca338]
        movq mm0, qword ptr [esi+0x18]
        mov edx, ecx
        mov ebp, ebx
        mov dword ptr [Var6ca43c], edx
        mov dword ptr [Var6ca2bc], ebp
        movd mm2, dword ptr [Var6ca2bc]
        and edx, 0xff0000
        pand mm2, qword ptr [Var4cec40]
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
        movd mm2, dword ptr [Var6ca43c]
        pand mm2, qword ptr [Var4cec40]
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
        mov dword ptr [Var6ca43c], edx
        mov dword ptr [Var6ca2bc], ebp
        movd mm2, dword ptr [Var6ca2bc]
        psraw mm6, 0x7
        and edx, 0xff0000
        paddw mm7, mm6
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        psrlw mm6, 0x8
        pand mm2, qword ptr [Var4cec40]
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
        add edi, dword ptr [Var6ca20c]
        cmp dword ptr [Var6ca248], esi
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

extern long Var6ca2fc;
extern long Var6ca4ac;
extern __int64 Var4cec48;

__declspec(naked) void Func495ed0(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [Var6ca4d0]
        cmp esi, dword ptr [Var6ca248]
        je L9
        push ebp
        mov edi, dword ptr [Var6ca2d0]
        mov eax, dword ptr [Var6ca460]
        imul eax, dword ptr [Var6ca20c]
        add edi, eax
        mov ebx, dword ptr [Var6ca4c8]
        mov eax, dword ptr [Var6ca2b0]
        mov edx, dword ptr [Var6ce4d8]
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
        movq mm1, qword ptr [Var6ca338]
        movq mm0, qword ptr [esi+0x18]
        mov edx, ecx
        mov ebp, ebx
        mov dword ptr [Var6ca2fc], edx
        mov dword ptr [Var6ca4ac], ebp
        movd mm2, dword ptr [Var6ca4ac]
        and edx, 0xff0000
        pand mm2, qword ptr [Var4cec48]
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
        movd mm2, dword ptr [Var6ca2fc]
        pand mm2, qword ptr [Var4cec48]
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
        mov dword ptr [Var6ca2fc], edx
        mov dword ptr [Var6ca4ac], ebp
        movd mm2, dword ptr [Var6ca4ac]
        psraw mm6, 0x7
        and edx, 0xff0000
        paddw mm7, mm6
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        psrlw mm6, 0x8
        pand mm2, qword ptr [Var4cec48]
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
        add edi, dword ptr [Var6ca20c]
        cmp dword ptr [Var6ca248], esi
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

extern long Var6ca398;
extern long Var6ca3f0;
extern __int64 Var4cec50;

__declspec(naked) void Func4960c0(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [Var6ca4d0]
        cmp esi, dword ptr [Var6ca248]
        je L8
        push ebp
        mov edi, dword ptr [Var6ca2d0]
        mov eax, dword ptr [Var6ca460]
        imul eax, dword ptr [Var6ca20c]
        add edi, eax
        mov edx, dword ptr [Var6ce4d8]
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
        movq mm1, qword ptr [Var6ca338]
        movq mm0, qword ptr [esi-0x8]
        mov edx, ecx
        mov ebp, ebx
        mov dword ptr [Var6ca398], edx
        mov dword ptr [Var6ca3f0], ebp
        movd mm2, dword ptr [Var6ca3f0]
        and edx, 0xff0000
        pand mm2, qword ptr [Var4cec50]
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
        movd mm2, dword ptr [Var6ca398]
        pand mm2, qword ptr [Var4cec50]
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
        mov dword ptr [Var6ca398], edx
        mov dword ptr [Var6ca3f0], ebp
        movd mm2, dword ptr [Var6ca3f0]
        psraw mm6, 0x7
        and edx, 0xff0000
        paddw mm7, mm6
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        psrlw mm6, 0x8
        pand mm2, qword ptr [Var4cec50]
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
        add edi, dword ptr [Var6ca20c]
        cmp dword ptr [Var6ca248], esi
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

extern long Var6ca214;
extern long Var6ca428;
extern __int64 Var4cec58;

__declspec(naked) void Func4962c0(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [Var6ca4d0]
        cmp esi, dword ptr [Var6ca248]
        je L8
        push ebp
        mov edi, dword ptr [Var6ca2d0]
        mov eax, dword ptr [Var6ca460]
        imul eax, dword ptr [Var6ca20c]
        add edi, eax
        mov edx, dword ptr [Var6ce4d8]
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
        movq mm1, qword ptr [Var6ca338]
        movq mm0, qword ptr [esi-0x8]
        mov edx, ecx
        mov ebp, ebx
        mov dword ptr [Var6ca214], edx
        mov dword ptr [Var6ca428], ebp
        movd mm2, dword ptr [Var6ca428]
        and edx, 0xff0000
        pand mm2, qword ptr [Var4cec58]
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
        movd mm2, dword ptr [Var6ca214]
        pand mm2, qword ptr [Var4cec58]
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
        mov dword ptr [Var6ca214], edx
        mov dword ptr [Var6ca428], ebp
        movd mm2, dword ptr [Var6ca428]
        psraw mm6, 0x7
        and edx, 0xff0000
        paddw mm7, mm6
        _emit 0x0f
        _emit 0x7f
        _emit 0xc6
        psrlw mm6, 0x8
        pand mm2, qword ptr [Var4cec58]
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
        add edi, dword ptr [Var6ca20c]
        cmp dword ptr [Var6ca248], esi
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

extern long Var6ca3f4;
extern long Var6ca268;
extern __int64 Var4cec68;
extern __int64 Var4cec70;
extern __int64 Var4cec60;

__declspec(naked) void Func4964c0(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [Var6ca4d0]
        cmp esi, dword ptr [Var6ca248]
        je L10
        push ebp
        mov edi, dword ptr [Var6ca2d0]
        mov eax, dword ptr [Var6ca460]
        imul eax, dword ptr [Var6ca20c]
        add edi, eax
        mov ebx, dword ptr [Var6ca4c8]
        mov eax, dword ptr [Var6ca2b0]
        mov edx, dword ptr [Var6ce4d8]
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
        movq mm1, qword ptr [Var6ca338]
        movq mm0, qword ptr [esi+0x18]
        mov edx, ecx
        mov ebp, ebx
        mov dword ptr [Var6ca3f4], edx
        mov dword ptr [Var6ca268], ebp
        and edx, 0xff0000
        and ebp, 0xff0000
        shr edx, 0x8
        shr ebp, 0x10
        or edx, ebp
        movd mm2, dword ptr [Var6ca268]
        pand mm2, qword ptr [Var4cec68]
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
        movd mm2, dword ptr [Var6ca3f4]
        pand mm2, qword ptr [Var4cec68]
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
        mov dword ptr [Var6ca3f4], edx
        mov dword ptr [Var6ca268], ebp
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
        pand mm2, qword ptr [Var4cec70]
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
        pxor mm4, qword ptr [Var4cec60]
        movd mm2, dword ptr [Var6ca268]
        pmullw mm6, mm4
        pand mm2, qword ptr [Var4cec68]
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
        add edi, dword ptr [Var6ca20c]
        cmp dword ptr [Var6ca248], esi
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

extern long Var6ca318;
extern long Var6ca450;
extern __int64 Var4cec80;
extern __int64 Var4cec88;
extern __int64 Var4cec78;

__declspec(naked) void Func4966f0(void)
{
    __asm {
        push ebx
        push esi
        push edi
        mov esi, dword ptr [Var6ca4d0]
        cmp esi, dword ptr [Var6ca248]
        je L9
        push ebp
        mov edi, dword ptr [Var6ca2d0]
        mov eax, dword ptr [Var6ca460]
        imul eax, dword ptr [Var6ca20c]
        add edi, eax
        mov edx, dword ptr [Var6ce4d8]
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
        movq mm1, qword ptr [Var6ca338]
        movq mm0, qword ptr [esi-0x8]
        mov edx, ecx
        mov ebp, ebx
        mov dword ptr [Var6ca318], edx
        mov dword ptr [Var6ca450], ebp
        and edx, 0xff0000
        and ebp, 0xff0000
        shr edx, 0x8
        shr ebp, 0x10
        or edx, ebp
        movd mm2, dword ptr [Var6ca450]
        pand mm2, qword ptr [Var4cec80]
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
        movd mm2, dword ptr [Var6ca318]
        pand mm2, qword ptr [Var4cec80]
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
        mov dword ptr [Var6ca318], edx
        mov dword ptr [Var6ca450], ebp
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
        pand mm2, qword ptr [Var4cec88]
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
        pxor mm4, qword ptr [Var4cec78]
        movd mm2, dword ptr [Var6ca450]
        pmullw mm6, mm4
        pand mm2, qword ptr [Var4cec80]
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
        add edi, dword ptr [Var6ca20c]
        cmp dword ptr [Var6ca248], esi
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
}
