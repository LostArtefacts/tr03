#include "standard.h"
#include "specific.h"
#include "winmain.h"

extern "C" {

#include "../game.h"

extern void (*InsertLine)(sint32 x1, sint32 y1, sint32 x2, sint32 y2, sint32 z, int col1, int col2);
extern int GlobalAlpha;
extern int BarsWidth;
extern sint16 DumpWidth;
extern int phd_znear;
extern bool bBlueEffect;

void HWR_EnableZBuffer(bool tWrite, bool tCompare);

void S_DrawHealthBar(int percent)
{
    int oldAlpha;
    int z;
    int w;

    oldAlpha = GlobalAlpha;
    GlobalAlpha = 0xFF000000;
    HWR_EnableZBuffer(true, true);
    bBlueEffect = false;
    w = BarsWidth * percent / 100;
    z = phd_znear + 50;

    InsertLine(6, 7, BarsWidth + 9, 7, z, 0, 0);
    InsertLine(6, 8, BarsWidth + 9, 8, z, 0, 0);
    InsertLine(6, 9, BarsWidth + 9, 9, z, 0, 0);
    InsertLine(6, 10, BarsWidth + 9, 10, z, 0, 0);
    InsertLine(6, 11, BarsWidth + 9, 11, z, 0, 0);
    InsertLine(6, 12, BarsWidth + 9, 12, z, 0, 0);
    InsertLine(6, 13, BarsWidth + 9, 13, z, 0, 0);

    z = phd_znear + 40;
    InsertLine(6, 14, BarsWidth + 11, 14, z, 0x7F7F7F, 0x7F7F7F);
    InsertLine(BarsWidth + 10, 6, BarsWidth + 10, 14, z, 0x7F7F7F, 0x7F7F7F);

    z = phd_znear + 30;
    InsertLine(6, 6, BarsWidth + 10, 6, z, 0xFFFFFF, 0xFFFFFF);
    InsertLine(6, 14, 6, 6, z, 0xFFFFFF, 0xFFFFFF);

    if (w) {
        if (lara.poisoned) {
            w += 8;
            z = phd_znear + 20;
            InsertLine(8, 9, w, 9, z, 0xFFFF00, 0xFFFF00);
            InsertLine(8, 8, w, 8, z, 0x4F4F00, 0x4F4F00);
            InsertLine(8, 10, w, 10, z, 0x7F7F00, 0x7F7F00);
            InsertLine(8, 11, w, 11, z, 0x7F7F00, 0x7F7F00);
            InsertLine(8, 12, w, 12, z, 0x4F4F00, 0x4F4F00);
        } else {
            w += 8;
            z = phd_znear + 20;
            InsertLine(8, 9, w, 9, z, 0xFF0000, 0xFF0000);
            InsertLine(8, 8, w, 8, z, 0x4F0000, 0x4F0000);
            InsertLine(8, 10, w, 10, z, 0x7F0000, 0x7F0000);
            InsertLine(8, 11, w, 11, z, 0x7F0000, 0x7F0000);
            InsertLine(8, 12, w, 12, z, 0x4F0000, 0x4F0000);
        }
    }

    GlobalAlpha = oldAlpha;
}

}
