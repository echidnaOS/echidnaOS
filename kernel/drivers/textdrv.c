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

int cursor_offset = 0;
int cursor_status = 1;
char cursor_palette = 0x70;
char text_palette = 0x07;

/* internal functions */

void clear_cursor(void) {
	mem_store_b(VIDEO_ADDRESS+cursor_offset+1, text_palette);
}

void draw_cursor(void) {
	if (cursor_status) {
		mem_store_b(VIDEO_ADDRESS+cursor_offset+1, cursor_palette);
	}
}

void scroll(void) {
	int offset;
	// move the text up by one row
	for (offset=0; offset<=VIDEO_BOTTOM-COLS; offset++) {
		mem_store_b(VIDEO_ADDRESS+offset, mem_load_b(VIDEO_ADDRESS+offset+COLS));
	}
	// clear the last line of the screen
	for (offset=VIDEO_BOTTOM; offset>VIDEO_BOTTOM-COLS; offset=offset-2) {
		mem_store_b(VIDEO_ADDRESS+offset, text_palette);
		mem_store_b(VIDEO_ADDRESS+offset-1, ' ');
	}
}

/* external functions */

void text_clear(void) {
	int offset;
	clear_cursor();
	for (offset=0; offset<VIDEO_BOTTOM; offset=offset+2) {
		mem_store_b(VIDEO_ADDRESS+offset, ' ');
		mem_store_b(VIDEO_ADDRESS+offset+1, text_palette);
	}
	cursor_offset=0;
	draw_cursor();
}

void text_enable_cursor(void) {
	cursor_status=1;
	draw_cursor();
}

void text_disable_cursor(void) {
	cursor_status=0;
	clear_cursor();
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
}

void text_putstring(const char *string) {
	int x;
	for (x=0; string[x]!=0; x++) {
		text_putchar(string[x]);
	}
}

void text_set_cursor_palette(char c) {
	cursor_palette = c;
	draw_cursor();
}

char text_get_cursor_palette(void) {
	return cursor_palette;
}

void text_set_text_palette(char c) {
	text_palette = c;
}

char text_get_text_palette(void) {
	return text_palette;
}

int text_get_cursor_pos_x(void) {
	return (cursor_offset/2) % ROWS;
}

int text_get_cursor_pos_y(void) {
	return (cursor_offset/2) / (COLS/2);
}

void text_set_cursor_pos(int x, int y) {
	clear_cursor();
	cursor_offset = (y*COLS)+(x*2);
	draw_cursor();
}
