extern main

section .text

bits 32

__task_header__:
    push 0          ; argc and argv placeholder
    push 0
    call main       ; call the main function
    
    mov ecx, eax    ; put the return value from EAX:EDX to ECX:EDX
    mov eax, 0x00   ; syscall 0, terminate
    int 0x80        ; syscall
