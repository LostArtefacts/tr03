#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

extern int water_effect;
extern int shade_effect;
extern bool bBlueEffect;
extern int framedump;
extern RECT phd_WindowRect;
extern sint16 phd_winxmin;
extern sint16 phd_winymin;
extern int phd_zfar;
extern unsigned short inv_colours[17];
extern void (*InsertFlatRect)(int x1, int y1, int x2, int y2, int zdepth, int col);

int Sync(void);
void ScreenPartialDump(void);

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

int S_DumpScreen(void)
{
    int nFrames;

    if (!framedump) {
        nFrames = Sync();
        while (nFrames < 2) {
            while (!Sync())
                ;
            nFrames++;
        }
    } else {
        nFrames = 2;
    }
    ScreenPartialDump();
    return nFrames;
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
