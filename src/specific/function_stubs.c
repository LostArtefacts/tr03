#include "../game.h"

extern unsigned short atanTab[];
extern sint32 atanOctantTab[];

unsigned long mGetAngle(sint32 x1, sint32 y1, sint32 x2, sint32 y2)
{
    sint32 x;
    sint32 y;
    sint32 octant;
    sint32 n;
    sint32 angle;

    x = x2 - x1;
    y = y2 - y1;
    if (x == 0 && y == 0)
        return 0;
    octant = 0;
    if (x < 0) {
        octant += 4;
        x = -x;
    }
    if (y < 0) {
        octant += 2;
        y = -y;
    }
    if (y > x) {
        octant++;
        n = x;
        x = y;
        y = n;
    }
    while ((sint16)y != y) {
        y >>= 1;
        x >>= 1;
    }
    n = atanTab[(y << 11) / x];
    if ((n += atanOctantTab[octant]) < 0)
        angle = -n;
    else
        angle = n;
    return -angle & 0xffff;
}
