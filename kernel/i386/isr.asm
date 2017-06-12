global handler_simple
global handler_code
global handler_irq_pic0
global handler_irq_pic1
global irq0_handler
global keyboard_isr
global syscall

extern keyboard_handler
extern task_switch

; API calls
extern char_to_stdout
extern char_from_stdin
extern alloc
extern freemem
extern text_clear

section .data

routine_list:
        times 0x20 dd 0
        dd      char_to_stdout          ; 0x20
        dd      char_from_stdin         ; 0x21
        dd      alloc                   ; 0x22
        dd      freemem                 ; 0x23
        dd      text_clear              ; 0x24

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
        mul ebx
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
