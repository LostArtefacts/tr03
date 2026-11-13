#include "standard.h"
#include "specific.h"
#include "winmain.h"
#include "global.h"

extern "C" {

extern int nReEntry;
extern bool LtWM_QUIT;
extern bool GtWindowClosed;
extern int game_closedown;

bool DD_SpinMessageLoop(bool tWait)
{
    ++nReEntry;
    if (LtWM_QUIT) {
        --nReEntry;
        return false;
    }
    if (tWait)
        WaitMessage();
    MSG msg;
    for (;;) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                break;
        } else if (App.bFocus) {
            --nReEntry;
            return true;
        } else {
            WaitMessage();
        }
    }
    LtWM_QUIT = true;
    GtWindowClosed = true;
    game_closedown = true;
    --nReEntry;
    return false;
}

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
extern char *malloc_ptr;
extern int SaveScreenNum;
extern char TgaHeader[18];
bool DD_SpinMessageLoop(bool tWait);
void DXGetSurfaceDesc(LPDIRECTDRAWSURFACE3 lpSurface, LPDDSURFACEDESC lpDDSurfaceDesc);
void MMXBlit32to15(void *dest, void *src, int w);
void MMXBlit32to16(void *dest, void *src, int w);
void MMXBlit32to24(void *dest, void *src, int w);
void SWRBlit32to15(unsigned long *dest, unsigned long *src, int w);

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

void DX_DoFlipWait(void)
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

void DXSaveScreen(LPDIRECTDRAWSURFACE3 lpSurface)
{
    FILE *file;
    DDSURFACEDESC ddsd;
    sint16 *pSurf;
    sint16 *pDest;
    unsigned long h;
    unsigned long w;
    unsigned long c;
    long r;
    long g;
    long b;
    char buf[16];

    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    if (DX_TRY(lpSurface->GetSurfaceDesc(&ddsd)))
        return;

    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    if (DX_TRY(DD_LockSurface(lpSurface, ddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY)))
        return;

    pSurf = (sint16 *)ddsd.lpSurface;
    sprintf(buf, "tomb%04d.tga", SaveScreenNum++);
    file = fopen(buf, "wb");
    if (file) {
        *(sint16 *)&TgaHeader[12] = (sint16)ddsd.dwWidth;
        *(sint16 *)&TgaHeader[14] = (sint16)ddsd.dwHeight;
        fwrite(TgaHeader, sizeof(TgaHeader), 1, file);
        pDest = (sint16 *)malloc_ptr;
        pSurf += ddsd.dwHeight * (ddsd.lPitch / 2);
        for (h = 0; h < ddsd.dwHeight; h++) {
            for (w = 0; w < ddsd.dwWidth; w++) {
                c = pSurf[w];
                if (ddsd.ddpfPixelFormat.dwRBitMask == 0xF800) {
                    r = (c >> 11) & 0x1F;
                    g = (c >> 6) & 0x1F;
                    b = c & 0x1F;
                    *pDest++ = (sint16)((r << 10) + (g << 5) + b);
                } else {
                    *pDest++ = (sint16)c;
                }
            }
            pSurf -= ddsd.lPitch / 2;
        }
        fwrite(malloc_ptr, 2 * ddsd.dwWidth * ddsd.dwHeight, 1, file);
        fclose(file);

        buf[7]++;
        if (buf[7] > '9') {
            buf[7] = '0';
            buf[6]++;
        }
    }

    DD_UnlockSurface(lpSurface, ddsd);
}

bool DXUpdateFrame(bool runMessageLoop, LPRECT rect)
{
    DIRECT3DINFO *d3dinfo;
    DDSURFACEDESC desc;
    DDSURFACEDESC backDesc;
    int w;
    int i;
    int n;

    App.nFrames++;
    DXCheckForLostSurfaces();
    d3dinfo = &App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D];
    w = d3dinfo->DisplayMode[App.DXConfigPtr->nVMode].w;

    if (d3dinfo->bHardware) {
        App.lpFrontBuffer->Flip(0, DDFLIP_WAIT);
    } else {
        memset(&desc, 0, sizeof(desc));
        memset(&backDesc, 0, sizeof(backDesc));
        desc.dwSize = sizeof(desc);
        backDesc.dwSize = sizeof(backDesc);
        DXGetSurfaceDesc(App.lpBackBuffer, &backDesc);
        (backDesc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY ? App.lpBackBuffer : App.lpFrontBuffer)->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0);

        if (App.DXConfig.MMX) {
            if (desc.ddpfPixelFormat.dwRGBBitCount == 32) {
                for (i = 0, n = 0; i < (int)desc.dwHeight; i++, n += w)
                    memcpy((unsigned char *)desc.lpSurface + desc.lPitch * i, &App.unk[n], w * 4);
            }
            if (desc.ddpfPixelFormat.dwRGBBitCount == 16 && desc.ddpfPixelFormat.dwGBitMask == 0x3E0) {
                for (i = 0, n = 0; i < (int)desc.dwHeight; i++, n += w)
                    MMXBlit32to15((unsigned char *)desc.lpSurface + desc.lPitch * i, &App.unk[n], w);
            } else if (desc.ddpfPixelFormat.dwRGBBitCount == 16 && desc.ddpfPixelFormat.dwGBitMask == 0x7E0) {
                for (i = 0, n = 0; i < (int)desc.dwHeight; i++, n += w)
                    MMXBlit32to16((unsigned char *)desc.lpSurface + desc.lPitch * i, &App.unk[n], w);
            } else if (desc.ddpfPixelFormat.dwRGBBitCount == 24) {
                for (i = 0, n = 0; i < (int)desc.dwHeight; i++, n += w)
                    MMXBlit32to24((unsigned char *)desc.lpSurface + desc.lPitch * i, &App.unk[n], w);
            }
        } else {
            if (desc.ddpfPixelFormat.dwGBitMask == 0x3E0) {
                for (i = 0, n = 0; i < (int)desc.dwHeight; i++, n += 2 * w)
                    SWRBlit32to15((unsigned long *)((unsigned char *)desc.lpSurface + desc.lPitch * i), (unsigned long *)((unsigned char *)App.unk + n), w);
            } else {
                for (i = 0, n = 0; i < (int)desc.dwHeight; i++, n += 2 * w)
                    memcpy((unsigned char *)desc.lpSurface + desc.lPitch * i, (unsigned char *)App.unk + n, 2 * w);
            }
        }

        if (backDesc.ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) {
            App.lpBackBuffer->Unlock(0);
            App.lpFrontBuffer->Flip(0, DDFLIP_WAIT);
        } else {
            App.lpFrontBuffer->Unlock(0);
        }
    }

    if (runMessageLoop)
        return DD_SpinMessageLoop(false);
    return true;
}


void SWRBlit32to15(unsigned long *dest, unsigned long *src, int w)
{
    __asm {
        mov ecx, dword ptr [w]
        mov esi, dword ptr [src]
        mov edi, dword ptr [dest]
        shr ecx, 0x2
        push ebp
        mov ebp, ecx
        _emit 0x8d
        _emit 0x64
        _emit 0x24
        _emit 0x00
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
    L1:
        mov eax, dword ptr [esi]
        mov ecx, dword ptr [esi+0x4]
        mov ebx, eax
        mov edx, ecx
        and eax, 0xffc0ffc0
        and ecx, 0xffc0ffc0
        shr eax, 1
        and ebx, 0x1f001f
        shr ecx, 1
        and edx, 0x1f001f
        or eax, ebx
        or ecx, edx
        mov dword ptr [edi], eax
        add esi, 0x8
        mov dword ptr [edi+0x4], ecx
        add edi, 0x8
        dec ebp
        jne L1
        pop ebp
    }
}
}

#ifdef TR3_EXE
extern "C" {

char TgaHeader[18] = {
    0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 64, 1, 0, 1, 16, 0,
};

}
#endif
