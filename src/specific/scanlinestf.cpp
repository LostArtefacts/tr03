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

}
