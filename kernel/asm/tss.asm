global TSS
global TSS_size

section .data

align 4
TSS_begin:
    dd 0
    dd 0xEFFFF0
    dd 0x10
    times 23 dd 0
TSS_end:

TSS dd TSS_begin
TSS_size dd TSS_end - TSS_begin
