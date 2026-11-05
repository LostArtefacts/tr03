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
