#include <kernel.h>
#include <stdint.h>
#include <paging.h>
#include <klib.h>
#include <task.h>
#include <cio.h>
#include <panic.h>
#include <system.h>

#define FAILURE -1

volatile int general_ts_enable = 0;

task_t **task_table;

void task_init(void) {
    kprint(KPRN_INFO, "Task: Initialising task table...");
    // allocate the task table
    if ((task_table = kalloc(KRNL_MAX_TASKS * sizeof(task_t *))) == 0)
        panic("unable to allocate task table", 0);
    kprint(KPRN_INFO, "Task: Creating PID 0...");
    // create kernel task
    if ((task_table[0] = kalloc(sizeof(task_t))) == 0)
        panic("unable to allocate kernel task", 0);
    kstrcpy(task_table[0]->pwd, "/");
    task_table[0]->status = KRN_STAT_RES_TASK;
    task_table[0]->page_directory = kernel_pagemap;
    kprint(KPRN_INFO, "Task: Init done.");
    return;
}

void task_spinup(void *, pt_entry_t *, void *);
static void zombie_eval(int pid);

static const cpu_t default_cpu_status = { 0x23,0x23,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x1b,0x202,0,0x23 };

static int task_create(task_t new_task) {
    // find an empty entry in the task table
    int new_pid;
    for (new_pid = 0; new_pid < KRNL_MAX_TASKS; new_pid++)
        if ((!task_table[new_pid]) || (task_table[new_pid] == EMPTY_PID)) break;
    if (new_pid == KRNL_MAX_TASKS)
        return FAILURE;
    
    // allocate a task entry
    if ((task_table[new_pid] = kalloc(sizeof(task_t))) == 0) {
        task_table[new_pid] = EMPTY_PID;
        return FAILURE;
    }

    *task_table[new_pid] = new_task;
    
    return new_pid;
}

extern filesystem_t *filesystems;
int vfs_translate_fs(int mountpoint);

static int do_not_schedule = 0;

int execve(char *path, char **argv, char **envp) {
    int i;
    int argc;
    char *tmp_argv[256];
    char *tmp_envp[256];

    path = (char *)get_phys_addr(task_table[get_current_task()]->page_directory, (size_t)path);
    argv = (char **)get_phys_addr(task_table[get_current_task()]->page_directory, (size_t)argv);
    envp = (char **)get_phys_addr(task_table[get_current_task()]->page_directory, (size_t)envp);

    vfs_metadata_t metadata;
    if (vfs_kget_metadata(path, &metadata, FILE_TYPE) == -2) return FAILURE;

    size_t pages = (TASK_RESERVED_SPACE + metadata.size + DEFAULT_STACK) / PAGE_SIZE;
    if ((TASK_RESERVED_SPACE + metadata.size + DEFAULT_STACK) % PAGE_SIZE) pages++;

    /* copy argv in a tmp buffer */
    for (i = 0; i < 256; i++) {
        if (!argv[i])
            break;
        char *tmp_ptr = (char *)get_phys_addr(task_table[get_current_task()]->page_directory, (size_t)argv[i]);
        tmp_argv[i] = kalloc(kstrlen(tmp_ptr) + 1);
        kstrcpy(tmp_argv[i], tmp_ptr);
    }
    tmp_argv[i] = 0;
    argc = i;

    /* copy envp in a tmp buffer */
    for (i = 0; i < 256; i++) {
        if (!envp[i])
            break;
        char *tmp_ptr = (char *)get_phys_addr(task_table[get_current_task()]->page_directory, (size_t)envp[i]);
        tmp_envp[i] = kalloc(kstrlen(tmp_ptr) + 1);
        kstrcpy(tmp_envp[i], tmp_ptr);
    }
    tmp_envp[i] = 0;

    /* now destroy old userspace for the process */
    if (task_table[get_current_task()]->page_directory != kernel_pagemap)
        destroy_userspace(task_table[get_current_task()]->page_directory);

    /* reset CPU status */
    task_table[get_current_task()]->cpu = default_cpu_status;
    for (size_t i = 0; i < 512; i++)
        task_table[get_current_task()]->fxstate[i] = 0;

    // load program into memory
    size_t base = (size_t)kmalloc(pages);

    // use the new VFS stack
    int tmp_handle = vfs_kopen(path, O_RDWR, 0);
    vfs_kuread(tmp_handle, (char *)(base + TASK_RESERVED_SPACE), metadata.size);
    vfs_kclose(tmp_handle);

    // generate the page tables
    pt_entry_t *pd = new_userspace();
    // map the process's memory
    for (size_t i = 0; i < pages; i++)
        map_page(pd, TASK_BASE + i * PAGE_SIZE, base + i * PAGE_SIZE, 0x07);
    task_table[get_current_task()]->page_directory = pd;

    task_table[get_current_task()]->cpu.rsp = TASK_BASE + (((TASK_RESERVED_SPACE + metadata.size + DEFAULT_STACK) - 1) & 0xfffffff0);
    task_table[get_current_task()]->cpu.rip = TASK_BASE + TASK_RESERVED_SPACE;

    task_table[get_current_task()]->heap_base = TASK_BASE + pages * PAGE_SIZE;
    task_table[get_current_task()]->heap_size = 0;

    *((int *)(base + 0x1000)) = argc;
    int argv_limit = 0x4000;
    char **dest_argv = (char **)(base + 0x1010);

    /* prepare argv */
    for (i = 0; tmp_argv[i]; i++) {
        kstrcpy((char *)(base + argv_limit), tmp_argv[i]);
        dest_argv[i] = (char *)(TASK_BASE + argv_limit);
        argv_limit += kstrlen(tmp_argv[i]) + 1;
        kfree(tmp_argv[i]);
    }
    /* argv null ptr as per standard */
    dest_argv[i] = (char *)0;

    int envp_limit = 0x8000;
    char **dest_envp = (char **)(base + 0x1020);

    /* prepare environ */
    for (i = 0; tmp_envp[i]; i++) {
        kstrcpy((char *)(base + envp_limit), tmp_envp[i]);
        dest_envp[i] = (char *)(TASK_BASE + envp_limit);
        envp_limit += kstrlen(tmp_envp[i]) + 1;
        kfree(tmp_envp[i]);
    }
    /* environ null ptr as per standard */
    dest_envp[i] = (char *)0;

    if (do_not_schedule)
        return 0;

    DISABLE_INTERRUPTS;
    set_ts_enable(1);
    task_scheduler();
}

int kexec(  char *path, char **argv, char **envp,
            char *stdin, char *stdout, char *stderr,
            char *pwd ) {
    int new_pid;
    task_t new_task = {0};
    if ((new_pid = task_create(new_task)) == FAILURE)
        return FAILURE;

    // create file handles for std streams
    // this is a huge hack FIXME
    int khandle;
    khandle = vfs_kopen(stdin, O_RDONLY, 0);
    create_file_handle(new_pid, task_table[0]->file_handles[khandle]);
    task_table[0]->file_handles[khandle].free = 1;
    khandle = vfs_kopen(stdout, O_WRONLY, 0);
    create_file_handle(new_pid, task_table[0]->file_handles[khandle]);
    task_table[0]->file_handles[khandle].free = 1;
    khandle = vfs_kopen(stderr, O_WRONLY, 0);
    create_file_handle(new_pid, task_table[0]->file_handles[khandle]);
    task_table[0]->file_handles[khandle].free = 1;

    task_table[new_pid]->status = KRN_STAT_ACTIVE_TASK;
    task_table[new_pid]->parent = 0;
    task_table[new_pid]->page_directory = kernel_pagemap;

    kstrcpy(task_table[new_pid]->pwd, pwd);

    set_current_task(new_pid);

    do_not_schedule = 1;
    if (execve(path, argv, envp) == FAILURE) {
        do_not_schedule = 0;
        task_table[new_pid] = EMPTY_PID;
        set_current_task(0);
        return FAILURE;
    } else {
        do_not_schedule = 0;
        set_current_task(0);
        task_table[new_pid]->cpu_number = 0;
        return 0;
    }
}

void task_fork(cpu_t *cpu_state) {

    // forks the current task in a Unix-like way

    task_table[get_current_task()]->cpu = *cpu_state;

    dump_fx_state(task_table[get_current_task()]->fxstate);

    task_t new_process = *task_table[get_current_task()];
    /* parent */
    new_process.parent = get_current_task();
    // generate the page tables
    new_process.page_directory = fork_userspace(new_process.page_directory);

    // allocate memory for the new VFS's file descriptors
    if (!(new_process.file_handles = kalloc(task_table[get_current_task()]->file_handles_ptr * sizeof(file_handle_t)))) {
        // fail
    }

    // clone new VFS descriptors
    for (int i = 0; i < task_table[get_current_task()]->file_handles_ptr; i++) {
        file_handle_t new_handle = {0};
        if (task_table[get_current_task()]->file_handles[i].free) {
            new_process.file_handles[i].free = 1;
            continue;
        }
        new_handle.mountpoint = task_table[get_current_task()]->file_handles[i].mountpoint;
        int filesystem = vfs_translate_fs(new_handle.mountpoint);
        new_handle.internal_handle = (*filesystems[filesystem].fork)(task_table[get_current_task()]->file_handles[i].internal_handle);
        new_process.file_handles[i] = new_handle;
    }

    // attempt to create task
    int new_pid = task_create(new_process);

    // return the PID to the forking process
    task_table[get_current_task()]->cpu.rax = (uint32_t)new_pid;

    // return 0 in the child process
    task_table[new_pid]->cpu.rax = 0;

    task_scheduler();
}

int swait(int *status) {
    task_table[get_current_task()]->status = KRN_STAT_PROCWAIT_TASK;  // start waiting for the child process
    task_table[get_current_task()]->waitstat = status;
    return 0;
}

void task_switch(cpu_t *cpu_state) {

    task_table[get_current_task()]->cpu = *cpu_state;

    dump_fx_state(task_table[get_current_task()]->fxstate);

    set_current_task(get_current_task() + 1);
    task_scheduler();
}

extern int read_stat;
extern int write_stat;

size_t syscall_execute(size_t, size_t, size_t, size_t, size_t);

void task_scheduler(void) {
    int c;

    while (!general_ts_enable);

    for (;;) {
        if (!task_table[get_current_task()]) {
            set_current_task(0);
            if (get_idle_cpu()) {
                /* if no process took CPU time, wait for the next */
                /* context switch idling */
                ENTER_IDLE;
            }
            set_idle_cpu(1);
            continue;
        }
        
        if (task_table[get_current_task()] == EMPTY_PID) {
            set_current_task(get_current_task() + 1);
            continue;
        }

        if (!get_cpu_number())
            goto parser;

        if (task_table[get_current_task()]->cpu_number != get_cpu_number()
            || task_table[get_current_task()]->status != KRN_STAT_ACTIVE_TASK) {
            set_current_task(get_current_task() + 1);
            continue;
        }
        
parser:
        /* only CPU #0 will parse any state other than active */
        switch (task_table[get_current_task()]->status) {
            case KRN_STAT_DEFER_TASK: ;
                size_t syscall_ret = syscall_execute(
                    task_table[get_current_task()]->defer_arg0,
                    task_table[get_current_task()]->defer_arg1,
                    task_table[get_current_task()]->defer_arg2,
                    task_table[get_current_task()]->defer_arg3,
                    task_table[get_current_task()]->defer_syscall
                );
                task_table[get_current_task()]->cpu.rax = (uint64_t)(syscall_ret);
                task_table[get_current_task()]->status = KRN_STAT_ACTIVE_TASK;
                set_current_task(get_current_task() + 1);
                continue;
            case KRN_STAT_IOWAIT_TASK:
                switch (task_table[get_current_task()]->iowait_type) {
                    int done;
                    case 2:
                        done = read(task_table[get_current_task()]->iowait_handle,
                                    (char *)(task_table[get_current_task()]->iowait_ptr + task_table[get_current_task()]->iowait_done),
                                    task_table[get_current_task()]->iowait_len - task_table[get_current_task()]->iowait_done);
                        if (read_stat) {
                            task_table[get_current_task()]->iowait_done += done;
                        } else {
                            task_table[get_current_task()]->cpu.rax = (uint64_t)(task_table[get_current_task()]->iowait_done + done);
                            task_table[get_current_task()]->status = KRN_STAT_ACTIVE_TASK;
                        }
                        set_current_task(get_current_task() + 1);
                        continue;
                    case 3:
                        done = write(task_table[get_current_task()]->iowait_handle,
                                     (char *)(task_table[get_current_task()]->iowait_ptr + task_table[get_current_task()]->iowait_done),
                                     task_table[get_current_task()]->iowait_len - task_table[get_current_task()]->iowait_done);
                        if (write_stat) {
                            task_table[get_current_task()]->iowait_done += done;
                        } else {
                            task_table[get_current_task()]->cpu.rax = (uint64_t)(task_table[get_current_task()]->iowait_done + done);
                            task_table[get_current_task()]->status = KRN_STAT_ACTIVE_TASK;
                        }
                        set_current_task(get_current_task() + 1);
                        continue;
                    default:
                        panic("unrecognised iowait_type", task_table[get_current_task()]->iowait_type);
                }
            case KRN_STAT_ACTIVE_TASK:
                if (task_table[get_current_task()]->cpu_number != get_cpu_number()) {
                    set_current_task(get_current_task() + 1);
                    continue;
                }
                set_idle_cpu(0);
                task_spinup((void *)(&(task_table[get_current_task()]->cpu)), task_table[get_current_task()]->page_directory, task_table[get_current_task()]->fxstate);
            case KRN_STAT_ZOMBIE_TASK:
                zombie_eval(get_current_task());
                set_current_task(get_current_task() + 1);
                continue;
            case KRN_STAT_PROCWAIT_TASK:
                set_current_task(get_current_task() + 1);
                continue;
            case KRN_STAT_RES_TASK:
                set_current_task(get_current_task() + 1);
                continue;
            default:
                panic("unrecognised task status", task_table[get_current_task()]->status);
        }

    }

}

static void zombie_eval(int pid) {
    int parent = task_table[pid]->parent;

    if (task_table[parent]->status == KRN_STAT_PROCWAIT_TASK) {
        task_table[parent]->cpu.rax = (uint64_t)pid;
        task_table[parent]->status = KRN_STAT_ACTIVE_TASK;
        kfree((void *)task_table[pid]);
        task_table[pid] = EMPTY_PID;
    }

    return;
}

void task_quit_self(int64_t return_value) {
    task_quit(get_current_task(), return_value);
}

void task_quit(int pid, int64_t return_value) {
    task_table[pid]->return_value = return_value;
    task_table[pid]->status = KRN_STAT_ZOMBIE_TASK;
    kfree((void *)task_table[pid]->file_handles);
    destroy_userspace(task_table[pid]->page_directory);
    DISABLE_INTERRUPTS;
    set_ts_enable(1);
    task_scheduler();
}
