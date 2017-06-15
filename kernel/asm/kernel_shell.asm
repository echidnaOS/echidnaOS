global kernel_shell

extern task_start
extern start_tasks

section .data

%define shell_size          shell_end - shell
shell:                      incbin "../shell/shell.bin"
shell_end:

section .text

bits 32

kernel_shell:
    push 0
    push shell_size
    push shell
    call task_start
    mov dword [esp+8], 1
    call task_start
    mov dword [esp+8], 2
    call task_start
    mov dword [esp+8], 3
    call task_start
    mov dword [esp+8], 4
    call task_start
    mov dword [esp+8], 5
    call task_start
    add esp, 12
    call start_tasks
