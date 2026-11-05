#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

extern int water_effect;
extern int shade_effect;
extern bool bBlueEffect;
extern int framedump;
extern RECT phd_WindowRect;

void DXSaveScreen(LPDIRECTDRAWSURFACE3 lpSurface);
void DXUpdateFrame(bool runMessageLoop, LPRECT rect);
void DXClearBuffers(DWORD dwClrFlags, DWORD dwColour);
void ScreenClear(bool tWindow);

int S_DumpCine(void)
{
    static int nf = 0;

    if (framedump) {
        nf++;
        DXSaveScreen(App.lpFrontBuffer);
        return 1;
    }
    return 0;
}

void S_ClearScreen(void)
{
    ScreenClear(false);
}

void S_SetupAboveWater(int underwater)
{
    water_effect = underwater;
    shade_effect = 0;
    bBlueEffect = underwater;
}

void ScreenPartialDump(void)
{
    DXUpdateFrame(true, &phd_WindowRect);
}

void ScreenClear(bool tWindow)
{
    DXClearBuffers(2, 0);
}

}
