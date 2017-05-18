;global vga_set_mode
;global vga_disable_cursor
global real_test

section .data

%define real_init_size real_init_end - real_init
real_init:          incbin "kernel/asm/i686/real_init.bin"
real_init_end:

%define test_size   test_end - test
test:               incbin "kernel/asm/i686/test.bin"
test_end:

section .text

bits 32

real_test:
    ; Real mode init blob to 0000:1000
    mov esi, real_init
    mov edi, 0x1000
    mov ecx, real_init_size
    rep movsb
    
    ; Test blob to 0000:8000
    mov esi, test
    mov edi, 0x8000
    mov ecx, test_size
    rep movsb
    
    ; Call module
    call 0x08:0x1000
    ret
