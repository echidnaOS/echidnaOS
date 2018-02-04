#include <cio.h>

uint32_t uptime_raw = 0;
uint32_t uptime_raw_high = 0;
uint32_t uptime_frac = 0;
uint32_t uptime_sec = 0;
uint32_t uptime_sec_high = 0;

void init_rtc(void) {
    port_out_b(0x70, 0x8b);
    uint8_t prev = port_in_b(0x71);
    port_out_b(0x70, 0x8b);
    port_out_b(0x71, prev | 0x40);

    return;
}
