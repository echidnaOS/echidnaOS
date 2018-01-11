extern real_routine

global graphics_init

section .data

%define graphics_init_size           graphics_init_end - graphics_init_bin
graphics_init_bin:                   incbin "blobs/graphics_init.bin"
graphics_init_end:

graphics_init:
    ; void graphics_init(vbe_info_struct_t* vbe_info_struct);
    push ebx
    push esi
    push edi
    push ebp

    mov ebx, dword [esp+20]
    mov esi, graphics_init_bin
    mov ecx, graphics_init_size
    call real_routine

    pop ebp
    pop edi
    pop esi
    pop ebx
    ret
