#include <kernel.h>
#include <stdint.h>

task_t** task_table;

void task_init(void) {
    // allocate the task table
    if ((task_table = kalloc(KRNL_MAX_TASKS * sizeof(task_t*))) == 0)
        panic("unable to allocate task table");
    // create kernel task
    if ((task_table[0] = kalloc(sizeof(task_t))) == 0)
        panic("unable to allocate kernel task");
    kstrcpy(task_table[0]->pwd, "/");
    kstrcpy(task_table[0]->name, "kernel");
    task_table[0]->status = KRN_STAT_RES_TASK;    
    return;
}

uint32_t current_task = 0;

int idle_cpu = 1;

void task_spinup(void*);

const task_t prototype_task = {KRN_STAT_ACTIVE_TASK,0,0,0,
                               0,0,0,0,0,0,0,0,0,
                               0x1b,0x23,0x23,0x23,0x23,0x23,0x202,
                               0,0,0,
                               "","","",
                               "","","",
                               "",0,
                               0,0};

uint32_t task_start(task_info_t* task_info) {
    // start new task
    // returns 0 on failure, PID on success

    // correct the struct pointer for kernel space
    uint32_t task_info_ptr = (uint32_t)task_info;
    task_info_ptr += task_table[current_task]->base;
    task_info = (task_info_t*)task_info_ptr;
    
    // correct the address for kernel space
    uint32_t task_addr = task_info->addr + task_table[current_task]->base;
    
    // correct pointers for kernel space
    char* pwd = task_info->pwd + task_table[current_task]->base;
    char* name = task_info->name + task_table[current_task]->base;
    char* server_name = task_info->server_name + task_table[current_task]->base;
    
    char* ptr_stdin = task_info->stdin + task_table[current_task]->base;
    char* ptr_stdout = task_info->stdout + task_table[current_task]->base;
    char* ptr_stderr = task_info->stderr + task_table[current_task]->base;
    
    // find an empty entry in the task table
    uint32_t new_task;
    for (new_task = 0; new_task < KRNL_MAX_TASKS; new_task++)
        if (!task_table[new_task]) break;
    if (new_task == KRNL_MAX_TASKS)
        return 0;
    
    // allocate a task entry
    if ((task_table[new_task] = kalloc(sizeof(task_t))) == 0)
        return 0;

    *task_table[new_task] = prototype_task;  // initialise struct
    
    task_table[new_task]->parent = current_task;    // set parent

    // get task size in pages
    task_table[new_task]->pages = (TASK_RESERVED_SPACE + task_info->size + task_info->stack + task_info->heap) / PAGE_SIZE;
    if ((TASK_RESERVED_SPACE + task_info->size + task_info->stack + task_info->heap) % PAGE_SIZE) task_table[new_task]->pages++;

    // allocate task space
    if ((task_table[new_task]->base = (uint32_t)kalloc(task_table[new_task]->pages * PAGE_SIZE)) == 0) {
        kfree(task_table[new_task]);
        task_table[new_task] = 0;
        return 0;
    }
    
    // copy task code into the running location
    kmemcpy((char*)(task_table[new_task]->base + TASK_RESERVED_SPACE), (char*)task_addr, task_info->size);
    
    // build first heap chunk identifier
    task_table[new_task]->heap_begin = (void*)(task_table[new_task]->base + TASK_RESERVED_SPACE + task_info->size + task_info->stack);
    task_table[new_task]->heap_size = task_info->heap;
    heap_chunk_t* heap_chunk = (heap_chunk_t*)task_table[new_task]->heap_begin;
    
    heap_chunk->free = 1;
    heap_chunk->size = task_info->heap - sizeof(heap_chunk_t);
    heap_chunk->prev_chunk = 0;
    
    task_table[new_task]->esp_p = ((TASK_RESERVED_SPACE + task_info->size + task_info->stack) - 1) & 0xfffffff0;
    task_table[new_task]->eip_p = TASK_RESERVED_SPACE;
    
    task_table[new_task]->tty = task_info->tty;
    
    kstrcpy(task_table[new_task]->pwd, pwd);
    kstrcpy(task_table[new_task]->name, name);
    kstrcpy(task_table[new_task]->server_name, server_name);
    
    kstrcpy(task_table[new_task]->stdin, ptr_stdin);
    kstrcpy(task_table[new_task]->stdout, ptr_stdout);
    kstrcpy(task_table[new_task]->stderr, ptr_stderr);
    
    // debug logging
    kputs("\nNew task startup request completed with:");
    kputs("\npid:         "); kuitoa((uint32_t)new_task);
    kputs("\nbase:        "); kxtoa(task_table[new_task]->base);
    kputs("\npages:       "); kxtoa(task_table[new_task]->pages);
    kputs("\ntty:         "); kuitoa((uint32_t)task_table[new_task]->tty);
    kputs("\npwd:         "); kputs(task_table[new_task]->pwd);
    kputs("\nname:        "); kputs(task_table[new_task]->name);
    kputs("\nserver name: "); kputs(task_table[new_task]->server_name);
    kputs("\nstdin:       "); kputs(task_table[new_task]->stdin);
    kputs("\nstdout:      "); kputs(task_table[new_task]->stdout);
    kputs("\nstderr:      "); kputs(task_table[new_task]->stderr);
    
    return new_task;
}

void task_switch(uint32_t eax_r, uint32_t ebx_r, uint32_t ecx_r, uint32_t edx_r, uint32_t esi_r, uint32_t edi_r, uint32_t ebp_r, uint32_t ds_r, uint32_t es_r, uint32_t fs_r, uint32_t gs_r, uint32_t eip_r, uint32_t cs_r, uint32_t eflags_r, uint32_t esp_r, uint32_t ss_r) {

    task_table[current_task]->eax_p = eax_r;
    task_table[current_task]->ebx_p = ebx_r;
    task_table[current_task]->ecx_p = ecx_r;
    task_table[current_task]->edx_p = edx_r;
    task_table[current_task]->esi_p = esi_r;
    task_table[current_task]->edi_p = edi_r;
    task_table[current_task]->ebp_p = ebp_r;
    task_table[current_task]->esp_p = esp_r;
    task_table[current_task]->eip_p = eip_r;
    task_table[current_task]->cs_p = cs_r;
    task_table[current_task]->ds_p = ds_r;
    task_table[current_task]->es_p = es_r;
    task_table[current_task]->fs_p = fs_r;
    task_table[current_task]->gs_p = gs_r;
    task_table[current_task]->ss_p = ss_r;
    task_table[current_task]->eflags_p = eflags_r;

    current_task++;
    task_scheduler();
}

void task_scheduler(void) {
    int c;

    for (;;) {
        if (!task_table[current_task]) {
            current_task = 0;
            if (idle_cpu) {
            // if no process took CPU time, wait for the next
            // context switch idling
                asm volatile (
                                "sti;"
                                "1:"
                                "mov esp, 0xefffff;"
                                "hlt;"
                                "jmp 1b;"
                             );
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
                if ((c = vfs_kread(task_table[current_task]->iowait_dev, task_table[current_task]->iowait_loc)) != IO_NOT_READY) {
                    // embed the result in EAX and continue
                    task_table[current_task]->eax_p = (uint32_t)c;
                    task_table[current_task]->status = KRN_STAT_ACTIVE_TASK;
                } else {
                    current_task++;
                    continue;
                }
            case KRN_STAT_ACTIVE_TASK:
                idle_cpu = 0;
                set_segment(0x3, task_table[current_task]->base, task_table[current_task]->pages);
                set_segment(0x4, task_table[current_task]->base, task_table[current_task]->pages);
                task_spinup((void*)task_table[current_task]);
            case KRN_STAT_IPCWAIT_TASK:
            case KRN_STAT_RES_TASK:
            default:
                current_task++;
                continue;
        }

    }

}

void task_quit(uint64_t return_value) {
    task_terminate(current_task);
    task_scheduler();
}

void task_terminate(int pid) {
    kfree((void*)task_table[pid]->base);
    kfree((void*)task_table[pid]);
    task_table[pid] = EMPTY_PID;
}
