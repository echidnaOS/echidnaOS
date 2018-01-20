global task_spinup

section .data

    new_cr3     dd  0

section .text

bits 32

task_spinup:

    mov ebx, dword [esp+4]
    mov eax, dword [esp+8]
    mov dword [new_cr3], eax
    mov eax, dword [ebx]
    mov ecx, dword [ebx+8]
    mov edx, dword [ebx+12]
    mov esi, dword [ebx+16]
    mov edi, dword [ebx+20]
    mov ebp, dword [ebx+24]

    push dword [ebx+56]
    push dword [ebx+28]
    push dword [ebx+60]
    push dword [ebx+36]
    push dword [ebx+32]

    push dword [ebx+44]
    pop es
    push dword [ebx+48]
    pop fs
    push dword [ebx+52]
    pop gs
    push dword [ebx+40]
    pop ds
    push dword [ebx+4]
    mov ebx, dword [new_cr3]
    mov cr3, ebx
    pop ebx

    iretd
