#include "standard.h"
#include "global.h"
#include "drawprimitive.h"
#include "scanlines.h"

extern "C" {

void ScanLinesBlack(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight)
{
    long yline;

    yline = long(Output) + YTable[pLeft->y];

    while (StepLeft->height--) {
        __asm {
            mov ebx, pLeft
            mov ecx, pRight
            mov ebx, [ebx]
            mov edi, yline
            mov ecx, [ecx]
            lea edi, [edi + ebx * 2]
            sub ecx, ebx
            jle done
            xor eax, eax
            test edi, 2
            je aligned
            mov [edi], ax
            add edi, 2
            dec ecx
        aligned:
            mov ebx, ecx
            shr ecx, 1
            rep stosd
            and ebx, 1
            je done
            mov [edi], ax
        done:
        }
        yline += bytepitch;
        StepLeft->x += StepLeft->xstep;
        StepLeft->y++;
        StepLeft->ErrorTerm += StepLeft->Numerator;
        if (StepLeft->ErrorTerm >= StepLeft->Denominator) {
            StepLeft->x++;
            StepLeft->ErrorTerm -= StepLeft->Denominator;
        }
        StepRight->x += StepRight->xstep;
        StepRight->y++;
        StepRight->ErrorTerm += StepRight->Numerator;
        if (StepRight->ErrorTerm >= StepRight->Denominator) {
            StepRight->x++;
            StepRight->ErrorTerm -= StepRight->Denominator;
        }
    }
}

void Func498860(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight)
{
    int Width;
    unsigned short *pDestination;
    long yline;
    short subdivcnt;
    long redleft, greenleft, blueleft;
    long dr, dg, db;
    
    yline = long(Output) + YTable[pLeft->y];
    dr = pLeft->rxtraf;
    dg = pLeft->gxtraf;
    db = pLeft->bxtraf;
    
    while (StepLeft->height--) {
        Width = pRight->x;
        Width -= pLeft->x;
        if (Width > 0) {
            pDestination = (unsigned short *)yline + pLeft->x;
            redleft = pLeft->rf;
            greenleft = pLeft->gf;
            blueleft = pLeft->bf;
            subdivcnt = Width >> 4;
            Width = Width - (subdivcnt << 4);
            while (subdivcnt--) {
                {
                    *(pDestination + 0) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    *(pDestination + 1) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    *(pDestination + 2) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    *(pDestination + 3) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    *(pDestination + 4) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    *(pDestination + 5) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    *(pDestination + 6) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    *(pDestination + 7) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    *(pDestination + 8) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    *(pDestination + 9) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    *(pDestination + 10) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    *(pDestination + 11) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    *(pDestination + 12) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    *(pDestination + 13) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    *(pDestination + 14) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    *(pDestination + 15) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                pDestination += 16;
            }
            for (int n = 0; n < Width; n++) {
                {
                    *(pDestination + 0) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                pDestination++;
            }
        }
        yline += bytepitch;
        StepLeft->x += StepLeft->xstep;
        StepLeft->y++;
        StepLeft->rf += StepLeft->rstepf;
        StepLeft->gf += StepLeft->gstepf;
        StepLeft->bf += StepLeft->bstepf;
        StepLeft->ErrorTerm += StepLeft->Numerator;
        if (StepLeft->ErrorTerm >= StepLeft->Denominator) {
            StepLeft->x++;
            StepLeft->ErrorTerm -= StepLeft->Denominator;
            StepLeft->rf += StepLeft->rxtraf;
            StepLeft->gf += StepLeft->gxtraf;
            StepLeft->bf += StepLeft->bxtraf;
        }
        StepRight->x += StepRight->xstep;
        StepRight->y++;
        StepRight->rf += StepRight->rstepf;
        StepRight->gf += StepRight->gstepf;
        StepRight->bf += StepRight->bstepf;
        StepRight->ErrorTerm += StepRight->Numerator;
        if (StepRight->ErrorTerm >= StepRight->Denominator) {
            StepRight->x++;
            StepRight->ErrorTerm -= StepRight->Denominator;
            StepRight->rf += StepRight->rxtraf;
            StepRight->gf += StepRight->gxtraf;
            StepRight->bf += StepRight->bxtraf;
        }
    }
}

}
