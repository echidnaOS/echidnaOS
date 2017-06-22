#include <stdint.h>
#include <kernel.h>

void kputs(const char* string) {
    uint32_t i;
    for (i = 0; string[i]; i++)
        char_to_stdout(string[i]);
    return;
}

void kuitoa(uint32_t x) {
    uint8_t i;
    char buf[11] = {0};

    for (i = 9; x; i--) {
        buf[i] = (x % 10) + 0x30;
        x = x / 10;
    }

    i++;
    kputs(buf + i);

    return;
}
