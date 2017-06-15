// general kernel header for direct (no syscall) function calling

#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <stdint.h>
#include <systemasm.h>

// typedefs

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

} task_t;

#define ROWS 50
#define COLS 160

typedef struct {
    uint32_t cursor_offset;
    int cursor_status;
    uint8_t cursor_palette;
    uint8_t text_palette;
    char field[ROWS*COLS];
} tty_t;

// globals

extern uint32_t memory_size;
extern uint32_t memory_bottom;
extern task_t* current_task;
extern uint8_t current_tty;

// libs
void kmemcpy(char* dest, char* source, uint32_t count);

// real
void vga_disable_cursor(void);
void vga_80_x_50(void);
void disk_load_sector(uint8_t drive, uint8_t* target_address, uint32_t source_sector, uint32_t count);
uint32_t detect_mem(void);

// io

void char_to_stdout(int c);
int char_from_stdin(void);

// alloc

void* alloc(uint32_t size);

// panic

void panic(const char *msg);

// textdrv

void init_textdrv(void);
void tty_refresh(void);

void text_putchar(char c);
uint32_t text_get_cursor_pos_x(void);
uint32_t text_get_cursor_pos_y(void);
void text_set_cursor_pos(uint32_t x, uint32_t y);
void text_set_cursor_palette(uint8_t c);
uint8_t text_get_cursor_palette(void);
void text_set_text_palette(uint8_t c);
uint8_t text_get_text_palette(void);
void text_clear(void);
void text_disable_cursor(void);
void text_enable_cursor(void);
void text_putstring(const char* string);
void text_putascii(const char* string, uint32_t length);

// pic

void map_PIC(uint8_t PIC0Offset, uint8_t PIC1Offset);

// pit

void set_pit_freq(uint32_t frequency);

// gdt

void load_GDT(void);
void load_TSS(void);
void set_userspace(uint32_t base, uint32_t page_count);

// idt

void load_IDT(void);

// keyboard

void keyboard_init(void);
void keyboard_handler(uint8_t input_byte);
char keyboard_fetch_char(void);

// end

#endif
