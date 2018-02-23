%macro pusham 0
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    xor rbx, rbx
    mov bx, ds
    push rbx
    mov bx, es
    push rbx
    mov bx, fs
    push rbx
    mov bx, gs
    push rbx
%endmacro

%macro popam 0
    mov bx, 0x10
    mov ss, bx
    pop rbx
    mov gs, bx
    pop rbx
    mov fs, bx
    pop rbx
    mov es, bx
    pop rbx
    mov ds, bx
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

%macro pushas 0
    push rbx
    push rcx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    xor rbx, rbx
    mov bx, ds
    push rbx
    mov bx, es
    push rbx
    mov bx, fs
    push rbx
    mov bx, gs
    push rbx
%endmacro

%macro popas 0
    mov bx, 0x10
    mov ss, bx
    pop rbx
    mov gs, bx
    pop rbx
    mov fs, bx
    pop rbx
    mov es, bx
    pop rbx
    mov ds, bx
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rcx
    pop rbx
%endmacro

; misc global references
global fxstate
global ts_enable
global read_stat
global write_stat

; IDT hooks
; ... CPU exceptions
global handler_irq_apic
global handler_irq_pic0
global handler_irq_pic1
global handler_div0
global handler_debug
global handler_nmi
global handler_breakpoint
global handler_overflow
global handler_bound_range_exceeded
global handler_invalid_opcode
global handler_device_not_available
global handler_double_fault
global handler_coprocessor_segment_overrun
global handler_invalid_tss
global handler_segment_not_present
global handler_stack_segment_fault
global handler_gpf
global handler_pf
global handler_x87_exception
global handler_alignment_check
global handler_machine_check
global handler_simd_exception
global handler_virtualisation_exception
global handler_security_exception
; ... misc
global irq0_handler
global keyboard_isr
global syscall

; CPU exception handlers
extern except_div0
extern except_debug
extern except_nmi
extern except_breakpoint
extern except_overflow
extern except_bound_range_exceeded
extern except_invalid_opcode
extern except_device_not_available
extern except_double_fault
extern except_coprocessor_segment_overrun
extern except_invalid_tss
extern except_segment_not_present
extern except_stack_segment_fault
extern except_gen_prot_fault
extern except_page_fault
extern except_x87_exception
extern except_alignment_check
extern except_machine_check
extern except_simd_exception
extern except_virtualisation_exception
extern except_security_exception

; misc external references
extern kernel_pagemap
extern eoi
extern timer_interrupt
extern keyboard_handler
extern task_switch

; API calls
extern open
extern close
extern read
extern write
extern lseek
extern getpid
extern signal
extern task_fork
extern task_quit_self
extern enter_iowait_status1
extern pwd
extern vfs_cd
extern vfs_remove
extern vfs_mkdir
extern vfs_create
extern vfs_list
extern vfs_get_metadata
extern execve
extern get_heap_base
extern get_heap_size
extern resize_heap
extern swait

section .data

align 16
fxstate: times 512 db 0

ts_enable dd 0
read_stat dd 0
write_stat dd 0
interrupted_cr3 dq 0

routine_list:
        dq      task_quit_self          ; 0x00
        dq      0                       ; 0x01
        dq      0                       ; 0x02
        dq      execve                  ; 0x03
        dq      0                       ; 0x04 - wait
        dq      0                       ; 0x05 - fork
        dq      0                       ; 0x06
        dq      0                       ; 0x07
        dq      0                       ; 0x08
        dq      0                       ; 0x09
        dq      0                       ; 0x0a
        dq      0                       ; 0x0b
        dq      0                       ; 0x0c
        dq      0                       ; 0x0d
        dq      0                       ; 0x0e
        dq      0                       ; 0x0f
        dq      get_heap_base           ; 0x10
        dq      get_heap_size           ; 0x11
        dq      resize_heap             ; 0x12
        dq      0                       ; 0x13
        dq      0                       ; 0x14
        dq      getpid                  ; 0x15
        dq      signal                  ; 0x16
        dq      0                       ; 0x17
        dq      0                       ; 0x18
        dq      0                       ; 0x19
        dq      pwd                     ; 0x1a
        dq      0                       ; 0x1b
        dq      0                       ; 0x1c
        dq      0                       ; 0x1d
        dq      0                       ; 0x1e
        dq      0                       ; 0x1f
        dq      0                       ; 0x20
        dq      0                       ; 0x21
        dq      0                       ; 0x22
        dq      0                       ; 0x23
        dq      0                       ; 0x24
        dq      0                       ; 0x25
        dq      0                       ; 0x26
        dq      0                       ; 0x27
        dq      0                       ; 0x28
        dq      0                       ; 0x29
        dq      open                    ; 0x2a
        dq      close                   ; 0x2b
        dq      0                       ; 0x2c - read
        dq      0                       ; 0x2d - write
        dq      lseek                   ; 0x2e
        dq      vfs_cd                  ; 0x2f
        dq      0                       ; 0x30
        dq      0                       ; 0x31
        dq      vfs_list                ; 0x32
        dq      vfs_get_metadata        ; 0x33
        dq      vfs_remove              ; 0x34
        dq      vfs_mkdir               ; 0x35
        dq      vfs_create              ; 0x36

section .text

bits 64

handler_irq_apic:
        pusham
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov rax, cr3        ; save context
        push qword [interrupted_cr3]
        mov qword [interrupted_cr3], rax
        mov rax, qword [kernel_pagemap]   ; context swap to kernel
        mov cr3, rax
        call eoi
        mov rax, qword [interrupted_cr3]
        mov cr3, rax    ; restore context
        pop qword [interrupted_cr3]
        popam
        iretq

handler_irq_pic0:
        push rax
        mov al, 0x20    ; acknowledge interrupt to PIC0
        out 0x20, al
        mov ax, 0x10
        mov ss, ax
        pop rax
        iretq

handler_irq_pic1:
        push rax
        mov al, 0x20    ; acknowledge interrupt to both PICs
        out 0xA0, al
        out 0x20, al
        mov ax, 0x10
        mov ss, ax
        pop rax
        iretq

except_handler_setup:
        mov rax, qword [kernel_pagemap]
        mov cr3, rax
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax
        ret

handler_div0:
        call except_handler_setup
        pop rdi
        pop rsi
        call except_div0

handler_debug:
        call except_handler_setup
        pop rdi
        pop rsi
        call except_debug

handler_nmi:
        call except_handler_setup
        pop rdi
        pop rsi
        call except_nmi

handler_breakpoint:
        call except_handler_setup
        pop rdi
        pop rsi
        call except_breakpoint

handler_overflow:
        call except_handler_setup
        pop rdi
        pop rsi
        call except_overflow

handler_bound_range_exceeded:
        call except_handler_setup
        pop rdi
        pop rsi
        call except_bound_range_exceeded

handler_invalid_opcode:
        call except_handler_setup
        pop rdi
        pop rsi
        call except_invalid_opcode

handler_device_not_available:
        call except_handler_setup
        pop rdi
        pop rsi
        call except_device_not_available

handler_double_fault:
        call except_handler_setup
        pop rdi
        pop rsi
        call except_double_fault

handler_coprocessor_segment_overrun:
        call except_handler_setup
        pop rdi
        pop rsi
        call except_coprocessor_segment_overrun

handler_invalid_tss:
        call except_handler_setup
        pop rdi
        pop rsi
        pop rdx
        call except_invalid_tss

handler_segment_not_present:
        call except_handler_setup
        pop rdi
        pop rsi
        pop rdx
        call except_segment_not_present

handler_stack_segment_fault:
        call except_handler_setup
        pop rdi
        pop rsi
        pop rdx
        call except_stack_segment_fault

handler_gpf:
        call except_handler_setup
        pop rdi
        pop rsi
        pop rdx
        call except_gen_prot_fault

handler_pf:
        call except_handler_setup
        pop rdi
        pop rsi
        pop rdx
        call except_page_fault

handler_x87_exception:
        call except_handler_setup
        pop rdi
        pop rsi
        call except_x87_exception

handler_alignment_check:
        call except_handler_setup
        pop rdi
        pop rsi
        pop rdx
        call except_alignment_check

handler_machine_check:
        call except_handler_setup
        pop rdi
        pop rsi
        call except_machine_check

handler_simd_exception:
        call except_handler_setup
        pop rdi
        pop rsi
        call except_simd_exception

handler_virtualisation_exception:
        call except_handler_setup
        pop rdi
        pop rsi
        call except_virtualisation_exception

handler_security_exception:
        call except_handler_setup
        pop rdi
        pop rsi
        call except_security_exception

irq0_handler:
        ; first execute all the time-based routines (tty refresh...)
        pusham
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov rax, cr3        ; save context
        push qword [interrupted_cr3]
        mov qword [interrupted_cr3], rax
        mov rax, qword [kernel_pagemap]   ; context swap to kernel
        mov cr3, rax
        call timer_interrupt
        call eoi
        ; check whether we want task switches or not
        cmp dword [ts_enable], 0
        je .ts_abort
        ; call task switcher
        add rsp, 8
        mov ax, 0x10
        mov fs, ax
        mov gs, ax
        mov ss, ax
        mov rdi, rsp
        fxsave [fxstate]
        call task_switch
    .ts_abort:
        mov rax, qword [interrupted_cr3]
        mov cr3, rax    ; restore context
        pop qword [interrupted_cr3]
        popam
        iretq

keyboard_isr:
        pusham
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov rax, cr3        ; save context
        push qword [interrupted_cr3]
        mov qword [interrupted_cr3], rax
        mov rax, qword [kernel_pagemap]   ; context swap to kernel
        mov cr3, rax
        xor rax, rax
        in al, 0x60     ; read from keyboard
        mov rdi, rax
        call keyboard_handler
        call eoi
        mov rax, qword [interrupted_cr3]
        mov cr3, rax    ; restore context
        pop qword [interrupted_cr3]
        popam
        iretq

syscall:
; ARGS in RAX (call code), RCX, RDX, RDI, RSI
; return value in RAX/RDX
        ; special routines check
        cmp rax, 0x05
        je fork_isr
        cmp rax, 0x04
        je wait_isr
        ; disable task switch, reenable all interrupts
        push rax
        push rbx
        mov ax, ds
        mov bx, 0x10
        mov ds, bx
        mov dword [ts_enable], 0
        mov ds, ax
        pop rbx
        pop rax
        sti
        ; special routines check
        cmp rax, 0x2c
        je read_isr
        cmp rax, 0x2d
        je write_isr
        ; end special routines check
        pushas
        mov bx, 0x10
        mov ds, bx
        mov es, bx
        mov fs, bx
        mov gs, bx
        mov rbx, cr3        ; save context
        mov qword [interrupted_cr3], rbx
        mov rbx, qword [kernel_pagemap]   ; context swap to kernel
        mov cr3, rbx
        mov rbx, 8
        push rdx
        mul rbx
        pop rdx
        ; push syscall args, and call
        push rcx
        push rdx
        push rdi
        push rsi
        pop rcx
        pop rdx
        pop rsi
        pop rdi
        call [routine_list+rax]
        ; disable all interrupts, reenable task switch
        cli
        mov qword [ts_enable], 1
        mov rbx, qword [interrupted_cr3]
        mov cr3, rbx    ; restore context
        ; return
        popas
        iretq

read_isr:
        ; check if I/O is ready
        pushas
        mov bx, 0x10
        mov ds, bx
        mov es, bx
        mov fs, bx
        mov gs, bx
        mov rbx, cr3        ; save context
        mov qword [interrupted_cr3], rbx
        mov rbx, qword [kernel_pagemap]   ; context swap to kernel
        mov cr3, rbx
        push rcx
        push rdx
        push rdi
        push rsi
        pop rcx
        pop rdx
        pop rsi
        pop rdi
        mov r12, rsi        ; preserve rdx
        call read
        mov rdx, r12
        ; disable all interrupts, reenable task switch
        cli
        mov dword [ts_enable], 1
        cmp dword [read_stat], 1     ; if I/O is not ready
        push rbx
        mov rbx, qword [interrupted_cr3]
        mov cr3, rbx    ; restore context
        pop rbx
        ; done
        popas
        je .enter_iowait
        iretq           ; else, just return
    .enter_iowait:
        pusham
        mov bx, 0x10
        mov ds, bx
        mov es, bx
        mov fs, bx
        mov gs, bx
        mov rbx, qword [kernel_pagemap]   ; context swap to kernel
        mov cr3, rbx
        mov r8, rax
        push rcx
        push rdx
        push rdi
        mov rcx, 2      ; read type
        pop rdx
        pop rsi
        pop rdi
        call enter_iowait_status1
        mov rdi, rsp
        fxsave [fxstate]
        call task_switch

write_isr:
        ; check if I/O is ready
        pushas
        mov bx, 0x10
        mov ds, bx
        mov es, bx
        mov fs, bx
        mov gs, bx
        mov rbx, cr3        ; save context
        mov qword [interrupted_cr3], rbx
        mov rbx, qword [kernel_pagemap]   ; context swap to kernel
        mov cr3, rbx
        push rcx
        push rdx
        push rdi
        push rsi
        pop rcx
        pop rdx
        pop rsi
        pop rdi
        mov r12, rsi        ; preserve rdx
        call write
        mov rdx, r12
        ; disable all interrupts, reenable task switch
        cli
        mov dword [ts_enable], 1
        cmp dword [write_stat], 1     ; if I/O is not ready
        push rbx
        mov rbx, qword [interrupted_cr3]
        mov cr3, rbx    ; restore context
        pop rbx
        ; done
        popas
        je .enter_iowait
        iretq           ; else, just return
    .enter_iowait:
        pusham
        mov bx, 0x10
        mov ds, bx
        mov es, bx
        mov fs, bx
        mov gs, bx
        mov rbx, qword [kernel_pagemap]   ; context swap to kernel
        mov cr3, rbx
        mov r8, rax
        push rcx
        push rdx
        push rdi
        mov rcx, 3      ; write type
        pop rdx
        pop rsi
        pop rdi
        call enter_iowait_status1
        mov rdi, rsp
        fxsave [fxstate]
        call task_switch

wait_isr:
        ; save task status
        pusham
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov rax, qword [kernel_pagemap]   ; context swap to kernel
        mov cr3, rax
        mov rdi, rcx
        call swait
        mov rdi, rsp
        fxsave [fxstate]
        call task_switch

fork_isr:
        ; save task status
        pusham
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov rax, qword [kernel_pagemap]   ; context swap to kernel
        mov cr3, rax
        mov rdi, rsp
        fxsave [fxstate]
        call task_fork
