#include "standard.h"
#include "global.h"
#include "winmain.h"

extern "C" {

extern float ZTable[40960];
extern float f_persp;
extern float f_oneopersp;

void InitZTable(void)
{
    int z;
    int n;

    z = 0;
    for (n = 0; n < 20480; n++) {
        ZTable[n * 2 + 0] = (float)(f_persp / (float)(z << 14));
        ZTable[n * 2 + 1] = (float)(f_persp / (float)(z << 14)) * f_oneopersp;
        z++;
    }
}

}
