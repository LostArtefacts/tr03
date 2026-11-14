#include "standard.h"
#include "specific.h"
#include "winmain.h"
#include "texture.h"

extern "C" {

#include "../types.h"
#include "../camera.h"

extern int CurPicTexIndices[5];
extern int OldPicTexIndices[5];
extern float f_zfar;
extern float f_znear;
extern int nLoadedPictures;
extern bool forceFadeDown;
extern bool pictureFading;
extern bool dontFadePicture;
extern bool pictureCrossFaded;
extern DXTEXTURE *TPages[32];
extern int nTPages;
extern sint16 phd_winxmin;
extern sint16 phd_winymin;
extern int phd_winwidth;
extern int phd_winheight;
void HWR_EnablePerspCorrect(bool enable);
void DrawTile(int x, int y, int w, int h, int tpage, int tU, int tV, int tW, int tH, int c0, int c1, int c2, int c3, float z);

void TRDrawPicture(int col, int *indices, float zpos);
void DrawPictureAlpha(int col, int *indices, float zpos);
void FadePictureDown(int steps);
void FreePictureTextures(int *indices);
void ConvertSurfaceToTextures(LPDIRECTDRAWSURFACE3 surf);
void ConvertSurfaceToTextures16Bit(LPDIRECTDRAWSURFACE3 surf);
void HWR_BeginScene(void);
void HWR_EndScene(void);
void HWR_EnableColorKey(bool enable);
void HWR_EnableAlphaBlend(bool enable);
void HWR_EnableColorAddition(bool enable);
void HWR_EnableZBuffer(bool write, bool compare);
void HWR_GetAllTextureHandles(void);
void HWR_SetCurrentTexture(DXTEXTURE *tex);
void DXUpdateFrame(bool runMessageLoop, LPRECT rect);
bool TIME_Init(void);
void DXTextureSetGreyScale(bool set);
void DXTextureCleanup(int nTexture, DXTEXTURE *pTextures);
void DXFreeTPages(void);
void DXCreateMaxTPages(int create);
const char *GetFullPath(const char *name);
void DXBitMask2ShiftCnt(unsigned long mask, unsigned char *shift, unsigned char *cnt);
int DXTextureAdd(int w, int h, unsigned char *src, DXTEXTURE *list, int bpp, unsigned long flags);
extern float one;
extern HRESULT (*SetRenderState)(D3DRENDERSTATETYPE dwRenderStateType, DWORD dwRenderState);
extern HRESULT (*DrawPrimitive)(D3DPRIMITIVETYPE dptPrimitiveType, D3DVERTEXTYPE dvtVertexType, LPVOID lpvVertices, DWORD dwVertexCount, DWORD dwFlags);

void SetPictureToFade(int fade)
{
    pictureFading = fade;
}

void ForceFadeDown(int fade)
{
    forceFadeDown = fade;
}

void DrawPictureAlpha(int col, int *indices, float zpos)
{
    int x[4];
    int y[4];
    int i;

    static int screenX[4] = { 0, 256, 512, 640 };
    int width;
    static int screenY[3] = { 0, 256, 480 };
    unsigned int c;

    width = phd_winwidth;
    c = col + 1;
    c = (0xff000000 * c) | 0xffffff;
    for (i = 0; i < 3; i++) {
        x[i] = phd_winxmin + width * screenX[i] / 640;
        y[i] = phd_winymin + phd_winheight * screenY[i] / 480;
    }
    x[3] = phd_winxmin + width * screenX[3] / 640;
    HWR_EnableAlphaBlend(true);
    HWR_EnablePerspCorrect(false);
    HWR_EnableColorAddition(false);
    DrawTile(x[0], y[0], x[1] - x[0], y[1] - y[0], indices[0], 0, 0, 256, 256, c, c, c, c, zpos);
    DrawTile(x[1], y[0], x[2] - x[1], y[1] - y[0], indices[1], 0, 0, 256, 256, c, c, c, c, zpos);
    DrawTile(x[2], y[0], x[3] - x[2], y[1] - y[0], indices[2], 0, 0, 128, 256, c, c, c, c, zpos);
    DrawTile(x[0], y[1], x[1] - x[0], y[2] - y[1], indices[3], 0, 0, 256, 224, c, c, c, c, zpos);
    DrawTile(x[1], y[1], x[2] - x[1], y[2] - y[1], indices[4], 0, 0, 256, 224, c, c, c, c, zpos);
    DrawTile(x[2], y[1], x[3] - x[2], y[2] - y[1], indices[2], 128, 0, 128, 224, c, c, c, c, zpos);
    HWR_EnablePerspCorrect(true);
}

void CrossFadePicture(void)
{
    int i;
    int j;

    for (j = 255, i = 0; i < 256; i += 8, j -= 8) {
        HWR_BeginScene();
        DrawPictureAlpha(i, CurPicTexIndices, f_znear);
        DrawPictureAlpha(j, OldPicTexIndices, f_zfar);
        HWR_EndScene();
        DXUpdateFrame(true, 0);
    }
    FreePictureTextures(CurPicTexIndices);
    CurPicTexIndices[0] = OldPicTexIndices[0];
    CurPicTexIndices[1] = OldPicTexIndices[1];
    CurPicTexIndices[2] = OldPicTexIndices[2];
    CurPicTexIndices[3] = OldPicTexIndices[3];
    CurPicTexIndices[4] = OldPicTexIndices[4];
    HWR_EnableColorKey(false);
    HWR_EnableAlphaBlend(false);
    HWR_EnableColorAddition(false);
    if (dontFadePicture)
        pictureFading = false;
    else
        pictureFading = true;
    HWR_BeginScene();
    TRDrawPicture(0, CurPicTexIndices, f_znear);
    HWR_EndScene();
    DXUpdateFrame(true, 0);
    TIME_Init();
}

bool LoadPicture(const char *name, LPDIRECTDRAWSURFACE3 surf, int a)
{
    HANDLE img;
    BITMAP bitmap;
    HDC cdc;
    HDC hdc;

    name = GetFullPath(name);
    img = LoadImage(0, name, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
    if (!img)
        return false;
    cdc = CreateCompatibleDC(0);
    SelectObject(cdc, img);
    GetObject(img, sizeof(BITMAP), &bitmap);
    surf->GetDC(&hdc);
    BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, cdc, 0, 0, SRCCOPY);
    surf->ReleaseDC(hdc);
    DeleteDC(cdc);
    ConvertSurfaceToTextures(surf);
    HWR_GetAllTextureHandles();
    nLoadedPictures++;
    return true;
}

void FreePictureTextures(int *indices)
{
    int i;

    DXTextureCleanup(indices[0], PictureTextures);
    DXTextureCleanup(indices[1], PictureTextures);
    DXTextureCleanup(indices[2], PictureTextures);
    DXTextureCleanup(indices[3], PictureTextures);
    DXTextureCleanup(indices[4], PictureTextures);
    if (App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].Texture[App.DXConfigPtr->D3DTF].bPalette) {
        DXFreeTPages();
        DXCreateMaxTPages(0);
    }
    HWR_GetAllTextureHandles();
    for (i = 0; i < nTPages; i++)
        HWR_SetCurrentTexture(TPages[i]);
}

void MemBlt(char *dest, int x, int y, int w, int h, int sz, char *source, int x2, int y2, DDSURFACEDESC desc)
{
    unsigned long stride;

    stride = desc.ddpfPixelFormat.dwRGBBitCount >> 3;
    dest += stride * (x + y * sz);
    source += y2 * desc.lPitch + x2 * (desc.ddpfPixelFormat.dwRGBBitCount >> 3);
    while (h--) {
        memcpy(dest, source, w * (desc.ddpfPixelFormat.dwRGBBitCount >> 3));
        dest += sz * (desc.ddpfPixelFormat.dwRGBBitCount >> 3);
        source += desc.lPitch;
    }
}

void ConvertSurfaceToTextures(LPDIRECTDRAWSURFACE3 surf)
{
    if (App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].Texture[App.DXConfigPtr->D3DTF].bPalette) {
        DXFreeTPages();
        DXCreateMaxTPages(1);
    }
    ConvertSurfaceToTextures16Bit(surf);
}

void ConvertSurfaceToTextures16Bit(LPDIRECTDRAWSURFACE3 surf)
{
    DDSURFACEDESC desc;
    DDSURFACEDESC desc2;
    int *pIndices;
    char *source;
    char *dest;
    long bitcnt;
    unsigned char rshift;
    unsigned char gshift;
    unsigned char bshift;
    unsigned char rcount;
    unsigned char gcount;
    unsigned char bcount;

    pIndices = CurPicTexIndices;
    if (nLoadedPictures)
        pIndices = OldPicTexIndices;
    memset(&desc, 0, sizeof(DDSURFACEDESC));
    desc.dwSize = sizeof(DDSURFACEDESC);
    surf->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0);
    source = (char *)desc.lpSurface;
    dest = (char *)malloc((256 * 256) * (desc.ddpfPixelFormat.dwRGBBitCount >> 3));
    DXBitMask2ShiftCnt(desc.ddpfPixelFormat.dwRBitMask, &rshift, &rcount);
    DXBitMask2ShiftCnt(desc.ddpfPixelFormat.dwGBitMask, &gshift, &gcount);
    DXBitMask2ShiftCnt(desc.ddpfPixelFormat.dwBBitMask, &bshift, &bcount);
    bitcnt = bcount + 10 * (gcount + 10 * rcount);
    if (desc.ddpfPixelFormat.dwRGBBitCount == 32)
        bitcnt = 8888;
    memcpy(&desc2, &desc, sizeof(DDSURFACEDESC));
    MemBlt(dest, 0, 0, 256, 256, 256, source, 0, 0, desc2);
    surf->Unlock(0);
    pIndices[0] = DXTextureAdd(256, 256, (unsigned char *)dest, PictureTextures, bitcnt, 16);
    memset(&desc, 0, sizeof(DDSURFACEDESC));
    desc.dwSize = sizeof(DDSURFACEDESC);
    surf->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0);
    memcpy(&desc2, &desc, sizeof(DDSURFACEDESC));
    source = (char *)desc.lpSurface;
    MemBlt(dest, 0, 0, 256, 256, 256, source, 256, 0, desc2);
    surf->Unlock(0);
    pIndices[1] = DXTextureAdd(256, 256, (unsigned char *)dest, PictureTextures, bitcnt, 16);
    memset(&desc, 0, sizeof(DDSURFACEDESC));
    desc.dwSize = sizeof(DDSURFACEDESC);
    surf->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0);
    memcpy(&desc2, &desc, sizeof(DDSURFACEDESC));
    MemBlt(dest, 0, 0, 128, 256, 256, source, 512, 0, desc2);
    memcpy(&desc2, &desc, sizeof(DDSURFACEDESC));
    MemBlt(dest, 128, 0, 128, 224, 256, source, 512, 256, desc2);
    surf->Unlock(0);
    pIndices[2] = DXTextureAdd(256, 256, (unsigned char *)dest, PictureTextures, bitcnt, 16);
    memset(&desc, 0, sizeof(DDSURFACEDESC));
    desc.dwSize = sizeof(DDSURFACEDESC);
    surf->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0);
    memcpy(&desc2, &desc, sizeof(DDSURFACEDESC));
    source = (char *)desc.lpSurface;
    MemBlt(dest, 0, 0, 256, 224, 256, source, 0, 256, desc2);
    surf->Unlock(0);
    pIndices[3] = DXTextureAdd(256, 256, (unsigned char *)dest, PictureTextures, bitcnt, 16);
    memset(&desc, 0, sizeof(DDSURFACEDESC));
    desc.dwSize = sizeof(DDSURFACEDESC);
    surf->Lock(0, &desc, DDLOCK_WAIT | DDLOCK_NOSYSLOCK, 0);
    memcpy(&desc2, &desc, sizeof(DDSURFACEDESC));
    source = (char *)desc.lpSurface;
    MemBlt(dest, 0, 0, 256, 224, 256, source, 256, 256, desc2);
    surf->Unlock(0);
    pIndices[4] = DXTextureAdd(256, 256, (unsigned char *)dest, PictureTextures, bitcnt, 16);
    free(dest);
}

void DrawTile(int x, int y, int w, int h, int tpage, int tU, int tV, int tW, int tH, int c0, int c1, int c2, int c3, float z)
{
    D3DTLVERTEX v[4];
    float fX1;
    float fY1;
    float fX2;
    float fY2;
    float fU1;
    float fV1;
    float fU2;
    float fV2;
    float fZ;
    float fRHW;

    fX1 = float(x);
    fY1 = float(y);
    fX2 = float(x + w);
    fY2 = float(y + h);
    fU1 = float(tU) * (1.0f / 256.0f);
    fV1 = float(tV) * (1.0f / 256.0f);
    fU2 = float(tU + tW) * (1.0f / 256.0f);
    fV2 = float(tV + tH) * (1.0f / 256.0f);
    fZ = 0.995f;
    fRHW = one / z;
    v[0].sx = fX1;
    v[0].sy = fY1;
    v[0].sz = fZ;
    v[0].rhw = fRHW;
    v[0].color = c0;
    v[0].specular = 0;
    v[0].tu = fU1;
    v[0].tv = fV1;
    v[1].sx = fX2;
    v[1].sy = fY1;
    v[1].sz = fZ;
    v[1].rhw = fRHW;
    v[1].color = c1;
    v[1].specular = 0;
    v[1].tu = fU2;
    v[1].tv = fV1;
    v[3].sx = fX1;
    v[3].sy = fY2;
    v[3].sz = fZ;
    v[3].rhw = fRHW;
    v[3].color = c2;
    v[3].specular = 0;
    v[3].tu = fU1;
    v[3].tv = fV2;
    v[2].sx = fX2;
    v[2].sy = fY2;
    v[2].sz = fZ;
    v[2].rhw = fRHW;
    v[2].color = c3;
    v[2].specular = 0;
    v[2].tu = fU2;
    v[2].tv = fV2;
    HWR_SetCurrentTexture(&PictureTextures[tpage]);
    HWR_EnableColorKey(false);
    HWR_EnableColorAddition(false);
    HWR_EnableZBuffer(false, false);
    SetRenderState(D3DRENDERSTATE_TEXTUREMAG, D3DFILTER_NEAREST);
    SetRenderState(D3DRENDERSTATE_TEXTUREMIN, D3DFILTER_NEAREST);
    DrawPrimitive(D3DPT_TRIANGLEFAN, D3DVT_TLVERTEX, v, 4, D3DDP_DONOTCLIP | D3DDP_DONOTUPDATEEXTENTS);
    SetRenderState(D3DRENDERSTATE_TEXTUREMAG, HWConfig.nFilter);
    SetRenderState(D3DRENDERSTATE_TEXTUREMIN, HWConfig.nFilter);
}

void TRDrawPicture(int col, int *indices, float zpos)
{
    int x[4];
    int y[4];
    int i;
    static int screenX[4] = { 0, 256, 512, 640 };
    static int screenY[3] = { 0, 256, 480 };
    unsigned long c;

    c = 255 - col;
    col = (c << 8) | (c << 16) | c | 0xff000000;
    for (i = 0; i < 3; i++) {
        x[i] = phd_winxmin + phd_winwidth * screenX[i] / 640;
        y[i] = phd_winymin + phd_winheight * screenY[i] / 480;
    }
    x[3] = phd_winxmin + phd_winwidth * screenX[3] / 640;
    HWR_EnableAlphaBlend(false);
    HWR_EnablePerspCorrect(false);
    HWR_EnableColorAddition(false);
    DrawTile(x[0], y[0], x[1] - x[0], y[1] - y[0], indices[0], 0, 0, 256, 256, col, col, col, col, zpos);
    DrawTile(x[1], y[0], x[2] - x[1], y[1] - y[0], indices[1], 0, 0, 256, 256, col, col, col, col, zpos);
    DrawTile(x[2], y[0], x[3] - x[2], y[1] - y[0], indices[2], 0, 0, 128, 256, col, col, col, col, zpos);
    DrawTile(x[0], y[1], x[1] - x[0], y[2] - y[1], indices[3], 0, 0, 256, 224, col, col, col, col, zpos);
    DrawTile(x[1], y[1], x[2] - x[1], y[2] - y[1], indices[4], 0, 0, 256, 224, col, col, col, col, zpos);
    DrawTile(x[2], y[1], x[3] - x[2], y[2] - y[1], indices[2], 128, 0, 128, 224, col, col, col, col, zpos);
    HWR_EnablePerspCorrect(true);
}

void FadePictureUp(int steps)
{
    int i;

    pictureCrossFaded = false;
    if (nLoadedPictures > 1) {
        CrossFadePicture();
        pictureCrossFaded = true;
    } else {
        for (i = 0; i < steps; i++) {
            HWR_BeginScene();
            TRDrawPicture(255 - i * (256 / steps), CurPicTexIndices, f_znear);
            HWR_EndScene();
            DXUpdateFrame(true, 0);
        }
    }
    HWR_BeginScene();
    TRDrawPicture(0, CurPicTexIndices, f_znear);
    HWR_EndScene();
    DXUpdateFrame(true, 0);
    TIME_Init();
}

void FadePictureDown(int steps)
{
    int i;

    if (!forceFadeDown) {
        if (!App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].Texture[App.DXConfigPtr->D3DTF].bPalette && App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].bHardware && App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].bAlpha) {
            HWR_BeginScene();
            TRDrawPicture(0, CurPicTexIndices, f_znear);
            HWR_EndScene();
            DXUpdateFrame(true, 0);
            return;
        }
    }
    for (i = steps; i > 0; i--) {
        HWR_BeginScene();
        TRDrawPicture(256 - i * (256 / steps), CurPicTexIndices, f_znear);
        HWR_EndScene();
        DXUpdateFrame(true, 0);
    }
    HWR_BeginScene();
    TRDrawPicture(255, CurPicTexIndices, f_znear);
    HWR_EndScene();
    DXUpdateFrame(true, 0);
    forceFadeDown = false;
    nLoadedPictures = 0;
    FreePictureTextures(CurPicTexIndices);
    TIME_Init();
}

void CreateMonoScreen(void)
{
    int w;
    int h;
    int i;

    DXTextureSetGreyScale(true);
    w = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].w;
    h = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].h;
    if (w == 640 && h == 480) {
        ConvertSurfaceToTextures(App.lpFrontBuffer);
    } else {
        App.lpPictureBuffer->Blt(0, App.lpFrontBuffer, 0, DDBLT_WAIT, 0);
        ConvertSurfaceToTextures(App.lpPictureBuffer);
    }
    HWR_GetAllTextureHandles();
    for (i = 0; i < nTPages; i++)
        HWR_SetCurrentTexture(TPages[i]);
    DXTextureSetGreyScale(false);
    nLoadedPictures++;
    TIME_Init();
}

void S_FadeToBlack(void)
{
    int w;
    int h;
    int i;

    w = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].w;
    h = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].h;
    if (w == 640 && h == 480) {
        ConvertSurfaceToTextures(App.lpFrontBuffer);
    } else {
        App.lpPictureBuffer->Blt(0, App.lpFrontBuffer, 0, DDBLT_WAIT, 0);
        ConvertSurfaceToTextures(App.lpPictureBuffer);
    }
    HWR_GetAllTextureHandles();
    for (i = 0; i < nTPages; i++)
        HWR_SetCurrentTexture(TPages[i]);
    nLoadedPictures++;
    TIME_Init();
}

void DrawMonoScreen(int r, int g, int b)
{
    TRDrawPicture(0, CurPicTexIndices, f_zfar);
}

void RemoveMonoScreen(int fade)
{
    if (fade) {
        nLoadedPictures = 1;
        FadePictureDown(32);
    } else {
        FreePictureTextures(CurPicTexIndices);
        nLoadedPictures = 0;
    }
}

void S_FadePicture(void)
{
    static int pos;
    static int max = 64;
    int nframes;

    if (!pictureFading || dontFadePicture)
        return;
    nframes = camera.number_frames;
    if (nframes > 10)
        nframes = 2;
    DrawPictureAlpha(256 - 256 / max * (max - pos), CurPicTexIndices, f_znear);
    pos += nframes;
    if (pos >= max) {
        pos = 0;
        pictureFading = false;
        nLoadedPictures = 0;
        FreePictureTextures(CurPicTexIndices);
    }
}

}
