// cflags: /Gi
#include "standard.h"
#include "drawprimitive.h"

extern "C" {

extern long bColorKey;
extern long bAlphaBlend;
extern long bSrcBlendOne;
extern unsigned short *TextureHandle;
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
void CalculateEdgeBlack(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);
void CalculateEdgeG(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);
void CalculateEdgeT(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);
void CalculateEdgeTA(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);
void CalculateEdgeTG(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);
void CalculateEdgeTGA(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);
void CalculateGradientsG(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
void CalculateGradientsTF(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
void CalculateGradientsTFA(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
void CalculateGradientsTG(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
void CalculateGradientsTGA(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);
void DrawLineList(D3DTLVERTEX *v, DWORD vcnt);
void DrawLineStrip(D3DTLVERTEX *v, DWORD vcnt);
void NullCalculateEdge(EDGE *Edge, GRADIENT *Gradients, D3DTLVERTEX *Top, D3DTLVERTEX *Bottom, int nTop, int nBottom);
void NullCalculateGradient(GRADIENT *Gradients, D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);

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
}
