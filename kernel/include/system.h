#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <stdint.h>
#include <stddef.h>

typedef struct {
    int cpu_number;
    uint8_t *kernel_stack;
    int current_task;
    int cpu_idle;
} cpu_local_t;

int get_cpu_number(void);
int get_cpu_kernel_stack(void);

void map_PIC(uint8_t PIC0Offset, uint8_t PIC1Offset);
void set_PIC0_mask(uint8_t mask);
void set_PIC1_mask(uint8_t mask);
uint8_t get_PIC0_mask(void);
uint8_t get_PIC1_mask(void);


extern size_t memory_size;
size_t detect_mem(void);

void set_pit_freq(uint32_t frequency);
void sleep(uint64_t time);

void load_IDT(void);

extern volatile uint64_t uptime_raw;
extern volatile uint64_t uptime_sec;

extern int ts_enable;

extern uint8_t fxstate[512];


#endif
