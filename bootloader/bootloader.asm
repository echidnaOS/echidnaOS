org 0x7C00						; BIOS loads us here (0000:7C00)
bits 16							; 16-bit real mode code

%include 'bootloader/includes/bootloader.inc'


arch_specific:

%include 'bootloader/includes/enter_pmode.inc'		; Enter Protected Mode

; *** Setup registers ***

mov esp, 0xEFFFF0

; void kernel_init(uint8_t boot_drive);
xor eax, eax
mov al, byte [drive_number]
push eax

xor eax, eax
xor ebx, ebx
xor ecx, ecx
xor edx, edx
xor esi, esi
xor edi, edi
xor ebp, ebp

call 0x100000					; Jump to the newly loaded kernel

times 4096-($-$$)			db 0x00				; Padding
