extern real_routine

global bios_print

section .data

%define bios_print_size           bios_print_end - bios_print_bin
bios_print_bin:                   incbin "blobs/bios_print.bin"
bios_print_end:

section .text

bios_print:
    ; void bios_print(const char *msg);
    push ebx
    push esi
    push edi
    push ebp

    mov ebx, dword [esp+20]
    mov esi, bios_print_bin
    mov ecx, bios_print_size
    call real_routine

    pop ebp
    pop edi
    pop esi
    pop ebx
    ret
