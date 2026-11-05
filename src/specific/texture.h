#ifndef TEXTURE_H
#define TEXTURE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct texture {
    LPVOID DXTex;
    LPDIRECTDRAWSURFACE3 pSurf;
    LPDIRECTDRAWPALETTE pPalette;
    LPDIRECT3DTEXTURE2 pTexture;
    D3DTEXTUREHANDLE handle;
    long num;
    unsigned long nFrames;
} TEXTURE;

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

extern DXTEXTURE PictureTextures[32];
extern LPDIRECTDRAWPALETTE DXPalette;

#ifdef __cplusplus
}
#endif

#endif
