#include "game.h"
#include <string.h>

extern int IM_rate, IM_frac;
extern sint32 *IMptr;
extern sint32 IMstack[];

void InitInterpolate2(int frac, int rate)
{
    IM_frac = frac;
    IM_rate = rate;
    IMptr = &IMstack[12 * 32];
    memcpy(IMptr, phd_mxptr, 12 * sizeof(sint32));
}
