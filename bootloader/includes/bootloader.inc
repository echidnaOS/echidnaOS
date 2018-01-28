
%include 'bootloader/includes/stage1.inc'

; ************************* STAGE 2 ************************

; ***** A20 *****

mov si, A20Msg					; Display A20 message
call simple_print

call enable_a20					; Enable the A20 address line to access the full memory
jc err							; If it fails, print an error and halt

mov si, DoneMsg
call simple_print				; Display done message

; ***** Unreal Mode *****

mov si, UnrealMsg				; Display unreal message
call simple_print

lgdt [GDT]						; Load the GDT

%include 'bootloader/includes/enter_unreal.inc'		; Enter Unreal Mode

mov si, DoneMsg
call simple_print				; Display done message

; ***** Kernel *****

; Load the kernel to 0x100000 (1 MiB)

mov si, KernelMsg				; Show loading kernel message
call simple_print

mov esi, kernel_name
mov ebx, 0x100000				; Load to offset 0x100000
call load_file

jc err							; Catch any error

mov si, DoneMsg
call simple_print				; Display done message

jmp arch_specific

;Data

kernel_name		db 'echidna.bin', 0x00
initfs_name		db 'initramfs.img', 0x00
A20Msg			db 'Enabling A20 line...', 0x00
UnrealMsg		db 'Entering Unreal Mode...', 0x00
KernelMsg		db 'Loading kernel...', 0x00

;Includes

%include 'bootloader/includes/echfs.inc'
%include 'bootloader/includes/disk2.inc'
%include 'bootloader/includes/a20_enabler.inc'
%include 'bootloader/includes/gdt.inc'
