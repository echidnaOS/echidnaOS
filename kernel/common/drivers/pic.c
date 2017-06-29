#include <kernel.h>
#include <stdint.h>
#include <cio.h>

void map_PIC(uint8_t PIC0Offset, uint8_t PIC1Offset) {
	uint8_t mask0;
	uint8_t mask1;

	mask0 = port_in_b(0x21);		// save PIC 0 and 1's masks
	mask1 = port_in_b(0xA1);

	port_out_b(0x20, 0x11);			// initialise
	io_wait();
	port_out_b(0xA0, 0x11);
	io_wait();

	port_out_b(0x21, PIC0Offset);	// set offsets
	io_wait();
	port_out_b(0xA1, PIC1Offset);
	io_wait();

	port_out_b(0x21, 0x04);			// PIC wiring info
	io_wait();
	port_out_b(0xA1, 0x02);
	io_wait();

	port_out_b(0x21, 0x01);			// environment info
	io_wait();
	port_out_b(0xA1, 0x01);
	io_wait();

	port_out_b(0x21, mask0);		// restore masks
	port_out_b(0xA1, mask1);
}
