#include <kernel.h>
#include <stdint.h>
#include <system.h>

extern uint32_t TSS;
extern uint32_t TSS_size;

static GDT_entry_t GDT[8];

typedef struct {
    uint16_t size;
    uint32_t base;
} __attribute__((packed)) GDT_ptr_t;

static GDT_ptr_t GDT_ptr = {
    (uint16_t)(sizeof(GDT) - 1),
    (uint32_t)GDT
};

void set_segment(uint16_t entry, uint32_t base, uint32_t page_count) {
    GDT[entry].base_low = (uint16_t)(base & 0x0000ffff);
    GDT[entry].base_mid = (uint8_t)((base & 0x00ff0000) / 0x10000);
    GDT[entry].base_high = (uint8_t)((base & 0xff000000) / 0x1000000);
    
    GDT[entry].limit_low = (uint16_t)((page_count - 1) & 0x0000ffff);
    GDT[entry].granularity = (uint8_t)((GDT[entry].granularity & 0b11110000) | ((page_count & 0x000f0000) / 0x10000));

    return;
}

void load_GDT(void) {

    // null pointer
    GDT[0].limit_low = 0;
    GDT[0].base_low = 0;
    GDT[0].base_mid = 0;
    GDT[0].access = 0;
    GDT[0].granularity = 0;
    GDT[0].base_high = 0;
    
    // define kernel code
    GDT[1].limit_low = 0xffff;
    GDT[1].base_low = 0x0000;
    GDT[1].base_mid = 0x00;
    GDT[1].access = 0b10011010;
    GDT[1].granularity = 0b11001111;
    GDT[1].base_high = 0x00;

    // define kernel data
    GDT[2].limit_low = 0xffff;
    GDT[2].base_low = 0x0000;
    GDT[2].base_mid = 0x00;
    GDT[2].access = 0b10010010;
    GDT[2].granularity = 0b11001111;
    GDT[2].base_high = 0x00;
    
    // define user code
    GDT[3].limit_low = 0xffff;
    GDT[3].base_low = 0x0000;
    GDT[3].base_mid = 0x00;
    GDT[3].access = 0b11111010;
    GDT[3].granularity = 0b11001111;
    GDT[3].base_high = 0x00;

    // define user data
    GDT[4].limit_low = 0xffff;
    GDT[4].base_low = 0x0000;
    GDT[4].base_mid = 0x00;
    GDT[4].access = 0b11110010;
    GDT[4].granularity = 0b11001111;
    GDT[4].base_high = 0x00;
    
    // define 16-bit code
    GDT[5].limit_low = 0xffff;
    GDT[5].base_low = 0x0000;
    GDT[5].base_mid = 0x00;
    GDT[5].access = 0b10011010;
    GDT[5].granularity = 0b10001111;
    GDT[5].base_high = 0x00;

    // define 16-bit data
    GDT[6].limit_low = 0xffff;
    GDT[6].base_low = 0x0000;
    GDT[6].base_mid = 0x00;
    GDT[6].access = 0b10010010;
    GDT[6].granularity = 0b10001111;
    GDT[6].base_high = 0x00;

    // define TSS segment
    GDT[7].limit_low = (uint16_t)(TSS_size & 0x0000ffff);
    GDT[7].base_low = (uint16_t)(TSS & 0x0000ffff);
    GDT[7].base_mid = (uint8_t)((TSS & 0x00ff0000) / 0x10000);
    GDT[7].access = 0b11101001;
    GDT[7].granularity = 0b00000000;
    GDT[7].base_high = (uint8_t)((TSS & 0xff000000) / 0x1000000);
    
    // effectively load the GDT
    asm volatile (
        "mov eax, %0;"
        "lgdt [eax];"
        "jmp 0x08:1f;"
        "1:"
        "mov ax, 0x10;"
        "mov ds, ax;"
        "mov es, ax;"
        "mov fs, ax;"
        "mov gs, ax;"
        "mov ss, ax;"
         :
         : "r" (&GDT_ptr)
         : "eax"
    );

    return;
}

void load_TSS(void) {

    asm volatile (
        "mov ax, 0x3B;"
        "ltr ax;"
         :
         :
         : "eax"
    );

    return;
}
