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

inline void ApplyMatrix(sint32 *mptr, PHD_VECTOR *objptr, sint32 &x, sint32 &y, sint32 &z)
{
    x = (mptr[0] * objptr->x + mptr[1] * objptr->y + mptr[2] * objptr->z) >> W2V_SHIFT;
    y = (mptr[4] * objptr->x + mptr[5] * objptr->y + mptr[6] * objptr->z) >> W2V_SHIFT;
    z = (mptr[8] * objptr->x + mptr[9] * objptr->y + mptr[10] * objptr->z) >> W2V_SHIFT;
}

void S_CalculateLight(sint32 x, sint32 y, sint32 z, sint16 room_number, ITEM_LIGHT *il)
{
    ROOM_INFO *r;
    LIGHT_INFO *sl, *light, *l;
    DYNAMIC *dl, *dl2;
    LIGHT_INFO sunlight;
    ITEM_LIGHT dummy;
    PHD_VECTOR ls, lc;
    sint32 brightest, distance, shade;
    uint16 ambience;
    uint8 sun;
    int i;

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
    r = &room[room_number];
    sun = 0;
    brightest = -1;
    ambience = ((0x1fff - r->ambient) >> 5) + 1;
    for (i = r->num_lights, light = r->light; i > 0; i--, light++) {
        if (light->type) {
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
    if (!sun && il->init & 1) {
        sunlight.l.sun.nx = (sint16)il->sun.x;
        sunlight.l.sun.ny = (sint16)il->sun.y;
        sunlight.l.sun.nz = (sint16)il->sun.z;
        sunlight.r = sunlight.g = sunlight.b = (0x1fff - r->ambient) >> 5;
        sl = &sunlight;
        sun = 1;
    }
    if (sun) {
        if (il->init & 1) {
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
        brightest = 0x1fff;
        ls.x = il->bulb.x;
        ls.y = il->bulb.y;
        ls.z = il->bulb.z;
        sunlight.r = sunlight.g = sunlight.b = (0x1fff - r->ambient) >> 5;
        l = &sunlight;
    }
    if (brightest != -1) {
        phd_NormaliseVector(ls.x, ls.y, ls.z, (sint32 *)&ls);
        if (il->init & 2) {
            il->bulb.x += (ls.x - il->bulb.x) >> 3;
            il->bulb.y += (ls.y - il->bulb.y) >> 3;
            il->bulb.z += (ls.z - il->bulb.z) >> 3;
            il->bulbr += (uint8)((((brightest * l->r) >> 13) - il->bulbr) >> 3);
            il->bulbg += (uint8)((((brightest * l->g) >> 13) - il->bulbg) >> 3);
            il->bulbb += (uint8)((((brightest * l->b) >> 13) - il->bulbb) >> 3);
        } else {
            il->bulb.x = ls.x;
            il->bulb.y = ls.y;
            il->bulb.z = ls.z;
            il->bulbr = (uint8)((brightest * l->r) >> 13);
            il->bulbg = (uint8)((brightest * l->g) >> 13);
            il->bulbb = (uint8)((brightest * l->b) >> 13);
            il->init |= 2;
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
        dl = &dynamics[i];
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
                dl2 = dl;
                ls.x = lc.x;
                ls.y = lc.y;
                ls.z = lc.z;
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
            il->dynamicg += (uint8)((((brightest * dl2->g) >> 10) - il->dynamicg) >> 1);
            il->dynamicb += (uint8)((((brightest * dl2->b) >> 10) - il->dynamicb) >> 1);
        } else {
            il->dynamic.x = ls.x;
            il->dynamic.y = ls.y;
            il->dynamic.z = ls.z;
            il->dynamicr = (uint8)((brightest * dl2->r) >> 10);
            il->dynamicg = (uint8)((brightest * dl2->g) >> 10);
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
    if (ambience > 255)
        ambience = 255;
    if (il->init & 8)
        il->ambient += (ambience - il->ambient) >> 3;
    else {
        il->ambient = (uint8)ambience;
        il->init |= 8;
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
    x = (LPos[0].x * w2v_matrix[0] + LPos[0].y * w2v_matrix[1] + LPos[0].z * w2v_matrix[2]) >> W2V_SHIFT;
    y = (LPos[0].x * w2v_matrix[4] + LPos[0].y * w2v_matrix[5] + LPos[0].z * w2v_matrix[6]) >> W2V_SHIFT;
    z = (LPos[0].x * w2v_matrix[8] + LPos[0].y * w2v_matrix[9] + LPos[0].z * w2v_matrix[10]) >> W2V_SHIFT;
    LPos[0].x = x;
    LPos[0].y = y;
    LPos[0].z = z;
    x = (LPos[1].x * w2v_matrix[0] + LPos[1].y * w2v_matrix[1] + LPos[1].z * w2v_matrix[2]) >> W2V_SHIFT;
    y = (LPos[1].x * w2v_matrix[4] + LPos[1].y * w2v_matrix[5] + LPos[1].z * w2v_matrix[6]) >> W2V_SHIFT;
    z = (LPos[1].x * w2v_matrix[8] + LPos[1].y * w2v_matrix[9] + LPos[1].z * w2v_matrix[10]) >> W2V_SHIFT;
    LPos[1].x = x;
    LPos[1].y = y;
    LPos[1].z = z;
    x = (LPos[2].x * w2v_matrix[0] + LPos[2].y * w2v_matrix[1] + LPos[2].z * w2v_matrix[2]) >> W2V_SHIFT;
    y = (LPos[2].x * w2v_matrix[4] + LPos[2].y * w2v_matrix[5] + LPos[2].z * w2v_matrix[6]) >> W2V_SHIFT;
    z = (LPos[2].x * w2v_matrix[8] + LPos[2].y * w2v_matrix[9] + LPos[2].z * w2v_matrix[10]) >> W2V_SHIFT;
    LPos[2].x = x;
    LPos[2].y = y;
    LPos[2].z = z;
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
