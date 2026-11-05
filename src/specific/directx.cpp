#include "standard.h"
#include "specific.h"
#include "winmain.h"
#include "global.h"

extern "C" {

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

void DXGetSurfaceDesc(LPDIRECTDRAWSURFACE3 lpSurface, DDSURFACEDESC *ddsd)
{
    lpSurface->GetSurfaceDesc(ddsd);
}

}
