#include "standard.h"
#include "specific.h"
#include "global.h"
#include "texture.h"

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
