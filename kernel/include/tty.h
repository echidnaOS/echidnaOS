#ifndef __TTY_H__
#define __TTY_H__

#include <kernel.h>
#include <stddef.h>

#ifdef _BIG_FONTS_
  #define VD_ROWS 25
#else
  #define VD_ROWS 50
#endif
#define VD_COLS 160

typedef struct {
    int cursor_x;
    int cursor_y;
    int cursor_status;
    uint32_t cursor_bg_col;
    uint32_t cursor_fg_col;
    uint32_t text_bg_col;
    uint32_t text_fg_col;
    uint32_t *field;
    char *grid;
    char kb_l1_buffer[KB_L1_SIZE];
    char kb_l2_buffer[KB_L2_SIZE];
    uint16_t kb_l1_buffer_index;
    uint16_t kb_l2_buffer_index;
    int escape;
    int *esc_value;
    int esc_value0;
    int esc_value1;
    int *esc_default;
    int esc_default0;
    int esc_default1;
    int raw;
    int noblock;
    int noscroll;
} tty_t;

extern uint8_t current_tty;
extern tty_t tty[KRNL_TTY_COUNT];

void switch_tty(uint8_t which_tty);
void text_putchar(char c, uint8_t which_tty);

void vga_disable_cursor(void);
void vga_80_x_50(void);

int keyboard_fetch_char(uint8_t which_tty);


#endif
