global kernel_shell

extern task_start
extern start_tasks

section .data

%define shell_size          shell_end - shell
shell:                      incbin "../shell/shell.bin"
shell_end:

task_info:
    .addr       dd  shell
    .size       dd  shell_size
    .stdin      dd  0
    .stdout     dd  0
    .stderr     dd  0
    .tty        dd  1
    .stack      dd  0x10000
    .heap       dd  0x100000
    .pwd        dd  pwd
    .name       dd  name

pwd db "/", 0
name db "shell", 0

section .text

bits 32

kernel_shell:
    push task_info
    call task_start
    mov dword [task_info.tty], 2
    call task_start
    ;mov dword [task_info.tty], 3
    ;call task_start
    ;mov dword [task_info.tty], 4
    ;call task_start
    ;mov dword [task_info.tty], 5
    ;call task_start
    ;mov dword [task_info.tty], 6
    ;call task_start
    add esp, 4
    ret
