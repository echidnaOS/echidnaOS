#include <kernel.h>

/* internal defines */
#define VIDEO_BOTTOM (ROWS*COLS)-1

/* internal functions */

static void clear_cursor(void);
static void draw_cursor(void);
static void scroll(void);

/* internal global variables */

static char* video_mem = (char*)0xB8000;
static tty_t tty[6];

/* internal functions */

void init_textdrv(void) {
    uint8_t i;
    for (i=0; i<6; i++) {
        tty[i].cursor_offset = 0;
        tty[i].cursor_status = 1;
        tty[i].cursor_palette = 0x70;
        tty[i].text_palette = 0x07;
    }
    return;
}

void tty_refresh(void) {
    kmemcpy(video_mem, tty[current_tty].field, ROWS*COLS);
    return;
}

static void clear_cursor(void) {
    tty[current_task->tty].field[(tty[current_task->tty].cursor_offset)+1] = tty[current_task->tty].text_palette;
    return;
}

static void draw_cursor(void) {
    if (tty[current_task->tty].cursor_status)
        tty[current_task->tty].field[(tty[current_task->tty].cursor_offset)+1] = tty[current_task->tty].cursor_palette;
    return;
}

static void scroll(void) {
    uint32_t i;
    // move the text up by one row
    for (i=0; i<=VIDEO_BOTTOM-COLS; i++)
        tty[current_task->tty].field[i] = tty[current_task->tty].field[i+COLS];
    // clear the last line of the screen
    for (i=VIDEO_BOTTOM; i>VIDEO_BOTTOM-COLS; i -= 2) {
        tty[current_task->tty].field[i] = tty[current_task->tty].text_palette;
        tty[current_task->tty].field[i-1] = ' ';
    }
    return;
}

/* external functions */

void text_clear(void) {
    uint32_t i;
    clear_cursor();
    for (i=0; i<VIDEO_BOTTOM; i += 2) {
        tty[current_task->tty].field[i] = ' ';
        tty[current_task->tty].field[i+1] = tty[current_task->tty].text_palette;
    }
    tty[current_task->tty].cursor_offset = 0;
    draw_cursor();
    tty_refresh();
    return;
}

void text_enable_cursor(void) {
    tty[current_task->tty].cursor_status=1;
    draw_cursor();
    tty_refresh();
    return;
}

void text_disable_cursor(void) {
    tty[current_task->tty].cursor_status=0;
    clear_cursor();
    tty_refresh();
    return;
}

void text_putchar(char c) {
    switch (c) {
        case 0x00:
            break;
        case 0x0A:
            if (text_get_cursor_pos_y() == (ROWS - 1)) {
                clear_cursor();
                scroll();
                text_set_cursor_pos(0, (ROWS - 1));
            } else
                text_set_cursor_pos(0, (text_get_cursor_pos_y() + 1));
            break;
        case 0x08:
            if (tty[current_task->tty].cursor_offset) {
                clear_cursor();
                tty[current_task->tty].cursor_offset -= 2;
                tty[current_task->tty].field[tty[current_task->tty].cursor_offset] = ' ';
                draw_cursor();
                tty_refresh();
            }
            break;
        default:
            clear_cursor();
            tty[current_task->tty].field[tty[current_task->tty].cursor_offset] = c;
            if (tty[current_task->tty].cursor_offset >= (VIDEO_BOTTOM - 1)) {
                scroll();
                tty[current_task->tty].cursor_offset = VIDEO_BOTTOM - (COLS - 1);
            } else
                tty[current_task->tty].cursor_offset += 2;
            draw_cursor();
            tty_refresh();
    }
    return;
}

void text_putstring(const char* string) {
    uint32_t x;
    for (x=0; string[x]!=0; x++)
        text_putchar(string[x]);
    return;
}

void text_putascii(const char* string, uint32_t length) {
    uint32_t x;
    for (x=0; x<length; x++)
        text_putchar(string[x]);
    return;
}

void text_set_cursor_palette(uint8_t c) {
    tty[current_task->tty].cursor_palette = c;
    draw_cursor();
    tty_refresh();
    return;
}

uint8_t text_get_cursor_palette(void) {
    return tty[current_task->tty].cursor_palette;
}

void text_set_text_palette(uint8_t c) {
    tty[current_task->tty].text_palette = c;
    return;
}

uint8_t text_get_text_palette(void) {
    return tty[current_task->tty].text_palette;
}

uint32_t text_get_cursor_pos_x(void) {
    return (tty[current_task->tty].cursor_offset/2) % ROWS;
}

uint32_t text_get_cursor_pos_y(void) {
    return (tty[current_task->tty].cursor_offset/2) / (COLS/2);
}

void text_set_cursor_pos(uint32_t x, uint32_t y) {
    clear_cursor();
    tty[current_task->tty].cursor_offset = (y*COLS)+(x*2);
    draw_cursor();
    tty_refresh();
    return;
}
