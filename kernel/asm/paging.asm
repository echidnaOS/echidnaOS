global full_identity_map
global kernel_pagemap

section .bss

align 4096
kernel_pt:
    resb (512 * 8) * 2048

kernel_pd:
    .1:
    resq 512
    .2:
    resq 512
    .3:
    resq 512
    .4:
    resq 512

kernel_pdpt:
    resq 512

kernel_pml4:
    resq 512

section .data

kernel_pagemap dq kernel_pml4

section .text

bits 64

full_identity_map:

    ; Identity map all 4GB of memory for the kernel

    push rbx
    push rbp

    mov rdi, kernel_pt
    mov rax, 0x03
    mov rcx, 512 * 2048
    .loop:
        stosq
        add rax, 0x1000
        loop .loop

    mov rdi, kernel_pd.1
    mov rax, kernel_pt
    or rax, 0x03
    mov rcx, 512 * 4
    .loop1:
        stosq
        add rax, 0x1000
        loop .loop1

    mov rdi, kernel_pdpt
    mov rax, kernel_pd.1
    or rax, 0x03
    stosq
    mov rax, kernel_pd.2
    or rax, 0x03
    stosq
    mov rax, kernel_pd.3
    or rax, 0x03
    stosq
    mov rax, kernel_pd.4
    or rax, 0x03
    stosq

    mov rdi, kernel_pml4
    mov rax, kernel_pdpt
    or rax, 0x03
    stosq

    mov rdx, kernel_pml4
    mov cr3, rdx

    pop rbp
    pop rbx
    ret
