#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

void DXBitMask2ShiftCnt(unsigned long mask, unsigned char *shift, unsigned char *cnt)
{
    unsigned long m;
    unsigned char n;

    for (n = 0, m = mask; !(m & 1); n++, m >>= 1)
        ;
    *shift = n;
    for (n = 0; m & 1; n++, m >>= 1)
        ;
    *cnt = n;
}

}
