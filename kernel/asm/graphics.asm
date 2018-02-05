extern real_routine

global get_vbe_info
global get_edid_info
global get_vbe_mode_info
global set_vbe_mode
global dump_vga_font

section .data

%define get_vbe_info_size           get_vbe_info_end - get_vbe_info_bin
get_vbe_info_bin:                   incbin "blobs/get_vbe_info.bin"
get_vbe_info_end:

%define get_edid_info_size           get_edid_info_end - get_edid_info_bin
get_edid_info_bin:                   incbin "blobs/get_edid_info.bin"
get_edid_info_end:

%define get_vbe_mode_info_size           get_vbe_mode_info_end - get_vbe_mode_info_bin
get_vbe_mode_info_bin:                   incbin "blobs/get_vbe_mode_info.bin"
get_vbe_mode_info_end:

%define set_vbe_mode_size           set_vbe_mode_end - set_vbe_mode_bin
set_vbe_mode_bin:                   incbin "blobs/set_vbe_mode.bin"
set_vbe_mode_end:

%define dump_vga_font_size           dump_vga_font_end - dump_vga_font_bin
dump_vga_font_bin:                   incbin "blobs/dump_vga_font.bin"
dump_vga_font_end:

get_vbe_info:
    ; void get_vbe_info(vbe_info_struct_t* vbe_info_struct);
    push ebx
    push esi
    push edi
    push ebp

    mov ebx, dword [esp+20]
    mov esi, get_vbe_info_bin
    mov ecx, get_vbe_info_size
    call real_routine

    pop ebp
    pop edi
    pop esi
    pop ebx
    ret

get_edid_info:
    ; void get_edid_info(edid_info_struct_t* edid_info_struct);
    push ebx
    push esi
    push edi
    push ebp

    mov ebx, dword [esp+20]
    mov esi, get_edid_info_bin
    mov ecx, get_edid_info_size
    call real_routine

    pop ebp
    pop edi
    pop esi
    pop ebx
    ret

get_vbe_mode_info:
    ; void get_vbe_mode_info(get_vbe_t* get_vbe);
    push ebx
    push esi
    push edi
    push ebp

    mov ebx, dword [esp+20]
    mov esi, get_vbe_mode_info_bin
    mov ecx, get_vbe_mode_info_size
    call real_routine

    pop ebp
    pop edi
    pop esi
    pop ebx
    ret

set_vbe_mode:
    ; void set_vbe_mode(uint16_t mode);
    push ebx
    push esi
    push edi
    push ebp

    mov ebx, dword [esp+20]
    mov esi, set_vbe_mode_bin
    mov ecx, set_vbe_mode_size
    call real_routine

    pop ebp
    pop edi
    pop esi
    pop ebx
    ret

dump_vga_font:
    ; void dump_vga_font(uint8_t *bitmap);
    push ebx
    push esi
    push edi
    push ebp

    mov ebx, dword [esp+20]
    mov esi, dump_vga_font_bin
    mov ecx, dump_vga_font_size
    call real_routine

    pop ebp
    pop edi
    pop esi
    pop ebx
    ret
