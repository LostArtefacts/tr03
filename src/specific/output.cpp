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

typedef struct phdtexturestruct {
    unsigned short drawtype;
    unsigned short tpage;
    unsigned short u1;
    unsigned short v1;
    unsigned short u2;
    unsigned short v2;
    unsigned short u3;
    unsigned short v3;
    unsigned short u4;
    unsigned short v4;
} PHDTEXTURESTRUCT;

extern PHDTEXTURESTRUCT phdtextinfo[];
extern sint16 *aranges;
extern int light_level[4];
extern sint16 GF_SunsetEnabled;
extern int SunsetTimer;

sint32 __fastcall phd_sin(sint32 angle);

extern unsigned char game_palette[768];
extern unsigned char water_palette[768];
extern int wet;

void FadeToPal(int nFrames, unsigned char *pPal);
void TempVideoRemove(void);
void HWR_InitState(void);
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

void S_InitialiseScreen(int type)
{
    if (type >= 0) {
        if (type != 0)
            TempVideoRemove();
        FadeToPal(30, game_palette);
    } else {
        FadeToPal(0, game_palette);
    }
    if (App.nRenderMode != 0)
        HWR_InitState();
}

void AnimateTextures(int n)
{
    static int comp;
    PHDTEXTURESTRUCT tex;
    sint16 *range;
    int nRanges;
    int nFrames;
    int i;

    for (comp += n; comp > 5; comp -= 5) {
        range = aranges;
        nRanges = *range++;
        for (i = 0; i < nRanges; i++) {
            nFrames = *range++;
            tex = phdtextinfo[*range];
            while (nFrames > 0) {
                phdtextinfo[range[0]] = phdtextinfo[range[1]];
                range++;
                nFrames--;
            }
            phdtextinfo[*range] = tex;
            range++;
        }
    }
}

void S_SetupBelowWater(int underwater)
{
    if (underwater ^ wet) {
        if (underwater)
            FadeToPal(1, water_palette);
        else
            FadeToPal(1, game_palette);
        wet = underwater;
    }
    water_effect = !underwater;
    shade_effect = 1;
    bBlueEffect = true;
}

void S_SetupAboveWater(int underwater)
{
    water_effect = underwater;
    shade_effect = 0;
    bBlueEffect = underwater;
}

static int pulse;

void S_AnimateTextures(int n)
{
    pulse = (n + pulse) & 31;
    light_level[1] = GetRandomDraw() & 31;
    light_level[2] = ((phd_sin((pulse << 16) / 32) + 0x4000) * 31) >> 15;
    if (GF_SunsetEnabled) {
        SunsetTimer += n;
        if (SunsetTimer < 72000)
            light_level[3] = 31 * SunsetTimer / 72000;
        else
            light_level[3] = 31;
    }
    AnimateTextures(n);
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
