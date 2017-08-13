global handler_simple
global handler_code
global handler_irq_pic0
global handler_irq_pic1
global handler_div0
global irq0_handler
global keyboard_isr
global syscall

extern keyboard_handler
extern task_switch
extern except_div0

; API calls
extern task_quit
extern alloc
extern free
extern realloc
extern char_to_stdout
extern enter_iowait_status
extern enter_ipcwait_status
extern pwd
extern ipc_send_packet
extern ipc_read_packet
extern ipc_resolve_name
extern ipc_payload_sender
extern ipc_payload_length
extern vfs_read
extern vfs_list

section .data

routine_list:
        dd      task_quit               ; 0x00
        dd      0                       ; 0x01
        dd      0                       ; 0x02
        dd      0                       ; 0x03
        dd      0                       ; 0x04
        dd      0                       ; 0x05
        dd      0                       ; 0x06
        dd      0                       ; 0x07
        dd      ipc_send_packet         ; 0x08
        dd      ipc_read_packet         ; 0x09
        dd      ipc_resolve_name        ; 0x0a
        dd      ipc_payload_sender      ; 0x0b
        dd      ipc_payload_length      ; 0x0c
        dd      0 ;ipc_await              0x0d - dummy entry
        dd      0                       ; 0x0e
        dd      0                       ; 0x0f
        dd      alloc                   ; 0x10
        dd      free                    ; 0x11
        dd      realloc                 ; 0x12
        dd      0                       ; 0x13
        dd      0                       ; 0x14
        dd      0                       ; 0x15
        dd      0                       ; 0x16
        dd      0                       ; 0x17
        dd      0                       ; 0x18
        dd      0                       ; 0x19
        dd      pwd                     ; 0x1a
        dd      0                       ; 0x1b
        dd      0                       ; 0x1c
        dd      0                       ; 0x1d
        dd      0                       ; 0x1e
        dd      0                       ; 0x1f
        dd      char_to_stdout          ; 0x20
        dd      0 ;char_from_stdin        0x21 - dummy entry
        dd      0                       ; 0x22
        dd      0                       ; 0x23
        dd      0                       ; 0x24
        dd      0                       ; 0x25
        dd      0                       ; 0x26
        dd      0                       ; 0x27
        dd      0                       ; 0x28
        dd      0                       ; 0x29
        dd      0                       ; 0x2a
        dd      0                       ; 0x2b
        dd      0                       ; 0x2c
        dd      0                       ; 0x2d
        dd      0                       ; 0x2e
        dd      0                       ; 0x2f
        dd      vfs_read                ; 0x30
        dd      0                       ; 0x31
        dd      vfs_list                ; 0x32

section .text

bits 32

handler_simple:
        iretd

handler_code:
        add esp, 4
        iretd

handler_irq_pic0:
        push eax
        mov al, 0x20    ; acknowledge interrupt to PIC0
        out 0x20, al
        pop eax
        iretd

handler_irq_pic1:
        push eax
        mov al, 0x20    ; acknowledge interrupt to both PICs
        out 0xA0, al
        out 0x20, al
        pop eax
        iretd

handler_div0:
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        call except_div0

irq0_handler:
        ; save task status
        push gs
        push fs
        push es
        push ds
        push ebp
        push edi
        push esi
        push edx
        push ecx
        push ebx
        push eax        
        mov al, 0x20    ; acknowledge interrupt to PIC0
        out 0x20, al
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        call task_switch

keyboard_isr:
        push eax
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        push ds
        push es
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        xor eax, eax
        in al, 0x60     ; read from keyboard
        push eax
        call keyboard_handler
        add esp, 4
        mov al, 0x20    ; acknowledge interrupt to PIC0
        out 0x20, al
        pop es
        pop ds
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        iretd

syscall:
; ARGS in EAX (call code), ECX, EDX, EDI, ESI
; return value in EAX/EDX
        ; special routines check
        cmp eax, 0x21
        je char_from_stdin
        cmp eax, 0x0d
        je ipc_await
        ; end special routines check
        push ebx
        push ecx
        push esi
        push edi
        push ebp
        push ds
        push es
        mov bx, 0x10
        mov ds, bx
        mov es, bx
        mov ebx, 4
        push edx
        mul ebx
        pop edx
        push esi
        push edi
        push edx
        push ecx
        call [routine_list+eax]
        add esp, 16
        pop es
        pop ds
        pop ebp
        pop edi
        pop esi
        pop ecx
        pop ebx
        iretd

char_from_stdin:
        ; save task status
        push gs
        push fs
        push es
        push ds
        push ebp
        push edi
        push esi
        push edx
        push ecx
        push ebx
        push eax
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        call enter_iowait_status
        call task_switch

ipc_await:
        ; save task status
        push gs
        push fs
        push es
        push ds
        push ebp
        push edi
        push esi
        push edx
        push ecx
        push ebx
        push eax
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        call enter_ipcwait_status
        call task_switch
