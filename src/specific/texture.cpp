#include "standard.h"
#include "specific.h"
#include "global.h"
#include "texture.h"
#include "winmain.h"

extern "C" {

extern bool bMakeGrey;

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
