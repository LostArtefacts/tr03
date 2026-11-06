#include "standard.h"
#include "global.h"
#include "drawprimitive.h"
#include "scanlines.h"

extern "C" {

void ScanLinesTDecalAffine(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight)
{
    int Width;
    unsigned short *pDestination;
    long u, v, du, dv;
    long yline;
    short subdivcnt;
    short texel;

    yline = long(Output) + YTable[pLeft->y];
    du = pLeft->uozxtraf;
    dv = pLeft->vozxtraf;

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
                texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                if (texel != 0)
                    *(pDestination + 0) = texel;
                u += du;
                v += dv;
                texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                if (texel != 0)
                    *(pDestination + 1) = texel;
                u += du;
                v += dv;
                texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                if (texel != 0)
                    *(pDestination + 2) = texel;
                u += du;
                v += dv;
                texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                if (texel != 0)
                    *(pDestination + 3) = texel;
                u += du;
                v += dv;
                texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                if (texel != 0)
                    *(pDestination + 4) = texel;
                u += du;
                v += dv;
                texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                if (texel != 0)
                    *(pDestination + 5) = texel;
                u += du;
                v += dv;
                texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                if (texel != 0)
                    *(pDestination + 6) = texel;
                u += du;
                v += dv;
                texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                if (texel != 0)
                    *(pDestination + 7) = texel;
                u += du;
                v += dv;
                texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                if (texel != 0)
                    *(pDestination + 8) = texel;
                u += du;
                v += dv;
                texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                if (texel != 0)
                    *(pDestination + 9) = texel;
                u += du;
                v += dv;
                texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                if (texel != 0)
                    *(pDestination + 10) = texel;
                u += du;
                v += dv;
                texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                if (texel != 0)
                    *(pDestination + 11) = texel;
                u += du;
                v += dv;
                texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                if (texel != 0)
                    *(pDestination + 12) = texel;
                u += du;
                v += dv;
                texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                if (texel != 0)
                    *(pDestination + 13) = texel;
                u += du;
                v += dv;
                texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                if (texel != 0)
                    *(pDestination + 14) = texel;
                u += du;
                v += dv;
                texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                if (texel != 0)
                    *(pDestination + 15) = texel;
                u += du;
                v += dv;
                pDestination += 16;
            }
            for (int n = 0; n < Width; n++) {
                texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                if (texel != 0)
                    *(pDestination + 0) = texel;
                u += du;
                v += dv;
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
