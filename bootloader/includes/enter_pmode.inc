enter_pmode:

; prepare the page directory and page table
; identity map the first 16 MiB of RAM

; build the 4 identity mapped page tables starting at 0x10000
mov edi, 0x10000
mov eax, 0x07
mov ecx, 1024 * 4
.loop:
    a32 o32 stosd
    add eax, 0x1000
    loop .loop
; build the page directory
mov edx, edi ; save starting address of page directory
mov eax, 0x10007
a32 o32 stosd
mov eax, 0x11007
a32 o32 stosd
mov eax, 0x12007
a32 o32 stosd
mov eax, 0x13007
a32 o32 stosd
mov ecx, 1024 - 4   ; zero the rest of the page directory
xor eax, eax
a32 o32 rep stosd

mov cr3, edx

cli						; Disable interrupts

mov eax, cr0			; enter pmode
or eax, 0x80000001
mov cr0, eax

jmp 0x18:.pmode

bits 32

.pmode:					; Now in protected mode

mov ax, 0x20
mov ds, ax
mov es, ax
mov fs, ax
mov gs, ax
mov ss, ax
