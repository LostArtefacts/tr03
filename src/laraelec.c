#include "specific/standard.h"
#include "game.h"

extern sint16 electricity_points[32][6];

void UpdateElectricityPoints(void)
{
    sint32 lp, random;
    sint32 tx, ty, tz, xv, yv, zv;
    sint16 *leccy;

    leccy = &electricity_points[0][0];
    for (lp = 0; lp < 32; lp++) {
        random = GetRandomDraw();
        tx = leccy[0];
        ty = leccy[1];
        tz = leccy[2];
        xv = leccy[3];
        yv = leccy[4];
        zv = leccy[5];
        if (((tx > 256 || tx < -256) && (ty > 256 || ty < -256) && (tz > 256 || tz < -256)) ||
            (tx > 384 || tx < -128 || ty > 384 || ty < -128 || tz > 384 || tz < -128))
            xv = tx = yv = ty = zv = tz = 0;
        if (!xv) {
            if (random & 1)
                xv = -(GetRandomDraw() & 3) - 1;
            else
                xv = (GetRandomDraw() & 3) + 1;
        } else if (xv < 0)
            xv -= 2;
        else
            xv += 2;
        if (!yv) {
            if (random & 2)
                yv = -(GetRandomDraw() & 3) - 1;
            else
                yv = (GetRandomDraw() & 3) + 1;
        } else if (yv < 0)
            yv -= 2;
        else
            yv += 2;
        if (!zv) {
            if (random & 4)
                zv = -(GetRandomDraw() & 3) - 1;
            else
                zv = (GetRandomDraw() & 3) + 1;
        } else if (zv < 0)
            zv--;
        else
            zv++;
        tx += xv;
        ty += yv;
        tz += zv;
        *leccy++ = tx;
        *leccy++ = ty;
        *leccy++ = tz;
        *leccy++ = xv;
        *leccy++ = yv;
        *leccy++ = zv;
    }
}
