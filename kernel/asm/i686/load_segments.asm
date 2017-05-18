global load_segments

section .data

section .text

bits 32

load_segments:
	jmp 0x08:.load_cs
	.load_cs:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss, ax
	ret
