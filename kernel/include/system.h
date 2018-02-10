#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <stdint.h>
#include <stddef.h>


void map_PIC(uint8_t PIC0Offset, uint8_t PIC1Offset);
void set_PIC0_mask(uint8_t mask);
void set_PIC1_mask(uint8_t mask);
uint8_t get_PIC0_mask(void);
uint8_t get_PIC1_mask(void);


typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) GDT_entry_t;


extern size_t memory_size;
size_t detect_mem(void);

void set_pit_freq(uint32_t frequency);

void load_GDT(void);
void load_TSS(void);

void load_IDT(void);

extern uint64_t uptime_raw;
extern uint64_t uptime_sec;

extern int ts_enable;


#endif
