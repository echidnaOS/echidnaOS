#include <kernel.h>
#include <stdint.h>

#define TASK_RESERVED_SPACE     0x10000
#define PAGE_SIZE               4096

task_t** task_table;

void task_init(void) {
    // allocate the task table
    if ((task_table = kalloc(KRNL_MAX_TASKS * sizeof(task_t*))) == 0)
        panic("unable to allocate task table");
    // create kernel task
    if ((task_table[0] = kalloc(sizeof(task_t))) == 0)
        panic("unable to allocate kernel task");
    task_table[0]->status = KRN_STAT_RES_TASK;    
    return;
}

int current_task = 0;

int idle_cpu = 1;

void task_spinup(void*);

const task_t prototype_task = {KRN_STAT_ACTIVE_TASK,0,0,0,
                               0,0,0,0,0,0,0,0,0,
                               0x1b,0x23,0x23,0x23,0x23,0x23,0x202,
                               0,0,0,
                               ""};

int task_start(task_info_t* task_info) {
    // start new task
    // returns 0 on failure, PID on success

    // correct the struct pointer for kernel space
    uint32_t task_info_ptr = (uint32_t)task_info;
    task_info_ptr += task_table[current_task]->base;
    task_info = (task_info_t*)task_info_ptr;
    
    // correct the address for kernel space
    uint32_t task_addr = task_info->addr + task_table[current_task]->base;
    
    // find an empty entry in the task table
    int new_task;
    for (new_task = 0; new_task < KRNL_MAX_TASKS; new_task++)
        if (!task_table[new_task]) break;
    if (new_task == KRNL_MAX_TASKS)
        return 0;
    
    // allocate a task entry
    if ((task_table[new_task] = kalloc(sizeof(task_t))) == 0)
        return 0;

    *task_table[new_task] = prototype_task;  // initialise struct

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
    
    kstrcpy(task_table[new_task]->pwd, task_info->pwd);
    
    // debug logging
    kputs("\n\nNew task startup request completed with:\n");
    kputs("\npid:    "); kuitoa((uint32_t)new_task);
    kputs("\nbase:   "); kxtoa(task_table[new_task]->base);
    kputs("\npages:  "); kxtoa(task_table[new_task]->pages);
    kputs("\ntty:    "); kuitoa((uint32_t)task_table[new_task]->tty);
    kputs("\npwd:    "); kputs(task_table[new_task]->pwd);
    
    return new_task;
}

void task_switch(uint32_t eax_r, uint32_t ebx_r, uint32_t ecx_r, uint32_t edx_r, uint32_t esi_r, uint32_t edi_r, uint32_t ebp_r, uint32_t ds_r, uint32_t es_r, uint32_t fs_r, uint32_t gs_r, uint32_t eip_r, uint32_t cs_r, uint32_t eflags_r, uint32_t esp_r, uint32_t ss_r) {

    uint32_t int_ptr;
    int c;

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

    // find next task
scheduler:
    for (current_task++; current_task < KRNL_MAX_TASKS; current_task++) {
        switch (task_table[current_task]->status) {
            case KRN_STAT_ACTIVE_TASK:
                idle_cpu = 0;
                set_segment(0x3, task_table[current_task]->base, task_table[current_task]->pages);
                set_segment(0x4, task_table[current_task]->base, task_table[current_task]->pages);
                task_spinup((void*)task_table[current_task]);
            case KRN_STAT_RES_TASK:
            case KRN_STAT_TERM_TASK:
                continue;
            case KRN_STAT_IOWAIT_TASK:
                if ((c = (int)keyboard_fetch_char(task_table[current_task]->tty))) {
                    // embed the result in EAX and continue
                    task_table[current_task]->eax_p = (uint32_t)c;
                    task_table[current_task]->status = KRN_STAT_ACTIVE_TASK;
                }
                continue;
            case KRN_STAT_ENDTABLE_TASK:
                break;
        }
        break;
    }

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
    goto scheduler;

}
