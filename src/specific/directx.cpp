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

bool DXCreateDirect3D(LPDIRECTDRAW2 lpDD2, LPDIRECT3D2 *lpD3D2)
{
    int RetVal;

    RetVal = lpDD2->QueryInterface(IID_IDirect3D2, (LPVOID *)lpD3D2);
    if (RetVal != DD_OK)
        return false;
    return true;
}

}
