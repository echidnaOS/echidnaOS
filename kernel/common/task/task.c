#include <kernel.h>
#include <stdint.h>

void task_spinup(void*);

typedef struct {

    int present;
    uint16_t pid;
    
    uint32_t base;
    uint32_t size;
    
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

task_t task_table[16];

static uint16_t current_task = 0;

void task_switch(uint32_t eax_r, uint32_t ebx_r, uint32_t ecx_r, uint32_t edx_r, uint32_t esi_r, uint32_t edi_r, uint32_t ebp_r, uint32_t ds_r, uint32_t es_r, uint32_t fs_r, uint32_t gs_r, uint32_t eip_r, uint32_t cs_r, uint32_t eflags_r, uint32_t esp_r, uint32_t ss_r) {

    task_table[current_task].eax_p = eax_r;
    task_table[current_task].ebx_p = ebx_r;
    task_table[current_task].ecx_p = ecx_r;
    task_table[current_task].edx_p = edx_r;
    task_table[current_task].esi_p = esi_r;
    task_table[current_task].edi_p = edi_r;
    task_table[current_task].ebp_p = ebp_r;
    task_table[current_task].esp_p = esp_r;
    task_table[current_task].eip_p = eip_r;
    task_table[current_task].cs_p = cs_r;
    task_table[current_task].ds_p = ds_r;
    task_table[current_task].es_p = es_r;
    task_table[current_task].fs_p = fs_r;
    task_table[current_task].gs_p = gs_r;
    task_table[current_task].ss_p = ss_r;
    task_table[current_task].eflags_p = eflags_r;
    
    //while (!task_table[++current_task].present);
    
    //set_userspace(task_table[current_task].base, task_table[current_task].size);
    task_spinup(&task_table[current_task]);

}
