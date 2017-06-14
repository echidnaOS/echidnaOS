global kernel_shell

extern set_userspace

section .data

%define shell_size          shell_end - shell
shell:                      incbin "../shell/shell.bin"
shell_end:

section .text

bits 32

kernel_shell:
    ; copy to 0x1000000
    mov esi, shell
    mov edi, 0x1000000
    mov ecx, shell_size
    rep movsb
    
    push 0x100
    push 0x1000000
    call set_userspace
    add esp, 8

    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push 0x23
    push 0xffff0
    push 0x202
    push 0x1B
    push 0x0
    iretd
