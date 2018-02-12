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
    xor rax, rax
    mov ax, ds
    push rax
    mov ax, es
    push rax
    mov ax, fs
    push rax
    mov ax, gs
    push rax
%endmacro

%macro popam 0
    pop rax
    mov gs, ax
    pop rax
    mov fs, ax
    pop rax
    mov es, ax
    pop rax
    mov ds, ax
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
    push rax
%endmacro

%macro popas 0
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

global handler_simple
global handler_code
global handler_irq_apic
global handler_irq_pic0
global handler_irq_pic1
global handler_div0
global handler_gpf
global handler_pf
global irq0_handler
global keyboard_isr
global syscall

global ts_enable
global read_stat
global write_stat

extern keyboard_handler
extern task_switch
extern except_div0
extern except_gen_prot_fault
extern except_page_fault

extern set_PIC0_mask
extern get_PIC0_mask

extern kernel_pagemap

extern eoi

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
extern alloc
extern free
extern realloc
extern enter_iowait_status
extern enter_iowait_status1
extern enter_ipcwait_status
extern enter_vdevwait_status
extern pwd
extern what_stdin
extern what_stdout
extern what_stderr
extern ipc_send_packet
extern ipc_read_packet
extern ipc_resolve_name
extern ipc_payload_sender
extern ipc_payload_length
extern vfs_cd
extern vfs_read
extern vfs_write
extern vfs_remove
extern vfs_mkdir
extern vfs_create
extern vfs_list
extern vfs_get_metadata
extern general_execute
extern general_execute_block
extern execve
extern register_vdev
extern vdev_in_ready
extern vdev_out_ready
extern get_heap_base
extern get_heap_size
extern resize_heap
extern swait

extern timer_interrupt

section .data

ts_enable dd 0
read_stat dd 0
write_stat dd 0
interrupted_cr3 dq 0

routine_list:
        dd      task_quit_self          ; 0x00
        dd      general_execute         ; 0x01
        dd      0 ;general_execute_block; 0x02 - dummy entry
        dd      execve                  ; 0x03
        dd      0 ;wait                 ; 0x04
        dd      0 ;task_fork            ; 0x05 - dummy entry
        dd      0                       ; 0x06
        dd      0                       ; 0x07
        dd      0;ipc_send_packet         ; 0x08
        dd      0;ipc_read_packet         ; 0x09
        dd      0;ipc_resolve_name        ; 0x0a
        dd      0;ipc_payload_sender      ; 0x0b
        dd      0;ipc_payload_length      ; 0x0c
        dd      0 ;ipc_await              0x0d - dummy entry
        dd      0                       ; 0x0e
        dd      0                       ; 0x0f
        dd      get_heap_base           ; 0x10
        dd      get_heap_size           ; 0x11
        dd      resize_heap             ; 0x12
        dd      0                       ; 0x13
        dd      0                       ; 0x14
        dd      getpid                  ; 0x15
        dd      signal                  ; 0x16
        dd      0                       ; 0x17
        dd      0                       ; 0x18
        dd      0                       ; 0x19
        dd      pwd                     ; 0x1a
        dd      what_stdin              ; 0x1b
        dd      what_stdout             ; 0x1c
        dd      what_stderr             ; 0x1d
        dd      0                       ; 0x1e
        dd      0                       ; 0x1f
        dd      0;register_vdev           ; 0x20
        dd      0;vdev_in_ready           ; 0x21
        dd      0;vdev_out_ready          ; 0x22
        dd      0 ;vdev_await           ; 0x23 - dummy entry
        dd      0                       ; 0x24
        dd      0                       ; 0x25
        dd      0                       ; 0x26
        dd      0                       ; 0x27
        dd      0                       ; 0x28
        dd      0                       ; 0x29
        dd      open                    ; 0x2a
        dd      close                   ; 0x2b
        dd      0 ;read                 ; 0x2c - dummy entry
        dd      0 ;write                ; 0x2d - dummy entry
        dd      lseek                   ; 0x2e
        dd      vfs_cd                  ; 0x2f
        dd      0 ;vfs_read             ; 0x30 - dummy entry
        dd      0 ;vfs_write            ; 0x31 - dummy entry
        dd      vfs_list                ; 0x32
        dd      vfs_get_metadata        ; 0x33
        dd      vfs_remove              ; 0x34
        dd      vfs_mkdir               ; 0x35
        dd      vfs_create              ; 0x36

section .text

bits 64

handler_simple:
        iretq

handler_code:
        add rsp, 8
        iretq

handler_irq_apic:
        call eoi_wrapper
        iretq

handler_irq_pic0:
        push rax
        mov al, 0x20    ; acknowledge interrupt to PIC0
        out 0x20, al
        pop rax
        iretq

handler_irq_pic1:
        push rax
        mov al, 0x20    ; acknowledge interrupt to both PICs
        out 0xA0, al
        out 0x20, al
        pop rax
        iretq

eoi_wrapper:
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
        ret

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
        call except_div0

handler_gpf:
        call except_handler_setup
        call except_gen_prot_fault

handler_pf:
        call except_handler_setup
        call except_page_fault

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
        add rdi, (24 - 1) * 8
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
        cmp rax, 0x30
        je vfs_read_isr
        cmp rax, 0x31
        je vfs_write_isr
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
        xchg rcx, rdi
        xchg rdx, rsi
        call [routine_list+rax]
        ; disable all interrupts, reenable task switch
        cli
        mov qword [ts_enable], 1
        mov rbx, qword [interrupted_cr3]
        mov cr3, rbx    ; restore context
        ; return
        popas
        iretq

vfs_read_isr:
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
        xchg rcx, rdi
        xchg rdx, rsi
        call vfs_read
        ; disable all interrupts, reenable task switch
        cli
        mov dword [ts_enable], 1
        push rbx
        mov rbx, qword [interrupted_cr3]
        mov cr3, rbx    ; restore context
        pop rbx
        ; done
        popas
        cmp rax, -5     ; if I/O is not ready
        je .enter_iowait
        iretq           ; else, just return
    .enter_iowait:
        pusham
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov rax, qword [kernel_pagemap]   ; context swap to kernel
        mov cr3, rax
        mov r8, 0      ; VFS read type
        xchg rcx, rdi
        xchg rdx, rsi
        call enter_iowait_status
        mov rdi, rsp
        add rdi, (24 - 1) * 8
        call task_switch

vfs_write_isr:
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
        xchg rcx, rdi
        xchg rdx, rsi
        call vfs_write
        ; disable all interrupts, reenable task switch
        cli
        mov dword [ts_enable], 1
        push rbx
        mov rbx, qword [interrupted_cr3]
        mov cr3, rbx    ; restore context
        pop rbx
        ; done
        popas
        cmp rax, -5     ; if I/O is not ready
        je .enter_iowait
        iretq           ; else, just return
    .enter_iowait:
        pusham
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov rax, qword [kernel_pagemap]   ; context swap to kernel
        mov cr3, rax
        mov r8, 1      ; VFS write type
        xchg rcx, rdi
        xchg rdx, rsi
        call enter_iowait_status
        mov rdi, rsp
        add rdi, (24 - 1) * 8
        call task_switch

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
        xchg rcx, rdi
        xchg rdx, rsi
        call read
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
        mov r9, rax
        mov r8, 2      ; read type
        xchg rcx, rdi
        xchg rdx, rsi
        call enter_iowait_status1
        mov rdi, rsp
        add rdi, (24 - 1) * 8
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
        xchg rcx, rdi
        xchg rdx, rsi
        call write
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
        mov r9, rax
        mov r8, 3
        xchg rcx, rdi
        xchg rdx, rsi
        call enter_iowait_status1
        mov rdi, rsp
        add rdi, (24 - 1) * 8
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
        add rdi, (24 - 1) * 8
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
        add rdi, (24 - 1) * 8
        call task_fork
