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

}
