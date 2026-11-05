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

int DXTextureFindTextureSlot(DXTEXTURE *tex)
{
    int i;

    for (i = 0; i < 32; i++) {
        if (!(tex[i].dwFlags & 1))
            return i;
    }
    return -1;
}

}
