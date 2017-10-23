/* note these headers are all provided by newlib - you don't need to provide them */
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>

#include "sys_api.h"
 
void _exit(void) {

    OS_exit(0);

}

int close(int file) {

    return OS_close(file);

}

char *__env[1] = { 0 };
char **environ = __env;

int execve(char *name, char **argv, char **env) {

    return -1;
    
}

int fork(void) {

    return OS_fork();

}

int fstat(int file, struct stat *st) {

    st->st_mode = S_IFCHR;
    return 0;

}

int getpid(void) {

    return OS_getpid();

}

int isatty(int file) {

    return 0;

}

int kill(int pid, int sig) {

    return -1;

}

int link(char *old, char *new) {

    return -1;

}

int lseek(int file, int ptr, int dir) {

    return OS_lseek(file, ptr, dir);

}

int open(const char *name, int flags, ...) {

    return OS_open(name, flags, 0);

}

int read(int file, char *ptr, int len) {

    return OS_read(file, ptr, len);

}

caddr_t sbrk(int size) {

    uint32_t ptr = OS_get_heap_base() + OS_get_heap_size();
    OS_resize_heap(OS_get_heap_size() + size);
    return (caddr_t)ptr;

}

int stat(const char *file, struct stat *st) {

    st->st_mode = S_IFCHR;
    return 0;

}

clock_t times(struct tms *buf) {

    return -1;

}

int unlink(char *name) {

    if (OS_vfs_remove(name) == VFS_FAILURE)
        return -1;
    
    return 0;

}

int wait(int *status) {

    return -1;

}

int write(int file, char *ptr, int len) {

    return OS_write(file, ptr, len);

}





















