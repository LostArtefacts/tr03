extern "C" {
#include "../3dglodef.h"
extern sint32 LightCol[12];
inline void ApplyTransposeMatrix(sint32 *mptr, PHD_VECTOR *objptr, sint32 &x, sint32 &y, sint32 &z)
{
    x = (mptr[0] * objptr->x + mptr[4] * objptr->y + mptr[8] * objptr->z) >> W2V_SHIFT;
    y = (mptr[1] * objptr->x + mptr[5] * objptr->y + mptr[9] * objptr->z) >> W2V_SHIFT;
    z = (mptr[2] * objptr->x + mptr[6] * objptr->y + mptr[10] * objptr->z) >> W2V_SHIFT;
}

}

#include "standard.h"
#include "specific.h"
#include "drawprimitive.h"

extern "C" {

#include "../game.h"

extern PHD_VECTOR LPos[3];
extern int smcr;
extern int smcg;
extern int smcb;

extern sint32 LightPos[12];
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
extern int distanceFogValue;


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
        float light0;
        float light1;
        float light2;
        float light5;
        float light6;
        float light4;
        float light10;
        float light8;
        float light9;
        mptr = phd_mxptr;
        ApplyTransposeMatrix(mptr, &LPos[0], LightPos[0], LightPos[1], LightPos[2]);
        ApplyTransposeMatrix(mptr, &LPos[1], LightPos[4], LightPos[5], LightPos[6]);
        ApplyTransposeMatrix(mptr, &LPos[2], LightPos[8], LightPos[9], LightPos[10]);
        light0 = float(LightPos[0]);
        light2 = float(LightPos[2]);
        light1 = float(LightPos[1]);
        light4 = float(LightPos[4]);
        light6 = float(LightPos[6]);
        light5 = float(LightPos[5]);
        light8 = float(LightPos[8]);
        light10 = float(LightPos[10]);
        light9 = float(LightPos[9]);
        fade = distanceFogValue;
        fade <<= W2V_SHIFT;
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

            fsdot = (v1 * light0) + (v2 * light1) + (v3 * light2);
            sdot = FTOL(fsdot);
            fodot = (v1 * light4) + (v2 * light5) + (v3 * light6);
            odot = FTOL(fodot);
            fddot = (v1 * light8) + (v2 * light9) + (v3 * light10);
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

#define SQUARE(A) ((A) * (A))

extern sint32 LightCol[];
extern PHD_VECTOR LPos[];
extern sint32 w2v_matrix[];
extern DYNAMIC dynamics[];
extern int number_dynamics;

uint32 __fastcall phd_sqrt(uint32 n);
void phd_NormaliseVector(sint32 x, sint32 y, sint32 z, sint32 *dest);
int abs(int x);
#pragma intrinsic(abs)

inline void ApplySunMatrix(sint32 *mptr, PHD_VECTOR *objptr, sint32 &x, sint32 &y, sint32 &z)
{
    sint32 vy;
    sint32 vx;
    sint32 vz;

    vx = objptr->x;
    vy = objptr->y;
    vz = objptr->z;
    x = (mptr[0] * vx + mptr[1] * vy + mptr[2] * vz) >> W2V_SHIFT;
    y = (mptr[4] * vx + mptr[5] * vy + mptr[6] * vz) >> W2V_SHIFT;
    z = (mptr[8] * vx + mptr[9] * vy + mptr[10] * vz) >> W2V_SHIFT;
}

inline void ApplyOmniMatrix(sint32 *mptr, PHD_VECTOR *objptr, sint32 &x, sint32 &y, sint32 &z)
{
    sint32 m1 = mptr[1];
    sint32 m5 = mptr[5];
    sint32 vx;
    sint32 vy;
    sint32 vz;

    vx = objptr->x;
    vy = objptr->y;
    vz = objptr->z;
    x = (mptr[0] * vx + m1 * vy + mptr[2] * vz) >> W2V_SHIFT;
    y = (mptr[4] * vx + m5 * vy + mptr[6] * vz) >> W2V_SHIFT;
    z = (mptr[8] * vx + mptr[9] * vy + mptr[10] * vz) >> W2V_SHIFT;
}

inline void ApplyDynamicMatrix(sint32 *mptr, PHD_VECTOR *objptr, sint32 &x, sint32 &y, sint32 &z)
{
    sint32 vz;
    sint32 vx;
    sint32 vy;

    vx = objptr->x;
    vy = objptr->y;
    vz = objptr->z;
    x = (mptr[0] * vx + mptr[1] * vy + mptr[2] * vz) >> W2V_SHIFT;
    y = (mptr[4] * vx + mptr[5] * vy + mptr[6] * vz) >> W2V_SHIFT;
    z = (mptr[8] * vx + mptr[9] * vy + mptr[10] * vz) >> W2V_SHIFT;
}

void S_CalculateLight(sint32 x, sint32 y, sint32 z, sint16 room_number, ITEM_LIGHT *il)
{
    PHD_VECTOR ls;
    ROOM_INFO *r;
    uint8 sun;
    DYNAMIC *dl;
    DYNAMIC *dl2;
    LIGHT_INFO *l;
    int i;
    sint32 distance;
    sint32 brightest;
    ITEM_LIGHT dummy;
    LIGHT_INFO *sl;
    sint32 shade;
    LIGHT_INFO *light;
    uint16 ambience;
    LIGHT_INFO sunlight;
    PHD_VECTOR lc;

    sl = NULL;
    dl2 = NULL;
    l = NULL;
    if (!il) {
        il = &dummy;
        il->init = 0;
    }
    LightPos[0] = LightPos[1] = LightPos[2] = LightPos[4] = LightPos[5] = LightPos[6] = LightPos[8] = LightPos[9] = LightPos[10] = 0;
    LightCol[0] = LightCol[1] = LightCol[2] = LightCol[4] = LightCol[5] = LightCol[6] = LightCol[8] = LightCol[9] = LightCol[10] = 0;
    LPos[0].x = LPos[0].y = LPos[0].z = 0;
    LPos[1].x = LPos[1].y = LPos[1].z = 0;
    LPos[2].x = LPos[2].y = LPos[2].z = 0;
    sun = 0;
    brightest = -1;
    r = &room[room_number];
    ambience = ((0x1fff - r->ambient) >> 5) + 1;
    for (i = r->num_lights, light = r->light; i > 0; i--, light++) {
        if (0 != light->type) {
            sun = 1;
            sl = light;
            continue;
        }
        lc.x = (light->x - x) >> 2;
        lc.y = (light->y - y) >> 2;
        lc.z = (light->z - z) >> 2;
        distance = phd_sqrt(SQUARE(lc.x) + SQUARE(lc.y) + SQUARE(lc.z));
        if (distance <= light->l.spot.falloff) {
            shade = light->l.spot.intensity - light->l.spot.intensity * distance / light->l.spot.falloff;
            ambience += (uint16)(shade >> 7);
            if (shade > brightest) {
                brightest = shade;
                l = light;
                ls.x = lc.x;
                ls.y = lc.y;
                ls.z = lc.z;
            }
        }
    }
    if (0 == sun && il->init & 1) {
        sunlight.l.sun.nx = (sint16)il->sun.x;
        sunlight.l.sun.ny = (sint16)il->sun.y;
        sunlight.l.sun.nz = (sint16)il->sun.z;
        sunlight.r = sunlight.g = sunlight.b = (0x1fff - r->ambient) >> 5;
        sl = &sunlight;
        sun = 1;
    }
    if (0 != sun) {
        if (1 & il->init) {
            il->sun.x += (sl->l.sun.nx - il->sun.x) >> 3;
            il->sun.y += (sl->l.sun.ny - il->sun.y) >> 3;
            il->sun.z += (sl->l.sun.nz - il->sun.z) >> 3;
            il->sunr += (sl->r - il->sunr) >> 3;
            il->sung += (sl->g - il->sung) >> 3;
            il->sunb += (sl->b - il->sunb) >> 3;
        } else {
            il->sun.x = sl->l.sun.nx;
            il->sun.y = sl->l.sun.ny;
            il->sun.z = sl->l.sun.nz;
            il->sunr = sl->r;
            il->sung = sl->g;
            il->sunb = sl->b;
            il->init |= 1;
        }
        LPos[0].x = il->sun.x;
        LPos[0].y = il->sun.y;
        LPos[0].z = il->sun.z;
        LightCol[0] = il->sunr << 4;
        LightCol[4] = il->sung << 4;
        LightCol[8] = il->sunb << 4;
    }
    if (brightest == -1 && il->init & 2) {
        ls.x = il->bulb.x;
        ls.y = il->bulb.y;
        ls.z = il->bulb.z;
        sunlight.r = sunlight.g = sunlight.b = (0x1fff - r->ambient) >> 5;
        l = &sunlight;
        brightest = 0x1fff;
    }
    if (brightest != -1) {
        phd_NormaliseVector(ls.x, ls.y, ls.z, (sint32 *)&ls);
        if (il->init & 2) {
            il->bulb.x += (ls.x - il->bulb.x) >> 3;
            il->bulb.y += (ls.y - il->bulb.y) >> 3;
            il->bulb.z += (ls.z - il->bulb.z) >> 3;
            il->bulbr += (uint8)((((l->r * brightest) >> 13) - il->bulbr) >> 3);
            il->bulbg += (uint8)((((brightest * l->g) >> 13) - il->bulbg) >> 3);
            il->bulbb += (uint8)((((brightest * l->b) >> 13) - il->bulbb) >> 3);
        } else {
            il->bulb.x = ls.x;
            il->bulb.y = ls.y;
            il->bulb.z = ls.z;
            il->bulbr = (uint8)((l->r * brightest) >> 13);
            il->bulbg = (uint8)((brightest * l->g) >> 13);
            il->bulbb = (uint8)((l->b * brightest) >> 13);
            il->init = il->init | 2;
        }
        LPos[1].x = il->bulb.x >> 2;
        LPos[1].y = il->bulb.y >> 2;
        LPos[1].z = il->bulb.z >> 2;
        LightCol[1] = il->bulbr << 4;
        LightCol[5] = il->bulbg << 4;
        LightCol[9] = il->bulbb << 4;
    }
    brightest = -1;
    for (i = 0; i < number_dynamics; i++) {
        dl = dynamics + i;
        lc.x = dl->x - x;
        lc.y = dl->y - y;
        lc.z = dl->z - z;
        if (abs(lc.x) > 0x2000 || abs(lc.y) > 0x2000 || abs(lc.z) > 0x2000)
            continue;
        distance = phd_sqrt(SQUARE(lc.x) + SQUARE(lc.y) + SQUARE(lc.z));
        if (distance <= dl->falloff >> 1) {
            shade = 0x1fff - 0x1fff * distance / (dl->falloff >> 1);
            ambience += (uint16)(shade >> 8);
            if (shade > brightest) {
                brightest = shade;
                ls.x = lc.x;
                ls.y = lc.y;
                ls.z = lc.z;
                dl2 = dl;
            }
        }
    }
    if (brightest != -1) {
        phd_NormaliseVector(ls.x, ls.y, ls.z, (sint32 *)&ls);
        if (il->init & 4) {
            il->dynamic.x += (ls.x - il->dynamic.x) >> 1;
            il->dynamic.y += (ls.y - il->dynamic.y) >> 1;
            il->dynamic.z += (ls.z - il->dynamic.z) >> 1;
            il->dynamicr += (uint8)((((brightest * dl2->r) >> 10) - il->dynamicr) >> 1);
            il->dynamicg += (uint8)((((dl2->g * brightest) >> 10) - il->dynamicg) >> 1);
            il->dynamicb += (uint8)((((dl2->b * brightest) >> 10) - il->dynamicb) >> 1);
        } else {
            il->dynamic.x = ls.x;
            il->dynamic.y = ls.y;
            il->dynamic.z = ls.z;
            il->dynamicr = (uint8)((brightest * dl2->r) >> 10);
            il->dynamicg = (uint8)((dl2->g * brightest) >> 10);
            il->dynamicb = (uint8)((brightest * dl2->b) >> 10);
            il->init |= 4;
        }
        LPos[2].x = il->dynamic.x >> 2;
        LPos[2].y = il->dynamic.y >> 2;
        LPos[2].z = il->dynamic.z >> 2;
        LightCol[2] = il->dynamicr << 4;
        LightCol[6] = il->dynamicg << 4;
        LightCol[10] = il->dynamicb << 4;
    }
    if (255 < ambience)
        ambience = 255;
    if (il->init & 8)
        il->ambient += (ambience - il->ambient) >> 3;
    else {
        il->ambient = (uint8)ambience;
        il->init = il->init | 8;
    }
    smcr = il->ambient;
    smcg = il->ambient;
    smcb = il->ambient;
    LPos[0].x <<= 2;
    LPos[0].y <<= 2;
    LPos[0].z <<= 2;
    LPos[1].x <<= 2;
    LPos[1].y <<= 2;
    LPos[1].z <<= 2;
    LPos[2].x <<= 2;
    LPos[2].y <<= 2;
    LPos[2].z <<= 2;
    ApplySunMatrix(w2v_matrix, &LPos[0], ls.x, ls.y, ls.z);
    LPos[0] = ls;
    ApplyOmniMatrix(w2v_matrix, &LPos[1], ls.x, ls.y, ls.z);
    LPos[1] = ls;
    ApplyDynamicMatrix(w2v_matrix, &LPos[2], ls.x, ls.y, ls.z);
    LPos[2] = ls;
}

void S_CalculateStaticLight(sint16 adder)
{
    smcr = (adder & 0x1f) << 3;
    smcg = (adder & 0x3e0) >> 2;
    smcb = (adder & 0x7c00) >> 7;
}

uint32 __fastcall phd_sqrt(uint32 n);

void S_CalculateStaticMeshLight(int x, int y, int z, int shade)
{
    sint16 b;
    sint16 r;
    uint16 *radius;
    int dz;
    DYNAMIC *dl;
    sint16 g;
    int dy;
    int i;
    int dx;
    int falloff;
    int dist;

    b = 0;
    g = 0;
    r = 0;

    for (i = 0; i < number_dynamics; i++) {
        radius = &dynamics[i].falloff;
        dl = (DYNAMIC *)((char *)radius - FIELD_OFFSET(DYNAMIC, falloff));
        dx = x - dl->x;
        dy = y - dl->y;
        dz = z - dl->z;
        if (abs(dx) > 0x2000 || abs(dy) > 0x2000 || abs(dz) > 0x2000)
            continue;

        dist = phd_sqrt(dx * dx + dy * dy + dz * dz);
        falloff = *radius >> 1;
        if (dist <= falloff) {
            falloff = 0x1FFF - 0x1FFF * dist / falloff;
            r = r + (sint16)((falloff * dl->r) >> 10);
            g += (sint16)((dl->g * falloff) >> 10);
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
