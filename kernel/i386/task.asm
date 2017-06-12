global task_spinup

section .text

bits 32

task_spinup:

    mov ebx, dword [esp+4]
    mov eax, dword [ebx+16]
    mov ecx, dword [ebx+24]
    mov edx, dword [ebx+28]
    mov esi, dword [ebx+32]
    mov edi, dword [ebx+36]
    mov ebp, dword [ebx+40]

    push dword [ebx+72]
    push dword [ebx+44]
    push dword [ebx+76]
    push dword [ebx+52]
    push dword [ebx+48]

    push dword [ebx+60]
    pop es
    push dword [ebx+64]
    pop fs
    push dword [ebx+68]
    pop gs
    push dword [ebx+56]
    mov ebx, dword [ebx+20]
    pop ds

    iretd
