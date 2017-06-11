extern real_routine

global vga_disable_cursor
global vga_80_x_50

section .data

%define vga_disable_cursor_size         vga_disable_cursor_end - vga_disable_cursor_bin
vga_disable_cursor_bin:                 incbin "blobs/vga_disable_cursor.bin"
vga_disable_cursor_end:

%define vga_80_x_50_size                vga_80_x_50_end - vga_80_x_50_bin
vga_80_x_50_bin:                        incbin "blobs/vga_80_x_50.bin"
vga_80_x_50_end:

section .text

bits 32

vga_disable_cursor:
    push ebx
    push esi
    push edi
    push ebp
    mov esi, vga_disable_cursor_bin
    mov ecx, vga_disable_cursor_size
    call real_routine
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret

vga_80_x_50:
    push ebx
    push esi
    push edi
    push ebp
    mov esi, vga_80_x_50_bin
    mov ecx, vga_80_x_50_size
    call real_routine
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret
