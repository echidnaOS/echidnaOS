#include <kernel.h>
#include <stdint.h>

#define DEFAULT_STACK 0x10000
#define RUNNING_FLAG 0x12344321

typedef struct {

    uint32_t status;
    uint32_t pid;
    
    uint32_t base;
    uint32_t pages;
    
    uint32_t eax_p;
    uint32_t ebx_p;
    uint32_t ecx_p;
    uint32_t edx_p;
    uint32_t esi_p;
    uint32_t edi_p;
    uint32_t ebp_p;
    uint32_t esp_p;
    uint32_t eip_p;
    uint32_t cs_p;
    uint32_t ds_p;
    uint32_t es_p;
    uint32_t fs_p;
    uint32_t gs_p;
    uint32_t ss_p;
    uint32_t eflags_p;

} task_t;

void task_spinup(void*);

task_t* current_task;
task_t prototype_task = {RUNNING_FLAG,0,0,0,
                         0,0,0,0,0,0,0,DEFAULT_STACK-0x10,DEFAULT_STACK,
                         0x1b,0x23,0x23,0x23,0x23,0x23,0x202};

void start_tasks(void) {
    current_task = (task_t*)0x1000000;
    set_userspace(current_task->base, current_task->pages);
    task_spinup((void*)current_task);
}

void task_start(uint32_t task_addr, uint32_t task_size) {
    // get task size in pages
    uint32_t task_pages = task_size / 4096;
    if (task_size % 4096) task_pages++;
    
    task_t* new_task = (task_t*)memory_bottom;
    memory_bottom += sizeof(task_t);

    *new_task = prototype_task;  // initialise struct

    // copy task code into the running location
    kmemcpy((char*)(memory_bottom + DEFAULT_STACK), (char*)task_addr, task_size);

    new_task->base = memory_bottom;
    new_task->pages = (DEFAULT_STACK / 4096) + task_pages;
    memory_bottom += DEFAULT_STACK + (task_pages * 4096);
    
    return;
}

void task_switch(uint32_t eax_r, uint32_t ebx_r, uint32_t ecx_r, uint32_t edx_r, uint32_t esi_r, uint32_t edi_r, uint32_t ebp_r, uint32_t ds_r, uint32_t es_r, uint32_t fs_r, uint32_t gs_r, uint32_t eip_r, uint32_t cs_r, uint32_t eflags_r, uint32_t esp_r, uint32_t ss_r) {

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
    uint32_t int_ptr = ((current_task->pages) * 4096) + sizeof(task_t);
    int_ptr += (uint32_t)current_task;
    current_task = (task_t*)int_ptr;
    if (current_task->status == RUNNING_FLAG) {
    } else
        current_task = (task_t*)0x1000000;
    
    set_userspace(current_task->base, current_task->pages);
    task_spinup((void*)current_task);

}
