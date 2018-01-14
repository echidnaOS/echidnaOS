extern real_routine

global disk_load_sector
global disk_write_sector
global read_drive_parameters

section .data

%define disk_load_sector_size           disk_load_sector_end - disk_load_sector_bin
disk_load_sector_bin:                   incbin "blobs/disk_load_sector.bin"
disk_load_sector_end:

%define disk_write_sector_size          disk_write_sector_end - disk_write_sector_bin
disk_write_sector_bin:                  incbin "blobs/disk_write_sector.bin"
disk_write_sector_end:

%define read_drive_parameters_size      read_drive_parameters_end - read_drive_parameters_bin
read_drive_parameters_bin:              incbin "blobs/read_drive_parameters.bin"
read_drive_parameters_end:

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

read_drive_parameters:
; void read_drive_parameters(drive_parameters_t* struct);
    push ebx
    push esi
    push edi
    push ebp

; Call real mode routine
    mov ebx, dword [esp+20]
    mov esi, read_drive_parameters_bin
    mov ecx, read_drive_parameters_size
    call real_routine
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret
