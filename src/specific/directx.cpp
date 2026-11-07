#include "standard.h"
#include "specific.h"
#include "winmain.h"
#include "global.h"
#include "cpu.h"

extern "C" {

extern LPDIRECTDRAW2 G_ddraw;
extern LPDIRECT3D2 G_d3d;
extern bool SoftwareRenderer;
HRESULT CALLBACK DXEnumDirect3D(LPGUID lpGuid, LPSTR description, LPSTR name, LPD3DDEVICEDESC lpHWDesc, LPD3DDEVICEDESC lpHELDesc, LPVOID lpContext);
HRESULT CALLBACK DXEnumDisplayModes(LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext);

extern HWND G_hwnd;
extern bool MMXSupported;
BOOL CALLBACK DXEnumDirectInput(LPCDIDEVICEINSTANCE lpDevInst, LPVOID lpContext);
BOOL CALLBACK DXEnumDirectSound(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext);
BOOL CALLBACK DXEnumDirectDraw(GUID FAR *lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext);

void DXGetDeviceInfo(DEVICEINFO *dd, HWND hWnd, HINSTANCE hInstance)
{
    LPDIRECTINPUT lpDinput;

    G_hwnd = hWnd;
    memset(dd, 0, sizeof(DEVICEINFO));
    CPUID cpu;
    MMXSupported = cpu.isMMX();
    DirectDrawEnumerate(DXEnumDirectDraw, dd);
    DirectSoundEnumerate(DXEnumDirectSound, dd);
    DirectInputCreate(hInstance, DIRECTINPUT_VERSION, &lpDinput, 0);
    lpDinput->EnumDevices(DIDEVTYPE_JOYSTICK, DXEnumDirectInput, dd, DIEDFL_ATTACHEDONLY);
    lpDinput->Release();
}

BOOL CALLBACK DXEnumDirectInput(LPCDIDEVICEINSTANCE lpDevInst, LPVOID lpContext)
{
    DEVICEINFO *dd;

    dd = (DEVICEINFO *)lpContext;
    if (!dd->nDIInfo)
        dd->DIInfo = (DXDIRECTINPUTINFO *)malloc(sizeof(DXDIRECTINPUTINFO));
    else
        dd->DIInfo = (DXDIRECTINPUTINFO *)realloc(dd->DIInfo, (dd->nDIInfo + 1) * sizeof(DXDIRECTINPUTINFO));
    memset(&dd->DIInfo[dd->nDIInfo], 0, sizeof(DXDIRECTINPUTINFO));

    if (!&lpDevInst->guidInstance) {
        dd->DIInfo[dd->nDIInfo].lpGuid = 0;
    } else {
        dd->DIInfo[dd->nDIInfo].lpGuid = &dd->DIInfo[dd->nDIInfo].Guid;
        dd->DIInfo[dd->nDIInfo].Guid = lpDevInst->guidInstance;
    }

    lstrcpy(dd->DIInfo[dd->nDIInfo].About, lpDevInst->tszProductName);
    lstrcpy(dd->DIInfo[dd->nDIInfo].Name, lpDevInst->tszInstanceName);
    dd->nDIInfo++;
    return DIENUM_CONTINUE;
}

BOOL CALLBACK DXEnumDirectSound(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
{
    DEVICEINFO *dd;

    dd = (DEVICEINFO *)lpContext;
    if (!dd->nDSInfo)
        dd->DSInfo = (DXDIRECTSOUNDINFO *)malloc(sizeof(DXDIRECTSOUNDINFO));
    else
        dd->DSInfo = (DXDIRECTSOUNDINFO *)realloc(dd->DSInfo, (dd->nDSInfo + 1) * sizeof(DXDIRECTSOUNDINFO));
    memset(&dd->DSInfo[dd->nDSInfo], 0, sizeof(DXDIRECTSOUNDINFO));

    if (!lpGuid) {
        dd->DSInfo[dd->nDSInfo].lpGuid = 0;
    } else {
        dd->DSInfo[dd->nDSInfo].lpGuid = &dd->DSInfo[dd->nDSInfo].Guid;
        dd->DSInfo[dd->nDSInfo].Guid = *lpGuid;
    }

    lstrcpy(dd->DSInfo[dd->nDSInfo].About, lpcstrDescription);
    lstrcpy(dd->DSInfo[dd->nDSInfo].Name, lpcstrModule);
    dd->nDSInfo++;
    return TRUE;
}

void DXFreeDeviceInfo(DEVICEINFO *dd)
{
    int i, j;

    for (i = 0; i < dd->nDDInfo; i++) {
        for (j = 0; j < dd->DDInfo[i].nD3DInfo; j++) {
            free(dd->DDInfo[i].D3DInfo[j].DisplayMode);
            free(dd->DDInfo[i].D3DInfo[j].Texture);
        }
        free(dd->DDInfo[i].D3DInfo);
        free(dd->DDInfo[i].DisplayMode);
    }
    free(dd->DDInfo);
    if (dd->DSInfo)
        free(dd->DSInfo);
    if (dd->DIInfo)
        free(dd->DIInfo);
    memset(dd, 0, sizeof(DEVICEINFO));
}

BOOL CALLBACK DXEnumDirectDraw(GUID FAR *lpGUID, LPSTR lpDriverDescription, LPSTR lpDriverName, LPVOID lpContext)
{
    DEVICEINFO *dd;
    LPDIRECTDRAW lpDD;

    dd = (DEVICEINFO *)lpContext;
    if (!dd->nDDInfo)
        dd->DDInfo = (DIRECTDRAWINFO *)malloc(sizeof(DIRECTDRAWINFO));
    else
        dd->DDInfo = (DIRECTDRAWINFO *)realloc(dd->DDInfo, (dd->nDDInfo + 1) * sizeof(DIRECTDRAWINFO));
    memset(&dd->DDInfo[dd->nDDInfo], 0, sizeof(DIRECTDRAWINFO));

    if (!lpGUID) {
        dd->DDInfo[dd->nDDInfo].lpGuid = 0;
    } else {
        dd->DDInfo[dd->nDDInfo].lpGuid = &dd->DDInfo[dd->nDDInfo].Guid;
        dd->DDInfo[dd->nDDInfo].Guid = *lpGUID;
    }

    lstrcpy(dd->DDInfo[dd->nDDInfo].About, lpDriverDescription);
    lstrcpy(dd->DDInfo[dd->nDDInfo].Name, lpDriverName);

    DirectDrawCreate(lpGUID, &lpDD, 0);
    lpDD->QueryInterface(IID_IDirectDraw2, (LPVOID *)&G_ddraw);
    if (lpDD) {
        lpDD->Release();
        lpDD = 0;
    }

    dd->DDInfo[dd->nDDInfo].DDCaps.dwSize = sizeof(DDCAPS);
    G_ddraw->GetCaps(&dd->DDInfo[dd->nDDInfo].DDCaps, 0);
    G_ddraw->SetCooperativeLevel(0, DDSCL_FULLSCREEN | DDSCL_NOWINDOWCHANGES | DDSCL_NORMAL | DDSCL_ALLOWMODEX);
    G_ddraw->EnumDisplayModes(0, 0, &dd->DDInfo[dd->nDDInfo], DXEnumDisplayModes);
    G_ddraw->QueryInterface(IID_IDirect3D2, (LPVOID *)&G_d3d);
    SoftwareRenderer = false;
    G_d3d->EnumDevices(DXEnumDirect3D, &dd->DDInfo[dd->nDDInfo]);
    G_ddraw->SetCooperativeLevel(0, DDSCL_NORMAL);
    G_d3d->Release();
    G_ddraw->Release();
    dd->nDDInfo++;
    return DDENUMRET_OK;
}

void DXBitMask2ShiftCnt(unsigned long mask, unsigned char *shift, unsigned char *cnt)
{
    unsigned long m;
    unsigned char n;

    for (n = 0, m = mask; !(m & 1); n++, m >>= 1)
        ;
    *shift = n;
    for (n = 0; m & 1; n++, m >>= 1)
        ;
    *cnt = n;
}

HRESULT CALLBACK DXEnumDisplayModes(LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext)
{
    DIRECTDRAWINFO *dd;

    dd = (DIRECTDRAWINFO *)lpContext;
    if (!dd->nDisplayMode)
        dd->DisplayMode = (DISPLAYMODE *)malloc(sizeof(DISPLAYMODE));
    else
        dd->DisplayMode = (DISPLAYMODE *)realloc(dd->DisplayMode, (dd->nDisplayMode + 1) * sizeof(DISPLAYMODE));
    memset(&dd->DisplayMode[dd->nDisplayMode], 0, sizeof(DISPLAYMODE));

    dd->DisplayMode[dd->nDisplayMode].w = lpDDSurfaceDesc->dwWidth;
    dd->DisplayMode[dd->nDisplayMode].h = lpDDSurfaceDesc->dwHeight;
    dd->DisplayMode[dd->nDisplayMode].bpp = lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount;
    dd->DisplayMode[dd->nDisplayMode].bPalette = (lpDDSurfaceDesc->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) != 0;
    memcpy(&dd->DisplayMode[dd->nDisplayMode].ddsd, lpDDSurfaceDesc, sizeof(DDSURFACEDESC));

    if (!dd->DisplayMode[dd->nDisplayMode].bPalette) {
        DXBitMask2ShiftCnt(lpDDSurfaceDesc->ddpfPixelFormat.dwRBitMask, &dd->DisplayMode[dd->nDisplayMode].rshift, &dd->DisplayMode[dd->nDisplayMode].rbpp);
        DXBitMask2ShiftCnt(lpDDSurfaceDesc->ddpfPixelFormat.dwGBitMask, &dd->DisplayMode[dd->nDisplayMode].gshift, &dd->DisplayMode[dd->nDisplayMode].gbpp);
        DXBitMask2ShiftCnt(lpDDSurfaceDesc->ddpfPixelFormat.dwBBitMask, &dd->DisplayMode[dd->nDisplayMode].bshift, &dd->DisplayMode[dd->nDisplayMode].bbpp);
        if (lpDDSurfaceDesc->ddpfPixelFormat.dwRGBAlphaBitMask)
            DXBitMask2ShiftCnt(lpDDSurfaceDesc->ddpfPixelFormat.dwRGBAlphaBitMask, &dd->DisplayMode[dd->nDisplayMode].ashift, &dd->DisplayMode[dd->nDisplayMode].abpp);
    }

    dd->nDisplayMode++;
    return DDENUMRET_OK;
}

HRESULT CALLBACK DXEnumTextureFormats(LPDDSURFACEDESC lpDDPixFmt, LPVOID lpContext)
{
    DIRECT3DINFO *d3d;

    d3d = (DIRECT3DINFO *)lpContext;
    if (!d3d->nTexture)
        d3d->Texture = (D3DTEXTUREINFO *)malloc(sizeof(D3DTEXTUREINFO));
    else
        d3d->Texture = (D3DTEXTUREINFO *)realloc(d3d->Texture, (d3d->nTexture + 1) * sizeof(D3DTEXTUREINFO));
    memset(&d3d->Texture[d3d->nTexture], 0, sizeof(D3DTEXTUREINFO));
    memcpy(&d3d->Texture[d3d->nTexture].ddsd, lpDDPixFmt, sizeof(DDSURFACEDESC));

    if (lpDDPixFmt->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8) {
        d3d->Texture[d3d->nTexture].bPalette = true;
        d3d->Texture[d3d->nTexture].bpp = 8;
    } else {
        if (lpDDPixFmt->ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED4)
            return D3DENUMRET_OK;
        d3d->Texture[d3d->nTexture].bPalette = false;
        d3d->Texture[d3d->nTexture].bpp = lpDDPixFmt->ddpfPixelFormat.dwRGBBitCount;
        DXBitMask2ShiftCnt(lpDDPixFmt->ddpfPixelFormat.dwRBitMask, &d3d->Texture[d3d->nTexture].rshift, &d3d->Texture[d3d->nTexture].rbpp);
        DXBitMask2ShiftCnt(lpDDPixFmt->ddpfPixelFormat.dwGBitMask, &d3d->Texture[d3d->nTexture].gshift, &d3d->Texture[d3d->nTexture].gbpp);
        DXBitMask2ShiftCnt(lpDDPixFmt->ddpfPixelFormat.dwBBitMask, &d3d->Texture[d3d->nTexture].bshift, &d3d->Texture[d3d->nTexture].bbpp);
        if (lpDDPixFmt->ddpfPixelFormat.dwRGBAlphaBitMask) {
            DXBitMask2ShiftCnt(lpDDPixFmt->ddpfPixelFormat.dwRGBAlphaBitMask, &d3d->Texture[d3d->nTexture].ashift, &d3d->Texture[d3d->nTexture].abpp);
            d3d->Texture[d3d->nTexture].bAlpha = true;
        }
    }

    d3d->nTexture++;
    return D3DENUMRET_OK;
}

DWORD BPPToDDBD(int bpp)
{
    switch (bpp) {
    case 1:
        return DDBD_1;
    case 2:
        return DDBD_2;
    case 4:
        return DDBD_4;
    case 8:
        return DDBD_8;
    case 16:
        return DDBD_16;
    case 24:
        return DDBD_24;
    case 32:
        return DDBD_32;
    }
    return 0;
}

bool DXCreateDirectDraw(DEVICEINFO *dd, DXCONFIG *DXConfig, LPDIRECTDRAW2 *lpDD2)
{
    LPDIRECTDRAW lpDD;
    int RetVal;

    RetVal = DirectDrawCreate(dd->DDInfo[DXConfig->nDD].lpGuid, &lpDD, 0);
    if (RetVal != DD_OK)
        return false;
    RetVal = lpDD->QueryInterface(IID_IDirectDraw2, (LPVOID *)lpDD2);
    if (lpDD) {
        lpDD->Release();
        lpDD = 0;
    }
    if (RetVal != DD_OK)
        return false;
    return true;
}

bool DXCreateDirect3D(LPDIRECTDRAW2 lpDD2, LPDIRECT3D2 *lpD3D2)
{
    int RetVal;

    RetVal = lpDD2->QueryInterface(IID_IDirect3D2, (LPVOID *)lpD3D2);
    if (RetVal != DD_OK)
        return false;
    return true;
}

bool DXSetCooperativeLevel(LPDIRECTDRAW2 lpDD2, HWND WindowHandle, int Flags)
{
    int RetVal;

    RetVal = lpDD2->SetCooperativeLevel(WindowHandle, Flags);
    if (RetVal != DD_OK)
        return false;
    return true;
}

bool DXSetVideoMode(LPDIRECTDRAW2 lpDD2, int w, int h, int bpp)
{
    int RetVal;

    RetVal = lpDD2->SetDisplayMode(w, h, bpp, 0, 0);
    if (RetVal != DD_OK)
        return false;
    return true;
}

bool DXCreateSurface(LPDIRECTDRAW2 lpDD2, DDSURFACEDESC *ddsd, LPDIRECTDRAWSURFACE3 *lpSurface)
{
    int RetVal;
    LPDIRECTDRAWSURFACE lps;

    RetVal = lpDD2->CreateSurface(ddsd, &lps, 0);
    if (RetVal != DD_OK)
        return false;
    RetVal = lps->QueryInterface(IID_IDirectDrawSurface3, (LPVOID *)lpSurface);
    if (lps) {
        lps->Release();
        lps = 0;
    }
    if (RetVal != DD_OK)
        return false;
    return true;
}

bool DXGetAttachedSurface(LPDIRECTDRAWSURFACE3 lpPrimary, DDSCAPS *ddscaps, LPDIRECTDRAWSURFACE3 *lpAttached)
{
    int RetVal;

    RetVal = lpPrimary->GetAttachedSurface(ddscaps, lpAttached);
    if (RetVal != DD_OK)
        return false;
    return true;
}

bool DXAddAttachedSurface(LPDIRECTDRAWSURFACE3 lpSurface, LPDIRECTDRAWSURFACE3 lpAddSurface)
{
    int RetVal;

    RetVal = lpSurface->AddAttachedSurface(lpAddSurface);
    if (RetVal != DD_OK)
        return false;
    return true;
}

bool DXCreateDirect3DDevice(LPDIRECT3D2 lpD3D2, GUID Guid, LPDIRECTDRAWSURFACE3 lpSurface, LPDIRECT3DDEVICE2 *lpD3DDevice2)
{
    int RetVal;

    RetVal = lpD3D2->CreateDevice(Guid, (LPDIRECTDRAWSURFACE)lpSurface, lpD3DDevice2);
    if (RetVal != DD_OK)
        return false;
    return true;
}

bool DXCreateViewPort(LPDIRECT3D2 lpD3D, LPDIRECT3DDEVICE2 lpD3DDevice, int w, int h, LPDIRECT3DVIEWPORT2 *lpViewport)
{
    int RetVal;
    D3DVIEWPORT2 viewdata;

    RetVal = lpD3D->CreateViewport(lpViewport, NULL);
    if (RetVal != DD_OK)
        return false;

    RetVal = lpD3DDevice->AddViewport(*lpViewport);
    if (RetVal != DD_OK)
        return false;

    {
        memset(&viewdata, 0, sizeof(viewdata));
        viewdata.dwSize = sizeof(viewdata);
    }
    viewdata.dwX = 0;
    viewdata.dwY = 0;
    viewdata.dwWidth = w;
    viewdata.dwHeight = h;
    float inva = (float)((float)h / (float)w);
    viewdata.dvClipX = -1.0F;
    viewdata.dvClipY = inva;
    viewdata.dvClipWidth = 2.0F;
    viewdata.dvClipHeight = 2.0F * inva;
    viewdata.dvMinZ = 0.0F;
    viewdata.dvMaxZ = 1.0F;

    RetVal = (*lpViewport)->SetViewport2(&viewdata);
    if (RetVal != DD_OK)
        return false;

    RetVal = lpD3DDevice->SetCurrentViewport(*lpViewport);

    return true;
}

void DXGetSurfaceDesc(LPDIRECTDRAWSURFACE3 lpSurface, DDSURFACEDESC *ddsd)
{
    lpSurface->GetSurfaceDesc(ddsd);
}

bool DXSurfBlt(LPDIRECTDRAWSURFACE3 lpSurface, LPRECT lpRect, int FillColor)
{
    DDBLTFX bltfx;
    int RetVal;

    memset(&bltfx, 0, sizeof(bltfx));
    bltfx.dwSize = sizeof(bltfx);
    bltfx.dwFillColor = FillColor;
    RetVal = lpSurface->Blt(lpRect, 0, 0, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx);
    if (RetVal != DD_OK)
        return false;
    return true;
}

}
