#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <stdint.h>
#include <stddef.h>

// misc tunables

#define KRNL_PIT_FREQ 4000
//#define _SERIAL_KERNEL_OUTPUT_


// task tunables

#define KRNL_MAX_TASKS 65536
#define DEFAULT_STACK 0x10000


// tty tunables

#define _BIG_FONTS_

#define KB_L1_SIZE 256
#define KB_L2_SIZE 2048

#define KRNL_TTY_COUNT 7
#define TTY_DEF_CUR_PAL 0x70
#define TTY_DEF_TXT_PAL 0x07


// vfs tunables

#define MAX_SIMULTANOUS_VFS_ACCESS  4096






typedef struct {
    uint8_t version_min;
    uint8_t version_maj;
    char* oem;
    uint32_t capabilities;
    uint16_t* vid_modes;
    uint16_t vid_mem_blocks;
    uint16_t software_rev;
    char* vendor;
    char* prod_name;
    char* prod_rev;
} __attribute__((packed)) vbe_info_struct_t;


// prototypes


typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) GDT_entry_t;

extern uint32_t memory_size;




void panic(const char *msg);

void vga_disable_cursor(void);
void vga_80_x_50(void);
uint32_t detect_mem(void);

void map_PIC(uint8_t PIC0Offset, uint8_t PIC1Offset);
void set_PIC0_mask(uint8_t mask);
void set_PIC1_mask(uint8_t mask);
uint8_t get_PIC0_mask(void);
uint8_t get_PIC1_mask(void);

void set_pit_freq(uint32_t frequency);

void load_GDT(void);
void load_TSS(void);

void load_IDT(void);

void keyboard_handler(uint8_t input_byte);
int keyboard_fetch_char(uint8_t which_tty);

#endif
