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

HRESULT HWSetRenderState(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState)
{
    return D3DDev->SetRenderState(dwRenderStateType, dwRenderState);
}

HRESULT HWDrawPrimitive(D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags)
{
    return D3DDev->DrawPrimitive(dptPrimitiveType, dvtVertexType, lpvVertices, dwVertexCount, dwFlags);
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

}
