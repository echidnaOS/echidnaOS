#include <kernel.h>
#include <stdint.h>

#define MAX_CODE 0x57
#define CAPSLOCK 0x3A
#define RIGHT_SHIFT 0x36
#define LEFT_SHIFT 0x2A
#define RIGHT_SHIFT_REL 0xB6
#define LEFT_SHIFT_REL 0xAA

static int capslock_active = 0;
static int shift_active = 0;
static uint16_t buffer_index = 0;
static uint8_t led_status = 0;

static char keyboard_buffer[256];

static const char ascii_capslock[] = {
    '\0', '?', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n', '\0', 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', '\0', '\\', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', ',', '.', '/', '\0', '\0', '\0', ' '
};

static const char ascii_shift[] = {
    '\0', '?', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', '\0', 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', '\0', '|', 'Z', 'X', 'C', 'V',
    'B', 'N', 'M', '<', '>', '?', '\0', '\0', '\0', ' '
};

static const char ascii_shift_capslock[] = {
    '\0', '?', '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '{', '}', '\n', '\0', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ':', '"', '~', '\0', '|', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', '<', '>', '?', '\0', '\0', '\0', ' '
};

static const char ascii_nomod[] = {
    '\0', '?', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', '\0', 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', '\0', '\\', 'z', 'x', 'c', 'v',
    'b', 'n', 'm', ',', '.', '/', '\0', '\0', '\0', ' '
};

void keyboard_init(void) {
    uint16_t i;
    // initialise the buffer to avoid some issues if
    // the buffer was not wiped on reset
    for (i = 0; i < 256; i++)
        keyboard_buffer[i] = 0;
    // reset keyboard LEDs
    port_out_b(0x60, 0xED);
    while (port_in_b(0x64) & 0x02);
    port_out_b(0x60, led_status);
    return;
}

void keyboard_handler(uint8_t input_byte) {
    char c = '\0';

    if (shift_active) {
        switch (input_byte) {
            case 0x3b:
                current_tty = 0;
                tty_refresh_force();
                return;
            case 0x3c:
                current_tty = 1;
                tty_refresh_force();
                return;
            case 0x3d:
                current_tty = 2;
                tty_refresh_force();
                return;
            case 0x3e:
                current_tty = 3;
                tty_refresh_force();
                return;
            case 0x3f:
                current_tty = 4;
                tty_refresh_force();
                return;
            case 0x40:
                current_tty = 5;
                tty_refresh_force();
                return;
            default:
                break;
        }
    }

    if (input_byte == CAPSLOCK) {

        if (!capslock_active)
            led_status = led_status | 0b00000100;
        else if (capslock_active)
            led_status = led_status & 0b11111011;
        capslock_active = !capslock_active;
        port_out_b(0x60, 0xED);
        while (port_in_b(0x64) & 0x02);
        port_out_b(0x60, led_status);

    } else if (input_byte == LEFT_SHIFT || input_byte == RIGHT_SHIFT || input_byte == LEFT_SHIFT_REL || input_byte == RIGHT_SHIFT_REL)
		shift_active = !shift_active;

    else if (buffer_index < 256) {

        if (input_byte < MAX_CODE) {

            if (!capslock_active && !shift_active)
                c = ascii_nomod[input_byte];

            else if (!capslock_active && shift_active)
                c = ascii_shift[input_byte];

            else if (capslock_active && shift_active)
                c = ascii_shift_capslock[input_byte];

            else
                c = ascii_capslock[input_byte];

            keyboard_buffer[buffer_index++] = c;

        }

    }

    return;
}

char keyboard_fetch_char(void) {
    uint16_t i;
    char c;
    if (current_tty != current_task->tty) return '\0';
    if (buffer_index) {
        buffer_index--;
        c = keyboard_buffer[0];
        for (i = 0; i < 255; i++)
            keyboard_buffer[i] = keyboard_buffer[i + 1];
        return c;
    } else
        return '\0';
}
