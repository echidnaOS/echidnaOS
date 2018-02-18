global task_spinup

section .data

    new_cr3     dq  0

section .text

bits 64

task_spinup:
    mov qword [new_cr3], rsi

    ; preserve RAX and RDI as a scratch registers for now

    ;mov rax, qword [rdi+(8*18)]     skip rax
    mov rbx, qword [rdi+(8*17)]
    mov rcx, qword [rdi+(8*16)]
    mov rdx, qword [rdi+(8*15)]
    mov rsi, qword [rdi+(8*14)]
    ;mov rdi, qword [rdi+(8*13)]     skip rdi
    mov rbp, qword [rdi+(8*12)]
    mov r8, qword [rdi+(8*11)]
    mov r9, qword [rdi+(8*10)]
    mov r10, qword [rdi+(8*9)]
    mov r11, qword [rdi+(8*8)]
    mov r12, qword [rdi+(8*7)]
    mov r13, qword [rdi+(8*6)]
    mov r14, qword [rdi+(8*5)]
    mov r15, qword [rdi+(8*4)]

    mov rax, qword [rdi+(8*2)]
    mov es, ax
    mov rax, qword [rdi+(8*1)]
    mov fs, ax
    mov rax, qword [rdi+(8*0)]
    mov gs, ax

    push qword [rdi+(8*23)]
    push qword [rdi+(8*22)]
    push qword [rdi+(8*21)]
    push qword [rdi+(8*20)]
    push qword [rdi+(8*19)]

    push qword [rdi+(8*18)]          ; rax
    push qword [rdi+(8*13)]          ; rdi
    push qword [rdi+(8*3)]         ; ds
    mov rax, qword [new_cr3]
    mov cr3, rax
    pop rax
    mov ds, ax
    mov ax, 0x10
    mov ss, ax
    pop rdi
    pop rax

    iretq
