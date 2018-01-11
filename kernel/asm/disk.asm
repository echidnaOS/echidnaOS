extern real_routine

global disk_load_sector
global disk_write_sector

section .data

%define disk_load_sector_size           disk_load_sector_end - disk_load_sector_bin
disk_load_sector_bin:                   incbin "blobs/disk_load_sector.bin"
disk_load_sector_end:

%define disk_write_sector_size          disk_write_sector_end - disk_write_sector_bin
disk_write_sector_bin:                  incbin "blobs/disk_write_sector.bin"
disk_write_sector_end:

function_struct:
    .source_sector_low      dd  0
    .source_sector_high     dd  0
    .target_address         dd  0
    .drive                  db  0

section .text

bits 32

disk_load_sector:
; void disk_load_sector(uint8_t drive, uint8_t* target_address, uint64_t source_sector);
    push ebx
    push esi
    push edi
    push ebp

; Prepare the struct
    mov eax, dword [esp+32]
    mov dword [function_struct.source_sector_high], eax
    mov eax, dword [esp+28]
    mov dword [function_struct.source_sector_low], eax
    mov eax, dword [esp+24]
    mov dword [function_struct.target_address], eax
    mov eax, dword [esp+20]
    mov byte [function_struct.drive], al

; Call real mode routine
    mov ebx, function_struct
    mov esi, disk_load_sector_bin
    mov ecx, disk_load_sector_size
    call real_routine
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret

disk_write_sector:
; void disk_load_sector(uint8_t drive, uint8_t* target_address, uint64_t source_sector);
    push ebx
    push esi
    push edi
    push ebp

; Prepare the struct
    mov eax, dword [esp+32]
    mov dword [function_struct.source_sector_high], eax
    mov eax, dword [esp+28]
    mov dword [function_struct.source_sector_low], eax
    mov eax, dword [esp+24]
    mov dword [function_struct.target_address], eax
    mov eax, dword [esp+20]
    mov byte [function_struct.drive], al

; Call real mode routine
    mov ebx, function_struct
    mov esi, disk_write_sector_bin
    mov ecx, disk_write_sector_size
    call real_routine
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret
