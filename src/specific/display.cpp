#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

extern double screen_sizer;
extern double game_sizer;
extern int VidSizeLocked;
extern int Inventory_Mode;
extern int fade_value;
extern int fade_limit;
extern int fade_adder;

void setup_screen_size(void);

extern int farz;
extern int BarsWidth;
extern sint16 DumpX;
extern sint16 DumpY;
extern sint16 DumpWidth;
extern sint16 DumpHeight;
extern bool GtFullScreenClearNeeded;
void phd_InitWindow(int x, int y, int w, int h, int znear, int zfar, int fov, int sw, int sh);

void IncreaseScreenSize(void)
{
    if (screen_sizer != 1.0) {
        screen_sizer += 0.08;
        if (screen_sizer > 1.0)
            screen_sizer = 1.0;
        game_sizer = screen_sizer;
        setup_screen_size();
    }
}

void DecreaseScreenSize(void)
{
    if (screen_sizer != 0.44) {
        screen_sizer -= 0.08;
        if (screen_sizer < 0.44)
            screen_sizer = 0.44;
        game_sizer = screen_sizer;
        setup_screen_size();
    }
}

void setup_screen_size(void)
{
    int w, h, sw, sh, x, y;

    w = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].w;
    sw = (int)(w * screen_sizer);
    h = App.DeviceInfoPtr->DDInfo[App.DXConfigPtr->nDD].D3DInfo[App.DXConfigPtr->nD3D].DisplayMode[App.DXConfigPtr->nVMode].h;
    sh = (int)(h * screen_sizer);
    if (sw > w)
        sw = w;
    if (sh > h)
        sh = h;
    x = (w - sw) / 2;
    y = (h - sh) / 2;
    phd_InitWindow(x, y, sw, sh, 20, farz, 80, w, h);
    DumpX = x;
    DumpY = y;
    DumpWidth = sw;
    DumpHeight = sh;
    GtFullScreenClearNeeded = true;
    BarsWidth = sw / 5;
    if (BarsWidth > 100)
        BarsWidth = 100;
}

void TempVideoAdjust(int hires, double sizer)
{
    VidSizeLocked = 1;
    if (sizer != screen_sizer) {
        screen_sizer = sizer;
        setup_screen_size();
    }
}

void TempVideoRemove(void)
{
    VidSizeLocked = 0;
    if (screen_sizer != game_sizer) {
        screen_sizer = game_sizer;
        setup_screen_size();
    }
}

void S_CopyScreenToBuffer(void);

void S_FadeInInventory(int fade)
{
    if (Inventory_Mode != 1)
        S_CopyScreenToBuffer();
    if (fade) {
        fade_value = 16 << 16;
        fade_limit = 24 << 16;
        fade_adder = 32768;
    }
}

void S_FadeOutInventory(int fade)
{
    if (fade) {
        fade_value = 24 << 16;
        fade_limit = 16 << 16;
        fade_adder = -32768;
    }
}

}

#ifdef TR3_EXE
extern "C" void S_CopyScreenToBuffer(void)
{
}
#endif
