global full_identity_map
global kernel_pagemap

section .data

align 4096
kernel_pagemap_data:
    times (1024 * 1024) + 1024 dd 0

kernel_pagemap dd kernel_pagemap_data + ((1024 * 1024) * 4)

section .text

bits 32

full_identity_map:

    ; Identity map all 4GB of memory for the kernel

    push eax
    push ecx
    push edx
    push edi

    mov edi, kernel_pagemap_data
    mov eax, 0x07
    mov ecx, 1024 * 1024
    .loop:
        stosd
        add eax, 0x1000
        loop .loop

    mov edx, edi        ; save starting address of page directory
    mov eax, kernel_pagemap_data
    or eax, 0x07
    mov ecx, 1024
    .loop1:
        stosd
        add eax, 0x1000
        loop .loop1

    mov cr3, edx

    pop edi
    pop edx
    pop ecx
    pop eax
    ret
