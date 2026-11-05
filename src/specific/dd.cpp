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

}
