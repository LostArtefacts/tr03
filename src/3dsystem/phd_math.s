    .486
    .model flat

    extern _phdtan2:byte
    extern _phdtantab:byte

    .code

@phd_atan@8 proc public
    push ebx
    push ecx
    push edx
    mov eax, ecx
    mov ebx, eax
    xor ecx, ecx
    or eax, edx
    je short L4b4c54
    test ebx, ebx
    jns short L4b4c26
    add ecx, 04h
    neg ebx
L4b4c26:
    test edx, edx
    jns short L4b4c2f
    add ecx, 02h
    neg edx
L4b4c2f:
    xor eax, eax
    cmp edx, ebx
    jle short L4b4c38
    inc ecx
    db 087h, 0dah
L4b4c38:
    shrd eax, edx, 015h
    shr edx, 015h
    idiv ebx
    mov ax, word ptr [eax*2+offset _phdtantab]
    add eax, dword ptr [ecx*4+offset _phdtan2]
    jns short L4b4c54
    neg eax
L4b4c54:
    pop edx
    pop ecx
    pop ebx
    ret
@phd_atan@8 endp

    end
