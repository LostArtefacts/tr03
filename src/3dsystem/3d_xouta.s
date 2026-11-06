    .486
    .model flat

    extern _depthq_table:byte
    extern _gouraud_table:byte
    extern _phd_scrwidth:byte
    extern _phd_winptr:byte
    extern _phd_winwidth:byte
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

_transA proc public
    pusha
    sub esp, 08h
    sub edx, eax
    jle near ptr L4b446a
    mov dword ptr [esp+04h], edx
    lea esi, [_depthq_table]
    cmp ebx, 020h
    ja near ptr L4b446a
    shl ebx, 08h
    add esi, ebx
    mov ebp, eax
    shl ebp, 03h
    add ebp, offset _xbuffer+2
    imul eax, dword ptr [_phd_scrwidth]
    add eax, dword ptr [_phd_winptr]
    mov dword ptr [esp], eax
    xor eax, eax
L4b4376:
    movsx edi, word ptr [ebp+00h]
    test edi, edi
    jns short L4b4380
    xor edi, edi
L4b4380:
    movsx ecx, word ptr [ebp+04h]
    cmp ecx, dword ptr [_phd_winwidth]
    jle short L4b4392
    mov ecx, dword ptr [_phd_winwidth]
L4b4392:
    sub ecx, edi
    jle near ptr L4b4454
    add edi, dword ptr [esp]
    test edi, 01h
    je short L4b43ae
    mov al, byte ptr [edi]
    mov dl, byte ptr [eax+esi*1]
    mov byte ptr [edi], dl
    dec ecx
    inc edi
L4b43ae:
    sub ecx, 010h
    js near ptr L4b4444
    mov ebx, dword ptr [edi]
    mov al, bl
    mov dl, byte ptr [eax+esi*1]
    mov al, bh
    mov dh, byte ptr [eax+esi*1]
    bswap ebx
    mov word ptr [edi], dx
    mov al, bh
    mov dl, byte ptr [eax+esi*1]
    mov al, bl
    mov dh, byte ptr [eax+esi*1]
    mov word ptr [edi+02h], dx
    add edi, 04h
    mov ebx, dword ptr [edi]
    mov al, bl
    mov dl, byte ptr [eax+esi*1]
    mov al, bh
    mov dh, byte ptr [eax+esi*1]
    bswap ebx
    mov word ptr [edi], dx
    mov al, bh
    mov dl, byte ptr [eax+esi*1]
    mov al, bl
    mov dh, byte ptr [eax+esi*1]
    mov word ptr [edi+02h], dx
    add edi, 04h
    mov ebx, dword ptr [edi]
    mov al, bl
    mov dl, byte ptr [eax+esi*1]
    mov al, bh
    mov dh, byte ptr [eax+esi*1]
    bswap ebx
    mov word ptr [edi], dx
    mov al, bh
    mov dl, byte ptr [eax+esi*1]
    mov al, bl
    mov dh, byte ptr [eax+esi*1]
    mov word ptr [edi+02h], dx
    add edi, 04h
    mov ebx, dword ptr [edi]
    mov al, bl
    mov dl, byte ptr [eax+esi*1]
    mov al, bh
    mov dh, byte ptr [eax+esi*1]
    bswap ebx
    mov word ptr [edi], dx
    mov al, bh
    mov dl, byte ptr [eax+esi*1]
    mov al, bl
    mov dh, byte ptr [eax+esi*1]
    mov word ptr [edi+02h], dx
    add edi, 04h
    jmp near ptr L4b43ae
L4b4444:
    add ecx, 010h
    jle short L4b4454
L4b4449:
    mov al, byte ptr [edi]
    mov dl, byte ptr [eax+esi*1]
    mov byte ptr [edi], dl
    inc edi
    dec ecx
    jne short L4b4449
L4b4454:
    mov ebx, dword ptr [_phd_scrwidth]
    add ebp, 08h
    add dword ptr [esp], ebx
    dec dword ptr [esp+04h]
    jne near ptr L4b4376
L4b446a:
    add esp, 08h
    popa
    ret
_transA endp

_gourA proc public
    push ecx
    push esi
    push edi
    push ebp
    sub esp, 010h
    sub edx, eax
    jle near ptr L4b45c7
    mov dword ptr [esp+04h], edx
    mov ecx, eax
    shl ecx, 04h
    add ecx, offset _xbuffer
    mov dword ptr [esp+08h], ecx
    imul eax, dword ptr [_phd_scrwidth]
    add eax, dword ptr [_phd_winptr]
    mov dword ptr [esp+0ch], eax
    movzx ebx, bx
    shl ebx, 05h
    add ebx, offset _gouraud_table
    mov dword ptr [esp], ebx
L4b44b1:
    mov eax, dword ptr [esp+08h]
    movsx ebx, word ptr [eax+0ah]
    movsx edi, word ptr [eax+02h]
    cmp edi, ebx
    jge near ptr L4b45af
    mov ecx, dword ptr [eax+04h]
    mov ebp, ebx
    mov edx, dword ptr [eax+0ch]
    sub edx, ecx
    sub ebx, edi
    mov eax, edx
    sar edx, 01fh
    idiv ebx
    test edi, edi
    jns short L4b44e3
    imul edi, eax
    sub ecx, edi
    xor edi, edi
L4b44e3:
    sub ebp, edi
    add edi, dword ptr [esp+0ch]
    mov edx, eax
    mov esi, ecx
    sar eax, 010h
    sar ecx, 010h
    shl edx, 010h
    shl esi, 010h
    add ecx, dword ptr [esp]
    test edi, 01h
    je short L4b450e
    mov bl, byte ptr [ecx]
    add esi, edx
    mov byte ptr [edi], bl
    adc ecx, eax
    dec ebp
    inc edi
L4b450e:
    sub ebp, 010h
    js near ptr L4b459e
    mov bl, byte ptr [ecx]
    add esi, edx
    adc ecx, eax
    mov bh, byte ptr [ecx]
    add esi, edx
    mov word ptr [edi], bx
    adc ecx, eax
    mov bl, byte ptr [ecx]
    add esi, edx
    adc ecx, eax
    mov bh, byte ptr [ecx]
    add esi, edx
    mov word ptr [edi+02h], bx
    adc ecx, eax
    mov bl, byte ptr [ecx]
    add esi, edx
    adc ecx, eax
    mov bh, byte ptr [ecx]
    add esi, edx
    mov word ptr [edi+04h], bx
    adc ecx, eax
    mov bl, byte ptr [ecx]
    add esi, edx
    adc ecx, eax
    mov bh, byte ptr [ecx]
    add esi, edx
    mov word ptr [edi+06h], bx
    adc ecx, eax
    mov bl, byte ptr [ecx]
    add esi, edx
    adc ecx, eax
    mov bh, byte ptr [ecx]
    add esi, edx
    mov word ptr [edi+08h], bx
    adc ecx, eax
    mov bl, byte ptr [ecx]
    add esi, edx
    adc ecx, eax
    mov bh, byte ptr [ecx]
    add esi, edx
    mov word ptr [edi+0ah], bx
    adc ecx, eax
    mov bl, byte ptr [ecx]
    add esi, edx
    adc ecx, eax
    mov bh, byte ptr [ecx]
    add esi, edx
    mov word ptr [edi+0ch], bx
    adc ecx, eax
    mov bl, byte ptr [ecx]
    add esi, edx
    adc ecx, eax
    mov bh, byte ptr [ecx]
    add esi, edx
    mov word ptr [edi+0eh], bx
    adc ecx, eax
    add edi, 010h
    jmp near ptr L4b450e
L4b459e:
    add ebp, 010h
    jle short L4b45af
L4b45a3:
    mov bl, byte ptr [ecx]
    add esi, edx
    mov byte ptr [edi], bl
    adc ecx, eax
    inc edi
    dec ebp
    jne short L4b45a3
L4b45af:
    mov eax, dword ptr [_phd_scrwidth]
    add dword ptr [esp+08h], 010h
    add dword ptr [esp+0ch], eax
    dec dword ptr [esp+04h]
    jne near ptr L4b44b1
L4b45c7:
    add esp, 010h
    pop ebp
    pop edi
    pop esi
    pop ecx
    ret
_gourA endp

    end
