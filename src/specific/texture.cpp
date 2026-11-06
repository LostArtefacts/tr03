#include "standard.h"
#include "specific.h"
#include "global.h"
#include "texture.h"
#include "winmain.h"

extern "C" {

extern bool bMakeGrey;
extern bool bSetColorKey;
extern TEXTURE Textures[32];
extern int nTPages;

HRESULT DD_CreateSurface(DDSURFACEDESC &ddsd, LPDIRECTDRAWSURFACE3 &lpSurface);
void DXTextureCleanup(int nTexture, DXTEXTURE *pTextures);


void DXTextureSetGreyScale(bool set)
{
    bMakeGrey = set;
}

LPDIRECT3DTEXTURE2 DXTextureGetInterface(LPDIRECTDRAWSURFACE3 lpSurface)
{
    LPDIRECT3DTEXTURE2 lpTexture;

    if (DX_TRY(lpSurface->QueryInterface(IID_IDirect3DTexture2, (LPVOID *)&lpTexture)))
        return 0;
    return lpTexture;
}

HRESULT DXTextureNewPalette(unsigned char *pPal)
{
    DWORD adwPal[256];
    int nEntry;

    if (DXPalette) {
        DXPalette->Release();
        DXPalette = 0;
    }
    for (nEntry = 0; nEntry < 256; ++nEntry, pPal += 3)
        adwPal[nEntry] = RGB(pPal[0], pPal[1], pPal[2]);
    return App.lpDD->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256, (PALETTEENTRY *)adwPal, &DXPalette, 0);
}

bool DXCreateTextureSurface(TEXTURE *tex, LPDDPIXELFORMAT ddpf)
{
    DDSURFACEDESC desc;
    DDCOLORKEY ckey;

    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    desc.ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_VIDEOMEMORY;
    if (App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].bAGP)
        desc.ddsCaps.dwCaps |= DDSCAPS_NONLOCALVIDMEM;
    desc.dwWidth = 256;
    desc.dwHeight = 256;
    memcpy(&desc.ddpfPixelFormat, ddpf, sizeof(DDPIXELFORMAT));
    if (DX_TRY(DD_CreateSurface(desc, tex->pSurf))) {
        tex->num = 4;
        return false;
    }
    if (desc.ddpfPixelFormat.dwRGBBitCount == 8 && DXPalette) {
        tex->pSurf->SetPalette(DXPalette);
        tex->pPalette = DXPalette;
        DXPalette->AddRef();
    }
    if (bSetColorKey && !App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].Texture[App.DXConfigPtr->D3DTF].bAlpha) {
        ckey.dwColorSpaceLowValue = 0;
        ckey.dwColorSpaceHighValue = 0;
        tex->pSurf->SetColorKey(DDCKEY_SRCBLT, &ckey);
    }
    tex->pTexture = DXTextureGetInterface(tex->pSurf);
    tex->pTexture->GetHandle(App.lpD3DDevice, &tex->handle);
    tex->num = 2;
    tex->DXTex = 0;
    tex->bpp = desc.ddpfPixelFormat.dwRGBBitCount;
    return true;
}

void DXCreateMaxTPages(int create)
{
    TEXTURE *tex;
    LPDDPIXELFORMAT ddpf;
    LPDDPIXELFORMAT ddpf2;
    int n;
    int oldTF;

    if (!App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].bHardware)
        return;
    ddpf = &App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].Texture[App.DXConfigPtr->D3DTF].ddsd.ddpfPixelFormat;
    DXCreateTextureSurface(Textures, ddpf);
    n = 1;
    if (create && ddpf->dwRGBBitCount == 8) {
        bSetColorKey = false;
        oldTF = App.DXConfigPtr->D3DTF;
        App.DXConfigPtr->D3DTF = 0;
        ddpf2 = &App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].Texture[App.DXConfigPtr->D3DTF].ddsd.ddpfPixelFormat;
        tex = &Textures[1];
        for (; n < 6; n++, tex++) {
            if (!DXCreateTextureSurface(tex, ddpf2))
                break;
        }
        App.DXConfigPtr->D3DTF = oldTF;
        ddpf = &App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].Texture[App.DXConfigPtr->D3DTF].ddsd.ddpfPixelFormat;
        bSetColorKey = true;
    }
    for (; n < 32; n++) {
        if (!DXCreateTextureSurface(&Textures[n], ddpf))
            break;
    }
    nTPages = n;
}

void DXFreeTPages(void)
{
    TEXTURE *tex;
    int i;

    if (!App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].bHardware)
        return;
    for (i = 0; i < 32; i++) {
        tex = &Textures[i];
        if (tex->DXTex)
            ((DXTEXTURE *)tex->DXTex)->tex = 0;
        if (tex->pTexture) {
            tex->pTexture->Release();
            tex->pTexture = 0;
        }
        if (tex->pSurf) {
            tex->pSurf->Release();
            tex->pSurf = 0;
        }
        if (tex->pPalette) {
            tex->pPalette->Release();
            tex->pPalette = 0;
        }
        memset(tex, 0, sizeof(TEXTURE));
    }
    nTPages = 0;
}

int DXTextureFindTextureSlot(DXTEXTURE *tex)
{
    int i;

    for (i = 0; i < 32; i++) {
        if (!(tex[i].dwFlags & 1))
            return i;
    }
    return -1;
}

bool DXTextureMakeSystemSurface(DXTEXTURE *tex, LPDDPIXELFORMAT ddpf)
{
    DDSURFACEDESC desc;

    memset(&desc, 0, sizeof(desc));
    desc.dwSize = sizeof(desc);
    desc.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    desc.ddsCaps.dwCaps = DDSCAPS_SYSTEMMEMORY | DDSCAPS_TEXTURE;
    desc.dwWidth = tex->nWidth;
    desc.dwHeight = tex->nHeight;
    memcpy(&desc.ddpfPixelFormat, ddpf, sizeof(DDPIXELFORMAT));
    if (DX_TRY(DD_CreateSurface(desc, tex->pSystemSurface)))
        return false;
    if (tex->pPalette && DX_TRY(tex->pSystemSurface->SetPalette(tex->pPalette)))
        return false;
    return true;
}

int DXTextureMakeDeviceSurface(int w, int h, LPDIRECTDRAWPALETTE palette, DXTEXTURE *list)
{
    DXTEXTURE *tex;
    LPDDPIXELFORMAT ddpf;
    int index;

    index = DXTextureFindTextureSlot(list);
    if (index < 0)
        return -1;
    tex = &list[index];
    memset(tex, 0, sizeof(DXTEXTURE));
    tex->nWidth = w;
    tex->dwFlags = 1;
    tex->nHeight = h;
    tex->pPalette = palette;
    ddpf = &App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].Texture[App.DXConfigPtr->D3DTF].ddsd.ddpfPixelFormat;
    tex->bpp = ddpf->dwRGBBitCount;
    if (!DXTextureMakeSystemSurface(tex, ddpf))
        return -1;
    return index;
}

void DXClearAllTextures(DXTEXTURE *list)
{
    int i;

    for (i = 0; i < 32; i++)
        DXTextureCleanup(i, list);
    if (DXPalette) {
        DXPalette->Release();
        DXPalette = 0;
    }
}

DXTEXTURE *DXRestoreSurfaceIfLost(int index, DXTEXTURE *list)
{
    DXTEXTURE *tex;

    tex = &list[index];
    if (tex->tex && tex->tex->pSurf && tex->tex->pSurf->IsLost() == DDERR_SURFACELOST) {
        tex->tex->pSurf->Restore();
        tex->tex->DXTex = 0;
        tex->tex = 0;
    }
    return &list[index];
}


void MMXTextureCopy(unsigned long *dest, unsigned char *src, unsigned long step)
{
    long x, y;
    long add;

    y = 0;
    add = step << 10;
    for (; y < 256; y += step) {
        for (x = 0; x < 256; x += step)
            *dest++ = ((unsigned long *)src)[x];
        src += add;
    }
}

}
