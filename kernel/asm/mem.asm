extern real_routine

global detect_mem

section .data

%define detect_mem_size         detect_mem_end - detect_mem_bin
detect_mem_bin:                 incbin "blobs/detect_mem.bin"
detect_mem_end:

align 4
mem_size        dd  0

section .text

bits 64

detect_mem:
    push rbx
    push rbp
    mov rsi, detect_mem_bin
    mov rcx, detect_mem_size
    mov rbx, mem_size
    call real_routine
    pop rbp
    pop rbx
    mov eax, dword [mem_size]
    ret
