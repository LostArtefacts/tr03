#ifndef DRAWPRIMITIVE_H
#define DRAWPRIMITIVE_H

#define MAGICNUM float((1 << 26) * ((1 << 26) * 1.5))
#define FTOL(x) ((fconv = (x) + (MAGICNUM)), *(int *)(&fconv))
#define FTOLFIX(x) ((fconv = x + MAGICNUM / 65536.0), *(int *)(&fconv))

struct GRADIENT {
    float ooz[3];
    float uoz[3];
    float voz[3];
    float r[3];
    float g[3];
    float b[3];
    float dozx, dozy;
    float duzx, duzy;
    float dvzx, dvzy;
    float drx, dry;
    float dgx, dgy;
    float dbx, dby;
    short rf, gf, bf;
};

struct EDGE {
    long x, xstep;
    long y;
    long height;
    long uozf, uozstepf;
    long vozf, vozstepf;
    long uozxtraf;
    long vozxtraf;
    long rf, rstepf, rxtraf;
    long gf, gstepf, gxtraf;
    long bf, bstepf, bxtraf;
    float ooz, oozstep, oozxtra;
    float uoz, uozstep, uozxtra;
    float voz, vozstep, vozxtra;
    float r, rstep, rxtra;
    float g, gstep, gxtra;
    float b, bstep, bxtra;
    long Numerator;
    long Denominator;
    long ErrorTerm;
};

extern "C" {

extern HRESULT (*DrawPrimitive)(D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags);
extern HRESULT (*BeginScene)(void);
extern HRESULT (*EndScene)(void);
extern HRESULT (*SetRenderState)(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState);
extern void (*DrawScanLines)(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);

HRESULT HWDrawPrimitive(D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags);
HRESULT SWDrawPrimitive(D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags);
HRESULT EmulateDrawPrimitive(D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags);
void InitDrawPrimitive(LPDIRECT3DDEVICE2 lpD3D, LPDIRECTDRAWSURFACE3 lpSurface, bool HW);
void DrawTriangleList(LPD3DTLVERTEX v, DWORD vcnt);
void DrawTriangle(D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
void DrawTriangleFan(LPD3DTLVERTEX v, DWORD vcnt);
void DrawTriangleStrip(LPD3DTLVERTEX v, DWORD vcnt);
void PrepareTriangleList(LPD3DTLVERTEX v, DWORD vcnt);
void BuildXYTables(void);
void BuildRGBTable(void);
HRESULT HWBeginScene(void);
HRESULT SWBeginScene(void);
HRESULT HWEndScene(void);
HRESULT SWEndScene(void);
HRESULT HWSetRenderState(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState);
HRESULT SWSetRenderState(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState);
void InitSoftwarePalette(LPDIRECTDRAWPALETTE DDPal);
void ScanLinesTG(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTGDecal(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTGAStipple(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTGA(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesG(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesBlack(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void CalculateGradientsTF(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
void CalculateGradientsTFA(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
void CalculateGradientsT(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
void CalculateEdgeT(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);
void CalculateGradientsTA(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
void CalculateEdgeTA(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);
void CalculateGradientsTGA(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
void CalculateEdgeTGA(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);
void CalculateGradientsTG(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
void CalculateEdgeTG(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);
void CalculateGradientsBlack(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
void CalculateEdgeBlack(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);

}

#endif
