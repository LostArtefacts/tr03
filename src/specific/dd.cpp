#include "standard.h"
#include "specific.h"
#include "winmain.h"
#include "global.h"

extern "C" {

HRESULT DD_LockSurface(LPDIRECTDRAWSURFACE3 lpSurface, DDSURFACEDESC &ddsd, DWORD dwFlags)
{
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    return lpSurface->Lock(0, &ddsd, dwFlags, 0);
}

HRESULT DD_UnlockSurface(LPDIRECTDRAWSURFACE3 lpSurface, DDSURFACEDESC &ddsd)
{
    return lpSurface->Unlock(ddsd.lpSurface);
}

bool DD_ClearSurface(LPDIRECTDRAWSURFACE3 lpSurface, LPRECT lpRect, DWORD dwColour);
void HWR_GetAllTextureHandles(void);
extern bool GtWindowClosed;

HRESULT DD_CreateSurface(DDSURFACEDESC &ddsd, LPDIRECTDRAWSURFACE3 &lpSurface)
{
    HRESULT hr;
    LPDIRECTDRAWSURFACE lps;

    if (DX_TRY(hr = App.lpDD->CreateSurface(&ddsd, &lps, 0)))
        return hr;
    hr = lps->QueryInterface(IID_IDirectDrawSurface3, (LPVOID *)&lpSurface);
    lps->Release();
    return hr;
}

HRESULT DD_EnsureSurfaceAvailable(LPDIRECTDRAWSURFACE3 lpSurface, LPDIRECTDRAWSURFACE3 lpTarget, bool tClear)
{
    HRESULT hr;

    hr = lpSurface->IsLost();
    if (hr == DDERR_SURFACELOST) {
        if (!lpTarget)
            lpTarget = lpSurface;
        hr = lpTarget->Restore();
        if (tClear && SUCCEEDED(hr))
            DD_ClearSurface(lpSurface, 0, 0);
    }
    return hr;
}

bool DD_ClearSurface(LPDIRECTDRAWSURFACE3 lpSurface, LPRECT lpRect, DWORD dwColour)
{
    DDBLTFX bltfx;

    memset(&bltfx, 0, sizeof(bltfx));
    bltfx.dwSize = sizeof(bltfx);
    bltfx.dwFillColor = dwColour;
    if (DX_TRY(lpSurface->Blt(lpRect, 0, 0, DDBLT_WAIT | DDBLT_COLORFILL, &bltfx)))
        return false;
    return true;
}

void DXDoFlipWait(void)
{
    while (App.lpFrontBuffer->GetFlipStatus(DDGFS_ISFLIPDONE) == DDERR_WASSTILLDRAWING)
        ;
}

void DXClearBuffers(DWORD flags, DWORD color)
{
    RECT r;
    D3DRECT vr;
    DWORD sflags;

    r.top = 0;
    r.left = 0;
    r.right = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].w;
    r.bottom = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].h;

    if (App.nRenderMode == 1) {
        sflags = 0;
        if (flags & 2)
            sflags = D3DCLEAR_TARGET;
        if (flags & 8)
            sflags |= D3DCLEAR_ZBUFFER;
        if (sflags) {
            vr.x1 = 0;
            vr.y1 = 0;
            vr.x2 = r.right;
            vr.y2 = r.bottom;
            App.lpViewPort->Clear(1, &vr, sflags);
        }
    } else if (flags & 2) {
        DD_ClearSurface(App.lpBackBuffer, &r, color);
    }

    if (flags & 1)
        DD_ClearSurface(App.lpFrontBuffer, &r, color);

    if (flags & 0x20) {
        r.top = 0;
        r.left = 0;
        r.right = 640;
        r.bottom = 480;
        DD_ClearSurface(App.lpPictureBuffer, &r, color);
    }

    if (!App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].bHardware && (flags & 2))
        memset(App.unk, 0, 4 * App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].w * App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].h);
}

bool DXCheckForLostSurfaces(void)
{
    bool lost;

    if (!App.lpFrontBuffer)
        S_ExitSystem("Oops... no front buffer");

    lost = DX_TRY(DD_EnsureSurfaceAvailable(App.lpFrontBuffer, 0, true)) || DX_TRY(DD_EnsureSurfaceAvailable(App.lpBackBuffer, App.lpFrontBuffer, true));
    if (App.lpZBuffer)
        lost = lost || DX_TRY(DD_EnsureSurfaceAvailable(App.lpZBuffer, 0, false));
    lost = lost || DX_TRY(DD_EnsureSurfaceAvailable(App.lpPictureBuffer, 0, false));

    if (lost && !GtWindowClosed && App.nRenderMode == 1)
        HWR_GetAllTextureHandles();

    return lost;
}

}
