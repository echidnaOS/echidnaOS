// general kernel header

#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <stdint.h>
#include <systemasm.h>

// kernel tunables

#ifdef _BIG_FONTS_
  #define VD_ROWS 25
#else
  #define VD_ROWS 50
#endif
#define VD_COLS 160

#define KB_L1_SIZE 256
#define KB_L2_SIZE 2048

#define KRNL_PIT_FREQ 0x8000
#define KRNL_TTY_COUNT 7

#define TTY_DEF_CUR_PAL 0x70
#define TTY_DEF_TXT_PAL 0x07

void kputs(const char* string);
void kuitoa(uint32_t x);
void kxtoa(uint32_t x);
void init_disk(uint8_t boot_drive);
uint8_t disk_read_b(uint8_t drive, uint32_t loc);
void disk_read_seq(uint8_t* buffer, uint8_t drive, uint32_t loc, uint32_t count);

void switch_tty(uint8_t which_tty);
void init_tty(void);

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
    
    uint8_t tty;
    void* heap_begin;
    uint32_t heap_size;

} task_t;

typedef struct {
    uint32_t addr;
    uint32_t size;
    uint32_t stdin;
    uint32_t stdout;
    uint32_t stderr;
    uint8_t tty;
    uint32_t stack;
    uint32_t heap;
} task_info_t;

void task_start(task_info_t* task_info);
void task_switch(uint32_t eax_r, uint32_t ebx_r, uint32_t ecx_r, uint32_t edx_r, uint32_t esi_r, uint32_t edi_r, uint32_t ebp_r, uint32_t ds_r, uint32_t es_r, uint32_t fs_r, uint32_t gs_r, uint32_t eip_r, uint32_t cs_r, uint32_t eflags_r, uint32_t esp_r, uint32_t ss_r);

typedef struct {
    int free;
    uint32_t size;
    uint32_t prev_chunk;
} heap_chunk_t;

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed)) GDT_entry_t;

typedef struct {
    uint32_t cursor_offset;
    int cursor_status;
    uint8_t cursor_palette;
    uint8_t text_palette;
    char field[VD_ROWS * VD_COLS];
    char kb_l1_buffer[KB_L1_SIZE];
    char kb_l2_buffer[KB_L2_SIZE];
    uint16_t kb_l1_buffer_index;
    uint16_t kb_l2_buffer_index;
} tty_t;

extern uint32_t memory_size;
extern uint32_t memory_bottom;
extern task_t* current_task;
extern uint8_t current_tty;

extern tty_t tty[KRNL_TTY_COUNT];

void kmemcpy(char* dest, char* source, uint32_t count);

void vga_disable_cursor(void);
void vga_80_x_50(void);
void disk_load_sector(uint8_t drive, uint8_t* target_address, uint32_t source_sector, uint32_t count);
uint32_t detect_mem(void);

void char_to_stdout(int c);
int char_from_stdin(void);

void* alloc(uint32_t size);

void tty_refresh(uint8_t which_tty);

void text_putchar(char c, uint8_t which_tty);
uint32_t text_get_cursor_pos_x(uint8_t which_tty);
uint32_t text_get_cursor_pos_y(uint8_t which_tty);
void text_set_cursor_pos(uint32_t x, uint32_t y, uint8_t which_tty);
void text_set_cursor_palette(uint8_t c, uint8_t which_tty);
uint8_t text_get_cursor_palette(uint8_t which_tty);
void text_set_text_palette(uint8_t c, uint8_t which_tty);
uint8_t text_get_text_palette(uint8_t which_tty);
void text_clear(uint8_t which_tty);
void text_disable_cursor(uint8_t which_tty);
void text_enable_cursor(uint8_t which_tty);

void map_PIC(uint8_t PIC0Offset, uint8_t PIC1Offset);

void set_pit_freq(uint32_t frequency);

void load_GDT(void);
void load_TSS(void);
void set_segment(uint16_t entry, uint32_t base, uint32_t page_count);

void load_IDT(void);

void keyboard_init(void);
void keyboard_handler(uint8_t input_byte);
char keyboard_fetch_char(uint8_t which_tty);

#endif
