global init_tasks

extern task_start

section .data

%define shell_size          shell_end - shell
shell:                      incbin "../shell/shell.bin"
shell_end:

%define server_size         server_end - server
server:                     incbin "../server/server.bin"
server_end:

shell1_info:
    .addr       dd  shell
    .size       dd  shell_size
    .stdin      dd  tty1
    .stdout     dd  tty1
    .stderr     dd  tty1
    .tty        dd  1
    .stack      dd  0x10000
    .heap       dd  0x100000
    .pwd        dd  pwd
    .name       dd  shell_name
    .server_name    dd none

shell2_info:
    .addr       dd  shell
    .size       dd  shell_size
    .stdin      dd  tty2
    .stdout     dd  tty2
    .stderr     dd  tty2
    .tty        dd  2
    .stack      dd  0x10000
    .heap       dd  0x100000
    .pwd        dd  pwd1
    .name       dd  shell_name
    .server_name    dd none

server_info:
    .addr       dd  server
    .size       dd  server_size
    .stdin      dd  tty3
    .stdout     dd  tty3
    .stderr     dd  tty3
    .tty        dd  3
    .stack      dd  0x10000
    .heap       dd  0x100000
    .pwd        dd  pwd
    .name       dd  server_name
    .server_name    dd server_name

pwd db "/", 0
pwd1 db "/dev", 0
shell_name db "shell", 0
server_name db "server", 0
none db 0

tty1 db '/dev/tty1', 0
tty2 db '/dev/tty2', 0
tty3 db '/dev/tty3', 0

section .text

bits 32

init_tasks:
    push shell1_info
    call task_start
    add esp, 4
    push shell2_info
    call task_start
    add esp, 4
    push server_info
    call task_start
    add esp, 4
    ret
