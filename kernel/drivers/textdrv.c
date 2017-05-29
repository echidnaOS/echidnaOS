#include <kernel.h>

/* internal defines */

#define VIDEO_ADDRESS 0xB8000
#define ROWS 50
#define COLS 160
#define VIDEO_BOTTOM (ROWS*COLS)-1

/* internal functions */

void clear_cursor(void);
void draw_cursor(void);
void scroll(void);

/* internal global variables */

uint32_t cursor_offset = 0;
int cursor_status = 1;
uint8_t cursor_palette = 0x70;
uint8_t text_palette = 0x07;

/* internal functions */

void clear_cursor(void) {
    mem_store_b(VIDEO_ADDRESS+cursor_offset+1, text_palette);
    return;
}

void draw_cursor(void) {
    if (cursor_status)
        mem_store_b(VIDEO_ADDRESS+cursor_offset+1, cursor_palette);
    return;
}

void scroll(void) {
    // move the text up by one row
    asm volatile (  "mov edi, 0xB8000;"
                    "mov esi, 0xB80A0;"
                    "mov ecx, 0x7A8;"
                    "rep movsd;"
                     :
                     :
                     : "edi", "esi", "ecx" );
    // clear the last line of the screen
    asm volatile (  "mov edi, 0xB9EA0;"
                    "mov ah, bl;"
                    "mov al, 0x20;"
                    "mov ecx, 80;"
                    "rep stosw;"
                     :
                     : "b" (text_palette)
                     : "edi", "eax", "ecx" );
    return;
}

/* external functions */

void text_clear(void) {
    clear_cursor();
    asm volatile (  "mov edi, 0xB8000;"
                    "mov ah, bl;"
                    "mov al, 0x20;"
                    "mov ecx, 0xFA0;"
                    "rep stosw;"
                     :
                     : "b" (text_palette)
                     : "edi", "eax", "ecx" );
    cursor_offset=0;
    draw_cursor();
    return;
}

void text_enable_cursor(void) {
    cursor_status=1;
    draw_cursor();
    return;
}

void text_disable_cursor(void) {
    cursor_status=0;
    clear_cursor();
    return;
}

void text_putchar(char c) {
	if (c == 0x00) {
	} else if (c == 0x0A) {
		if (text_get_cursor_pos_y() == ROWS - 1) {
			clear_cursor();
			scroll();
			text_set_cursor_pos(0,  ROWS - 1);
		} else text_set_cursor_pos(0, (text_get_cursor_pos_y()+1));
	} else if (c == 0x08) {
		if (cursor_offset) {
			clear_cursor();
			cursor_offset = cursor_offset-2;
			mem_store_b(VIDEO_ADDRESS+cursor_offset, ' ');
			draw_cursor();
		}
	} else {
		clear_cursor();
		mem_store_b(VIDEO_ADDRESS+cursor_offset, c);
		if (cursor_offset >= VIDEO_BOTTOM-1) {
			scroll();
			cursor_offset = VIDEO_BOTTOM - (COLS-1);
		} else {
			cursor_offset = cursor_offset+2;
		}
		draw_cursor();
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
    cursor_palette = c;
    draw_cursor();
    return;
}

uint8_t text_get_cursor_palette(void) {
    return cursor_palette;
}

void text_set_text_palette(uint8_t c) {
    text_palette = c;
    return;
}

uint8_t text_get_text_palette(void) {
    return text_palette;
}

uint32_t text_get_cursor_pos_x(void) {
    return (cursor_offset/2) % ROWS;
}

uint32_t text_get_cursor_pos_y(void) {
    return (cursor_offset/2) / (COLS/2);
}

void text_set_cursor_pos(uint32_t x, uint32_t y) {
    clear_cursor();
    cursor_offset = (y*COLS)+(x*2);
    draw_cursor();
    return;
}
