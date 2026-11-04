#include "specific.h"

extern "C" sint32 rand_1;
extern "C" sint32 rand_2;

sint32 GetRandomControl(void)
{
    rand_1 = rand_1 * 0x41c64e6d + 0x3039;
    return (rand_1 >> 10) & 0x7fff;
}
