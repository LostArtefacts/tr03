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
