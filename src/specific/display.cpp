#include "standard.h"
#include "specific.h"

extern "C" {

extern double screen_sizer;
extern double game_sizer;
extern int VidSizeLocked;
extern int Inventory_Mode;
extern int fade_value;
extern int fade_limit;
extern int fade_adder;

void setup_screen_size(void);

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
