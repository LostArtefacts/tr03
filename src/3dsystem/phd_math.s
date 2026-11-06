    .486
    .model flat

    extern _phdsintab:byte
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

@phd_cos@4 proc public
    add ecx, 04000h
@phd_cos@4 endp

@phd_sin@4 proc public
    mov eax, ecx
    and eax, 0ffffh
    cmp eax, 08000h
    jl short L4b4c79
    sub eax, 08000h
    call L4b4c79
    neg eax
    ret
L4b4c79:
    cmp eax, 04000h
    jle short L4b4c87
    neg eax
    add eax, 08000h
L4b4c87:
    shr eax, 04h
    mov ax, word ptr [eax*2+offset _phdsintab]
    ret
@phd_sin@4 endp

@phd_sqrt@4 proc public
    push ebx
    push ecx
    push edx
    mov ebx, 040000000h
    xor eax, eax
L4b4c9d:
    mov edx, eax
    add eax, ebx
    shr edx, 1
    cmp eax, ecx
    ja short L4b4cb4
    sub ecx, eax
    mov eax, ebx
    or eax, edx
    shr ebx, 02h
    jne short L4b4c9d
    jmp short L4b4cbb
L4b4cb4:
    mov eax, edx
    shr ebx, 02h
    jne short L4b4c9d
L4b4cbb:
    pop edx
    pop ecx
    pop ebx
    ret
@phd_sqrt@4 endp

    end
