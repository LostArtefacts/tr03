#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

extern int fmv_playing;
extern bool GtWindowClosed;
extern void *MovieContext;
extern void *FmvContext;
extern void *FmvSoundContext;

int Player_StopTimer(void *ctx);
int Player_ShutDownSound(void **ctx);
int Player_ShutDownVideo(void **ctx);
int Player_ShutDownMovie(void **ctx);
int Player_ShutDownSoundSystem(void);
int Player_ReturnPlaybackMode(void);
void S_CDStop(void);
void WinFreeDX(bool free_dd);
bool WinDXInit(DEVICEINFO *DeviceInfo, DXCONFIG *DXConfig, bool createNew);
void WinPlayFMV(const char *name, bool play);
const char *GetFullPath(const char *name);
int Player_PassInDirectDrawObject(LPDIRECTDRAW2 dd);
int Player_InitMovie(void **ctx, int a, int b, const char *name, int flags);
int Movie_GetFormat(void *ctx);
int Movie_GetXSize(void *ctx);
int Movie_GetYSize(void *ctx);
int Player_InitVideo(void **ctx, void *movie, int xsize, int ysize, int xoff, int yoff, int a, int b, int w, int h, int c, int d, int e);
int Player_InitPlaybackMode(HWND hwnd, void *ctx, int a, int b);
int Player_BlankScreen(RECT r);
int Player_InitSoundSystem(HWND hwnd);
int Player_GetDSErrorCode(void);
int Movie_GetSoundPrecision(void *ctx);
int Movie_GetSoundChannels(void *ctx);
int Movie_GetSoundRate(void *ctx);
int Player_InitSound(void **ctx, int size, int a, int b, int c, int channels, int rate, int precision, int d);
int Player_SetVolume(void *ctx, int volume);
int Movie_SetSyncAdjust(void *movie, void *sound, int adjust);
int Player_InitMoviePlayback(void *movie, void *video, void *sound);
int Player_StartTimer(void *ctx);
int Movie_GetTotalFrames(void *ctx);
int Movie_GetCurrentFrame(void *ctx);
int Player_PlayFrame(void *movie, void *video, void *sound, int a, RECT *r, int b, int c, int d);
int S_UpdateInput(void);
extern int acm_volume;
extern int input;

void WinPlayFMV(const char *name, bool play)
{
    RECT r;
    int lp;

    r.left = 0;
    r.top = 0;
    r.right = 640;
    r.bottom = 480;
    if (Player_PassInDirectDrawObject(App.lpDD) || Player_InitMovie(&MovieContext, 0, 0, name, 0x200000) ||
        Movie_GetFormat(MovieContext) != 130)
        return;
    if (Player_InitVideo(&FmvContext, MovieContext, Movie_GetXSize(MovieContext), Movie_GetYSize(MovieContext), 0,
            240 - Movie_GetYSize(MovieContext), 0, 0, 640, 480, 0, 1, 13))
        return;
    if (play && Player_InitPlaybackMode(App.WindowHandle, FmvContext, 1, 0))
        return;
    Player_BlankScreen(r);
    if (Player_InitSoundSystem(App.WindowHandle) || Player_GetDSErrorCode() < 0)
        return;
    if (Player_InitSound(&FmvSoundContext, 0x4000, Movie_GetSoundPrecision(MovieContext) == 4 ? 4 : 1,
            Movie_GetSoundPrecision(MovieContext) != 4, 4096, Movie_GetSoundChannels(MovieContext),
            Movie_GetSoundRate(MovieContext), Movie_GetSoundPrecision(MovieContext), 2))
        return;
    Player_SetVolume(FmvSoundContext, acm_volume);
    Movie_SetSyncAdjust(MovieContext, FmvSoundContext, 4);
    lp = Player_InitMoviePlayback(MovieContext, FmvContext, FmvSoundContext);
    if (lp)
        return;
    S_UpdateInput();
    Player_StartTimer(MovieContext);
    Player_BlankScreen(r);
    S_UpdateInput();
    while (Movie_GetCurrentFrame(MovieContext) < Movie_GetTotalFrames(MovieContext) && !lp)
    {
        lp = Player_PlayFrame(MovieContext, FmvContext, FmvSoundContext, 0, &r, 0, 0, 0);
        if (S_UpdateInput())
            break;
        if (input & 0x100)
            break;
    }
}


void WinStopFMV(bool play)
{
    Player_StopTimer(MovieContext);
    Player_ShutDownSound(&FmvSoundContext);
    Player_ShutDownVideo(&FmvContext);
    Player_ShutDownMovie(&MovieContext);
    Player_ShutDownSoundSystem();
    if (play)
        Player_ReturnPlaybackMode();
}

int FMV_Play(char *name)
{
    fmv_playing = 1;
    S_CDStop();
    ShowCursor(0);
    WinFreeDX(false);
    name = (char *)GetFullPath(name);
    WinPlayFMV(name, true);
    WinStopFMV(true);
    fmv_playing = 0;
    if (!GtWindowClosed)
        WinDXInit(&App.DeviceInfo, &App.DXConfig, false);
    ShowCursor(1);
    return GtWindowClosed;
}

int FMV_PlayIntro(char *name1, char *name2)
{
    fmv_playing = 1;
    ShowCursor(0);
    WinFreeDX(false);
    name1 = (char *)GetFullPath(name1);
    WinPlayFMV(name1, true);
    WinStopFMV(true);
    name2 = (char *)GetFullPath(name2);
    WinPlayFMV(name2, true);
    WinStopFMV(true);
    fmv_playing = 0;
    if (!GtWindowClosed)
        WinDXInit(&App.DeviceInfo, &App.DXConfig, false);
    ShowCursor(1);
    return GtWindowClosed;
}

}
