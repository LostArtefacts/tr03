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

void SetPictureToFade(int fade)
{
    pictureFading = fade;
}

void ForceFadeDown(int fade)
{
    forceFadeDown = fade;
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

void ConvertSurfaceToTextures(LPDIRECTDRAWSURFACE3 surf)
{
    if (App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].Texture[App.DXConfigPtr->D3DTF].bPalette) {
        DXFreeTPages();
        DXCreateMaxTPages(1);
    }
    ConvertSurfaceToTextures16Bit(surf);
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

}
