#include "standard.h"
#include "specific.h"
#include "winmain.h"

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
