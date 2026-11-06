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

    end
