global prepare_smp_trampoline
global check_ap_flag
global get_cpu_number
global get_cpu_kernel_stack
global get_current_task
global set_current_task
global get_cpu_idle
global set_cpu_idle

section .data

%define smp_trampoline_size  smp_trampoline_end - smp_trampoline
smp_trampoline:              incbin "blobs/smp_trampoline.bin"
smp_trampoline_end:

section .text

bits 64

%define TRAMPOLINE_ADDR     0x1000
%define PAGE_SIZE           4096

prepare_smp_trampoline:
    ; entry point in rdi, page table in rsi
    ; stack pointer in rdx, cpu number in rcx
    push rdi
    push rsi
    push rcx

    ; prepare variables
    mov byte [0x510], 0
    mov qword [0x520], rdi
    mov qword [0x540], rsi
    mov qword [0x550], rdx
    mov qword [0x560], rcx
    a32 o32 sgdt [0x580]
    a32 o32 sidt [0x590]

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

get_cpu_number:
    mov rax, qword [fs:0000]
    ret

get_cpu_kernel_stack:
    mov rax, qword [fs:0008]
    ret

get_current_task:
    mov rax, qword [fs:0016]
    ret

set_current_task:
    mov qword [fs:0016], rdi
    ret

get_cpu_idle:
    mov rax, qword [fs:0024]
    ret

set_cpu_idle:
    mov qword [fs:0024], rdi
    ret
