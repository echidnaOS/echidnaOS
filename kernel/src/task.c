#include <kernel.h>
#include <stdint.h>
#include <paging.h>
#include <klib.h>
#include <task.h>
#include <cio.h>
#include <panic.h>

#define FAILURE -1

task_t **task_table;

void task_init(void) {
    // allocate the task table
    if ((task_table = kalloc(KRNL_MAX_TASKS * sizeof(task_t *))) == 0)
        panic("unable to allocate task table");
    // create kernel task
    if ((task_table[0] = kalloc(sizeof(task_t))) == 0)
        panic("unable to allocate kernel task");
    kstrcpy(task_table[0]->pwd, "/");
    task_table[0]->status = KRN_STAT_RES_TASK;
    task_table[0]->page_directory = (pt_entry_t*)KERNEL_PAGE;
    return;
}

int current_task = 0;

static int idle_cpu = 1;

void task_spinup(void *, pt_entry_t *);

static const cpu_t default_cpu_status = { 0,0,0,0,0,0,0,0,0,0x1b,0x23,0x23,0x23,0x23,0x23,0x202 };

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

int execve(char *path, char **argv, char **envp) {
    int i;
    int argc;
    char *tmp_argv[256];
    char *tmp_envp[256];

    path = (char *)get_phys_addr(task_table[current_task]->page_directory, (size_t)path);
    argv = (char **)get_phys_addr(task_table[current_task]->page_directory, (size_t)argv);
    envp = (char **)get_phys_addr(task_table[current_task]->page_directory, (size_t)envp);

    vfs_metadata_t metadata;
    if (vfs_kget_metadata(path, &metadata, FILE_TYPE) == -2) return FAILURE;

    size_t pages = (TASK_RESERVED_SPACE + metadata.size + DEFAULT_STACK) / PAGE_SIZE;
    if ((TASK_RESERVED_SPACE + metadata.size + DEFAULT_STACK) % PAGE_SIZE) pages++;

    /* copy argv in a tmp buffer */
    for (i = 0; i < 256; i++) {
        if (!argv[i])
            break;
        char *tmp_ptr = (char *)get_phys_addr(task_table[current_task]->page_directory, (size_t)argv[i]);
        tmp_argv[i] = kalloc(kstrlen(tmp_ptr) + 1);
        kstrcpy(tmp_argv[i], argv[i]);
    }
    tmp_argv[i] = 0;
    argc = i;

    /* copy envp in a tmp buffer */
    for (i = 0; i < 256; i++) {
        if (!envp[i])
            break;
        char *tmp_ptr = (char *)get_phys_addr(task_table[current_task]->page_directory, (size_t)envp[i]);
        tmp_envp[i] = kalloc(kstrlen(tmp_ptr) + 1);
        kstrcpy(tmp_envp[i], envp[i]);
    }
    tmp_envp[i] = 0;

    /* now destroy old userspace for the process */
    destroy_userspace(task_table[current_task]->page_directory);

    /* reset CPU status */
    task_table[current_task]->cpu = default_cpu_status;

    // load program into memory
    size_t base = (size_t)kmalloc(pages);

    // use the new VFS stack
    int tmp_handle = vfs_kopen(path, O_RDWR, 0);
    vfs_kuread(tmp_handle, (char *)(base + TASK_RESERVED_SPACE), metadata.size);
    vfs_kclose(tmp_handle);

    // generate the page tables
    pt_entry_t* pd = new_userspace();
    // map the process's memory
    for (size_t i = 0; i < pages; i++)
        map_page(pd, TASK_BASE + i * PAGE_SIZE, base + i * PAGE_SIZE, 0x07);
    task_table[current_task]->page_directory = pd;

    task_table[current_task]->cpu.esp = TASK_BASE + (((TASK_RESERVED_SPACE + metadata.size + DEFAULT_STACK) - 1) & 0xfffffff0);
    task_table[current_task]->cpu.eip = TASK_BASE + TASK_RESERVED_SPACE;

    task_table[current_task]->heap_base = TASK_BASE + pages * PAGE_SIZE;
    task_table[current_task]->heap_size = 0;

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

    DISABLE_INTERRUPTS;
    ts_enable = 1;
    task_scheduler();
}

void task_fork(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t edi, uint32_t ebp, uint32_t ds, uint32_t es, uint32_t fs, uint32_t gs, uint32_t eip, uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss) {

    // forks the current task in a Unix-like way

    task_table[current_task]->cpu.eax = eax;
    task_table[current_task]->cpu.ebx = ebx;
    task_table[current_task]->cpu.ecx = ecx;
    task_table[current_task]->cpu.edx = edx;
    task_table[current_task]->cpu.esi = esi;
    task_table[current_task]->cpu.edi = edi;
    task_table[current_task]->cpu.ebp = ebp;
    task_table[current_task]->cpu.esp = esp;
    task_table[current_task]->cpu.eip = eip;
    task_table[current_task]->cpu.cs = cs;
    task_table[current_task]->cpu.ds = ds;
    task_table[current_task]->cpu.es = es;
    task_table[current_task]->cpu.fs = fs;
    task_table[current_task]->cpu.gs = gs;
    task_table[current_task]->cpu.ss = ss;
    task_table[current_task]->cpu.eflags = eflags;

    task_t new_process = *task_table[current_task];
    /* parent */
    new_process.parent = current_task;
    // generate the page tables
    new_process.page_directory = fork_userspace(new_process.page_directory);

    // allocate memory for the new VFS's file descriptors
    if (!(new_process.file_handles_v2 = kalloc(task_table[current_task]->file_handles_v2_ptr * sizeof(file_handle_v2_t)))) {
        // fail
    }

    // clone new VFS descriptors
    for (int i = 0; i < task_table[current_task]->file_handles_v2_ptr; i++) {
        file_handle_v2_t new_handle = {0};
        if (task_table[current_task]->file_handles_v2[i].free) {
            new_process.file_handles_v2[i].free = 1;
            continue;
        }
        new_handle.mountpoint = task_table[current_task]->file_handles_v2[i].mountpoint;
        int filesystem = vfs_translate_fs(new_handle.mountpoint);
        new_handle.internal_handle = (*filesystems[filesystem].fork)(task_table[current_task]->file_handles_v2[i].internal_handle);
        new_process.file_handles_v2[i] = new_handle;
    }

    // attempt to create task
    int new_pid = task_create(new_process);

    // return the PID to the forking process
    task_table[current_task]->cpu.eax = (uint32_t)new_pid;

    // return 0 in the child process
    task_table[new_pid]->cpu.eax = 0;

    task_scheduler();
}

int general_execute_block(task_info_t *task_info) {
    if (general_execute(task_info) == FAILURE)
        return -1;
    task_table[current_task]->status = KRN_STAT_PROCWAIT_TASK;  // start waiting for the child process
    return 0;
}

int general_execute(task_info_t *task_info) {
    // correct the struct pointer for kernel space
    task_info = (task_info_t *)get_phys_addr(task_table[current_task]->page_directory, (size_t)task_info);
    
    // correct pointers for kernel space
    char *path = (char *)get_phys_addr(task_table[current_task]->page_directory, (size_t)task_info->path);
    char *pwd = (char *)get_phys_addr(task_table[current_task]->page_directory, (size_t)task_info->pwd);
    
    char *ptr_stdin = (char *)get_phys_addr(task_table[current_task]->page_directory, (size_t)task_info->stdin);
    char *ptr_stdout = (char *)get_phys_addr(task_table[current_task]->page_directory, (size_t)task_info->stdout);
    char *ptr_stderr = (char *)get_phys_addr(task_table[current_task]->page_directory, (size_t)task_info->stderr);
    
    vfs_metadata_t metadata;
    
    if (vfs_kget_metadata(path, &metadata, FILE_TYPE) == -2) return FAILURE;

    task_t new_task = {0};
    new_task.status = KRN_STAT_ACTIVE_TASK;
    new_task.cpu = default_cpu_status;
    new_task.parent = current_task;    // set parent
    
    size_t pages = (TASK_RESERVED_SPACE + metadata.size + DEFAULT_STACK) / PAGE_SIZE;
    if ((TASK_RESERVED_SPACE + metadata.size + DEFAULT_STACK) % PAGE_SIZE) pages++;
    
    // load program into memory
    size_t base = (size_t)kmalloc(pages);

    // use the new VFS stack
    int tmp_handle = vfs_kopen(path, O_RDWR, 0);
    vfs_kuread(tmp_handle, (char *)(base + TASK_RESERVED_SPACE), metadata.size);
    vfs_kclose(tmp_handle);

    // generate the page tables
    pt_entry_t* pd = new_userspace();
    // map the process's memory
    for (size_t i = 0; i < pages; i++)
        map_page(pd, TASK_BASE + i * PAGE_SIZE, base + i * PAGE_SIZE, 0x07);
    new_task.page_directory = pd;
    
    // attempt to create task
    int new_pid = task_create(new_task);
    
    if (new_pid == FAILURE) {
        // fail
        kfree((void *)base);
        return FAILURE;
    }
    
    task_table[new_pid]->cpu.esp = TASK_BASE + (((TASK_RESERVED_SPACE + metadata.size + DEFAULT_STACK) - 1) & 0xfffffff0);
    task_table[new_pid]->cpu.eip = TASK_BASE + TASK_RESERVED_SPACE;
    
    task_table[new_pid]->heap_base = TASK_BASE + pages * PAGE_SIZE;
    task_table[new_pid]->heap_size = 0;
    
    kstrcpy(task_table[new_pid]->pwd, pwd);

    // create file handles for std streams
    // this is a huge hack FIXME
    int khandle;
    khandle = vfs_kopen(ptr_stdin, O_RDONLY, 0);
    create_file_handle_v2(new_pid, task_table[0]->file_handles_v2[khandle]);
    task_table[0]->file_handles_v2[khandle].free = 1;
    khandle = vfs_kopen(ptr_stdout, O_WRONLY, 0);
    create_file_handle_v2(new_pid, task_table[0]->file_handles_v2[khandle]);
    task_table[0]->file_handles_v2[khandle].free = 1;
    khandle = vfs_kopen(ptr_stderr, O_WRONLY, 0);
    create_file_handle_v2(new_pid, task_table[0]->file_handles_v2[khandle]);
    task_table[0]->file_handles_v2[khandle].free = 1;

    int i;

    *((int *)(base + 0x1000)) = task_info->argc;
    int argv_limit = 0x4000;
    char **argv = (char **)(base + 0x1010);
    char **src_argv = (char **)get_phys_addr(task_table[current_task]->page_directory, (size_t)task_info->argv);

    /* prepare argv */
    for (i = 0; src_argv[i]; i++) {
        kstrcpy( (char *)(base + argv_limit),
                 (char *)get_phys_addr(task_table[current_task]->page_directory, (size_t)src_argv[i]) );
        argv[i] = (char *)(TASK_BASE + argv_limit);
        argv_limit += kstrlen((char *)get_phys_addr(task_table[current_task]->page_directory, (size_t)src_argv[i])) + 1;
    }
    /* argv null ptr as per standard */
    argv[i] = (char *)0;

    int environ_limit = 0x8000;
    char **environ = (char **)(base + 0x1020);
    char **src_environ = (char **)get_phys_addr(task_table[current_task]->page_directory, (size_t)task_info->environ);

    /* prepare environ */
    for (i = 0; src_environ[i]; i++) {
        kstrcpy( (char *)(base + environ_limit),
                 (char *)get_phys_addr(task_table[current_task]->page_directory, (size_t)src_environ[i]) );
        environ[i] = (char *)(TASK_BASE + environ_limit);
        environ_limit += kstrlen((char *)get_phys_addr(task_table[current_task]->page_directory, (size_t)src_environ[i])) + 1;
    }
    /* environ null ptr as per standard */
    environ[i] = (char *)0;

    // debug logging
    /*
    kputs("\nNew task startup request completed with:");
    kputs("\npid:         "); kuitoa((uint32_t)new_pid);
    kputs("\nppid:        "); kuitoa((uint32_t)task_table[new_pid]->parent);
    kputs("\nbase:        "); kxtoa(base);
    kputs("\npages:       "); kxtoa(pages);
    kputs("\npwd:         "); kputs(task_table[new_pid]->pwd);
    kputs("\nstdin:       "); kputs(task_table[new_pid]->stdin);
    kputs("\nstdout:      "); kputs(task_table[new_pid]->stdout);
    kputs("\nstderr:      "); kputs(task_table[new_pid]->stderr);
    */
    return new_pid;
}

void task_switch(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx, uint32_t esi, uint32_t edi, uint32_t ebp, uint32_t ds, uint32_t es, uint32_t fs, uint32_t gs, uint32_t eip, uint32_t cs, uint32_t eflags, uint32_t esp, uint32_t ss) {

    task_table[current_task]->cpu.eax = eax;
    task_table[current_task]->cpu.ebx = ebx;
    task_table[current_task]->cpu.ecx = ecx;
    task_table[current_task]->cpu.edx = edx;
    task_table[current_task]->cpu.esi = esi;
    task_table[current_task]->cpu.edi = edi;
    task_table[current_task]->cpu.ebp = ebp;
    task_table[current_task]->cpu.esp = esp;
    task_table[current_task]->cpu.eip = eip;
    task_table[current_task]->cpu.cs = cs;
    task_table[current_task]->cpu.ds = ds;
    task_table[current_task]->cpu.es = es;
    task_table[current_task]->cpu.fs = fs;
    task_table[current_task]->cpu.gs = gs;
    task_table[current_task]->cpu.ss = ss;
    task_table[current_task]->cpu.eflags = eflags;

    current_task++;
    task_scheduler();
}

extern int read_stat;
extern int write_stat;

void task_scheduler(void) {
    int c;

    for (;;) {
        if (!task_table[current_task]) {
            current_task = 0;
            if (idle_cpu) {
            // if no process took CPU time, wait for the next
            // context switch idling
                ENTER_IDLE;
            }
            idle_cpu = 1;
            continue;
        }
        
        if (task_table[current_task] == EMPTY_PID) {
            current_task++;
            continue;
        }
        
        switch (task_table[current_task]->status) {
            case KRN_STAT_IOWAIT_TASK:
                switch (task_table[current_task]->iowait_type) {
                int done;
                case 0:
                    if ((c = vfs_kread(task_table[current_task]->iowait_dev, task_table[current_task]->iowait_loc)) != IO_NOT_READY) {
                        // embed the result in EAX and continue
                        task_table[current_task]->cpu.eax = (uint32_t)c;
                        task_table[current_task]->status = KRN_STAT_ACTIVE_TASK;
                    } else {
                        current_task++;
                        continue;
                    }
                    break;
                case 1:
                    if ((c = vfs_kwrite(task_table[current_task]->iowait_dev, task_table[current_task]->iowait_loc,
                                        task_table[current_task]->iowait_payload)) != IO_NOT_READY) {
                        // embed the result in EAX and continue
                        task_table[current_task]->cpu.eax = (uint32_t)c;
                        task_table[current_task]->status = KRN_STAT_ACTIVE_TASK;
                    } else {
                        current_task++;
                        continue;
                    }
                    break;
                case 2:
                    done = read(    task_table[current_task]->iowait_handle,
                                    (char *)(task_table[current_task]->iowait_ptr + task_table[current_task]->iowait_done),
                                    task_table[current_task]->iowait_len - task_table[current_task]->iowait_done);
                    if (read_stat) {
                        task_table[current_task]->iowait_done += done;
                        current_task++;
                        continue;
                    } else {
                        task_table[current_task]->cpu.eax = (uint32_t)(task_table[current_task]->iowait_done + done);
                        task_table[current_task]->status = KRN_STAT_ACTIVE_TASK;
                    }
                    break;
                case 3:
                    done = write(    task_table[current_task]->iowait_handle,
                                    (char *)(task_table[current_task]->iowait_ptr + task_table[current_task]->iowait_done),
                                    task_table[current_task]->iowait_len - task_table[current_task]->iowait_done);
                    if (write_stat) {
                        task_table[current_task]->iowait_done += done;
                        current_task++;
                        continue;
                    } else {
                        task_table[current_task]->cpu.eax = (uint32_t)(task_table[current_task]->iowait_done + done);
                        task_table[current_task]->status = KRN_STAT_ACTIVE_TASK;
                    }
                    break;
                default:
                    panic("unrecognised iowait_type");
                }
            case KRN_STAT_ACTIVE_TASK:
                idle_cpu = 0;
                //set_segment(0x3, task_table[current_task]->base, task_table[current_task]->pages);
                //set_segment(0x4, task_table[current_task]->base, task_table[current_task]->pages);
                task_spinup((void *)(&(task_table[current_task]->cpu)), task_table[current_task]->page_directory);
            case KRN_STAT_IPCWAIT_TASK:
            case KRN_STAT_PROCWAIT_TASK:
            case KRN_STAT_VDEVWAIT_TASK:
            case KRN_STAT_RES_TASK:
                current_task++;
                continue;
            default:
                panic("unrecognised task status");
        }

    }

}

void task_quit_self(int64_t return_value) {
    task_quit(current_task, return_value);
}

void task_quit(int pid, int64_t return_value) {
    int parent = task_table[pid]->parent;
    if (task_table[parent]->status == KRN_STAT_PROCWAIT_TASK) {
        task_table[parent]->cpu.eax = (uint32_t)(return_value & 0xffffffff);
        task_table[parent]->cpu.edx = (uint32_t)((return_value >> 32) & 0xffffffff);
        task_table[parent]->status = KRN_STAT_ACTIVE_TASK;
    }
    kfree((void *)task_table[pid]->file_handles_v2);
    destroy_userspace(task_table[pid]->page_directory);
    kfree((void *)task_table[pid]);
    task_table[pid] = EMPTY_PID;
    DISABLE_INTERRUPTS;
    ts_enable = 1;
    task_scheduler();
}
