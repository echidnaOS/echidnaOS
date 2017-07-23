#include <stdint.h>
#include <kernel.h>

void kputs(const char* string) {
    tty_kputs(string, 0);
    
    return;
}

void tty_kputs(const char* string, uint8_t which_tty) {
    uint32_t i;
    for (i = 0; string[i]; i++)
        text_putchar(string[i], which_tty);
    return;
}

void kuitoa(uint64_t x) {
    tty_kuitoa(x, 0);

    return;
}

void tty_kuitoa(uint64_t x, uint8_t which_tty) {
    uint8_t i;
    char buf[21] = {0};

    if (!x) {
        tty_kputs("0", which_tty);
        return;
    }

    for (i = 19; x; i--) {
        buf[i] = (x % 10) + 0x30;
        x = x / 10;
    }

    i++;
    tty_kputs(buf + i, which_tty);

    return;
}

static const char hex_to_ascii_tab[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

void kxtoa(uint64_t x) {
    tty_kxtoa(x, 0);

    return;
}

void tty_kxtoa(uint64_t x, uint8_t which_tty) {
    uint8_t i;
    char buf[17] = {0};

    if (!x) {
        tty_kputs("0x0", which_tty);
        return;
    }

    for (i = 15; x; i--) {
        buf[i] = hex_to_ascii_tab[(x % 16)];
        x = x / 16;
    }

    i++;
    tty_kputs("0x", which_tty);
    tty_kputs(buf + i, which_tty);

    return;
}
