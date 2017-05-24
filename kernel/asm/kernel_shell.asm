global kernel_shell

section .data

%define shell_size          shell_end - shell
shell:                      incbin "shell/shell.bin"
shell_end:

section .text

bits 32

kernel_shell:
    ; copy to 0x1000000
    mov esi, shell
    mov edi, 0x1000000
    mov ecx, shell_size
    rep movsb
    
    push 0
    push 0
    call 0x1000000
