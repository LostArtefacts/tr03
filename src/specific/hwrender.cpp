#include "standard.h"
#include "specific.h"
#include "winmain.h"
#include "texture.h"
#include "global.h"

extern "C" {

extern HRESULT (*EndScene)(void);
extern HRESULT (*BeginScene)(void);
extern bool bBlueEffect;
extern int nDrawnPoints;
extern void *VertexBuffer;
extern D3DTLVERTEX *CurrentTLVertex;
extern HRESULT (*SetRenderState)(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState);
extern bool GtZBufferCompareEnabled;
extern bool GtZBufferWriteEnabled;
extern bool GtColorAdditionEnabled;
extern bool GtPerspCorrectEnabled;
extern bool GtFilterEnabled;
extern bool GtAlphaBlendEnabled;
extern bool GtColorKeyEnabled;

void HWR_InitState(void);
void InitBuckets(void);
void HWR_FreeTexturePages(void);
HRESULT DXTextureNewPalette(unsigned char *pPal);
void DXFreeTPages(void);
void DXCreateMaxTPages(int create);
int DXTextureAddPal(int w, int h, unsigned char *src, DXTEXTURE *list, unsigned long flags);
int DXTextureAdd(int w, int h, unsigned char *src, DXTEXTURE *list, int bpp, unsigned long flags);
extern int nTPages;
void HWR_GetAllTextureHandles(void);
void DXTextureCleanup(int nTexture, DXTEXTURE *pTextures);
DXTEXTURE *DXRestoreSurfaceIfLost(int nTexture, DXTEXTURE *pTextures);

extern DXTEXTURE *TPages[32];
void HWR_SetCurrentTexture(DWORD handle);
void HWR_EnableAlphaBlend(bool enable);
void HWR_EnableColorKey(bool enable);
void HWR_EnableColorAddition(bool enable);

void HWR_ResetCurrentTexture(void)
{
    HWR_SetCurrentTexture(0);
}

void HWR_ResetColorKey(void)
{
    HWR_EnableAlphaBlend(false);
    HWR_EnableColorKey(false);
    HWR_EnableColorAddition(false);
}

void HWR_ResetZBuffer(void)
{
    GtZBufferWriteEnabled = false;
    GtZBufferCompareEnabled = false;
    if (App.lpZBuffer) {
        SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);
    } else {
        SetRenderState(D3DRENDERSTATE_ZENABLE, false);
        SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, false);
    }
}

void HWR_EnableColorAddition(bool enable)
{
    if (enable) {
        if (!GtColorAdditionEnabled) {
            GtColorAdditionEnabled = true;
            SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
            SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);
        }
    } else if (GtColorAdditionEnabled) {
        GtColorAdditionEnabled = false;
        SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
        SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
    }
}

void HWR_EnablePerspCorrect(bool enable)
{
    if (!App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].bHardware) {
        if (enable) {
            if (!GtPerspCorrectEnabled) {
                GtPerspCorrectEnabled = true;
                SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, true);
            }
        } else if (GtPerspCorrectEnabled) {
            GtPerspCorrectEnabled = false;
            SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, false);
        }
    } else {
        SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, true);
    }
}

void HWR_EnableFilter(bool enable)
{
    if (!App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].bHardware) {
        if (enable) {
            if (!GtFilterEnabled) {
                GtFilterEnabled = true;
                SetRenderState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_MIPNEAREST);
            }
        } else if (GtFilterEnabled) {
            GtFilterEnabled = false;
            SetRenderState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_NEAREST);
        }
    }
}

void HWR_EnableAlphaBlend(bool enable)
{
    if (enable) {
        if (!GtAlphaBlendEnabled) {
            SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, true);
            GtAlphaBlendEnabled = true;
        }
    } else if (GtAlphaBlendEnabled) {
        SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
        GtAlphaBlendEnabled = false;
    }
}

void HWR_EnableColorKey(bool enable)
{
    if (enable) {
        if (!GtColorKeyEnabled) {
            SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, true);
            GtColorKeyEnabled = true;
        }
    } else if (GtColorKeyEnabled) {
        SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, false);
        GtColorKeyEnabled = false;
    }
}

void HWR_EnableZBuffer(bool tWrite, bool tCompare)
{
    if (!App.lpZBuffer)
        return;
    if (tWrite != GtZBufferWriteEnabled) {
        SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, tWrite);
        GtZBufferWriteEnabled = tWrite;
    }
    if (tCompare != GtZBufferCompareEnabled) {
        if (App.lpZBuffer)
            SetRenderState(D3DRENDERSTATE_ZFUNC, tCompare ? D3DCMP_LESSEQUAL : D3DCMP_ALWAYS);
        else
            SetRenderState(D3DRENDERSTATE_ZENABLE, tCompare);
        GtZBufferCompareEnabled = tCompare;
    }
}

void HWR_BeginScene(void)
{
    bBlueEffect = false;
    HWR_GetAllTextureHandles();
    HWR_EnableAlphaBlend(false);
    HWR_EnableColorKey(false);
    HWR_EnableColorAddition(false);
    HWR_ResetCurrentTexture();
    BeginScene();
    nDrawnPoints = 0;
    if (App.lpZBuffer)
        InitBuckets();
}

void HWR_EndScene(void)
{
    EndScene();
}

extern HRESULT (*DrawPrimitive)(D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags);
extern D3DPRIMITIVETYPE dpPrimitiveType;

void HWR_DrawRoutinesNoAlpha(int nVtx, D3DTLVERTEX *vtx, int nDrawType, int TPage)
{
    switch (nDrawType) {
    case 9:
        HWR_SetCurrentTexture((DWORD)TPages[TPage]);
        HWR_EnableAlphaBlend(0);
        HWR_EnableColorKey(0);
        DrawPrimitive(dpPrimitiveType, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        break;
    case 10:
        HWR_SetCurrentTexture((DWORD)TPages[TPage]);
        HWR_EnableAlphaBlend(1);
        HWR_EnableColorKey(1);
        DrawPrimitive(dpPrimitiveType, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        break;
    case 11:
        HWR_SetCurrentTexture(0);
        HWR_EnableAlphaBlend(0);
        HWR_EnableColorKey(0);
        DrawPrimitive(dpPrimitiveType, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        break;
    case 13:
        HWR_SetCurrentTexture(0);
        HWR_EnableAlphaBlend(1);
        DrawPrimitive(dpPrimitiveType, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        break;
    case 14:
        HWR_SetCurrentTexture((DWORD)TPages[TPage]);
        HWR_EnableAlphaBlend(1);
        HWR_EnableColorKey(1);
        DrawPrimitive(dpPrimitiveType, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        break;
    case 12:
        HWR_SetCurrentTexture(0);
        HWR_EnableColorKey(0);
        HWR_EnableAlphaBlend(1);
        DrawPrimitive(D3DPT_LINELIST, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        break;
    }
}

void HWR_LoadTexturePages(int nPages, unsigned char *src, unsigned char *palette)
{
    int i;

    HWR_FreeTexturePages();
    if (palette) {
        DXTextureNewPalette(palette);
        DXFreeTPages();
        DXCreateMaxTPages(1);
    }
    for (i = 0; i < nPages; i++) {
        if (palette) {
            DXTextureAddPal(256, 256, src, PictureTextures, 8);
            src += 0x10000;
        } else {
            DXTextureAdd(256, 256, src, PictureTextures, 555, 8);
            src += 0x20000;
        }
    }
    HWR_GetAllTextureHandles();
    for (i = 0; i < nTPages; i++)
        HWR_SetCurrentTexture((DWORD)TPages[i]);
}

void HWR_FreeTexturePages(void)
{
    int i;

    for (i = 0; i < 32; i++) {
        if (PictureTextures[i].dwFlags & 8)
            DXTextureCleanup(i, PictureTextures);
    }
    if (DXPalette) {
        DXPalette->Release();
        DXPalette = 0;
    }
}

void HWR_GetAllTextureHandles(void)
{
    DXTEXTURE *pTex;
    int n;

    n = 0;
    memset(TPages, 0, sizeof(TPages));
    for (int i = 0; i < ArraySize(PictureTextures); i++) {
        pTex = DXRestoreSurfaceIfLost(i, PictureTextures);
        if (pTex->dwFlags & 8)
            TPages[n++] = pTex;
    }
    for (int j = 0; j < ArraySize(PictureTextures); j++) {
        pTex = DXRestoreSurfaceIfLost(j, PictureTextures);
        if (pTex->dwFlags & 16)
            TPages[n++] = pTex;
    }
}

void HWR_InitVertexList(void)
{
    CurrentTLVertex = (D3DTLVERTEX *)VertexBuffer;
}

bool HWR_Init(void)
{
    VertexBuffer = 0;
    HWR_InitState();
    return true;
}

}
