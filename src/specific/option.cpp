#include "specific.h"

extern "C" int phd_winheight;
extern "C" int phd_winwidth;

int GetRenderHeight(void)
{
    return phd_winheight;
}

int GetRenderWidth(void)
{
    return phd_winwidth;
}
