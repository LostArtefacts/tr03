#include "standard.h"
#include <io.h>
#include "global.h"
#include "drawprimitive.h"
#include "scanlines.h"

extern "C" {

void Func49ea50(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight)
{
    int Width;
    long u, v;
    long du, dv;
    unsigned short *pDestination;
    int subdivcnt;
    int pixelsleft;
    float oozr, uozr, vozr;
    float ZLeft, ULeft, VLeft;
    float ZRight, URight, VRight;
    float oozdx, uozdx, vozdx;
    float one = 1.0f;
    long yline;
    unsigned short r, g, b;
    double fconv;

    r = Gradients->rf;
    g = Gradients->gf;
    b = Gradients->bf;
    r >>= 2;
    g >>= 2;
    b >>= 2;

    oozdx = Gradients->dozx * float(16);
    uozdx = Gradients->duzx * float(16);
    vozdx = Gradients->dvzx * float(16);

    yline = long(Output) + YTable[pLeft->y];

    while (StepLeft->height--) {
        ZLeft = pLeft->ooz;
        __asm {
            fld [one]
            fdiv [ZLeft]
        }
        pDestination = (unsigned short *)yline + pLeft->x;
        Width = pRight->x - pLeft->x;
        __asm {
            fstp [ZLeft]
        }
        oozr = pLeft->ooz + oozdx;
        uozr = pLeft->uoz + uozdx;
        vozr = pLeft->voz + vozdx;
        ULeft = pLeft->uoz * ZLeft;
        VLeft = pLeft->voz * ZLeft;
        if (Width > 0) {
            __asm {
                fld one
                fdiv [oozr]
            }
            subdivcnt = Width >> 4;
            pixelsleft = Width - (subdivcnt << 4);
            __asm {
                fstp [ZRight]
            }
            while (subdivcnt-- > 0) {
                URight = uozr * ZRight;
                VRight = vozr * ZRight;
                u = FTOLFIX(ULeft);
                v = FTOLFIX(VLeft);
                du = FTOLFIX(URight - ULeft) >> 4;
                dv = FTOLFIX(VRight - VLeft) >> 4;
                oozr += oozdx;
                __asm {
                    fld [one]
                    fdiv [oozr]
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 0);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 0) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 1);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 1) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 2);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 2) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 3);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 3) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 4);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 4) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 5);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 5) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 6);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 6) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 7);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 7) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 8);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 8) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 9);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 9) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 10);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 10) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 11);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 11) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 12);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 12) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 13);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 13) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 14);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 14) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 15);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 15) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                pDestination += 16;
                __asm {
                    fstp [ZRight]
                }
                ULeft = URight;
                VLeft = VRight;
                vozr += vozdx;
                uozr += uozdx;
            }
            if (pixelsleft) {
                u = FTOLFIX(ULeft);
                v = FTOLFIX(VLeft);
                if (--pixelsleft) {
                    ZRight = 1.0f / (pRight->ooz - Gradients->dozx);
                    URight = ZRight * (pRight->uoz - Gradients->duzx);
                    VRight = ZRight * (pRight->voz - Gradients->dvzx);
                    float opixf;
                    opixf = WidthTable[pixelsleft];
                    du = FTOLFIX(URight - ULeft);
                    dv = FTOLFIX(VRight - VLeft);
                    du = FTOL(float(du) * opixf);
                    dv = FTOL(float(dv) * opixf);
                }
                for (int n = 0; n <= pixelsleft; n++) {
                    {
                        unsigned short texel, alpha;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        long ar, ag, ab;
                        texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                        if (texel != 0) {
                            alpha = *(pDestination + 0);
                            ar = alpha >> 11;
                            ag = (alpha >> 6) & 0x1f;
                            ab = alpha & 0x1f;
                            tr = (texel >> 5) & 0x7c0;
                            tg = texel & 0x7c0;
                            tb = (texel & 0x1f) << 6;
                            ro = *(Var6ca12c + tr + r);
                            ro += ar;
                            go = *(Var6ca12c + tg + g);
                            go += ag;
                            bo = *(Var6ca12c + tb + b);
                            bo += ab;
                            if (ro > 31)
                                ro = 31;
                            if (go > 31)
                                go = 31;
                            if (bo > 31)
                                bo = 31;
                            ro <<= 11;
                            go <<= 6;
                            *(pDestination + 0) = ro | go | bo;
                        }
                        v += dv;
                        u += du;
                    }
                    pDestination++;
                }
            }
        }
        yline += bytepitch;
        StepLeft->x += StepLeft->xstep;
        StepLeft->y++;
        StepLeft->uoz += StepLeft->uozstep;
        StepLeft->voz += StepLeft->vozstep;
        StepLeft->ooz += StepLeft->oozstep;
        StepLeft->ErrorTerm += StepLeft->Numerator;
        if (StepLeft->ErrorTerm >= StepLeft->Denominator) {
            StepLeft->x++;
            StepLeft->ooz += StepLeft->oozxtra;
            StepLeft->uoz += StepLeft->uozxtra;
            StepLeft->voz += StepLeft->vozxtra;
            StepLeft->ErrorTerm -= StepLeft->Denominator;
        }
        StepRight->x += StepRight->xstep;
        StepRight->y++;
        StepRight->uoz += StepRight->uozstep;
        StepRight->voz += StepRight->vozstep;
        StepRight->ooz += StepRight->oozstep;
        StepRight->ErrorTerm += StepRight->Numerator;
        if (StepRight->ErrorTerm >= StepRight->Denominator) {
            StepRight->x++;
            StepRight->ooz += StepRight->oozxtra;
            StepRight->uoz += StepRight->uozxtra;
            StepRight->voz += StepRight->vozxtra;
            StepRight->ErrorTerm -= StepRight->Denominator;
        }
    }
}

void Func49fe10(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight)
{
    int Width;
    unsigned short *pDestination;
    long u, v, du, dv;
    long yline;
    short subdivcnt;
    short r, g, b;

    r = Gradients->rf >> 2;
    g = Gradients->gf >> 2;
    b = Gradients->bf >> 2;
    du = pLeft->uozxtraf;
    dv = pLeft->vozxtraf;

    yline = long(Output) + YTable[pLeft->y];

    while (StepLeft->height--) {
        Width = pRight->x;
        Width -= pLeft->x;
        if (Width > 0) {
            pDestination = (unsigned short *)yline + pLeft->x;
            u = pLeft->uozf;
            v = pLeft->vozf;
            subdivcnt = Width >> 4;
            Width = Width - (subdivcnt << 4);
            while (subdivcnt--) {
                if (pLeft->y & 1) {
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 0) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 0) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 1) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 1) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 2) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 2) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 3) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 3) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 4) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 4) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 5) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 5) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 6) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 6) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 7) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 7) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 8) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 8) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 9) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 9) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 10) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 10) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 11) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 11) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 12) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 12) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 13) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 13) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 14) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 14) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 15) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 15) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                } else {
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 0) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 0) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 1) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 1) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 2) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 2) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 3) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 3) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 4) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 4) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 5) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 5) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 6) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 6) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 7) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 7) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 8) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 8) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 9) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 9) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 10) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 10) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 11) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 11) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 12) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 12) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 13) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 13) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 14) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 14) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 15) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 15) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                }
                pDestination += 16;
            }
            if (pLeft->y & 1) {
                for (int n = 0; n < Width; n++) {
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 0) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 0) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    pDestination++;
                }
            } else {
                for (int n = 0; n < Width; n++) {
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 0) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 0) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    pDestination++;
                }
            }
        }
        yline += bytepitch;
        StepLeft->uozf += StepLeft->uozstepf;
        StepLeft->vozf += StepLeft->vozstepf;
        StepLeft->x += StepLeft->xstep;
        StepLeft->y++;
        StepLeft->ErrorTerm += StepLeft->Numerator;
        if (StepLeft->ErrorTerm >= StepLeft->Denominator) {
            StepLeft->x++;
            StepLeft->uozf += StepLeft->uozxtraf;
            StepLeft->vozf += StepLeft->vozxtraf;
            StepLeft->ErrorTerm -= StepLeft->Denominator;
        }
        StepRight->x += StepRight->xstep;
        StepRight->uozf += StepRight->uozstepf;
        StepRight->vozf += StepRight->vozstepf;
        StepRight->y++;
        StepRight->ErrorTerm += StepRight->Numerator;
        if (StepRight->ErrorTerm >= StepRight->Denominator) {
            StepRight->x++;
            StepRight->uozf += StepRight->uozxtraf;
            StepRight->vozf += StepRight->vozxtraf;
            StepRight->ErrorTerm -= StepRight->Denominator;
        }
    }
}

void Func4a2280(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight)
{
    int Width;
    long u, v;
    long du, dv;
    unsigned short *pDestination;
    int subdivcnt;
    int pixelsleft;
    float oozr, uozr, vozr;
    float ZLeft, ULeft, VLeft;
    float ZRight, URight, VRight;
    float oozdx, uozdx, vozdx;
    float one = 1.0f;
    long yline;
    unsigned short r, g, b;
    double fconv;

    r = Gradients->rf;
    g = Gradients->gf;
    b = Gradients->bf;
    r >>= 2;
    g >>= 2;
    b >>= 2;

    oozdx = Gradients->dozx * float(16);
    uozdx = Gradients->duzx * float(16);
    vozdx = Gradients->dvzx * float(16);

    yline = long(Output) + YTable[pLeft->y];

    while (StepLeft->height--) {
        ZLeft = pLeft->ooz;
        __asm {
            fld [one]
            fdiv [ZLeft]
        }
        pDestination = (unsigned short *)yline + pLeft->x;
        Width = pRight->x - pLeft->x;
        __asm {
            fstp [ZLeft]
        }
        oozr = pLeft->ooz + oozdx;
        uozr = pLeft->uoz + uozdx;
        vozr = pLeft->voz + vozdx;
        ULeft = pLeft->uoz * ZLeft;
        VLeft = pLeft->voz * ZLeft;
        if (Width > 0) {
            __asm {
                fld one
                fdiv [oozr]
            }
            subdivcnt = Width >> 4;
            pixelsleft = Width - (subdivcnt << 4);
            __asm {
                fstp [ZRight]
            }
            while (subdivcnt-- > 0) {
                URight = uozr * ZRight;
                VRight = vozr * ZRight;
                u = FTOLFIX(ULeft);
                v = FTOLFIX(VLeft);
                du = FTOLFIX(URight - ULeft) >> 4;
                dv = FTOLFIX(VRight - VLeft) >> 4;
                oozr += oozdx;
                __asm {
                    fld [one]
                    fdiv [oozr]
                }
                if (pLeft->y & 1) {
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 0) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 0) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 1) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 1) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 2) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 2) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 3) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 3) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 4) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 4) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 5) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 5) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 6) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 6) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 7) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 7) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 8) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 8) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 9) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 9) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 10) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 10) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 11) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 11) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 12) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 12) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 13) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 13) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 14) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 14) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if (!((long)(pDestination + 15) & 2)) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 15) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                } else {
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 0) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 0) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 1) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 1) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 2) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 2) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 3) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 3) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 4) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 4) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 5) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 5) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 6) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 6) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 7) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 7) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 8) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 8) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 9) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 9) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 10) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 10) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 11) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 11) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 12) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 12) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 13) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 13) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 14) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 14) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                    {
                        unsigned short texel;
                        long tr, tg, tb;
                        unsigned long ro, go, bo;
                        if ((long)(pDestination + 15) & 2) {
                            texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                            if (texel != 0) {
                                tr = (texel >> 5) & 0x7c0;
                                tg = texel & 0x7c0;
                                tb = (texel & 0x1f) << 6;
                                ro = *(Var6ca12c + tr + r);
                                ro += r >> 2;
                                go = *(Var6ca12c + tg + g);
                                go += g >> 2;
                                bo = *(Var6ca12c + tb + b);
                                bo += b >> 2;
                                if (ro > 31)
                                    ro = 31;
                                if (go > 31)
                                    go = 31;
                                if (bo > 31)
                                    bo = 31;
                                ro <<= 11;
                                go <<= 6;
                                *(pDestination + 15) = ro | go | bo;
                            }
                        }
                        v += dv;
                        u += du;
                    }
                }
                pDestination += 16;
                __asm {
                    fstp [ZRight]
                }
                ULeft = URight;
                VLeft = VRight;
                vozr += vozdx;
                uozr += uozdx;
            }
            if (pixelsleft) {
                u = FTOLFIX(ULeft);
                v = FTOLFIX(VLeft);
                if (--pixelsleft) {
                    ZRight = 1.0f / (pRight->ooz - Gradients->dozx);
                    URight = ZRight * (pRight->uoz - Gradients->duzx);
                    VRight = ZRight * (pRight->voz - Gradients->dvzx);
                    float opixf;
                    opixf = WidthTable[pixelsleft];
                    du = FTOLFIX(URight - ULeft);
                    dv = FTOLFIX(VRight - VLeft);
                    du = FTOL(float(du) * opixf);
                    dv = FTOL(float(dv) * opixf);
                }
                if (pLeft->y & 1) {
                    for (int n = 0; n <= pixelsleft; n++) {
                        {
                            unsigned short texel;
                            long tr, tg, tb;
                            unsigned long ro, go, bo;
                            if (!((long)(pDestination + 0) & 2)) {
                                texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                                if (texel != 0) {
                                    tr = (texel >> 5) & 0x7c0;
                                    tg = texel & 0x7c0;
                                    tb = (texel & 0x1f) << 6;
                                    ro = *(Var6ca12c + tr + r);
                                    ro += r >> 2;
                                    go = *(Var6ca12c + tg + g);
                                    go += g >> 2;
                                    bo = *(Var6ca12c + tb + b);
                                    bo += b >> 2;
                                    if (ro > 31)
                                        ro = 31;
                                    if (go > 31)
                                        go = 31;
                                    if (bo > 31)
                                        bo = 31;
                                    ro <<= 11;
                                    go <<= 6;
                                    *(pDestination + 0) = ro | go | bo;
                                }
                            }
                            v += dv;
                            u += du;
                        }
                        pDestination++;
                    }
                } else {
                    for (int n = 0; n <= pixelsleft; n++) {
                        {
                            unsigned short texel;
                            long tr, tg, tb;
                            unsigned long ro, go, bo;
                            if ((long)(pDestination + 0) & 2) {
                                texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                                if (texel != 0) {
                                    tr = (texel >> 5) & 0x7c0;
                                    tg = texel & 0x7c0;
                                    tb = (texel & 0x1f) << 6;
                                    ro = *(Var6ca12c + tr + r);
                                    ro += r >> 2;
                                    go = *(Var6ca12c + tg + g);
                                    go += g >> 2;
                                    bo = *(Var6ca12c + tb + b);
                                    bo += b >> 2;
                                    if (ro > 31)
                                        ro = 31;
                                    if (go > 31)
                                        go = 31;
                                    if (bo > 31)
                                        bo = 31;
                                    ro <<= 11;
                                    go <<= 6;
                                    *(pDestination + 0) = ro | go | bo;
                                }
                            }
                            v += dv;
                            u += du;
                        }
                        pDestination++;
                    }
                }
            }
        }
        yline += bytepitch;
        StepLeft->x += StepLeft->xstep;
        StepLeft->y++;
        StepLeft->uoz += StepLeft->uozstep;
        StepLeft->voz += StepLeft->vozstep;
        StepLeft->ooz += StepLeft->oozstep;
        StepLeft->ErrorTerm += StepLeft->Numerator;
        if (StepLeft->ErrorTerm >= StepLeft->Denominator) {
            StepLeft->x++;
            StepLeft->ooz += StepLeft->oozxtra;
            StepLeft->uoz += StepLeft->uozxtra;
            StepLeft->voz += StepLeft->vozxtra;
            StepLeft->ErrorTerm -= StepLeft->Denominator;
        }
        StepRight->x += StepRight->xstep;
        StepRight->y++;
        StepRight->uoz += StepRight->uozstep;
        StepRight->voz += StepRight->vozstep;
        StepRight->ooz += StepRight->oozstep;
        StepRight->ErrorTerm += StepRight->Numerator;
        if (StepRight->ErrorTerm >= StepRight->Denominator) {
            StepRight->x++;
            StepRight->ooz += StepRight->oozxtra;
            StepRight->uoz += StepRight->uozxtra;
            StepRight->voz += StepRight->vozxtra;
            StepRight->ErrorTerm -= StepRight->Denominator;
        }
    }
}

void ScanLinesTFAlphaDecalAffine(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight)
{
    int Width;
    unsigned short *pDestination;
    long u, v, du, dv;
    long yline;
    short subdivcnt;
    short r, g, b;

    r = Gradients->rf >> 2;
    g = Gradients->gf >> 2;
    b = Gradients->bf >> 2;
    du = pLeft->uozxtraf;
    dv = pLeft->vozxtraf;

    yline = long(Output) + YTable[pLeft->y];

    while (StepLeft->height--) {
        Width = pRight->x;
        Width -= pLeft->x;
        if (Width > 0) {
            pDestination = (unsigned short *)yline + pLeft->x;
            u = pLeft->uozf;
            v = pLeft->vozf;
            subdivcnt = Width >> 4;
            Width = Width - (subdivcnt << 4);
            while (subdivcnt--) {
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 0);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 0) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 1);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 1) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 2);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 2) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 3);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 3) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 4);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 4) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 5);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 5) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 6);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 6) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 7);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 7) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 8);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 8) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 9);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 9) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 10);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 10) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 11);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 11) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 12);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 12) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 13);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 13) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 14);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 14) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 15);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 15) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                pDestination += 16;
            }
            for (int n = 0; n < Width; n++) {
                {
                    unsigned short texel, alpha;
                    long tr, tg, tb;
                    unsigned long ro, go, bo;
                    long ar, ag, ab;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        alpha = *(pDestination + 0);
                        ar = alpha >> 11;
                        ag = (alpha >> 6) & 0x1f;
                        ab = alpha & 0x1f;
                        tr = (texel >> 5) & 0x7c0;
                        tg = texel & 0x7c0;
                        tb = (texel & 0x1f) << 6;
                        ro = *(Var6ca12c + tr + r);
                        ro += ar;
                        go = *(Var6ca12c + tg + g);
                        go += ag;
                        bo = *(Var6ca12c + tb + b);
                        bo += ab;
                        if (ro > 31)
                            ro = 31;
                        if (go > 31)
                            go = 31;
                        if (bo > 31)
                            bo = 31;
                        ro <<= 11;
                        go <<= 6;
                        *(pDestination + 0) = ro | go | bo;
                    }
                    v += dv;
                    u += du;
                }
                pDestination++;
            }
        }
        yline += bytepitch;
        StepLeft->uozf += StepLeft->uozstepf;
        StepLeft->vozf += StepLeft->vozstepf;
        StepLeft->x += StepLeft->xstep;
        StepLeft->y++;
        StepLeft->ErrorTerm += StepLeft->Numerator;
        if (StepLeft->ErrorTerm >= StepLeft->Denominator) {
            StepLeft->x++;
            StepLeft->uozf += StepLeft->uozxtraf;
            StepLeft->vozf += StepLeft->vozxtraf;
            StepLeft->ErrorTerm -= StepLeft->Denominator;
        }
        StepRight->x += StepRight->xstep;
        StepRight->uozf += StepRight->uozstepf;
        StepRight->vozf += StepRight->vozstepf;
        StepRight->y++;
        StepRight->ErrorTerm += StepRight->Numerator;
        if (StepRight->ErrorTerm >= StepRight->Denominator) {
            StepRight->x++;
            StepRight->uozf += StepRight->uozxtraf;
            StepRight->vozf += StepRight->vozxtraf;
            StepRight->ErrorTerm -= StepRight->Denominator;
        }
    }
}

}
