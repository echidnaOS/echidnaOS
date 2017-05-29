#include <kernel.h>
#include <stdint.h>

#define SC_MAX 0x57
#define SC_CAPSLOCK 0x3A
#define SC_ENTER 0x1C
#define SC_BACKSPACE 0x0E
#define SC_RIGHT_SHIFT 0x36
#define SC_LEFT_SHIFT 0x2A
#define SC_RIGHT_SHIFT_REL 0xB6
#define SC_LEFT_SHIFT_REL 0xAA

static uint8_t capslock_active = 0;
static uint8_t shift_active = 0;
static char keyboard_buffer[256];
static uint16_t buffer_index=0;
static uint8_t led_status=0;

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

void keyboard_init(void) {
    uint32_t index;
    // initialise the buffer to avoid some issues if
    // the buffer was not wiped on reset
    for (index=0; index<256; index++)
        keyboard_buffer[index] = 0;
    port_out_b(0x60, 0xED);
    while (port_in_b(0x64) & 0x02);
    port_out_b(0x60, led_status);
    return;
}

void keyboard_handler(uint8_t input_byte) {
	char c='\0';
	if ( input_byte == SC_CAPSLOCK ) {
		if (!capslock_active)
	    	led_status = led_status | 0b00000100;
	    else if (capslock_active)
	    	led_status = led_status & 0b11111011;
	    capslock_active = !capslock_active;
        port_out_b(0x60, 0xED);
        while (port_in_b(0x64) & 0x02);
        port_out_b(0x60, led_status);
	}

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
	
	return;
}

char keyboard_fetch_char(void) {
	uint16_t x;
	char c;
	if (buffer_index) {
		buffer_index--;
		c = keyboard_buffer[0];
		for (x=0; x<255; x++)
			keyboard_buffer[x] = keyboard_buffer[x+1];
		return c;
	} else
		return '\0';
}
