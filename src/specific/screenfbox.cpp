// cflags: /Gi
#include "specific.h"

extern "C" {

extern int phd_znear;
extern sint16 phd_winxmin;
extern sint16 phd_winymin;
extern void (*InsertTransQuad)(int x, int y, int w, int h, int z);

void S_DrawScreenFBox(int sx, int sy, int z, int w, int h, int col, unsigned short *grdptr, unsigned short flags)
{
    InsertTransQuad(sx + phd_winxmin, sy + phd_winymin, w + 1, h + 1, phd_znear + 0x50000);
}

}
