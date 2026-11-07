#include "standard.h"
#include "specific.h"
#include "drawprimitive.h"

extern "C" {

#include "../game.h"

extern int smcr;
extern int smcg;
extern int smcb;

typedef struct phd_vbuf {
    float xv;
    float yv;
    float zv;
    float ooz;
    float xs;
    float ys;
    sint32 z;
    char clip;
    char fog;
    sint16 g;
    unsigned short u;
    unsigned short v;
    int dynamic;
} PHD_VBUF;

extern PHD_VBUF vbuf[1500];
extern sint32 LightPos[12];
extern sint32 LightCol[12];
extern PHD_VECTOR LPos[3];
extern int distanceFogValue;

inline void ApplyTransposeMatrix(sint32 *mptr, PHD_VECTOR *objptr, sint32 &x, sint32 &y, sint32 &z)
{
    x = (mptr[0] * objptr->x + mptr[4] * objptr->y + mptr[8] * objptr->z) >> W2V_SHIFT;
    y = (mptr[1] * objptr->x + mptr[5] * objptr->y + mptr[9] * objptr->z) >> W2V_SHIFT;
    z = (mptr[2] * objptr->x + mptr[6] * objptr->y + mptr[10] * objptr->z) >> W2V_SHIFT;
}

sint16 *calc_vertice_light(sint16 *objptr, sint16 *objptr1)
{
    int numvert;
    PHD_VBUF *dest;
    sint32 *mptr;
    int r;
    int g;
    int b;
    int shade;
    int fade;
    double fconv;

    dest = &vbuf[0];
    numvert = (int)*(objptr++);
    if (numvert > 0) {
        mptr = phd_mxptr;
        ApplyTransposeMatrix(mptr, &LPos[0], LightPos[0], LightPos[1], LightPos[2]);
        ApplyTransposeMatrix(mptr, &LPos[1], LightPos[4], LightPos[5], LightPos[6]);
        ApplyTransposeMatrix(mptr, &LPos[2], LightPos[8], LightPos[9], LightPos[10]);
        fade = distanceFogValue << W2V_SHIFT;
        for (; numvert > 0; numvert--, dest++, objptr += 3) {
            sint32 sdot;
            sint32 odot;
            sint32 ddot;
            float v1;
            float v3;
            float v2;

            v1 = float(objptr[0]);
            v2 = float(objptr[1]);
            v3 = float(objptr[2]);
            float fsdot;
            float fodot;
            float fddot;

            fsdot = (v1 * float(LightPos[0])) + (v2 * float(LightPos[1])) + (v3 * float(LightPos[2]));
            sdot = FTOL(fsdot);
            fodot = (v1 * float(LightPos[4])) + (v2 * float(LightPos[5])) + (v3 * float(LightPos[6]));
            odot = FTOL(fodot);
            fddot = (v1 * float(LightPos[8])) + (v2 * float(LightPos[9])) + (v3 * float(LightPos[10]));
            ddot = FTOL(fddot);
            sdot >>= 16;
            odot >>= 16;
            ddot >>= 16;
            if (sdot < 0)
                sdot = 0;
            if (odot < 0)
                odot = 0;
            if (ddot < 0)
                ddot = 0;
            r = smcr + ((LightCol[0] * sdot + LightCol[1] * odot + LightCol[2] * ddot) >> 16);
            g = smcg + ((LightCol[4] * sdot + LightCol[5] * odot + LightCol[6] * ddot) >> 16);
            b = smcb + ((LightCol[8] * sdot + LightCol[9] * odot + LightCol[10] * ddot) >> 16);
            if (r > 255)
                r = 255;
            if (g > 255)
                g = 255;
            if (b > 255)
                b = 255;
            if (r < 0)
                r = 0;
            if (g < 0)
                g = 0;
            if (b < 0)
                b = 0;
            r >>= 3;
            g >>= 3;
            b >>= 3;
            if (dest->z > fade) {
                int v;

                r <<= 3;
                g <<= 3;
                b <<= 3;
                v = 2048 - ((dest->z - fade) >> 16);
                r *= v;
                g *= v;
                b *= v;
                r >>= 14;
                g >>= 14;
                b >>= 14;
                if (r < 0)
                    r = 0;
                if (g < 0)
                    g = 0;
                if (b < 0)
                    b = 0;
            }
            dest->g = r << 10 | g << 5 | b;
        }
    } else if (numvert < 0) {
        fade = distanceFogValue << W2V_SHIFT;
        for (; numvert < 0; numvert++, dest++, objptr++) {
            shade = (0x1fff - *objptr) >> 5;
            if (!shade)
                shade = 255;
            r = (smcr * shade) >> 11;
            g = (smcg * shade) >> 11;
            b = (smcb * shade) >> 11;
            if (r > 31)
                r = 31;
            if (g > 31)
                g = 31;
            if (b > 31)
                b = 31;
            if (dest->z > fade) {
                int v;

                r <<= 3;
                g <<= 3;
                b <<= 3;
                v = 2048 - ((dest->z - fade) >> 16);
                r *= v;
                g *= v;
                b *= v;
                r >>= 14;
                g >>= 14;
                b >>= 14;
                if (r < 0)
                    r = 0;
                if (g < 0)
                    g = 0;
                if (b < 0)
                    b = 0;
            }
            dest->g = r << 10 | g << 5 | b;
        }
    }
    return objptr;
}

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
