#include "standard.h"
#include "global.h"
#include "drawprimitive.h"
#include "scanlines.h"

extern "C" {

static long u, v, du, dv;
static long redleft, greenleft, blueleft;
static long dr, dg, db;
static unsigned short *pDestination;

void ScanLinesTG(GRADIENT *Gradients, EDGE *pLeft, EDGE *pRight, EDGE *StepLeft, EDGE *StepRight)
{
    int Width;
    int subdivcnt;
    int pixelsleft;
    float oozr, uozr, vozr;
    float ZLeft, ULeft, VLeft;
    float ZRight, URight, VRight;
    float oozdx, uozdx, vozdx;
    float one = 1.0f;
    long yline;
    short texel;
    double fconv;

    dr = pLeft->rxtraf;
    dg = pLeft->gxtraf;
    db = pLeft->bxtraf;
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
                    push ebp
                    mov eax, dword ptr [u]
                    sar eax, 16
                    mov ebx, dword ptr [v]
                    sar ebx, 8
                    mov edx, [redleft]
                    mov esi, [TextureHandle]
                    and ebx, 0xffffff00
                    mov edi, [pDestination]
                    add ebx, eax
                    sar edx, 18
                    mov eax, [greenleft]
                    mov cx, word ptr [esi + ebx * 2]
                    mov esi, [blueleft]
                    sar eax, 18
                    mov ebx, ecx
                    shr ecx, 5
                    mov ebp, [Var6ca12c]
                    and ecx, 0x7c0
                    add ebp, edx
                    movzx edx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca1a8]
                    shl edx, 11
                    mov ecx, ebx
                    and ecx, 0x7c0
                    add ebp, eax
                    movzx ecx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca12c]
                    shl ecx, 5
                    and ebx, 0x1f
                    sar esi, 18
                    or edx, ecx
                    shl ebx, 6
                    mov eax, dword ptr [du]
                    add ebp, esi
                    mov ecx, dword ptr [u]
                    movzx esi, byte ptr [ebp + ebx]
                    mov ebx, dword ptr [dv]
                    or edx, esi
                    mov [edi + 0], dx
                    mov edx, dword ptr [v]
                    add ecx, eax
                    add edx, ebx
                    mov [u], ecx
                    mov [v], edx
                    mov eax, dword ptr [dr]
                    mov ecx, dword ptr [redleft]
                    mov esi, dword ptr [db]
                    mov edx, dword ptr [greenleft]
                    mov ebx, dword ptr [dg]
                    mov edi, dword ptr [blueleft]
                    add ecx, eax
                    mov [redleft], ecx
                    add edx, ebx
                    mov [greenleft], edx
                    add edi, esi
                    mov [blueleft], edi
                    mov eax, dword ptr [u]
                    sar eax, 16
                    mov ebx, dword ptr [v]
                    sar ebx, 8
                    mov edx, [redleft]
                    mov esi, [TextureHandle]
                    and ebx, 0xffffff00
                    mov edi, [pDestination]
                    add ebx, eax
                    sar edx, 18
                    mov eax, [greenleft]
                    mov cx, word ptr [esi + ebx * 2]
                    mov esi, [blueleft]
                    sar eax, 18
                    mov ebx, ecx
                    shr ecx, 5
                    mov ebp, [Var6ca12c]
                    and ecx, 0x7c0
                    add ebp, edx
                    movzx edx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca1a8]
                    shl edx, 11
                    mov ecx, ebx
                    and ecx, 0x7c0
                    add ebp, eax
                    movzx ecx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca12c]
                    shl ecx, 5
                    and ebx, 0x1f
                    sar esi, 18
                    or edx, ecx
                    shl ebx, 6
                    mov eax, dword ptr [du]
                    add ebp, esi
                    mov ecx, dword ptr [u]
                    movzx esi, byte ptr [ebp + ebx]
                    mov ebx, dword ptr [dv]
                    or edx, esi
                    mov [edi + 2], dx
                    mov edx, dword ptr [v]
                    add ecx, eax
                    add edx, ebx
                    mov [u], ecx
                    mov [v], edx
                    mov eax, dword ptr [dr]
                    mov ecx, dword ptr [redleft]
                    mov esi, dword ptr [db]
                    mov edx, dword ptr [greenleft]
                    mov ebx, dword ptr [dg]
                    mov edi, dword ptr [blueleft]
                    add ecx, eax
                    mov [redleft], ecx
                    add edx, ebx
                    mov [greenleft], edx
                    add edi, esi
                    mov [blueleft], edi
                    mov eax, dword ptr [u]
                    sar eax, 16
                    mov ebx, dword ptr [v]
                    sar ebx, 8
                    mov edx, [redleft]
                    mov esi, [TextureHandle]
                    and ebx, 0xffffff00
                    mov edi, [pDestination]
                    add ebx, eax
                    sar edx, 18
                    mov eax, [greenleft]
                    mov cx, word ptr [esi + ebx * 2]
                    mov esi, [blueleft]
                    sar eax, 18
                    mov ebx, ecx
                    shr ecx, 5
                    mov ebp, [Var6ca12c]
                    and ecx, 0x7c0
                    add ebp, edx
                    movzx edx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca1a8]
                    shl edx, 11
                    mov ecx, ebx
                    and ecx, 0x7c0
                    add ebp, eax
                    movzx ecx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca12c]
                    shl ecx, 5
                    and ebx, 0x1f
                    sar esi, 18
                    or edx, ecx
                    shl ebx, 6
                    mov eax, dword ptr [du]
                    add ebp, esi
                    mov ecx, dword ptr [u]
                    movzx esi, byte ptr [ebp + ebx]
                    mov ebx, dword ptr [dv]
                    or edx, esi
                    mov [edi + 4], dx
                    mov edx, dword ptr [v]
                    add ecx, eax
                    add edx, ebx
                    mov [u], ecx
                    mov [v], edx
                    mov eax, dword ptr [dr]
                    mov ecx, dword ptr [redleft]
                    mov esi, dword ptr [db]
                    mov edx, dword ptr [greenleft]
                    mov ebx, dword ptr [dg]
                    mov edi, dword ptr [blueleft]
                    add ecx, eax
                    mov [redleft], ecx
                    add edx, ebx
                    mov [greenleft], edx
                    add edi, esi
                    mov [blueleft], edi
                    mov eax, dword ptr [u]
                    sar eax, 16
                    mov ebx, dword ptr [v]
                    sar ebx, 8
                    mov edx, [redleft]
                    mov esi, [TextureHandle]
                    and ebx, 0xffffff00
                    mov edi, [pDestination]
                    add ebx, eax
                    sar edx, 18
                    mov eax, [greenleft]
                    mov cx, word ptr [esi + ebx * 2]
                    mov esi, [blueleft]
                    sar eax, 18
                    mov ebx, ecx
                    shr ecx, 5
                    mov ebp, [Var6ca12c]
                    and ecx, 0x7c0
                    add ebp, edx
                    movzx edx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca1a8]
                    shl edx, 11
                    mov ecx, ebx
                    and ecx, 0x7c0
                    add ebp, eax
                    movzx ecx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca12c]
                    shl ecx, 5
                    and ebx, 0x1f
                    sar esi, 18
                    or edx, ecx
                    shl ebx, 6
                    mov eax, dword ptr [du]
                    add ebp, esi
                    mov ecx, dword ptr [u]
                    movzx esi, byte ptr [ebp + ebx]
                    mov ebx, dword ptr [dv]
                    or edx, esi
                    mov [edi + 6], dx
                    mov edx, dword ptr [v]
                    add ecx, eax
                    add edx, ebx
                    mov [u], ecx
                    mov [v], edx
                    mov eax, dword ptr [dr]
                    mov ecx, dword ptr [redleft]
                    mov esi, dword ptr [db]
                    mov edx, dword ptr [greenleft]
                    mov ebx, dword ptr [dg]
                    mov edi, dword ptr [blueleft]
                    add ecx, eax
                    mov [redleft], ecx
                    add edx, ebx
                    mov [greenleft], edx
                    add edi, esi
                    mov [blueleft], edi
                    mov eax, dword ptr [u]
                    sar eax, 16
                    mov ebx, dword ptr [v]
                    sar ebx, 8
                    mov edx, [redleft]
                    mov esi, [TextureHandle]
                    and ebx, 0xffffff00
                    mov edi, [pDestination]
                    add ebx, eax
                    sar edx, 18
                    mov eax, [greenleft]
                    mov cx, word ptr [esi + ebx * 2]
                    mov esi, [blueleft]
                    sar eax, 18
                    mov ebx, ecx
                    shr ecx, 5
                    mov ebp, [Var6ca12c]
                    and ecx, 0x7c0
                    add ebp, edx
                    movzx edx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca1a8]
                    shl edx, 11
                    mov ecx, ebx
                    and ecx, 0x7c0
                    add ebp, eax
                    movzx ecx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca12c]
                    shl ecx, 5
                    and ebx, 0x1f
                    sar esi, 18
                    or edx, ecx
                    shl ebx, 6
                    mov eax, dword ptr [du]
                    add ebp, esi
                    mov ecx, dword ptr [u]
                    movzx esi, byte ptr [ebp + ebx]
                    mov ebx, dword ptr [dv]
                    or edx, esi
                    mov [edi + 8], dx
                    mov edx, dword ptr [v]
                    add ecx, eax
                    add edx, ebx
                    mov [u], ecx
                    mov [v], edx
                    mov eax, dword ptr [dr]
                    mov ecx, dword ptr [redleft]
                    mov esi, dword ptr [db]
                    mov edx, dword ptr [greenleft]
                    mov ebx, dword ptr [dg]
                    mov edi, dword ptr [blueleft]
                    add ecx, eax
                    mov [redleft], ecx
                    add edx, ebx
                    mov [greenleft], edx
                    add edi, esi
                    mov [blueleft], edi
                    mov eax, dword ptr [u]
                    sar eax, 16
                    mov ebx, dword ptr [v]
                    sar ebx, 8
                    mov edx, [redleft]
                    mov esi, [TextureHandle]
                    and ebx, 0xffffff00
                    mov edi, [pDestination]
                    add ebx, eax
                    sar edx, 18
                    mov eax, [greenleft]
                    mov cx, word ptr [esi + ebx * 2]
                    mov esi, [blueleft]
                    sar eax, 18
                    mov ebx, ecx
                    shr ecx, 5
                    mov ebp, [Var6ca12c]
                    and ecx, 0x7c0
                    add ebp, edx
                    movzx edx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca1a8]
                    shl edx, 11
                    mov ecx, ebx
                    and ecx, 0x7c0
                    add ebp, eax
                    movzx ecx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca12c]
                    shl ecx, 5
                    and ebx, 0x1f
                    sar esi, 18
                    or edx, ecx
                    shl ebx, 6
                    mov eax, dword ptr [du]
                    add ebp, esi
                    mov ecx, dword ptr [u]
                    movzx esi, byte ptr [ebp + ebx]
                    mov ebx, dword ptr [dv]
                    or edx, esi
                    mov [edi + 10], dx
                    mov edx, dword ptr [v]
                    add ecx, eax
                    add edx, ebx
                    mov [u], ecx
                    mov [v], edx
                    mov eax, dword ptr [dr]
                    mov ecx, dword ptr [redleft]
                    mov esi, dword ptr [db]
                    mov edx, dword ptr [greenleft]
                    mov ebx, dword ptr [dg]
                    mov edi, dword ptr [blueleft]
                    add ecx, eax
                    mov [redleft], ecx
                    add edx, ebx
                    mov [greenleft], edx
                    add edi, esi
                    mov [blueleft], edi
                    mov eax, dword ptr [u]
                    sar eax, 16
                    mov ebx, dword ptr [v]
                    sar ebx, 8
                    mov edx, [redleft]
                    mov esi, [TextureHandle]
                    and ebx, 0xffffff00
                    mov edi, [pDestination]
                    add ebx, eax
                    sar edx, 18
                    mov eax, [greenleft]
                    mov cx, word ptr [esi + ebx * 2]
                    mov esi, [blueleft]
                    sar eax, 18
                    mov ebx, ecx
                    shr ecx, 5
                    mov ebp, [Var6ca12c]
                    and ecx, 0x7c0
                    add ebp, edx
                    movzx edx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca1a8]
                    shl edx, 11
                    mov ecx, ebx
                    and ecx, 0x7c0
                    add ebp, eax
                    movzx ecx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca12c]
                    shl ecx, 5
                    and ebx, 0x1f
                    sar esi, 18
                    or edx, ecx
                    shl ebx, 6
                    mov eax, dword ptr [du]
                    add ebp, esi
                    mov ecx, dword ptr [u]
                    movzx esi, byte ptr [ebp + ebx]
                    mov ebx, dword ptr [dv]
                    or edx, esi
                    mov [edi + 12], dx
                    mov edx, dword ptr [v]
                    add ecx, eax
                    add edx, ebx
                    mov [u], ecx
                    mov [v], edx
                    mov eax, dword ptr [dr]
                    mov ecx, dword ptr [redleft]
                    mov esi, dword ptr [db]
                    mov edx, dword ptr [greenleft]
                    mov ebx, dword ptr [dg]
                    mov edi, dword ptr [blueleft]
                    add ecx, eax
                    mov [redleft], ecx
                    add edx, ebx
                    mov [greenleft], edx
                    add edi, esi
                    mov [blueleft], edi
                    mov eax, dword ptr [u]
                    sar eax, 16
                    mov ebx, dword ptr [v]
                    sar ebx, 8
                    mov edx, [redleft]
                    mov esi, [TextureHandle]
                    and ebx, 0xffffff00
                    mov edi, [pDestination]
                    add ebx, eax
                    sar edx, 18
                    mov eax, [greenleft]
                    mov cx, word ptr [esi + ebx * 2]
                    mov esi, [blueleft]
                    sar eax, 18
                    mov ebx, ecx
                    shr ecx, 5
                    mov ebp, [Var6ca12c]
                    and ecx, 0x7c0
                    add ebp, edx
                    movzx edx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca1a8]
                    shl edx, 11
                    mov ecx, ebx
                    and ecx, 0x7c0
                    add ebp, eax
                    movzx ecx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca12c]
                    shl ecx, 5
                    and ebx, 0x1f
                    sar esi, 18
                    or edx, ecx
                    shl ebx, 6
                    mov eax, dword ptr [du]
                    add ebp, esi
                    mov ecx, dword ptr [u]
                    movzx esi, byte ptr [ebp + ebx]
                    mov ebx, dword ptr [dv]
                    or edx, esi
                    mov [edi + 14], dx
                    mov edx, dword ptr [v]
                    add ecx, eax
                    add edx, ebx
                    mov [u], ecx
                    mov [v], edx
                    mov eax, dword ptr [dr]
                    mov ecx, dword ptr [redleft]
                    mov esi, dword ptr [db]
                    mov edx, dword ptr [greenleft]
                    mov ebx, dword ptr [dg]
                    mov edi, dword ptr [blueleft]
                    add ecx, eax
                    mov [redleft], ecx
                    add edx, ebx
                    mov [greenleft], edx
                    add edi, esi
                    mov [blueleft], edi
                    mov eax, dword ptr [u]
                    sar eax, 16
                    mov ebx, dword ptr [v]
                    sar ebx, 8
                    mov edx, [redleft]
                    mov esi, [TextureHandle]
                    and ebx, 0xffffff00
                    mov edi, [pDestination]
                    add ebx, eax
                    sar edx, 18
                    mov eax, [greenleft]
                    mov cx, word ptr [esi + ebx * 2]
                    mov esi, [blueleft]
                    sar eax, 18
                    mov ebx, ecx
                    shr ecx, 5
                    mov ebp, [Var6ca12c]
                    and ecx, 0x7c0
                    add ebp, edx
                    movzx edx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca1a8]
                    shl edx, 11
                    mov ecx, ebx
                    and ecx, 0x7c0
                    add ebp, eax
                    movzx ecx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca12c]
                    shl ecx, 5
                    and ebx, 0x1f
                    sar esi, 18
                    or edx, ecx
                    shl ebx, 6
                    mov eax, dword ptr [du]
                    add ebp, esi
                    mov ecx, dword ptr [u]
                    movzx esi, byte ptr [ebp + ebx]
                    mov ebx, dword ptr [dv]
                    or edx, esi
                    mov [edi + 16], dx
                    mov edx, dword ptr [v]
                    add ecx, eax
                    add edx, ebx
                    mov [u], ecx
                    mov [v], edx
                    mov eax, dword ptr [dr]
                    mov ecx, dword ptr [redleft]
                    mov esi, dword ptr [db]
                    mov edx, dword ptr [greenleft]
                    mov ebx, dword ptr [dg]
                    mov edi, dword ptr [blueleft]
                    add ecx, eax
                    mov [redleft], ecx
                    add edx, ebx
                    mov [greenleft], edx
                    add edi, esi
                    mov [blueleft], edi
                    mov eax, dword ptr [u]
                    sar eax, 16
                    mov ebx, dword ptr [v]
                    sar ebx, 8
                    mov edx, [redleft]
                    mov esi, [TextureHandle]
                    and ebx, 0xffffff00
                    mov edi, [pDestination]
                    add ebx, eax
                    sar edx, 18
                    mov eax, [greenleft]
                    mov cx, word ptr [esi + ebx * 2]
                    mov esi, [blueleft]
                    sar eax, 18
                    mov ebx, ecx
                    shr ecx, 5
                    mov ebp, [Var6ca12c]
                    and ecx, 0x7c0
                    add ebp, edx
                    movzx edx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca1a8]
                    shl edx, 11
                    mov ecx, ebx
                    and ecx, 0x7c0
                    add ebp, eax
                    movzx ecx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca12c]
                    shl ecx, 5
                    and ebx, 0x1f
                    sar esi, 18
                    or edx, ecx
                    shl ebx, 6
                    mov eax, dword ptr [du]
                    add ebp, esi
                    mov ecx, dword ptr [u]
                    movzx esi, byte ptr [ebp + ebx]
                    mov ebx, dword ptr [dv]
                    or edx, esi
                    mov [edi + 18], dx
                    mov edx, dword ptr [v]
                    add ecx, eax
                    add edx, ebx
                    mov [u], ecx
                    mov [v], edx
                    mov eax, dword ptr [dr]
                    mov ecx, dword ptr [redleft]
                    mov esi, dword ptr [db]
                    mov edx, dword ptr [greenleft]
                    mov ebx, dword ptr [dg]
                    mov edi, dword ptr [blueleft]
                    add ecx, eax
                    mov [redleft], ecx
                    add edx, ebx
                    mov [greenleft], edx
                    add edi, esi
                    mov [blueleft], edi
                    mov eax, dword ptr [u]
                    sar eax, 16
                    mov ebx, dword ptr [v]
                    sar ebx, 8
                    mov edx, [redleft]
                    mov esi, [TextureHandle]
                    and ebx, 0xffffff00
                    mov edi, [pDestination]
                    add ebx, eax
                    sar edx, 18
                    mov eax, [greenleft]
                    mov cx, word ptr [esi + ebx * 2]
                    mov esi, [blueleft]
                    sar eax, 18
                    mov ebx, ecx
                    shr ecx, 5
                    mov ebp, [Var6ca12c]
                    and ecx, 0x7c0
                    add ebp, edx
                    movzx edx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca1a8]
                    shl edx, 11
                    mov ecx, ebx
                    and ecx, 0x7c0
                    add ebp, eax
                    movzx ecx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca12c]
                    shl ecx, 5
                    and ebx, 0x1f
                    sar esi, 18
                    or edx, ecx
                    shl ebx, 6
                    mov eax, dword ptr [du]
                    add ebp, esi
                    mov ecx, dword ptr [u]
                    movzx esi, byte ptr [ebp + ebx]
                    mov ebx, dword ptr [dv]
                    or edx, esi
                    mov [edi + 20], dx
                    mov edx, dword ptr [v]
                    add ecx, eax
                    add edx, ebx
                    mov [u], ecx
                    mov [v], edx
                    mov eax, dword ptr [dr]
                    mov ecx, dword ptr [redleft]
                    mov esi, dword ptr [db]
                    mov edx, dword ptr [greenleft]
                    mov ebx, dword ptr [dg]
                    mov edi, dword ptr [blueleft]
                    add ecx, eax
                    mov [redleft], ecx
                    add edx, ebx
                    mov [greenleft], edx
                    add edi, esi
                    mov [blueleft], edi
                    mov eax, dword ptr [u]
                    sar eax, 16
                    mov ebx, dword ptr [v]
                    sar ebx, 8
                    mov edx, [redleft]
                    mov esi, [TextureHandle]
                    and ebx, 0xffffff00
                    mov edi, [pDestination]
                    add ebx, eax
                    sar edx, 18
                    mov eax, [greenleft]
                    mov cx, word ptr [esi + ebx * 2]
                    mov esi, [blueleft]
                    sar eax, 18
                    mov ebx, ecx
                    shr ecx, 5
                    mov ebp, [Var6ca12c]
                    and ecx, 0x7c0
                    add ebp, edx
                    movzx edx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca1a8]
                    shl edx, 11
                    mov ecx, ebx
                    and ecx, 0x7c0
                    add ebp, eax
                    movzx ecx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca12c]
                    shl ecx, 5
                    and ebx, 0x1f
                    sar esi, 18
                    or edx, ecx
                    shl ebx, 6
                    mov eax, dword ptr [du]
                    add ebp, esi
                    mov ecx, dword ptr [u]
                    movzx esi, byte ptr [ebp + ebx]
                    mov ebx, dword ptr [dv]
                    or edx, esi
                    mov [edi + 22], dx
                    mov edx, dword ptr [v]
                    add ecx, eax
                    add edx, ebx
                    mov [u], ecx
                    mov [v], edx
                    mov eax, dword ptr [dr]
                    mov ecx, dword ptr [redleft]
                    mov esi, dword ptr [db]
                    mov edx, dword ptr [greenleft]
                    mov ebx, dword ptr [dg]
                    mov edi, dword ptr [blueleft]
                    add ecx, eax
                    mov [redleft], ecx
                    add edx, ebx
                    mov [greenleft], edx
                    add edi, esi
                    mov [blueleft], edi
                    mov eax, dword ptr [u]
                    sar eax, 16
                    mov ebx, dword ptr [v]
                    sar ebx, 8
                    mov edx, [redleft]
                    mov esi, [TextureHandle]
                    and ebx, 0xffffff00
                    mov edi, [pDestination]
                    add ebx, eax
                    sar edx, 18
                    mov eax, [greenleft]
                    mov cx, word ptr [esi + ebx * 2]
                    mov esi, [blueleft]
                    sar eax, 18
                    mov ebx, ecx
                    shr ecx, 5
                    mov ebp, [Var6ca12c]
                    and ecx, 0x7c0
                    add ebp, edx
                    movzx edx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca1a8]
                    shl edx, 11
                    mov ecx, ebx
                    and ecx, 0x7c0
                    add ebp, eax
                    movzx ecx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca12c]
                    shl ecx, 5
                    and ebx, 0x1f
                    sar esi, 18
                    or edx, ecx
                    shl ebx, 6
                    mov eax, dword ptr [du]
                    add ebp, esi
                    mov ecx, dword ptr [u]
                    movzx esi, byte ptr [ebp + ebx]
                    mov ebx, dword ptr [dv]
                    or edx, esi
                    mov [edi + 24], dx
                    mov edx, dword ptr [v]
                    add ecx, eax
                    add edx, ebx
                    mov [u], ecx
                    mov [v], edx
                    mov eax, dword ptr [dr]
                    mov ecx, dword ptr [redleft]
                    mov esi, dword ptr [db]
                    mov edx, dword ptr [greenleft]
                    mov ebx, dword ptr [dg]
                    mov edi, dword ptr [blueleft]
                    add ecx, eax
                    mov [redleft], ecx
                    add edx, ebx
                    mov [greenleft], edx
                    add edi, esi
                    mov [blueleft], edi
                    mov eax, dword ptr [u]
                    sar eax, 16
                    mov ebx, dword ptr [v]
                    sar ebx, 8
                    mov edx, [redleft]
                    mov esi, [TextureHandle]
                    and ebx, 0xffffff00
                    mov edi, [pDestination]
                    add ebx, eax
                    sar edx, 18
                    mov eax, [greenleft]
                    mov cx, word ptr [esi + ebx * 2]
                    mov esi, [blueleft]
                    sar eax, 18
                    mov ebx, ecx
                    shr ecx, 5
                    mov ebp, [Var6ca12c]
                    and ecx, 0x7c0
                    add ebp, edx
                    movzx edx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca1a8]
                    shl edx, 11
                    mov ecx, ebx
                    and ecx, 0x7c0
                    add ebp, eax
                    movzx ecx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca12c]
                    shl ecx, 5
                    and ebx, 0x1f
                    sar esi, 18
                    or edx, ecx
                    shl ebx, 6
                    mov eax, dword ptr [du]
                    add ebp, esi
                    mov ecx, dword ptr [u]
                    movzx esi, byte ptr [ebp + ebx]
                    mov ebx, dword ptr [dv]
                    or edx, esi
                    mov [edi + 26], dx
                    mov edx, dword ptr [v]
                    add ecx, eax
                    add edx, ebx
                    mov [u], ecx
                    mov [v], edx
                    mov eax, dword ptr [dr]
                    mov ecx, dword ptr [redleft]
                    mov esi, dword ptr [db]
                    mov edx, dword ptr [greenleft]
                    mov ebx, dword ptr [dg]
                    mov edi, dword ptr [blueleft]
                    add ecx, eax
                    mov [redleft], ecx
                    add edx, ebx
                    mov [greenleft], edx
                    add edi, esi
                    mov [blueleft], edi
                    mov eax, dword ptr [u]
                    sar eax, 16
                    mov ebx, dword ptr [v]
                    sar ebx, 8
                    mov edx, [redleft]
                    mov esi, [TextureHandle]
                    and ebx, 0xffffff00
                    mov edi, [pDestination]
                    add ebx, eax
                    sar edx, 18
                    mov eax, [greenleft]
                    mov cx, word ptr [esi + ebx * 2]
                    mov esi, [blueleft]
                    sar eax, 18
                    mov ebx, ecx
                    shr ecx, 5
                    mov ebp, [Var6ca12c]
                    and ecx, 0x7c0
                    add ebp, edx
                    movzx edx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca1a8]
                    shl edx, 11
                    mov ecx, ebx
                    and ecx, 0x7c0
                    add ebp, eax
                    movzx ecx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca12c]
                    shl ecx, 5
                    and ebx, 0x1f
                    sar esi, 18
                    or edx, ecx
                    shl ebx, 6
                    mov eax, dword ptr [du]
                    add ebp, esi
                    mov ecx, dword ptr [u]
                    movzx esi, byte ptr [ebp + ebx]
                    mov ebx, dword ptr [dv]
                    or edx, esi
                    mov [edi + 28], dx
                    mov edx, dword ptr [v]
                    add ecx, eax
                    add edx, ebx
                    mov [u], ecx
                    mov [v], edx
                    mov eax, dword ptr [dr]
                    mov ecx, dword ptr [redleft]
                    mov esi, dword ptr [db]
                    mov edx, dword ptr [greenleft]
                    mov ebx, dword ptr [dg]
                    mov edi, dword ptr [blueleft]
                    add ecx, eax
                    mov [redleft], ecx
                    add edx, ebx
                    mov [greenleft], edx
                    add edi, esi
                    mov [blueleft], edi
                    mov eax, dword ptr [u]
                    sar eax, 16
                    mov ebx, dword ptr [v]
                    sar ebx, 8
                    mov edx, [redleft]
                    mov esi, [TextureHandle]
                    and ebx, 0xffffff00
                    mov edi, [pDestination]
                    add ebx, eax
                    sar edx, 18
                    mov eax, [greenleft]
                    mov cx, word ptr [esi + ebx * 2]
                    mov esi, [blueleft]
                    sar eax, 18
                    mov ebx, ecx
                    shr ecx, 5
                    mov ebp, [Var6ca12c]
                    and ecx, 0x7c0
                    add ebp, edx
                    movzx edx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca1a8]
                    shl edx, 11
                    mov ecx, ebx
                    and ecx, 0x7c0
                    add ebp, eax
                    movzx ecx, byte ptr [ebp + ecx]
                    mov ebp, [Var6ca12c]
                    shl ecx, 5
                    and ebx, 0x1f
                    sar esi, 18
                    or edx, ecx
                    shl ebx, 6
                    mov eax, dword ptr [du]
                    add ebp, esi
                    mov ecx, dword ptr [u]
                    movzx esi, byte ptr [ebp + ebx]
                    mov ebx, dword ptr [dv]
                    or edx, esi
                    mov [edi + 30], dx
                    mov edx, dword ptr [v]
                    add ecx, eax
                    add edx, ebx
                    mov [u], ecx
                    mov [v], edx
                    mov eax, dword ptr [dr]
                    mov ecx, dword ptr [redleft]
                    mov esi, dword ptr [db]
                    mov edx, dword ptr [greenleft]
                    mov ebx, dword ptr [dg]
                    mov edi, dword ptr [blueleft]
                    add ecx, eax
                    mov [redleft], ecx
                    add edx, ebx
                    mov [greenleft], edx
                    add edi, esi
                    mov [blueleft], edi
                    mov edi, [pDestination]
                    add edi, 32
                    mov [pDestination], edi
                    pop ebp
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
                        unsigned long out;
                        texel = *(TextureHandle + ((v >> 8) & 0xffffff00) + (u >> 16));
                        out = *(Var6ca1a8 + (texel & 0x7c0) + (greenleft >> 18));
                        out |= *(Var6ca12c + ((texel >> 5) & 0x7c0) + (redleft >> 18)) << 6;
                        out <<= 5;
                        out |= *(Var6ca12c + ((texel & 0x1f) << 6) + (blueleft >> 18));
                        *(pDestination + 0) = (unsigned short)out;
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
