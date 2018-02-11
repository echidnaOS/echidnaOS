; This file contains the code that is gonna be linked at the beginning of
; the kernel binary.
; It should contain core CPU initialisation routines such as entering
; long mode, then it should call 'kernel_init'.

extern kernel_init

section .startup

startup:
    jmp kernel_init
