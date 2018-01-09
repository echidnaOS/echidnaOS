#include <kernel.h>
#include <stdint.h>
#include <cio.h>

#define MAX_CODE 0x57
#define CAPSLOCK 0x3A
#define RIGHT_SHIFT 0x36
#define LEFT_SHIFT 0x2A
#define RIGHT_SHIFT_REL 0xB6
#define LEFT_SHIFT_REL 0xAA
#define LEFT_CTRL 0x1D
#define LEFT_CTRL_REL 0x9D

static int capslock_active = 0;
static int shift_active = 0;
static int ctrl_active = 0;
static uint8_t led_status = 0;

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
    // reset keyboard LEDs
    /*
    port_out_b(0x60, 0xED);
    while (port_in_b(0x64) & 0x02);
    port_out_b(0x60, led_status);
    */
    return;
}

void keyboard_handler(uint8_t input_byte) {
    char c = '\0';

    // tty switch handling
    if (shift_active) {
        switch (input_byte) {
            case 0x58:
                switch_tty(0);
                return;
            case 0x3b:
                switch_tty(1);
                return;
            case 0x3c:
                switch_tty(2);
                return;
            case 0x3d:
                switch_tty(3);
                return;
            case 0x3e:
                switch_tty(4);
                return;
            case 0x3f:
                switch_tty(5);
                return;
            case 0x40:
                switch_tty(6);
                return;
            default:
                break;
        }
    }
    
    // ctrl sequences handling
    if (ctrl_active) {
        switch (input_byte) {
            case 0x2e:      // ctrl+c
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
        /*
        port_out_b(0x60, 0xED);
        while (port_in_b(0x64) & 0x02);
        port_out_b(0x60, led_status);
        */

    } else if (input_byte == LEFT_SHIFT || input_byte == RIGHT_SHIFT || input_byte == LEFT_SHIFT_REL || input_byte == RIGHT_SHIFT_REL)
		shift_active = !shift_active;

    else if (input_byte == LEFT_CTRL || input_byte == LEFT_CTRL_REL)
        ctrl_active = !ctrl_active;

    else if (tty[current_tty].kb_l1_buffer_index < KB_L1_SIZE) {

        if (input_byte < MAX_CODE) {
            
            if (!capslock_active && !shift_active)
                c = ascii_nomod[input_byte];

            else if (!capslock_active && shift_active)
                c = ascii_shift[input_byte];

            else if (capslock_active && shift_active)
                c = ascii_shift_capslock[input_byte];

            else
                c = ascii_capslock[input_byte];
            
            if (tty[current_tty].raw) {
                tty[current_tty].kb_l2_buffer[tty[current_tty].kb_l2_buffer_index++] = c;
                return;
            }
            
            if (c == '\b') {
                if (!tty[current_tty].kb_l1_buffer_index) return;
                text_putchar(c, current_tty);
                tty[current_tty].kb_l1_buffer[tty[current_tty].kb_l1_buffer_index--] = 0;
            }
            
            else if (c == '\n') {
                text_putchar(c, current_tty);
                tty[current_tty].kb_l1_buffer[tty[current_tty].kb_l1_buffer_index++] = c;
                kmemcpy(&tty[current_tty].kb_l2_buffer[tty[current_tty].kb_l2_buffer_index],
                        tty[current_tty].kb_l1_buffer,
                        tty[current_tty].kb_l1_buffer_index + 1);
                tty[current_tty].kb_l2_buffer_index += tty[current_tty].kb_l1_buffer_index;
                tty[current_tty].kb_l1_buffer[0] = 0;
                tty[current_tty].kb_l1_buffer_index = 0;
            }
            
            else {
                text_putchar(c, current_tty);
                tty[current_tty].kb_l1_buffer[tty[current_tty].kb_l1_buffer_index++] = c;
            }

        }

    }

    return;
}

int is_eof = 0;

int keyboard_fetch_char(uint8_t which_tty) {
    uint16_t i;
    char c;

    if (is_eof) {
        is_eof = 0;
        return -1;
    }

    if (tty[which_tty].kb_l2_buffer_index) {
        tty[which_tty].kb_l2_buffer_index--;
        c = tty[which_tty].kb_l2_buffer[0];
        for (i = 0; i < (KB_L2_SIZE - 1); i++)
            tty[which_tty].kb_l2_buffer[i] = tty[which_tty].kb_l2_buffer[i + 1];
        if (c == '\n') is_eof = 1;
        return (int)c;
    } else
        if (tty[which_tty].noblock)
            return 0;
        return IO_NOT_READY;
}
