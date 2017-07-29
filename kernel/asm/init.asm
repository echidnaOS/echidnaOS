global init_tasks

extern task_start

section .data

%define shell_size          shell_end - shell
shell:                      incbin "../shell/shell.bin"
shell_end:

%define server_size         server_end - server
server:                     incbin "../server/server.bin"
server_end:

%define vfs_size            vfs_end - vfs
vfs:                        incbin "../vfs/vfs.bin"
vfs_end:

vfs_info:
    .addr       dd  vfs
    .size       dd  vfs_size
    .stdin      dd  0
    .stdout     dd  0
    .stderr     dd  0
    .tty        dd  0
    .stack      dd  0x10000
    .heap       dd  0x100000
    .pwd        dd  pwd
    .name       dd  vfs_name
    .server_name    dd vfs_name

shell1_info:
    .addr       dd  shell
    .size       dd  shell_size
    .stdin      dd  0
    .stdout     dd  0
    .stderr     dd  0
    .tty        dd  1
    .stack      dd  0x10000
    .heap       dd  0x100000
    .pwd        dd  pwd
    .name       dd  shell_name
    .server_name    dd none

shell2_info:
    .addr       dd  shell
    .size       dd  shell_size
    .stdin      dd  0
    .stdout     dd  0
    .stderr     dd  0
    .tty        dd  2
    .stack      dd  0x10000
    .heap       dd  0x100000
    .pwd        dd  pwd
    .name       dd  shell_name
    .server_name    dd none

server_info:
    .addr       dd  server
    .size       dd  server_size
    .stdin      dd  0
    .stdout     dd  0
    .stderr     dd  0
    .tty        dd  3
    .stack      dd  0x10000
    .heap       dd  0x100000
    .pwd        dd  pwd
    .name       dd  server_name
    .server_name    dd server_name

pwd db "/", 0
shell_name db "shell", 0
server_name db "server", 0
vfs_name db "vfs", 0
none db 0

section .text

bits 32

init_tasks:
    push vfs_info
    call task_start
    add esp, 4
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
