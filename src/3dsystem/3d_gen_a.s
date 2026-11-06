    .486
    .model flat

    extern _phd_mxptr:byte

    .code

_phd_PushMatrix proc public
    push esi
    push edi
    push ecx
    cld
    mov esi, dword ptr [_phd_mxptr]
    mov ecx, 0ch
    lea edi, [esi+ecx*4]
    rep movsd
    mov dword ptr [_phd_mxptr], esi
    pop ecx
    pop edi
    pop esi
    ret
_phd_PushMatrix endp

_phd_PushUnitMatrix proc public
    add dword ptr [_phd_mxptr], 030h
    push edi
    push eax
    mov edi, dword ptr [_phd_mxptr]
    mov eax, 04000h
    mov dword ptr [edi], eax
    mov dword ptr [edi+014h], eax
    mov dword ptr [edi+028h], eax
    xor eax, eax
    mov dword ptr [edi+04h], eax
    mov dword ptr [edi+08h], eax
    mov dword ptr [edi+010h], eax
    mov dword ptr [edi+018h], eax
    mov dword ptr [edi+020h], eax
    mov dword ptr [edi+024h], eax
    pop eax
    pop edi
    ret
_phd_PushUnitMatrix endp

    end
