#include <kernel.h>

void panic(const char *msg) {
	disable_ints();
	text_set_text_palette(0x4E);
	text_clear();
	text_disable_cursor();
	text_putstring("\n!!! KERNEL PANIC !!!\n\nError info: ");
	text_putstring(msg);
	text_putstring("\n\nSYSTEM HALTED");
	system_halt();
}
