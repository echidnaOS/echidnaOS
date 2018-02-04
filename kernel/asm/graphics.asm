extern real_routine

global get_vbe_info
global get_edid_info
global get_vbe_mode_info

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
