global load_IDT

extern handler_simple
extern handler_code
extern handler_irq_pic0
extern handler_irq_pic1
extern keyboard_isr
extern syscall

section .data

align 4
IDT:
    dw .IDTEnd - .IDTStart - 1	; IDT size
    dd .IDTStart				; IDT start

    align 4
    .IDTStart:
        times 0x81 dq 0
    .IDTEnd:

section .text

bits 32

make_entry:
; EBX = address
; CX = selector
; DL = type
; DI = vector

    push eax
    push ebx
    push ecx
    push edx
    push edi

    push edx

    mov eax, 8
    and edi, 0x0000FFFF
    mul edi
    add eax, IDT.IDTStart
    mov edi, eax

    mov ax, bx
    stosw
    mov ax, cx
    stosw
    inc edi
    pop edx
    mov al, dl
    stosb
    shr ebx, 16
    mov ax, bx
    stosw

    pop edi
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret

load_IDT:
    push ebx
    push ecx
    push edx
    push edi
    
    xor di, di
    mov dl, 10001110b
    mov cx, 0x08
    mov ebx, handler_simple
    call make_entry                 ; int 0x00, divide by 0

    inc di
    call make_entry                 ; int 0x01, debug
    
    inc di
    call make_entry                 ; int 0x02, NMI
    
    inc di
    call make_entry                 ; int 0x03, breakpoint
    
    inc di
    call make_entry                 ; int 0x04, overflow
    
    inc di
    call make_entry                 ; int 0x05, bound range exceeded
    
    inc di
    call make_entry                 ; int 0x06, invalid opcode
    
    inc di
    call make_entry                 ; int 0x07, device not available
    
    inc di
    mov ebx, handler_code
    call make_entry                 ; int 0x08, double fault
    
    inc di
    mov ebx, handler_simple
    call make_entry                 ; int 0x09, coprocessor segment overrun
    
    add di, 2
    mov ebx, handler_code
    call make_entry                 ; int 0x0A, invalid TSS
    
    inc di
    call make_entry                 ; int 0x0B, segment not present
    
    inc di
    call make_entry                 ; int 0x0C, stack-segment fault
    
    inc di
    call make_entry                 ; int 0x0D, general protection fault
    
    inc di
    call make_entry                 ; int 0x0E, page fault
    
    add di, 2
    mov ebx, handler_simple
    call make_entry                 ; int 0x10, x87 floating point exception
    
    inc di
    mov ebx, handler_code
    call make_entry                 ; int 0x11, alignment check
    
    inc di
    mov ebx, handler_simple
    call make_entry                 ; int 0x12, machine check
    
    inc di
    call make_entry                 ; int 0x13, SIMD floating point exception
    
    inc di
    call make_entry                 ; int 0x14, virtualisation exception
    
    mov di, 0x1E
    mov ebx, handler_code
    call make_entry                 ; int 0x1E, security exception
    
    add di, 2
    mov ebx, handler_irq_pic0
    call make_entry
    
    inc di
    mov ebx, keyboard_isr
    call make_entry
    
    inc di
    mov ebx, handler_irq_pic0
    call make_entry
    
    inc di
    call make_entry
    
    inc di
    call make_entry
    
    inc di
    call make_entry
    
    inc di
    call make_entry
    
    inc di
    call make_entry
    
    inc di
    mov ebx, handler_irq_pic1
    call make_entry
    
    inc di
    call make_entry
    
    inc di
    call make_entry
    
    inc di
    call make_entry
    
    inc di
    call make_entry
    
    inc di
    call make_entry
    
    inc di
    call make_entry
    
    inc di
    call make_entry
    
    mov di, 0x80
    mov dl, 11101110b
    mov ebx, syscall
    call make_entry  
    
    lidt [IDT]
    
    pop edi
    pop edx
    pop ecx
    pop ebx
    ret
