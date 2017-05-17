global handler_simple
global handler_code
global handler_irq_pic0
global handler_irq_pic1
global keyboard_isr
global syscall

extern keyboard_handler

; API calls
extern char_to_stdout
extern char_from_stdin

section .data

cpu_state_esp                   dd      cpu_state_stack
interrupted_esp                 dd      0

routine_list:
        times 0x20 dd 0                 ; Reserved syscalls
        dd      char_to_stdout
        dd      char_from_stdin

section .bss

align 4
resb 16384 ; 16 KiB
cpu_state_stack:

section .text

bits 32

handler_simple:
        iret

handler_code:
        add esp, 4
        iret

handler_irq_pic0:
        push eax
        mov al, 0x20    ; acknowledge interrupt to PIC0
        out 0x20, al
        pop eax
        iret

handler_irq_pic1:
        push eax
        mov al, 0x20    ; acknowledge interrupt to both PICs
        out 0xA0, al
        out 0x20, al
        pop eax
        iret

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
        push fs
        push gs
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        xor eax, eax
        in al, 0x60     ; read from keyboard
        push eax
        call keyboard_handler
        add esp, 4
        mov al, 0x20    ; acknowledge interrupt to PIC0
        out 0x20, al
        pop gs
        pop fs
        pop es
        pop ds
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        pop eax
        iret

syscall:
        mov dword [interrupted_esp], esp
        mov esp, dword [cpu_state_esp]
        push ebx
        push ecx
        push edx
        push esi
        push edi
        push ebp
        push ds
        push es
        push fs
        push gs
        mov bx, 0x10
        mov ds, bx
        mov es, bx
        mov fs, bx
        mov gs, bx
        mov dword [cpu_state_esp], esp
        mov esp, dword [interrupted_esp]
        pop ebx         ; EIP
        pop ecx         ; CS
        pop edx         ; FLAGS
        mov dword [interrupted_esp], esp
        mov esp, dword [cpu_state_esp]
        push edx
        push ecx
        push ebx
        mov dword [cpu_state_esp], esp
        mov esp, dword [interrupted_esp]
        mov ebx, 4
        mul ebx
        sti                             ; Interrupts enabled
        call [routine_list+eax]
        cli                             ; Interrupts disabled
xchg bx,bx
        mov dword [interrupted_esp], esp
        mov esp, dword [cpu_state_esp]
        pop ebx
        pop ecx
        pop edx
        mov dword [cpu_state_esp], esp
        mov esp, dword [interrupted_esp]
        push edx
        push ecx
        push ebx
        mov dword [interrupted_esp], esp
        mov esp, dword [cpu_state_esp]
        pop gs
        pop fs
        pop es
        pop ds
        pop ebp
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
        mov dword [cpu_state_esp], esp
        mov esp, dword [interrupted_esp]
        iret
