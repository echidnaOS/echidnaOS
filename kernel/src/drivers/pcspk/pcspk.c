#include <stdint.h>
#include <kernel.h>
#include <cio.h>

uint16_t div = 0;

int pcspk_io_wrapper(uint32_t unused, uint64_t freq, int type, uint8_t payload) {

    if (type == 1) {

        if (!freq) {
            port_out_b(0x61, (port_in_b(0x61) & 0b11111100));
            div = 0;
            return 0;
        }

        port_out_b(0x43, 0xb6);
    
        div = 1193180 / freq;
    
        port_out_b(0x42, (div & 0xff));
        port_out_b(0x42, (div & 0xff00) >> 8);
        
        port_out_b(0x61, (port_in_b(0x61) | 0b00000011));
    
    } else if (type == 0) {
    
        if (div)
            return 1;
        else
            return 0;
        
    }

    return 0;

}

void init_pcspk(void) {
    kernel_add_device("pcspk", 0, 0, &pcspk_io_wrapper);
    return;
}
