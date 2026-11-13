#include "standard.h"
#include "global.h"
#include "drawprimitive.h"
#include "scanlines.h"

extern "C" {

void ScanLinesTGDecalAffine(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTGDecal(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);

void ScanLinesTGDecalAffine(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight)
{
    int Width;
    unsigned short *pDestination;
    long u, v, du, dv;
    long yline;
    short subdivcnt;
    short texel;
    long redleft, greenleft, blueleft;
    long dr, dg, db;

    yline = long(Output) + YTable[pLeft->y];
    du = pLeft->uozxtraf;
    dv = pLeft->vozxtraf;
    dr = pLeft->rxtraf;
    dg = pLeft->gxtraf;
    db = pLeft->bxtraf;

    while (StepLeft->height--) {
        Width = pRight->x;
        Width -= pLeft->x;
        if (Width > 0) {
            pDestination = (unsigned short *)yline + pLeft->x;
            u = pLeft->uozf;
            v = pLeft->vozf;
            redleft = pLeft->rf;
            greenleft = pLeft->gf;
            blueleft = pLeft->bf;
            subdivcnt = Width >> 4;
            Width = Width - (subdivcnt << 4);
            while (subdivcnt--) {
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 0) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 1) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 2) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 3) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 4) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 5) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 6) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 7) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 8) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 9) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 10) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 11) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 12) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 13) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 14) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 15) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                pDestination += 16;
            }
            for (int n = 0; n < Width; n++) {
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 0) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                pDestination++;
            }
        }
        yline += bytepitch;
        StepLeft->uozf += StepLeft->uozstepf;
        StepLeft->vozf += StepLeft->vozstepf;
        StepLeft->x += StepLeft->xstep;
        StepLeft->y++;
        StepLeft->rf += StepLeft->rstepf;
        StepLeft->gf += StepLeft->gstepf;
        StepLeft->bf += StepLeft->bstepf;
        StepLeft->ErrorTerm += StepLeft->Numerator;
        if (StepLeft->ErrorTerm >= StepLeft->Denominator) {
            StepLeft->x++;
            StepLeft->uozf += StepLeft->uozxtraf;
            StepLeft->vozf += StepLeft->vozxtraf;
            StepLeft->ErrorTerm -= StepLeft->Denominator;
            StepLeft->rf += StepLeft->rxtraf;
            StepLeft->gf += StepLeft->gxtraf;
            StepLeft->bf += StepLeft->bxtraf;
        }
        StepRight->x += StepRight->xstep;
        StepRight->uozf += StepRight->uozstepf;
        StepRight->vozf += StepRight->vozstepf;
        StepRight->y++;
        StepRight->rf += StepRight->rstepf;
        StepRight->gf += StepRight->gstepf;
        StepRight->bf += StepRight->bstepf;
        StepRight->ErrorTerm += StepRight->Numerator;
        if (StepRight->ErrorTerm >= StepRight->Denominator) {
            StepRight->x++;
            StepRight->uozf += StepRight->uozxtraf;
            StepRight->vozf += StepRight->vozxtraf;
            StepRight->ErrorTerm -= StepRight->Denominator;
            StepRight->rf += StepRight->rxtraf;
            StepRight->gf += StepRight->gxtraf;
            StepRight->bf += StepRight->bxtraf;
        }
    }
}


void ScanLinesTDecalAffine(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTDecal(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTFDecal(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);
void ScanLinesTFDecalAffine(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight);

void ScanLinesTGDecal(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight)
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
    long redleft, greenleft, blueleft;
    long dr, dg, db;
    short texel;
    double fconv;

    oozdx = Gradients->dozx * float(16);
    uozdx = Gradients->duzx * float(16);
    vozdx = Gradients->dvzx * float(16);
    dr = pLeft->rxtraf;
    dg = pLeft->gxtraf;
    db = pLeft->bxtraf;

    yline = long(Output) + YTable[pLeft->y];

    while (StepLeft->height--) {
        ZLeft = pLeft->ooz;
        __asm {
            fld [one]
            fdiv [ZLeft]
        }
        pDestination = (unsigned short *)yline + pLeft->x;
        Width = pRight->x - pLeft->x;
        redleft = pLeft->rf;
        greenleft = pLeft->gf;
        blueleft = pLeft->bf;
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
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 0) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 1) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 2) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 3) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 4) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 5) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 6) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 7) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 8) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 9) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 10) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 11) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 12) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 13) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 14) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    if (texel != 0) {
                        out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 15) = out;
                    }
                    u += du;
                    v += dv;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
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
                        unsigned short out;
                        texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                        if (texel != 0) {
                            out = *(RGBTableG + (texel & 0x7c0) + (greenleft >> 18));
                            out |= *(RGBTableRB + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                            out <<= 5;
                            out |= *(RGBTableRB + ((texel & 0x1f) << 6) + (blueleft >> 18));
                            *(pDestination + 0) = out;
                        }
                        u += du;
                        v += dv;
                        redleft += dr;
                        greenleft += dg;
                        blueleft += db;
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
        StepLeft->rf += StepLeft->rstepf;
        StepLeft->gf += StepLeft->gstepf;
        StepLeft->bf += StepLeft->bstepf;
        StepLeft->ErrorTerm += StepLeft->Numerator;
        if (StepLeft->ErrorTerm >= StepLeft->Denominator) {
            StepLeft->x++;
            StepLeft->ooz += StepLeft->oozxtra;
            StepLeft->uoz += StepLeft->uozxtra;
            StepLeft->voz += StepLeft->vozxtra;
            StepLeft->ErrorTerm -= StepLeft->Denominator;
            StepLeft->rf += StepLeft->rxtraf;
            StepLeft->gf += StepLeft->gxtraf;
            StepLeft->bf += StepLeft->bxtraf;
        }
        StepRight->x += StepRight->xstep;
        StepRight->y++;
        StepRight->uoz += StepRight->uozstep;
        StepRight->voz += StepRight->vozstep;
        StepRight->ooz += StepRight->oozstep;
        StepRight->rf += StepRight->rstepf;
        StepRight->gf += StepRight->gstepf;
        StepRight->bf += StepRight->bstepf;
        StepRight->ErrorTerm += StepRight->Numerator;
        if (StepRight->ErrorTerm >= StepRight->Denominator) {
            StepRight->x++;
            StepRight->ooz += StepRight->oozxtra;
            StepRight->uoz += StepRight->uozxtra;
            StepRight->voz += StepRight->vozxtra;
            StepRight->ErrorTerm -= StepRight->Denominator;
            StepRight->rf += StepRight->rxtraf;
            StepRight->gf += StepRight->gxtraf;
            StepRight->bf += StepRight->bxtraf;
        }
    }
}

}
