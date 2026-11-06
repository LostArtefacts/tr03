#include "standard.h"
#include "global.h"
#include "drawprimitive.h"
#include "scanlines.h"

extern "C" {

extern unsigned char *Var6ca1f0;

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

void DrawTriangle(D3DTLVERTEX *v1, D3DTLVERTEX *v2, D3DTLVERTEX *v3);

void Func498da0(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight)
{
    int Width;
    unsigned short *pDestination;
    long yline;
    short subdivcnt;
    long redleft, greenleft, blueleft;
    long dr, dg, db;
    unsigned short a;
    double fconv;

    a = 255 - FTOL(Gradients->ooz[0]);
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
                    unsigned short pix, tr, tg, tb;
                    unsigned short ro, go, bo;
                    pix = *(pDestination + 0);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    ro = *(Var6ca1f0 + tr + a) + (redleft >> 19);
                    go = *(Var6ca1f0 + tg + a) + (greenleft >> 19);
                    bo = *(Var6ca1f0 + tb + a) + (blueleft >> 19);
                    if (ro > 31)
                        ro = 31;
                    if (go > 31)
                        go = 31;
                    if (bo > 31)
                        bo = 31;
                    *(pDestination + 0) = ((ro << 5 | go) << 6) | bo;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short pix, tr, tg, tb;
                    unsigned short ro, go, bo;
                    pix = *(pDestination + 1);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    ro = *(Var6ca1f0 + tr + a) + (redleft >> 19);
                    go = *(Var6ca1f0 + tg + a) + (greenleft >> 19);
                    bo = *(Var6ca1f0 + tb + a) + (blueleft >> 19);
                    if (ro > 31)
                        ro = 31;
                    if (go > 31)
                        go = 31;
                    if (bo > 31)
                        bo = 31;
                    *(pDestination + 1) = ((ro << 5 | go) << 6) | bo;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short pix, tr, tg, tb;
                    unsigned short ro, go, bo;
                    pix = *(pDestination + 2);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    ro = *(Var6ca1f0 + tr + a) + (redleft >> 19);
                    go = *(Var6ca1f0 + tg + a) + (greenleft >> 19);
                    bo = *(Var6ca1f0 + tb + a) + (blueleft >> 19);
                    if (ro > 31)
                        ro = 31;
                    if (go > 31)
                        go = 31;
                    if (bo > 31)
                        bo = 31;
                    *(pDestination + 2) = ((ro << 5 | go) << 6) | bo;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short pix, tr, tg, tb;
                    unsigned short ro, go, bo;
                    pix = *(pDestination + 3);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    ro = *(Var6ca1f0 + tr + a) + (redleft >> 19);
                    go = *(Var6ca1f0 + tg + a) + (greenleft >> 19);
                    bo = *(Var6ca1f0 + tb + a) + (blueleft >> 19);
                    if (ro > 31)
                        ro = 31;
                    if (go > 31)
                        go = 31;
                    if (bo > 31)
                        bo = 31;
                    *(pDestination + 3) = ((ro << 5 | go) << 6) | bo;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short pix, tr, tg, tb;
                    unsigned short ro, go, bo;
                    pix = *(pDestination + 4);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    ro = *(Var6ca1f0 + tr + a) + (redleft >> 19);
                    go = *(Var6ca1f0 + tg + a) + (greenleft >> 19);
                    bo = *(Var6ca1f0 + tb + a) + (blueleft >> 19);
                    if (ro > 31)
                        ro = 31;
                    if (go > 31)
                        go = 31;
                    if (bo > 31)
                        bo = 31;
                    *(pDestination + 4) = ((ro << 5 | go) << 6) | bo;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short pix, tr, tg, tb;
                    unsigned short ro, go, bo;
                    pix = *(pDestination + 5);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    ro = *(Var6ca1f0 + tr + a) + (redleft >> 19);
                    go = *(Var6ca1f0 + tg + a) + (greenleft >> 19);
                    bo = *(Var6ca1f0 + tb + a) + (blueleft >> 19);
                    if (ro > 31)
                        ro = 31;
                    if (go > 31)
                        go = 31;
                    if (bo > 31)
                        bo = 31;
                    *(pDestination + 5) = ((ro << 5 | go) << 6) | bo;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short pix, tr, tg, tb;
                    unsigned short ro, go, bo;
                    pix = *(pDestination + 6);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    ro = *(Var6ca1f0 + tr + a) + (redleft >> 19);
                    go = *(Var6ca1f0 + tg + a) + (greenleft >> 19);
                    bo = *(Var6ca1f0 + tb + a) + (blueleft >> 19);
                    if (ro > 31)
                        ro = 31;
                    if (go > 31)
                        go = 31;
                    if (bo > 31)
                        bo = 31;
                    *(pDestination + 6) = ((ro << 5 | go) << 6) | bo;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short pix, tr, tg, tb;
                    unsigned short ro, go, bo;
                    pix = *(pDestination + 7);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    ro = *(Var6ca1f0 + tr + a) + (redleft >> 19);
                    go = *(Var6ca1f0 + tg + a) + (greenleft >> 19);
                    bo = *(Var6ca1f0 + tb + a) + (blueleft >> 19);
                    if (ro > 31)
                        ro = 31;
                    if (go > 31)
                        go = 31;
                    if (bo > 31)
                        bo = 31;
                    *(pDestination + 7) = ((ro << 5 | go) << 6) | bo;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short pix, tr, tg, tb;
                    unsigned short ro, go, bo;
                    pix = *(pDestination + 8);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    ro = *(Var6ca1f0 + tr + a) + (redleft >> 19);
                    go = *(Var6ca1f0 + tg + a) + (greenleft >> 19);
                    bo = *(Var6ca1f0 + tb + a) + (blueleft >> 19);
                    if (ro > 31)
                        ro = 31;
                    if (go > 31)
                        go = 31;
                    if (bo > 31)
                        bo = 31;
                    *(pDestination + 8) = ((ro << 5 | go) << 6) | bo;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short pix, tr, tg, tb;
                    unsigned short ro, go, bo;
                    pix = *(pDestination + 9);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    ro = *(Var6ca1f0 + tr + a) + (redleft >> 19);
                    go = *(Var6ca1f0 + tg + a) + (greenleft >> 19);
                    bo = *(Var6ca1f0 + tb + a) + (blueleft >> 19);
                    if (ro > 31)
                        ro = 31;
                    if (go > 31)
                        go = 31;
                    if (bo > 31)
                        bo = 31;
                    *(pDestination + 9) = ((ro << 5 | go) << 6) | bo;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short pix, tr, tg, tb;
                    unsigned short ro, go, bo;
                    pix = *(pDestination + 10);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    ro = *(Var6ca1f0 + tr + a) + (redleft >> 19);
                    go = *(Var6ca1f0 + tg + a) + (greenleft >> 19);
                    bo = *(Var6ca1f0 + tb + a) + (blueleft >> 19);
                    if (ro > 31)
                        ro = 31;
                    if (go > 31)
                        go = 31;
                    if (bo > 31)
                        bo = 31;
                    *(pDestination + 10) = ((ro << 5 | go) << 6) | bo;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short pix, tr, tg, tb;
                    unsigned short ro, go, bo;
                    pix = *(pDestination + 11);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    ro = *(Var6ca1f0 + tr + a) + (redleft >> 19);
                    go = *(Var6ca1f0 + tg + a) + (greenleft >> 19);
                    bo = *(Var6ca1f0 + tb + a) + (blueleft >> 19);
                    if (ro > 31)
                        ro = 31;
                    if (go > 31)
                        go = 31;
                    if (bo > 31)
                        bo = 31;
                    *(pDestination + 11) = ((ro << 5 | go) << 6) | bo;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short pix, tr, tg, tb;
                    unsigned short ro, go, bo;
                    pix = *(pDestination + 12);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    ro = *(Var6ca1f0 + tr + a) + (redleft >> 19);
                    go = *(Var6ca1f0 + tg + a) + (greenleft >> 19);
                    bo = *(Var6ca1f0 + tb + a) + (blueleft >> 19);
                    if (ro > 31)
                        ro = 31;
                    if (go > 31)
                        go = 31;
                    if (bo > 31)
                        bo = 31;
                    *(pDestination + 12) = ((ro << 5 | go) << 6) | bo;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short pix, tr, tg, tb;
                    unsigned short ro, go, bo;
                    pix = *(pDestination + 13);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    ro = *(Var6ca1f0 + tr + a) + (redleft >> 19);
                    go = *(Var6ca1f0 + tg + a) + (greenleft >> 19);
                    bo = *(Var6ca1f0 + tb + a) + (blueleft >> 19);
                    if (ro > 31)
                        ro = 31;
                    if (go > 31)
                        go = 31;
                    if (bo > 31)
                        bo = 31;
                    *(pDestination + 13) = ((ro << 5 | go) << 6) | bo;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short pix, tr, tg, tb;
                    unsigned short ro, go, bo;
                    pix = *(pDestination + 14);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    ro = *(Var6ca1f0 + tr + a) + (redleft >> 19);
                    go = *(Var6ca1f0 + tg + a) + (greenleft >> 19);
                    bo = *(Var6ca1f0 + tb + a) + (blueleft >> 19);
                    if (ro > 31)
                        ro = 31;
                    if (go > 31)
                        go = 31;
                    if (bo > 31)
                        bo = 31;
                    *(pDestination + 14) = ((ro << 5 | go) << 6) | bo;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                {
                    unsigned short pix, tr, tg, tb;
                    unsigned short ro, go, bo;
                    pix = *(pDestination + 15);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    ro = *(Var6ca1f0 + tr + a) + (redleft >> 19);
                    go = *(Var6ca1f0 + tg + a) + (greenleft >> 19);
                    bo = *(Var6ca1f0 + tb + a) + (blueleft >> 19);
                    if (ro > 31)
                        ro = 31;
                    if (go > 31)
                        go = 31;
                    if (bo > 31)
                        bo = 31;
                    *(pDestination + 15) = ((ro << 5 | go) << 6) | bo;
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                pDestination += 16;
            }
            for (int n = 0; n < Width; n++) {
                {
                    unsigned short pix, tr, tg, tb;
                    unsigned short ro, go, bo;
                    pix = *(pDestination + 0);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    ro = *(Var6ca1f0 + tr + a) + (redleft >> 19);
                    go = *(Var6ca1f0 + tg + a) + (greenleft >> 19);
                    bo = *(Var6ca1f0 + tb + a) + (blueleft >> 19);
                    if (ro > 31)
                        ro = 31;
                    if (go > 31)
                        go = 31;
                    if (bo > 31)
                        bo = 31;
                    *(pDestination + 0) = ((ro << 5 | go) << 6) | bo;
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

void Func499de0(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight)
{
    int Width;
    unsigned short *pDestination;
    long yline;
    short subdivcnt;
    unsigned short a;
    double fconv;

    a = 255 - FTOL(Gradients->ooz[0]);
    yline = long(Output) + YTable[pLeft->y];

    while (StepLeft->height--) {
        Width = pRight->x;
        Width -= pLeft->x;
        if (Width > 0) {
            pDestination = (unsigned short *)yline + pLeft->x;
            subdivcnt = Width >> 4;
            Width = Width - (subdivcnt << 4);
            while (subdivcnt--) {
                {
                    unsigned short pix, tr, tg, tb;
                    pix = *(pDestination + 0);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    *(pDestination + 0) = ((*(Var6ca1f0 + tr + a) << 5 | *(Var6ca1f0 + tg + a)) << 6) | *(Var6ca1f0 + tb + a);
                }
                {
                    unsigned short pix, tr, tg, tb;
                    pix = *(pDestination + 1);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    *(pDestination + 1) = ((*(Var6ca1f0 + tr + a) << 5 | *(Var6ca1f0 + tg + a)) << 6) | *(Var6ca1f0 + tb + a);
                }
                {
                    unsigned short pix, tr, tg, tb;
                    pix = *(pDestination + 2);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    *(pDestination + 2) = ((*(Var6ca1f0 + tr + a) << 5 | *(Var6ca1f0 + tg + a)) << 6) | *(Var6ca1f0 + tb + a);
                }
                {
                    unsigned short pix, tr, tg, tb;
                    pix = *(pDestination + 3);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    *(pDestination + 3) = ((*(Var6ca1f0 + tr + a) << 5 | *(Var6ca1f0 + tg + a)) << 6) | *(Var6ca1f0 + tb + a);
                }
                {
                    unsigned short pix, tr, tg, tb;
                    pix = *(pDestination + 4);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    *(pDestination + 4) = ((*(Var6ca1f0 + tr + a) << 5 | *(Var6ca1f0 + tg + a)) << 6) | *(Var6ca1f0 + tb + a);
                }
                {
                    unsigned short pix, tr, tg, tb;
                    pix = *(pDestination + 5);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    *(pDestination + 5) = ((*(Var6ca1f0 + tr + a) << 5 | *(Var6ca1f0 + tg + a)) << 6) | *(Var6ca1f0 + tb + a);
                }
                {
                    unsigned short pix, tr, tg, tb;
                    pix = *(pDestination + 6);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    *(pDestination + 6) = ((*(Var6ca1f0 + tr + a) << 5 | *(Var6ca1f0 + tg + a)) << 6) | *(Var6ca1f0 + tb + a);
                }
                {
                    unsigned short pix, tr, tg, tb;
                    pix = *(pDestination + 7);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    *(pDestination + 7) = ((*(Var6ca1f0 + tr + a) << 5 | *(Var6ca1f0 + tg + a)) << 6) | *(Var6ca1f0 + tb + a);
                }
                {
                    unsigned short pix, tr, tg, tb;
                    pix = *(pDestination + 8);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    *(pDestination + 8) = ((*(Var6ca1f0 + tr + a) << 5 | *(Var6ca1f0 + tg + a)) << 6) | *(Var6ca1f0 + tb + a);
                }
                {
                    unsigned short pix, tr, tg, tb;
                    pix = *(pDestination + 9);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    *(pDestination + 9) = ((*(Var6ca1f0 + tr + a) << 5 | *(Var6ca1f0 + tg + a)) << 6) | *(Var6ca1f0 + tb + a);
                }
                {
                    unsigned short pix, tr, tg, tb;
                    pix = *(pDestination + 10);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    *(pDestination + 10) = ((*(Var6ca1f0 + tr + a) << 5 | *(Var6ca1f0 + tg + a)) << 6) | *(Var6ca1f0 + tb + a);
                }
                {
                    unsigned short pix, tr, tg, tb;
                    pix = *(pDestination + 11);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    *(pDestination + 11) = ((*(Var6ca1f0 + tr + a) << 5 | *(Var6ca1f0 + tg + a)) << 6) | *(Var6ca1f0 + tb + a);
                }
                {
                    unsigned short pix, tr, tg, tb;
                    pix = *(pDestination + 12);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    *(pDestination + 12) = ((*(Var6ca1f0 + tr + a) << 5 | *(Var6ca1f0 + tg + a)) << 6) | *(Var6ca1f0 + tb + a);
                }
                {
                    unsigned short pix, tr, tg, tb;
                    pix = *(pDestination + 13);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    *(pDestination + 13) = ((*(Var6ca1f0 + tr + a) << 5 | *(Var6ca1f0 + tg + a)) << 6) | *(Var6ca1f0 + tb + a);
                }
                {
                    unsigned short pix, tr, tg, tb;
                    pix = *(pDestination + 14);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    *(pDestination + 14) = ((*(Var6ca1f0 + tr + a) << 5 | *(Var6ca1f0 + tg + a)) << 6) | *(Var6ca1f0 + tb + a);
                }
                {
                    unsigned short pix, tr, tg, tb;
                    pix = *(pDestination + 15);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    *(pDestination + 15) = ((*(Var6ca1f0 + tr + a) << 5 | *(Var6ca1f0 + tg + a)) << 6) | *(Var6ca1f0 + tb + a);
                }
                pDestination += 16;
            }
            for (int n = 0; n < Width; n++) {
                {
                    unsigned short pix, tr, tg, tb;
                    pix = *(pDestination + 0);
                    tr = (pix >> 3) & 0x1f00;
                    tg = (pix & 0x7c0) << 2;
                    tb = (pix & 0x1f) << 8;
                    *(pDestination + 0) = ((*(Var6ca1f0 + tr + a) << 5 | *(Var6ca1f0 + tg + a)) << 6) | *(Var6ca1f0 + tb + a);
                }
                pDestination++;
            }
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

void Func49a510(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight)
{
    int Width;
    unsigned short *pDestination;
    long yline;
    short subdivcnt;

    yline = long(Output) + YTable[pLeft->y];

    while (StepLeft->height--) {
        Width = pRight->x;
        Width -= pLeft->x;
        if (Width > 0) {
            pDestination = (unsigned short *)yline + pLeft->x;
            subdivcnt = Width >> 4;
            Width = Width - (subdivcnt << 4);
            while (subdivcnt--) {
                if (pLeft->y & 1) {
                    if (!((long)(pDestination + 0) & 2))
                        *(pDestination + 0) = 0;
                    if (!((long)(pDestination + 1) & 2))
                        *(pDestination + 1) = 0;
                    if (!((long)(pDestination + 2) & 2))
                        *(pDestination + 2) = 0;
                    if (!((long)(pDestination + 3) & 2))
                        *(pDestination + 3) = 0;
                    if (!((long)(pDestination + 4) & 2))
                        *(pDestination + 4) = 0;
                    if (!((long)(pDestination + 5) & 2))
                        *(pDestination + 5) = 0;
                    if (!((long)(pDestination + 6) & 2))
                        *(pDestination + 6) = 0;
                    if (!((long)(pDestination + 7) & 2))
                        *(pDestination + 7) = 0;
                    if (!((long)(pDestination + 8) & 2))
                        *(pDestination + 8) = 0;
                    if (!((long)(pDestination + 9) & 2))
                        *(pDestination + 9) = 0;
                    if (!((long)(pDestination + 10) & 2))
                        *(pDestination + 10) = 0;
                    if (!((long)(pDestination + 11) & 2))
                        *(pDestination + 11) = 0;
                    if (!((long)(pDestination + 12) & 2))
                        *(pDestination + 12) = 0;
                    if (!((long)(pDestination + 13) & 2))
                        *(pDestination + 13) = 0;
                    if (!((long)(pDestination + 14) & 2))
                        *(pDestination + 14) = 0;
                    if (!((long)(pDestination + 15) & 2))
                        *(pDestination + 15) = 0;
                } else {
                    if ((long)(pDestination + 0) & 2)
                        *(pDestination + 0) = 0;
                    if ((long)(pDestination + 1) & 2)
                        *(pDestination + 1) = 0;
                    if ((long)(pDestination + 2) & 2)
                        *(pDestination + 2) = 0;
                    if ((long)(pDestination + 3) & 2)
                        *(pDestination + 3) = 0;
                    if ((long)(pDestination + 4) & 2)
                        *(pDestination + 4) = 0;
                    if ((long)(pDestination + 5) & 2)
                        *(pDestination + 5) = 0;
                    if ((long)(pDestination + 6) & 2)
                        *(pDestination + 6) = 0;
                    if ((long)(pDestination + 7) & 2)
                        *(pDestination + 7) = 0;
                    if ((long)(pDestination + 8) & 2)
                        *(pDestination + 8) = 0;
                    if ((long)(pDestination + 9) & 2)
                        *(pDestination + 9) = 0;
                    if ((long)(pDestination + 10) & 2)
                        *(pDestination + 10) = 0;
                    if ((long)(pDestination + 11) & 2)
                        *(pDestination + 11) = 0;
                    if ((long)(pDestination + 12) & 2)
                        *(pDestination + 12) = 0;
                    if ((long)(pDestination + 13) & 2)
                        *(pDestination + 13) = 0;
                    if ((long)(pDestination + 14) & 2)
                        *(pDestination + 14) = 0;
                    if ((long)(pDestination + 15) & 2)
                        *(pDestination + 15) = 0;
                }
                pDestination += 16;
            }
            if (pLeft->y & 1) {
                for (int n = 0; n < Width; n++) {
                    if (!((long)(pDestination + 0) & 2))
                        *(pDestination + 0) = 0;
                    pDestination++;
                }
            } else {
                for (int n = 0; n < Width; n++) {
                    if ((long)(pDestination + 0) & 2)
                        *(pDestination + 0) = 0;
                    pDestination++;
                }
            }
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

void Func49a790(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight)
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
                if (pLeft->y & 1) {
                    if (!((long)(pDestination + 0) >> 1) & 1) {
                        *(pDestination + 0) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if (!((long)(pDestination + 1) >> 1) & 1) {
                        *(pDestination + 1) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if (!((long)(pDestination + 2) >> 1) & 1) {
                        *(pDestination + 2) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if (!((long)(pDestination + 3) >> 1) & 1) {
                        *(pDestination + 3) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if (!((long)(pDestination + 4) >> 1) & 1) {
                        *(pDestination + 4) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if (!((long)(pDestination + 5) >> 1) & 1) {
                        *(pDestination + 5) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if (!((long)(pDestination + 6) >> 1) & 1) {
                        *(pDestination + 6) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if (!((long)(pDestination + 7) >> 1) & 1) {
                        *(pDestination + 7) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if (!((long)(pDestination + 8) >> 1) & 1) {
                        *(pDestination + 8) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if (!((long)(pDestination + 9) >> 1) & 1) {
                        *(pDestination + 9) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if (!((long)(pDestination + 10) >> 1) & 1) {
                        *(pDestination + 10) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if (!((long)(pDestination + 11) >> 1) & 1) {
                        *(pDestination + 11) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if (!((long)(pDestination + 12) >> 1) & 1) {
                        *(pDestination + 12) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if (!((long)(pDestination + 13) >> 1) & 1) {
                        *(pDestination + 13) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if (!((long)(pDestination + 14) >> 1) & 1) {
                        *(pDestination + 14) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if (!((long)(pDestination + 15) >> 1) & 1) {
                        *(pDestination + 15) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                } else {
                    if ((long)(pDestination + 0) & 2) {
                        *(pDestination + 0) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if ((long)(pDestination + 1) & 2) {
                        *(pDestination + 1) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if ((long)(pDestination + 2) & 2) {
                        *(pDestination + 2) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if ((long)(pDestination + 3) & 2) {
                        *(pDestination + 3) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if ((long)(pDestination + 4) & 2) {
                        *(pDestination + 4) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if ((long)(pDestination + 5) & 2) {
                        *(pDestination + 5) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if ((long)(pDestination + 6) & 2) {
                        *(pDestination + 6) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if ((long)(pDestination + 7) & 2) {
                        *(pDestination + 7) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if ((long)(pDestination + 8) & 2) {
                        *(pDestination + 8) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if ((long)(pDestination + 9) & 2) {
                        *(pDestination + 9) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if ((long)(pDestination + 10) & 2) {
                        *(pDestination + 10) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if ((long)(pDestination + 11) & 2) {
                        *(pDestination + 11) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if ((long)(pDestination + 12) & 2) {
                        *(pDestination + 12) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if ((long)(pDestination + 13) & 2) {
                        *(pDestination + 13) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if ((long)(pDestination + 14) & 2) {
                        *(pDestination + 14) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    if ((long)(pDestination + 15) & 2) {
                        *(pDestination + 15) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                }
                pDestination += 16;
            }
            if (pLeft->y & 1) {
                for (int n = 0; n < Width; n++) {
                    if (!((long)(pDestination + 0) >> 1) & 1) {
                        *(pDestination + 0) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    pDestination++;
                }
            } else {
                for (int n = 0; n < Width; n++) {
                    if ((long)(pDestination + 0) & 2) {
                        *(pDestination + 0) = ((redleft >> 8) & 0xf81f | greenleft >> 13) & 0xffe0 | blueleft >> 19;
                    }
                    redleft += dr;
                    greenleft += dg;
                    blueleft += db;
                    pDestination++;
                }
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
