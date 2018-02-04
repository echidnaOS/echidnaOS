#include <cio.h>

void init_rtc(void) {
    port_out_b(0x70, 0x8b);
    uint8_t prev = port_in_b(0x71);
    port_out_b(0x70, 0x8b);
    port_out_b(0x71, prev | 0x40);

    return;
}
