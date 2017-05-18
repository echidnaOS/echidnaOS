global load_GDT

section .data

align 4
GDT:
    dw .GDTEnd - .GDTStart - 1	; GDT size
    dd .GDTStart				; GDT start

    align 4
    .GDTStart:

    .NullDescriptor:
        dw 0x0000			; Limit
        dw 0x0000			; Base (low 16 bits)
        db 0x00				; Base (mid 8 bits)
        db 00000000b		; Access
        db 00000000b		; Granularity
        db 0x00				; Base (high 8 bits)

    .KernelCode:
        dw 0xFFFF			; Limit
        dw 0x0000			; Base (low 16 bits)
        db 0x00				; Base (mid 8 bits)
        db 10011010b		; Access
        db 11001111b		; Granularity
        db 0x00				; Base (high 8 bits)

    .KernelData:
        dw 0xFFFF			; Limit
        dw 0x0000			; Base (low 16 bits)
        db 0x00				; Base (mid 8 bits)
        db 10010010b		; Access
        db 11001111b		; Granularity
        db 0x00				; Base (high 8 bits)

    .UserCode:
        dw 0xFFFF			; Limit
        dw 0x0000			; Base (low 16 bits)
        db 0x00				; Base (mid 8 bits)
        db 11111010b		; Access
        db 11001111b		; Granularity
        db 0x00				; Base (high 8 bits)

    .UserData:
        dw 0xFFFF			; Limit
        dw 0x0000			; Base (low 16 bits)
        db 0x00				; Base (mid 8 bits)
        db 11110010b		; Access
        db 11001111b		; Granularity
        db 0x00				; Base (high 8 bits)

    .UnrealCode:
        dw 0xFFFF			; Limit
        dw 0x0000			; Base (low 16 bits)
        db 0x00				; Base (mid 8 bits)
        db 10011010b		; Access
        db 10001111b		; Granularity
        db 0x00				; Base (high 8 bits)

    .UnrealData:
        dw 0xFFFF			; Limit
        dw 0x0000			; Base (low 16 bits)
        db 0x00				; Base (mid 8 bits)
        db 10010010b		; Access
        db 10001111b		; Granularity
        db 0x00				; Base (high 8 bits)

    .GDTEnd:

section .text

bits 32

load_GDT:
    lgdt [GDT]
    jmp 0x08:.load_cs
    .load_cs:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret
