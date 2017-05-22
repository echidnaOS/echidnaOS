extern real_routine

global detect_mem

section .data

%define detect_mem_size         detect_mem_end - detect_mem_bin
detect_mem_bin:                 incbin "kernel/asm/i686/detect_mem.bin"
detect_mem_end:

align 4
mem_size        dd  0

section .text

bits 32

detect_mem:
    push ebx
    push esi
    push edi
    push ebp
    mov esi, detect_mem_bin
    mov ecx, detect_mem_size
    mov ebx, mem_size
    call real_routine
    pop ebp
    pop edi
    pop esi
    pop ebx
    mov eax, dword [mem_size]
    ret
