extern map_PIC
extern get_PIC0_mask
extern get_PIC1_mask
extern set_PIC0_mask
extern set_PIC1_mask

global real_routine

section .data

%define real_init_size  real_init_end - real_init
real_init:              incbin "blobs/real_init.bin"
real_init_end:

PIC0_saved_mask dd 0
PIC1_saved_mask dd 0

section .text

bits 32

real_routine:
    ; ESI = routine location
    ; ECX = routine size
    
    push esi
    push ecx
    
    ; Remap PIC for a real mode environment
    push 0x00000070
    push 0x00000008
    call map_PIC
    add esp, 8
    
    ; Save PIC masks
    push eax
    call get_PIC0_mask
    mov dword [PIC0_saved_mask], eax
    call get_PIC1_mask
    mov dword [PIC1_saved_mask], eax
    pop eax
    
    ; Change PIC masks
    push 10111110b
    call set_PIC0_mask
    add esp, 4
    push 00111110b
    call set_PIC1_mask
    add esp, 4

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
    
    ; Restore PIC masks
    push dword [PIC0_saved_mask]
    call set_PIC0_mask
    add esp, 4
    push dword [PIC1_saved_mask]
    call set_PIC1_mask
    add esp, 4
    
    ret
