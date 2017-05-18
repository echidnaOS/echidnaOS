extern real_routine

global vga_disable_cursor

section .data

%define vga_disable_cursor_size         vga_disable_cursor_end - vga_disable_cursor_bin
vga_disable_cursor_bin:                 incbin "kernel/asm/i686/vga_disable_cursor.bin"
vga_disable_cursor_end:

section .text

bits 32

vga_disable_cursor:
    mov esi, vga_disable_cursor_bin
    mov ecx, vga_disable_cursor_size
    call real_routine
    ret
