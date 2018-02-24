global prepare_smp_trampoline
global check_ap_flag

section .data

%define smp_trampoline_size  smp_trampoline_end - smp_trampoline
smp_trampoline:              incbin "blobs/smp_trampoline.bin"
smp_trampoline_end:

section .text

bits 64

%define TRAMPOLINE_ADDR     0x1000
%define PAGE_SIZE           4096

prepare_smp_trampoline:
    ; entry point in rdi, GDT in rsi, page table in rdx
    ; stack pointer in ecx, cpu number in r8
    push rdi
    push rsi
    push rcx

    ; Clear AP flag
    mov byte [0x510], 0

    ; copy entry point
    mov qword [0x520], rdi
    mov qword [0x530], rsi
    mov qword [0x540], rdx
    mov qword [0x550], rcx
    mov qword [0x560], r8

    ; Copy trampoline blob to 0x1000
    mov rsi, smp_trampoline
    mov rdi, TRAMPOLINE_ADDR
    mov rcx, smp_trampoline_size
    rep movsb

    pop rcx
    pop rsi
    pop rdi
    mov rax, TRAMPOLINE_ADDR / PAGE_SIZE
    ret

check_ap_flag:
    xor rax, rax
    mov al, byte [0x510]
    ret
