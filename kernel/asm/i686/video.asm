extern real_routine

;global vga_set_mode
;global vga_disable_cursor
global real_test

section .data

%define test_size   test_end - test
test:               incbin "kernel/asm/i686/test.bin"
test_end:

section .text

bits 32

real_test:
xchg bx, bx
    mov esi, test
    mov ecx, test_size
    call real_routine
    ret
