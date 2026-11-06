    .486
    .model flat

    extern _phd_scrwidth:byte
    extern _phd_winptr:byte
    extern _xbuffer:byte

    .code

_flatA proc public
    pusha
    sub edx, eax
    jle short L4b4333
    mov esi, eax
    shl esi, 03h
    add esi, offset _xbuffer+2
    imul eax, dword ptr [_phd_scrwidth]
    add eax, dword ptr [_phd_winptr]
    mov ebp, eax
    cld
    mov bh, bl
    mov ax, bx
    bswap eax
    mov ax, bx
L4b42fe:
    movsx ecx, word ptr [esi]
    movsx ebx, word ptr [esi+04h]
    sub ebx, ecx
    jle short L4b4327
    lea edi, [ecx+ebp*1]
    neg ecx
    and ecx, 03h
    cmp ecx, ebx
    jge short L4b4323
    sub ebx, ecx
    rep stosb
    mov ecx, ebx
    sar ecx, 02h
    rep stosd
    and ebx, 03h
L4b4323:
    mov ecx, ebx
    rep stosb
L4b4327:
    add esi, 08h
    add ebp, dword ptr [_phd_scrwidth]
    dec edx
    jne short L4b42fe
L4b4333:
    popa
    ret
_flatA endp

    end
