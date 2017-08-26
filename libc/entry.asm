global _entry

extern main

section .text

bits 32

_entry:
    jmp short .entry_point
    ; executable file header (not specified yet)
    db "ECH_EXEC", 0
    
    align 16
  .entry_point:
    push 0x1010     ; argc and argv
    push dword [0x1000]
    call main       ; call the main function
    add esp, 8
    
    mov ecx, eax    ; put the return value from EAX:EDX to ECX:EDX
    mov eax, 0x00   ; syscall 0, terminate
    int 0x80        ; syscall
