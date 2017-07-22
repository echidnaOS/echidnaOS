#include <kernel.h>
#include <stdint.h>

#define TASK_RESERVED_SPACE     0x10000
#define PAGE_SIZE               4096

task_t* current_task = (task_t*)KRNL_MEMORY_BASE;

void task_spinup(void*);

const task_t prototype_task = {KRN_STAT_ACTIVE_TASK,0,0,0,
                               0,0,0,0,0,0,0,0,0,
                               0x1b,0x23,0x23,0x23,0x23,0x23,0x202,
                               0,0,0,
                               ""};

void task_start(task_info_t* task_info) {

    // correct the struct pointer for kernel space
    uint32_t task_info_ptr = (uint32_t)task_info;
    task_info_ptr += current_task->base;
    task_info = (task_info_t*)task_info_ptr;
    
    // correct the address for kernel space
    uint32_t task_addr = task_info->addr + current_task->base;
    
    task_t* new_task = (task_t*)memory_bottom;
    memory_bottom += sizeof(task_t);

    *new_task = prototype_task;  // initialise struct

    // get task size in pages
    new_task->pages = (TASK_RESERVED_SPACE + task_info->size + task_info->stack + task_info->heap) / PAGE_SIZE;
    if ((TASK_RESERVED_SPACE + task_info->size + task_info->stack + task_info->heap) % PAGE_SIZE) new_task->pages++;

    // copy task code into the running location
    kmemcpy((char*)(memory_bottom + TASK_RESERVED_SPACE), (char*)task_addr, task_info->size);
    
    // build first heap chunk identifier
    new_task->heap_begin = (void*)(memory_bottom + TASK_RESERVED_SPACE + task_info->size + task_info->stack);
    new_task->heap_size = task_info->heap;
    heap_chunk_t* heap_chunk = (heap_chunk_t*)new_task->heap_begin;
    
    heap_chunk->free = 1;
    heap_chunk->size = task_info->heap - sizeof(heap_chunk_t);
    heap_chunk->prev_chunk = 0;

    new_task->base = memory_bottom;
    
    new_task->esp_p = ((TASK_RESERVED_SPACE + task_info->size + task_info->stack) - 1) & 0xfffffff0;
    new_task->eip_p = TASK_RESERVED_SPACE;
    
    memory_bottom += new_task->pages * PAGE_SIZE;
    
    new_task->tty = task_info->tty;
    
    kstrcpy(new_task->pwd, task_info->pwd);
    
    // debug logging
    kputs("\n\nNew task startup request completed with:\n");
    kputs("\npid:    "); kuitoa((uint32_t)new_task->pid);
    kputs("\nbase:   "); kxtoa(new_task->base);
    kputs("\npages:  "); kxtoa(new_task->pages);
    kputs("\ntty:    "); kuitoa((uint32_t)new_task->tty);
    kputs("\npwd:    "); kputs(new_task->pwd);
    
    return;
}

void task_switch(uint32_t eax_r, uint32_t ebx_r, uint32_t ecx_r, uint32_t edx_r, uint32_t esi_r, uint32_t edi_r, uint32_t ebp_r, uint32_t ds_r, uint32_t es_r, uint32_t fs_r, uint32_t gs_r, uint32_t eip_r, uint32_t cs_r, uint32_t eflags_r, uint32_t esp_r, uint32_t ss_r) {

    uint32_t int_ptr;

    current_task->eax_p = eax_r;
    current_task->ebx_p = ebx_r;
    current_task->ecx_p = ecx_r;
    current_task->edx_p = edx_r;
    current_task->esi_p = esi_r;
    current_task->edi_p = edi_r;
    current_task->ebp_p = ebp_r;
    current_task->esp_p = esp_r;
    current_task->eip_p = eip_r;
    current_task->cs_p = cs_r;
    current_task->ds_p = ds_r;
    current_task->es_p = es_r;
    current_task->fs_p = fs_r;
    current_task->gs_p = gs_r;
    current_task->ss_p = ss_r;
    current_task->eflags_p = eflags_r;

    // find next task table
next_task:
    int_ptr = ((current_task->pages) * PAGE_SIZE) + sizeof(task_t);
    int_ptr += (uint32_t)current_task;
    current_task = (task_t*)int_ptr;

check_task:
    switch (current_task->status) {
        case KRN_STAT_ACTIVE_TASK:
            break;
        case KRN_STAT_RES_TASK:
            goto next_task;
        case KRN_STAT_TERM_TASK:
            goto next_task;
        default:
            current_task = (task_t*)KRNL_MEMORY_BASE;
            goto check_task;
    }
    
    set_segment(0x3, current_task->base, current_task->pages);
    set_segment(0x4, current_task->base, current_task->pages);
    task_spinup((void*)current_task);

}
