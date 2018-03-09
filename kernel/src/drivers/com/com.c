#include <stdint.h>
#include <kernel.h>
#include <cio.h>
#include <klib.h>
#include <vfs.h>
#include <dev.h>

#define SUCCESS 0
#define EOF -1
#define FAILURE -2

#define MAX_PORTS 4

typedef struct {
    int exists;
    uint16_t data_reg;
    uint16_t int_reg;
    uint16_t fifo_reg;
    uint16_t line_ctrl_reg;
    uint16_t mdm_ctrl_reg;
    uint16_t line_stat_reg;
    uint16_t mdm_stat_reg;
    uint16_t scratch_reg;
} com_device;

static char *com_names[] = { "com1", "com2", "com3", "com4" };

static uint16_t com_ports[] = { 0x3f8, 0x2f8, 0x3e8, 0x2e8 };

static com_device devices[MAX_PORTS];

com_device init_com_device(uint16_t port);

#ifdef _SERIAL_KERNEL_OUTPUT_

  int serial_kernel_output_ready = 0;

  void debug_kernel_console_init(void) {

      devices[0] = init_com_device(com_ports[0]);

      serial_kernel_output_ready = 1;

      return;

  }
#endif

int com_io_wrapper(uint32_t dev, uint64_t loc, int type, uint8_t payload) {
    if (type == 0) {
        if (!(port_in_b(devices[dev].line_stat_reg) & 0x01))
            return IO_NOT_READY;
        else
            return (int)port_in_b(devices[dev].data_reg);
    }
    else if (type == 1) {
        while (!(port_in_b(devices[dev].line_stat_reg) & 0x20));
        port_out_b(devices[dev].data_reg, payload);
        return SUCCESS;
    }
}

void init_com(void) {

    kprint(KPRN_INFO, "Initialising COM device driver...");
    
    int ii = 0;
    for (int i = 0; i < MAX_PORTS; i++) {
        if (ii >= MAX_PORTS) return;
        while (!(devices[i] = init_com_device(com_ports[ii])).exists) {
            ii++;
            if (ii >= MAX_PORTS) return;
        }
        ii++;
        kernel_add_device(com_names[i], i, 0, &com_io_wrapper);
        kprint(KPRN_INFO, "Initialised %s", com_names[i]);
    }

}

com_device init_com_device(uint16_t port) {
    com_device dev;
    
    dev.data_reg = port;
    dev.int_reg = port + 1;
    dev.fifo_reg = port + 2;
    dev.line_ctrl_reg = port + 3;
    dev.mdm_ctrl_reg = port + 4;
    dev.line_stat_reg = port + 5;
    dev.mdm_stat_reg = port + 6;
    dev.scratch_reg = port + 7;
    
    // test if the device exists
    dev.exists = 1;
    port_out_b(dev.scratch_reg, 0x55);
    if (port_in_b(dev.scratch_reg) != 0x55)
        dev.exists = 0;
    port_out_b(dev.scratch_reg, 0xaa);
    if (port_in_b(dev.scratch_reg) != 0xaa)
        dev.exists = 0;
    
    if (!dev.exists) return dev;
    
    // code taken from the OSDev wiki
    port_out_b(dev.int_reg, 0x00);
    port_out_b(dev.line_ctrl_reg, 0x80);
    port_out_b(dev.data_reg, 0x03);
    port_out_b(dev.int_reg, 0x00);
    port_out_b(dev.line_ctrl_reg, 0x03);
    port_out_b(dev.fifo_reg, 0xC7);
    port_out_b(dev.mdm_ctrl_reg, 0x0B);
    
    return dev;
}
