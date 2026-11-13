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
extern DWORD GhCurTextureHandle;

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
extern TEXTURE Textures[];
LPDIRECT3DTEXTURE2 DXTextureGetInterface(LPDIRECTDRAWSURFACE3 lpSurface);
void HWR_SetCurrentTexture(DXTEXTURE *tex);
void HWR_EnableAlphaBlend(bool enable);
void HWR_EnableColorKey(bool enable);
void HWR_EnableColorAddition(bool enable);

extern float GammaOption;
extern unsigned char ColorTable[256];
extern unsigned long GlobalAlpha;
extern bool bAlphaTesting;
extern void (*DrawRoutine)(int nVtx, D3DTLVERTEX *vtx, int nDrawType, int TPage);
void HWR_ResetZBuffer(void);
void HWR_ResetCurrentTexture(void);
void HWR_ResetColorKey(void);
void HWR_EnablePerspCorrect(bool enable);
void HWR_DrawRoutines(int nVtx, D3DTLVERTEX *vtx, int nDrawType, int TPage);
void HWR_DrawRoutinesStippledAlpha(int nVtx, D3DTLVERTEX *vtx, int nDrawType, int TPage);
void HWR_DrawRoutinesNoAlpha(int nVtx, D3DTLVERTEX *vtx, int nDrawType, int TPage);

void HWR_InitState(void)
{
    float g;
    float gamma;
    int i;
    bool blendOne;

    g = GammaOption * 0.1f;
    g *= 4.0f;
    gamma = 1.0f / g;
    for (i = 0; i < 256; i++)
        ColorTable[i] = (unsigned char)((float)pow(i / 256.0f, gamma) * 256.0f);

    if (!SetRenderState)
        return;

    HWR_ResetZBuffer();
    SetRenderState(D3DRENDERSTATE_FILLMODE, HWConfig.nFillMode);
    SetRenderState(D3DRENDERSTATE_SHADEMODE, HWConfig.nShadeMode);
    SetRenderState(D3DRENDERSTATE_TEXTUREMAG, HWConfig.nFilter);
    SetRenderState(D3DRENDERSTATE_TEXTUREMIN, HWConfig.nFilter);
    SetRenderState(D3DRENDERSTATE_DITHERENABLE, HWConfig.Dither);
    HWR_EnablePerspCorrect(HWConfig.Perspective);
    SetRenderState(D3DRENDERSTATE_TEXTUREADDRESS, D3DTADDRESS_CLAMP);
    SetRenderState(D3DRENDERSTATE_TEXTUREMAPBLEND, D3DTBLEND_MODULATEALPHA);
    SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
    SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
    SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);
    SetRenderState(D3DRENDERSTATE_SPECULARENABLE, FALSE);
    SetRenderState(D3DRENDERSTATE_STIPPLEDALPHA, FALSE);
    SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
    SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, FALSE);
    HWR_ResetCurrentTexture();
    HWR_ResetColorKey();

    if (!App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].bHardware) {
        GlobalAlpha = 0xff000000;
        DrawRoutine = HWR_DrawRoutines;
        return;
    }

    if (App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DeviceDesc.dpcTriCaps.dwAlphaCmpCaps & D3DPCMPCAPS_NOTEQUAL) {
        SetRenderState(D3DRENDERSTATE_ALPHAREF, 0);
        SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_NOTEQUAL);
        bAlphaTesting = true;
    } else {
        bAlphaTesting = false;
    }

    blendOne = true;
    if (!(App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DeviceDesc.dpcTriCaps.dwDestBlendCaps & D3DPBLENDCAPS_ONE))
        blendOne = false;
    GlobalAlpha = 0xff000000;
    if (App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DeviceDesc.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_ALPHAFLATSTIPPLED) {
        if (App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DeviceDesc.dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_ALPHAFLATBLEND) {
            if (blendOne) {
                DrawRoutine = HWR_DrawRoutines;
            } else {
                DrawRoutine = HWR_DrawRoutinesNoAlpha;
                GlobalAlpha = 0x80000000;
            }
        } else {
            DrawRoutine = HWR_DrawRoutinesStippledAlpha;
            GlobalAlpha = 0x80000000;
        }
    } else if (blendOne) {
        DrawRoutine = HWR_DrawRoutines;
    } else {
        DrawRoutine = HWR_DrawRoutinesNoAlpha;
        GlobalAlpha = 0x80000000;
    }
}

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

void HWR_SetCurrentTexture(DXTEXTURE *tex)
{
    TEXTURE *tdata;
    TEXTURE *temp;
    D3DTEXTUREHANDLE handle;
    unsigned long n;
    unsigned long d;
    int i;

    handle = 0;
    if (App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].bHardware) {
        if (!nTPages)
            return;
        if (tex) {
            for (i = 0; i < nTPages; i++) {
                if (Textures[i].DXTex == tex) {
                    handle = Textures[i].handle;
                    Textures[i].nFrames = App.nFrames;
                    break;
                }
            }
            if (!handle) {
                n = 0;
                tdata = (TEXTURE *)tex;
                for (i = 0; i < nTPages; i++) {
                    temp = &Textures[i];
                    if (!temp->DXTex && tex->bpp == temp->bpp) {
                        tdata = &Textures[i];
                        break;
                    }
                    d = App.nFrames - temp->nFrames;
                    if (d >= n && tex->bpp == temp->bpp) {
                        n = d;
                        tdata = temp;
                    }
                }
                handle = tdata->handle;
                if (!n)
                    SetRenderState(D3DRENDERSTATE_FLUSHBATCH, 0);
                if (tdata->pSurf->IsLost() == DDERR_SURFACELOST)
                    tdata->pSurf->Restore();
                if (tex->pSystemSurface) {
                    if (tdata->pTexture->Load(DXTextureGetInterface(tex->pSystemSurface)) != D3D_OK)
                        tdata->pTexture->Load(DXTextureGetInterface(tex->pSystemSurface));
                }
                tdata->DXTex = tex;
                tdata->nFrames = App.nFrames;
                tex->tex = tdata;
            }
        }
    } else {
        if (!tex)
            SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, 0);
        else
            SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, (unsigned long)tex->pData);
    }
    if (handle != GhCurTextureHandle) {
        SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, handle);
        GhCurTextureHandle = handle;
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

void HWR_DrawRoutines(int nVtx, D3DTLVERTEX *vtx, int nDrawType, int TPage)
{
    switch (nDrawType) {
    case 9:
        HWR_EnableAlphaBlend(0);
        HWR_EnableColorKey(0);
        HWR_SetCurrentTexture(TPages[TPage]);
        DrawPrimitive(dpPrimitiveType, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        return;
    case 10:
        HWR_SetCurrentTexture(TPages[TPage]);
        HWR_EnableZBuffer(1, 1);
        HWR_EnableColorKey(1);
        HWR_EnableAlphaBlend(1);
        HWR_EnableColorAddition(0);
        DrawPrimitive(dpPrimitiveType, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        HWR_EnableZBuffer(0, 1);
        return;
    case 11:
        HWR_SetCurrentTexture(0);
        HWR_EnableAlphaBlend(0);
        HWR_EnableColorKey(0);
        DrawPrimitive(dpPrimitiveType, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        return;
    case 13:
        HWR_SetCurrentTexture(0);
        HWR_EnableAlphaBlend(1);
        HWR_EnableColorAddition(0);
        DrawPrimitive(dpPrimitiveType, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        return;
    case 16:
        HWR_SetCurrentTexture(0);
        HWR_EnableAlphaBlend(1);
        HWR_EnableColorAddition(1);
        DrawPrimitive(dpPrimitiveType, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        return;
    case 14:
        HWR_EnableZBuffer(0, 1);
        HWR_SetCurrentTexture(TPages[TPage]);
        HWR_EnableColorAddition(1);
        HWR_EnableAlphaBlend(1);
        HWR_EnableColorKey(1);
        DrawPrimitive(dpPrimitiveType, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        return;
    case 12:
        HWR_SetCurrentTexture(0);
        HWR_EnableAlphaBlend(1);
        HWR_EnableColorKey(0);
        HWR_EnableColorAddition(0);
        DrawPrimitive(D3DPT_LINELIST, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        return;
    }
}

void HWR_DrawRoutinesStippledAlpha(int nVtx, D3DTLVERTEX *vtx, int nDrawType, int TPage)
{
    switch (nDrawType) {
    case 9:
        HWR_SetCurrentTexture(TPages[TPage]);
        HWR_EnableAlphaBlend(0);
        HWR_EnableColorKey(0);
        DrawPrimitive(dpPrimitiveType, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        return;
    case 10:
        HWR_SetCurrentTexture(TPages[TPage]);
        HWR_EnableZBuffer(1, 1);
        HWR_EnableAlphaBlend(1);
        HWR_EnableColorKey(1);
        HWR_EnableColorAddition(0);
        DrawPrimitive(dpPrimitiveType, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        HWR_EnableZBuffer(0, 1);
        return;
    case 11:
        HWR_SetCurrentTexture(0);
        HWR_EnableAlphaBlend(0);
        HWR_EnableColorKey(0);
        DrawPrimitive(dpPrimitiveType, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        return;
    case 13:
        HWR_SetCurrentTexture(0);
        HWR_EnableAlphaBlend(1);
        HWR_EnableColorAddition(0);
        SetRenderState(D3DRENDERSTATE_STIPPLEDALPHA, 1);
        DrawPrimitive(dpPrimitiveType, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        SetRenderState(D3DRENDERSTATE_STIPPLEDALPHA, 0);
        return;
    case 16:
        HWR_SetCurrentTexture(0);
        HWR_EnableAlphaBlend(1);
        HWR_EnableColorAddition(1);
        SetRenderState(D3DRENDERSTATE_STIPPLEDALPHA, 1);
        DrawPrimitive(dpPrimitiveType, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        SetRenderState(D3DRENDERSTATE_STIPPLEDALPHA, 0);
        return;
    case 14:
        HWR_SetCurrentTexture(TPages[TPage]);
        HWR_EnableColorAddition(1);
        HWR_EnableAlphaBlend(1);
        HWR_EnableColorKey(1);
        SetRenderState(D3DRENDERSTATE_STIPPLEDALPHA, 1);
        DrawPrimitive(dpPrimitiveType, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        SetRenderState(D3DRENDERSTATE_STIPPLEDALPHA, 0);
        return;
    case 12:
        HWR_SetCurrentTexture(0);
        HWR_EnableAlphaBlend(1);
        HWR_EnableColorKey(0);
        DrawPrimitive(D3DPT_LINELIST, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        return;
    }
}

void HWR_DrawRoutinesNoAlpha(int nVtx, D3DTLVERTEX *vtx, int nDrawType, int TPage)
{
    switch (nDrawType) {
    case 9:
        HWR_SetCurrentTexture(TPages[TPage]);
        HWR_EnableAlphaBlend(0);
        HWR_EnableColorKey(0);
        DrawPrimitive(dpPrimitiveType, D3DVT_TLVERTEX, vtx, nVtx, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
        break;
    case 10:
        HWR_SetCurrentTexture(TPages[TPage]);
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
        HWR_SetCurrentTexture(TPages[TPage]);
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

extern void (*DrawRoutine)(int nVtx, D3DTLVERTEX *vtx, int nDrawType, int TPage);

void HWR_DrawPolyList(int num, sint32 *sortptr)
{
    sint16 *pInfo;
    sint32 *pSort;
    int nSurf;
    D3DTLVERTEX *pV;
    int Routine;
    int Tpage;
    int nV;

    dpPrimitiveType = D3DPT_TRIANGLEFAN;
    if (!App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].bHardware) {
        pSort = (sint32 *)sortptr;
        for (nSurf = 0; nSurf < num; nSurf++) {
            pInfo = (sint16 *)(*pSort);
            switch (pSort[2]) {
            case 1:
            case 2:
            case 5:
                HWR_EnablePerspCorrect(false);
                HWR_EnableFilter(false);
                break;
            case 3:
                HWR_EnablePerspCorrect(true);
                HWR_EnableFilter(true);
                break;
            default:
                HWR_EnablePerspCorrect(true);
                HWR_EnableFilter(false);
                break;
            }
            Routine = *pInfo++;
            Tpage = pInfo[0];
            nV = pInfo[1];
            pV = *((D3DTLVERTEX **)(pInfo + 2));
            DrawRoutine(nV, pV, Routine, Tpage);
            pSort += 3;
        }
    } else {
        pSort = (sint32 *)sortptr;
        for (nSurf = 0; nSurf < num; nSurf++) {
            pInfo = (sint16 *)(*pSort);
            Routine = *pInfo++;
            Tpage = pInfo[0];
            nV = pInfo[1];
            pV = *((D3DTLVERTEX **)(pInfo + 2));
            DrawRoutine(nV, pV, Routine, Tpage);
            pSort += 3;
        }
    }
}

extern D3DTLVERTEX *UnRollBuffer;

inline void TLVERT_To_TLVERT(D3DTLVERTEX *s, D3DTLVERTEX *d)
{
    d->sx = s->sx;
    d->sy = s->sy;
    d->sz = s->sz;
    d->rhw = s->rhw;
    d->tu = s->tu;
    d->tv = s->tv;
    d->color = s->color;
    d->specular = s->specular;
}

void HWR_DrawPolyListBF(int num, sint32 *sortptr)
{
    sint32 *pSort;
    sint16 *pInfo;
    int nSurf;
    D3DTLVERTEX *pV;
    int Routine1;
    int Tpage1;
    int Routine2;
    int Tpage2;
    int nV;
    int n;
    int vbcnt;

    dpPrimitiveType = D3DPT_TRIANGLELIST;
    if (num == 0)
        return;
    pSort = (sint32 *)sortptr;
    vbcnt = 0;
    pInfo = (sint16 *)(*pSort);
    Routine1 = *pInfo++;
    Tpage1 = pInfo[0];
    nV = pInfo[1];
    pV = *((D3DTLVERTEX **)(pInfo + 2));
    for (nSurf = 0; nSurf < num; nSurf++) {
        pInfo = (sint16 *)(*pSort);
        Routine2 = *pInfo++;
        Tpage2 = pInfo[0];
        nV = pInfo[1];
        pV = *((D3DTLVERTEX **)(pInfo + 2));
        if (Routine2 != Routine1 || Tpage1 != Tpage2 || vbcnt > 256) {
            DrawRoutine(vbcnt, &UnRollBuffer[0], Routine1, Tpage1);
            vbcnt = 0;
            Routine1 = Routine2;
            Tpage1 = Tpage2;
        }
        if (Routine1 != 12) {
            D3DTLVERTEX *pFanStart;

            pFanStart = pV;
            TLVERT_To_TLVERT(pV, &UnRollBuffer[vbcnt + 0]);
            pV++;
            TLVERT_To_TLVERT(pV, &UnRollBuffer[vbcnt + 1]);
            pV++;
            TLVERT_To_TLVERT(pV, &UnRollBuffer[vbcnt + 2]);
            vbcnt += 3;
            nV -= 3;
            for (n = nV; n > 0; n--) {
                TLVERT_To_TLVERT(pFanStart, &UnRollBuffer[vbcnt + 0]);
                TLVERT_To_TLVERT(pV, &UnRollBuffer[vbcnt + 1]);
                pV++;
                TLVERT_To_TLVERT(pV, &UnRollBuffer[vbcnt + 2]);
                vbcnt += 3;
            }
        } else {
            D3DTLVERTEX *pFanStart;

            pFanStart = pV;
            TLVERT_To_TLVERT(pV, &UnRollBuffer[vbcnt + 0]);
            pV++;
            TLVERT_To_TLVERT(pV, &UnRollBuffer[vbcnt + 1]);
            vbcnt += 2;
        }
        pSort += 3;
    }
    if (vbcnt > 0)
        DrawRoutine(vbcnt, &UnRollBuffer[0], Routine1, Tpage1);
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
        HWR_SetCurrentTexture(TPages[i]);
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

#ifdef TR3_EXE
extern "C" {

int LhPalette = -1;
float GammaOption = 3.0F;

}
#endif
