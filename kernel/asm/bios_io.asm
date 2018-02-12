extern real_routine

global bios_print

section .data

%define bios_print_size           bios_print_end - bios_print_bin
bios_print_bin:                   incbin "blobs/bios_print.bin"
bios_print_end:

section .text

bios_print:
    ; void bios_print(const char *msg);
    push rbx
    push rbp

    mov rbx, rdi
    mov rsi, bios_print_bin
    mov rcx, bios_print_size
    call real_routine

    pop rbp
    pop rbx
    ret
