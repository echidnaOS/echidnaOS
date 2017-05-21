extern map_PIC

global real_routine

section .data

%define real_init_size  real_init_end - real_init
real_init:              incbin "kernel/asm/i686/real_init.bin"
real_init_end:

section .text

bits 32

real_routine:
    ; ESI = routine location
    ; ECX = routine size
    
    push esi
    push ecx
    
    ; Disable ints
    cli
    
    ; Remap PIC for a real mode environment
    push 0x00000070
    push 0x00000008
    call map_PIC
    add esp, 8

    ; Real mode init blob to 0000:1000
    mov esi, real_init
    mov edi, 0x1000
    mov ecx, real_init_size
    rep movsb
    
    ; Routine's blob to 0000:8000
    pop ecx
    pop esi
    mov edi, 0x8000
    rep movsb
    
    ; Call module
    call 0x1000
    
    ; Remap PIC for a pmode environment
    push 0x00000028
    push 0x00000020
    call map_PIC
    add esp, 8
    
    ; Reenable ints and return
    sti
    ret
