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

}
