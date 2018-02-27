global task_spinup
extern fxstate

section .data

    new_cr3     dq  0

section .text

bits 64

task_spinup:
    ; task fxstate in RDX
    ; load state
    fxrstor [rdx]

    mov qword [new_cr3], rsi

    ; preserve RAX and RDI as a scratch registers for now

    ;mov rax, qword [rdi+(8*16)]     skip rax
    mov rbx, qword [rdi+(8*15)]
    mov rcx, qword [rdi+(8*14)]
    mov rdx, qword [rdi+(8*13)]
    mov rsi, qword [rdi+(8*12)]
    ;mov rdi, qword [rdi+(8*11)]     skip rdi
    mov rbp, qword [rdi+(8*10)]
    mov r8, qword [rdi+(8*9)]
    mov r9, qword [rdi+(8*8)]
    mov r10, qword [rdi+(8*7)]
    mov r11, qword [rdi+(8*6)]
    mov r12, qword [rdi+(8*5)]
    mov r13, qword [rdi+(8*4)]
    mov r14, qword [rdi+(8*3)]
    mov r15, qword [rdi+(8*2)]

    mov rax, qword [rdi+(8*0)]
    mov es, ax

    push qword [rdi+(8*21)]
    push qword [rdi+(8*20)]
    push qword [rdi+(8*19)]
    push qword [rdi+(8*18)]
    push qword [rdi+(8*17)]

    push qword [rdi+(8*16)]          ; rax
    push qword [rdi+(8*11)]          ; rdi
    push qword [rdi+(8*1)]         ; ds
    mov rax, qword [new_cr3]
    mov cr3, rax
    pop rax
    mov ds, ax
    mov ax, 0x10
    mov ss, ax
    pop rdi
    pop rax

    iretq
