#include "standard.h"
#include "specific.h"

extern "C" {

extern double screen_sizer;
extern double game_sizer;
extern int VidSizeLocked;
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

}
