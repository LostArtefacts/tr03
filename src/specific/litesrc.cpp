#include "standard.h"
#include "specific.h"

extern "C" {

#include "../game.h"

extern int smcr;
extern int smcg;
extern int smcb;

void S_CalculateStaticLight(sint16 adder)
{
    smcr = (adder & 0x1f) << 3;
    smcg = (adder & 0x3e0) >> 2;
    smcb = (adder & 0x7c00) >> 7;
}

sint32 __fastcall phd_sqrt(sint32 n);

void S_CalculateStaticMeshLight(int x, int y, int z, int shade)
{
    DYNAMIC *dl;
    int dx;
    int dy;
    int dz;
    int dist;
    int falloff;
    int i;
    sint16 r;
    sint16 g;
    sint16 b;

    r = 0;
    g = 0;
    b = 0;

    for (i = 0; i < number_dynamics; i++) {
        dl = &dynamics[i];
        dx = x - dl->x;
        dy = y - dl->y;
        dz = z - dl->z;
        if (abs(dx) > 0x2000 || abs(dy) > 0x2000 || abs(dz) > 0x2000)
            continue;

        dist = phd_sqrt(dx * dx + dy * dy + dz * dz);
        falloff = dl->falloff >> 1;
        if (dist <= falloff) {
            falloff = 0x1FFF - 0x1FFF * dist / falloff;
            r += (sint16)((falloff * dl->r) >> 10);
            g += (sint16)((falloff * dl->g) >> 10);
            b += (sint16)((falloff * dl->b) >> 10);
        }
    }

    if (r + g + b) {
        r += (shade & 0x1F) << 3;
        g += (shade & 0x3E0) >> 2;
        b += (shade & 0x7C00) >> 7;
        if (r > 255)
            r = 255;
        if (g > 255)
            g = 255;
        if (b > 255)
            b = 255;
        smcr = r;
        smcg = g;
        smcb = b;
    } else {
        S_CalculateStaticLight(shade);
    }
}

}
