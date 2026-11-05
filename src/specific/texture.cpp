#include "standard.h"
#include "specific.h"
#include "global.h"

extern "C" {

typedef struct dxtexture {
    LPDIRECTDRAWSURFACE3 pSystemSurface;
    LPDIRECTDRAWPALETTE pPalette;
    long nWidth;
    long nHeight;
    unsigned long dwFlags;
    unsigned long *pData;
    struct texture *tex;
    unsigned long bpp;
} DXTEXTURE;

extern bool bMakeGrey;

void DXTextureSetGreyScale(bool set)
{
    bMakeGrey = set;
}

}
