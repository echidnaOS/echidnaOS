#include <kernel.h>
#include <stdint.h>

#define DEFAULT_STACK 0x10000
#define PRESENT_FLAG 0x12344321

typedef struct {

    uint32_t present;
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
task_t empty_task = {0};

void task_start(uint32_t task_addr, uint32_t task_size) {
    // get task size in pages
    uint32_t task_pages = task_size / 4096;
    if (task_size % 4096) task_pages++;
    
    current_task = (task_t*)memory_bottom;
    memory_bottom += sizeof(task_t);

    *current_task = empty_task;  // initialise struct
    
    current_task->present = PRESENT_FLAG;

    kmemcpy((char*)(memory_bottom + DEFAULT_STACK), (char*)task_addr, task_size);

    current_task->base = memory_bottom;
    current_task->pages = (DEFAULT_STACK / 4096) + task_pages;
    memory_bottom += DEFAULT_STACK + (task_pages * 4096);
    
    current_task->cs_p = 0x1b;
    current_task->ds_p = 0x23;
    current_task->es_p = 0x23;
    current_task->fs_p = 0x23;
    current_task->gs_p = 0x23;
    current_task->ss_p = 0x23;
    current_task->eflags_p = 0x202;
    
    current_task->eip_p = DEFAULT_STACK;
    current_task->esp_p = DEFAULT_STACK - 0x10;

    set_userspace(current_task->base, current_task->pages);
    task_spinup((void*)current_task);
    
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
    current_task += (current_task->pages);
    if (current_task->present != PRESENT_FLAG)
        current_task = (task_t*)0x1000000;
    
    set_userspace(current_task->base, current_task->pages);
    task_spinup((void*)current_task);

}
