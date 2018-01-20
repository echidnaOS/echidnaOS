#ifndef __TASK_H__
#define __TASK_H__

#include <stdint.h>
#include <stddef.h>
#include <paging.h>
#include <vfs.h>

#define KRN_STAT_ACTIVE_TASK    1
#define KRN_STAT_RES_TASK       2
#define KRN_STAT_IOWAIT_TASK    3
#define KRN_STAT_IPCWAIT_TASK   4
#define KRN_STAT_PROCWAIT_TASK  5
#define KRN_STAT_VDEVWAIT_TASK  6

#define EMPTY_PID               (task_t*)0xffffffff
#define TASK_RESERVED_SPACE     0x10000
#define TASK_BASE               0x1000000

#define DEFAULT_STACK 0x10000

#define KRNL_MAX_TASKS 65536

/* i386 cpu struct */
typedef struct {

    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t eip;
    uint32_t cs;
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
    uint32_t ss;
    uint32_t eflags;

} cpu_t;

typedef struct {

    int status;
    int parent;

    pt_entry_t* page_directory;
    size_t text_base;

    cpu_t cpu;

    char pwd[2048];

    char stdin[2048];
    char stdout[2048];
    char stderr[2048];

    char iowait_dev[2048];
    uint64_t iowait_loc;
    int iowait_type;
    uint8_t iowait_payload;
    int iowait_handle;
    uint32_t iowait_ptr;
    int iowait_len;
    int iowait_done;

    size_t heap_base;
    size_t heap_size;

    // signals
    size_t sigabrt;
    size_t sigfpe;
    size_t sigill;
    size_t sigint;
    size_t sigsegv;
    size_t sigterm;

    file_handle_v2_t* file_handles_v2;
    int file_handles_v2_ptr;

} task_t;

typedef struct {
    char* path;
    char* stdin;
    char* stdout;
    char* stderr;
    char* pwd;
    char* unused0;
    char* unused1;
    int argc;
    char** argv;
} task_info_t;

extern task_t** task_table;
extern int current_task;

void task_init(void);
int general_execute(task_info_t*);
int general_execute_block(task_info_t*);
void task_scheduler(void);
void task_quit(int, int64_t);




#endif
