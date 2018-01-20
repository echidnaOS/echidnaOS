global full_identity_map

section .text

bits 32

full_identity_map:

    ; Identity map all 4GB of memory for the kernel
    ; we need 4 MiB of page tables ((1024 * 4) * 1024), and
    ; 1 KiB for the page directory.

    push eax
    push ecx
    push edx
    push edi

    mov edi, [esp+20]
    mov eax, 0x07
    mov ecx, 1024 * 1024
    .loop:
        stosd
        add eax, 0x1000
        loop .loop

    mov edx, edi        ; save starting address of page directory
    mov eax, [esp+20]
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
