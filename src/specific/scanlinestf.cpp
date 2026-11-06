#include "standard.h"
#include "global.h"
#include "drawprimitive.h"
#include "scanlines.h"

extern "C" {

void ScanLinesTAffine(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight)
{
    int Width;
    unsigned short *pDestination;
    long v, u, du, dv;
    long yline;
    short subdivcnt;
    unsigned short *pTexture;

    pTexture = TextureHandle;
    yline = long(Output) + YTable[pLeft->y];
    du = pLeft->uozxtraf;
    dv = pLeft->vozxtraf;

    while (StepLeft->height--) {
        Width = pRight->x;
        Width -= pLeft->x;
        if (Width > 0) {
            pDestination = (unsigned short *)yline + pLeft->x;
            v = pLeft->vozf;
            u = pLeft->uozf;
            subdivcnt = Width >> 4;
            Width = Width - (subdivcnt << 4);
            while (subdivcnt--) {
                *(pDestination + 0) = *(pTexture + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 1) = *(pTexture + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 2) = *(pTexture + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 3) = *(pTexture + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 4) = *(pTexture + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 5) = *(pTexture + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 6) = *(pTexture + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 7) = *(pTexture + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 8) = *(pTexture + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 9) = *(pTexture + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 10) = *(pTexture + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 11) = *(pTexture + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 12) = *(pTexture + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 13) = *(pTexture + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 14) = *(pTexture + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 15) = *(pTexture + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                pDestination += 16;
            }
            switch (Width) {
            case 15:
                *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
            case 14:
                *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
            case 13:
                *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
            case 12:
                *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
            case 11:
                *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
            case 10:
                *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
            case 9:
                *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
            case 8:
                *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
            case 7:
                *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
            case 6:
                *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
            case 5:
                *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
            case 4:
                *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
            case 3:
                *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
            case 2:
                *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
            case 1:
                *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
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

void ScanLinesT(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight)
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
    double fconv;
    
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
                *(pDestination + 0) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 1) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 2) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 3) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 4) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 5) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 6) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 7) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 8) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 9) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 10) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 11) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 12) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 13) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 14) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
                *(pDestination + 15) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                u += du;
                v += dv;
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
                switch (pixelsleft) {
                case 15:
                    *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    u += du;
                    v += dv;
                case 14:
                    *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    u += du;
                    v += dv;
                case 13:
                    *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    u += du;
                    v += dv;
                case 12:
                    *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    u += du;
                    v += dv;
                case 11:
                    *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    u += du;
                    v += dv;
                case 10:
                    *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    u += du;
                    v += dv;
                case 9:
                    *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    u += du;
                    v += dv;
                case 8:
                    *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    u += du;
                    v += dv;
                case 7:
                    *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    u += du;
                    v += dv;
                case 6:
                    *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    u += du;
                    v += dv;
                case 5:
                    *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    u += du;
                    v += dv;
                case 4:
                    *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    u += du;
                    v += dv;
                case 3:
                    *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    u += du;
                    v += dv;
                case 2:
                    *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    u += du;
                    v += dv;
                case 1:
                    *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    u += du;
                    v += dv;
                case 0:
                    *(pDestination++) = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    u += du;
                    v += dv;
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

void ScanLinesTF(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight)
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
    short r, g, b;
    short texel;
    double fconv;
    
    r = Gradients->rf;
    g = Gradients->gf;
    b = Gradients->bf;
    if (r == 255 && g == 255 && b == 255) {
        ScanLinesT(Gradients, pLeft, pRight, StepLeft, StepRight);
        return;
    }
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
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    out = *(Var6ca1a8 + (texel & 0x7c0) + g);
                    out |= *(Var6ca12c + ((texel >> 5) & 0x7c0) + r) << 6;
                    out <<= 5;
                    out |= *(Var6ca12c + ((texel & 0x1f) << 6) + b);
                    *(pDestination + 0) = out;
                    u += du;
                    v += dv;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    out = *(Var6ca1a8 + (texel & 0x7c0) + g);
                    out |= *(Var6ca12c + ((texel >> 5) & 0x7c0) + r) << 6;
                    out <<= 5;
                    out |= *(Var6ca12c + ((texel & 0x1f) << 6) + b);
                    *(pDestination + 1) = out;
                    u += du;
                    v += dv;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    out = *(Var6ca1a8 + (texel & 0x7c0) + g);
                    out |= *(Var6ca12c + ((texel >> 5) & 0x7c0) + r) << 6;
                    out <<= 5;
                    out |= *(Var6ca12c + ((texel & 0x1f) << 6) + b);
                    *(pDestination + 2) = out;
                    u += du;
                    v += dv;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    out = *(Var6ca1a8 + (texel & 0x7c0) + g);
                    out |= *(Var6ca12c + ((texel >> 5) & 0x7c0) + r) << 6;
                    out <<= 5;
                    out |= *(Var6ca12c + ((texel & 0x1f) << 6) + b);
                    *(pDestination + 3) = out;
                    u += du;
                    v += dv;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    out = *(Var6ca1a8 + (texel & 0x7c0) + g);
                    out |= *(Var6ca12c + ((texel >> 5) & 0x7c0) + r) << 6;
                    out <<= 5;
                    out |= *(Var6ca12c + ((texel & 0x1f) << 6) + b);
                    *(pDestination + 4) = out;
                    u += du;
                    v += dv;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    out = *(Var6ca1a8 + (texel & 0x7c0) + g);
                    out |= *(Var6ca12c + ((texel >> 5) & 0x7c0) + r) << 6;
                    out <<= 5;
                    out |= *(Var6ca12c + ((texel & 0x1f) << 6) + b);
                    *(pDestination + 5) = out;
                    u += du;
                    v += dv;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    out = *(Var6ca1a8 + (texel & 0x7c0) + g);
                    out |= *(Var6ca12c + ((texel >> 5) & 0x7c0) + r) << 6;
                    out <<= 5;
                    out |= *(Var6ca12c + ((texel & 0x1f) << 6) + b);
                    *(pDestination + 6) = out;
                    u += du;
                    v += dv;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    out = *(Var6ca1a8 + (texel & 0x7c0) + g);
                    out |= *(Var6ca12c + ((texel >> 5) & 0x7c0) + r) << 6;
                    out <<= 5;
                    out |= *(Var6ca12c + ((texel & 0x1f) << 6) + b);
                    *(pDestination + 7) = out;
                    u += du;
                    v += dv;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    out = *(Var6ca1a8 + (texel & 0x7c0) + g);
                    out |= *(Var6ca12c + ((texel >> 5) & 0x7c0) + r) << 6;
                    out <<= 5;
                    out |= *(Var6ca12c + ((texel & 0x1f) << 6) + b);
                    *(pDestination + 8) = out;
                    u += du;
                    v += dv;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    out = *(Var6ca1a8 + (texel & 0x7c0) + g);
                    out |= *(Var6ca12c + ((texel >> 5) & 0x7c0) + r) << 6;
                    out <<= 5;
                    out |= *(Var6ca12c + ((texel & 0x1f) << 6) + b);
                    *(pDestination + 9) = out;
                    u += du;
                    v += dv;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    out = *(Var6ca1a8 + (texel & 0x7c0) + g);
                    out |= *(Var6ca12c + ((texel >> 5) & 0x7c0) + r) << 6;
                    out <<= 5;
                    out |= *(Var6ca12c + ((texel & 0x1f) << 6) + b);
                    *(pDestination + 10) = out;
                    u += du;
                    v += dv;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    out = *(Var6ca1a8 + (texel & 0x7c0) + g);
                    out |= *(Var6ca12c + ((texel >> 5) & 0x7c0) + r) << 6;
                    out <<= 5;
                    out |= *(Var6ca12c + ((texel & 0x1f) << 6) + b);
                    *(pDestination + 11) = out;
                    u += du;
                    v += dv;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    out = *(Var6ca1a8 + (texel & 0x7c0) + g);
                    out |= *(Var6ca12c + ((texel >> 5) & 0x7c0) + r) << 6;
                    out <<= 5;
                    out |= *(Var6ca12c + ((texel & 0x1f) << 6) + b);
                    *(pDestination + 12) = out;
                    u += du;
                    v += dv;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    out = *(Var6ca1a8 + (texel & 0x7c0) + g);
                    out |= *(Var6ca12c + ((texel >> 5) & 0x7c0) + r) << 6;
                    out <<= 5;
                    out |= *(Var6ca12c + ((texel & 0x1f) << 6) + b);
                    *(pDestination + 13) = out;
                    u += du;
                    v += dv;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    out = *(Var6ca1a8 + (texel & 0x7c0) + g);
                    out |= *(Var6ca12c + ((texel >> 5) & 0x7c0) + r) << 6;
                    out <<= 5;
                    out |= *(Var6ca12c + ((texel & 0x1f) << 6) + b);
                    *(pDestination + 14) = out;
                    u += du;
                    v += dv;
                }
                {
                    unsigned short out;
                    texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                    out = *(Var6ca1a8 + (texel & 0x7c0) + g);
                    out |= *(Var6ca12c + ((texel >> 5) & 0x7c0) + r) << 6;
                    out <<= 5;
                    out |= *(Var6ca12c + ((texel & 0x1f) << 6) + b);
                    *(pDestination + 15) = out;
                    u += du;
                    v += dv;
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
                        out = *(Var6ca1a8 + (texel & 0x7c0) + g);
                        out |= *(Var6ca12c + ((texel >> 5) & 0x7c0) + r) << 6;
                        out <<= 5;
                        out |= *(Var6ca12c + ((texel & 0x1f) << 6) + b);
                        *(pDestination + 0) = out;
                        u += du;
                        v += dv;
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

}
