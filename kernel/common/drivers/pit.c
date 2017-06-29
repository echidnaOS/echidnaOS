#include <kernel.h>
#include <stdint.h>
#include <cio.h>

void set_pit_freq(uint32_t frequency) {
    uint16_t x = 1193182 / frequency;
    if ((1193182 % frequency) > (frequency / 2))
        x++;
        
    port_out_b(0x40, (uint8_t)(x & 0x00ff));
    port_out_b(0x40, (uint8_t)(x / 0x0100));

    return;
}
