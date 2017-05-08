#include <kernel.h>
#include <stdint.h>

static uint8_t capslock_active = 0;
static uint8_t shift_active = 0;
static char keyboard_buffer[256];
static uint16_t buffer_index=0;

const char sc_ascii_capslock[] = { '\0', '?', '1', '2', '3', '4', '5', '6',     
		'7', '8', '9', '0', '-', '=', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 
				'U', 'I', 'O', 'P', '[', ']', '\n', '\0', 'A', 'S', 'D', 'F', 'G', 
				'H', 'J', 'K', 'L', ';', '\'', '`', '\0', '\\', 'Z', 'X', 'C', 'V', 
				'B', 'N', 'M', ',', '.', '/', '\0', '\0', '\0', ' '};

const char sc_ascii_shift[] = { '\0', '?', '!', '@', '#', '$', '%', '^',     
		'&', '*', '(', ')', '_', '+', '\b', '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 
				'U', 'I', 'O', 'P', '{', '}', '\n', '\0', 'A', 'S', 'D', 'F', 'G', 
				'H', 'J', 'K', 'L', ':', '"', '~', '\0', '|', 'Z', 'X', 'C', 'V', 
				'B', 'N', 'M', '<', '>', '?', '\0', '\0', '\0', ' '};

const char sc_ascii_shift_capslock[] = { '\0', '?', '!', '@', '#', '$', '%', '^',     
		'&', '*', '(', ')', '_', '+', '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 
				'u', 'i', 'o', 'p', '{', '}', '\n', '\0', 'a', 's', 'd', 'f', 'g', 
				'h', 'j', 'k', 'l', ':', '"', '~', '\0', '|', 'z', 'x', 'c', 'v', 
				'b', 'n', 'm', '<', '>', '?', '\0', '\0', '\0', ' '};

const char sc_ascii_nomod[] = { '\0', '?', '1', '2', '3', '4', '5', '6',     
		'7', '8', '9', '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 
				'u', 'i', 'o', 'p', '[', ']', '\n', '\0', 'a', 's', 'd', 'f', 'g', 
				'h', 'j', 'k', 'l', ';', '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v', 
				'b', 'n', 'm', ',', '.', '/', '\0', '\0', '\0', ' '};

void keyboard_handler(uint8_t input_byte) {
	char c='\0';
	if ( input_byte == SC_CAPSLOCK )
		capslock_active = !capslock_active;

	else if ( input_byte == SC_LEFT_SHIFT || input_byte == SC_RIGHT_SHIFT || input_byte == SC_LEFT_SHIFT_REL || input_byte == SC_RIGHT_SHIFT_REL )
		shift_active = !shift_active;

	else if (buffer_index < 256)
	{
		if (input_byte < SC_MAX)
		{
			if ( !capslock_active && ! shift_active )
				c = sc_ascii_nomod[input_byte];

			else if ( !capslock_active && shift_active )
				c = sc_ascii_shift[input_byte];

			else if ( capslock_active && shift_active )
				c = sc_ascii_shift_capslock[input_byte];

			else
				c = sc_ascii_capslock[input_byte];

			keyboard_buffer[buffer_index] = c;
			buffer_index++;
		}
	}

	else
	{
		panic("keyboard buffer overflow");
	}
}

char keyboard_fetch_char(void) {
	uint16_t x;
	char c;
	if (buffer_index) {
		buffer_index--;
		c = keyboard_buffer[0];
		for (x=0; x<255; x++) {
			keyboard_buffer[x] = keyboard_buffer[x+1];
		}
		return c;
	} else {
		return '\0';
	}
}

char keyboard_getchar(void) {
	char c='\0';
	while (c=='\0') {
		c = keyboard_fetch_char();
	}
	return c;
}

void keyboard_getstring(char* string, uint32_t limit) {
	uint32_t x=0;
	char c='\0';
	while (1) {
		c = keyboard_getchar();
		if (c=='\b') {
			if (x) {
				x--;
				text_putchar(c);
			}
		} else if (c=='\n') {
			break;
		} else if (x<(limit-1)) {
			string[x] = c;
			x++;
			text_putchar(c);
		}
	}
	string[x] = 0x00;
}
